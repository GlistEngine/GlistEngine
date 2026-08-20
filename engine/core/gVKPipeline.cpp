/*
 * gVKPipeline.cpp
 */

#include "gVKPipeline.h"

#ifdef GVK_VULKAN

#include "gVKReflect.h"
#include "gVKShaderCompiler.h"
#include "gVKShaders.h"
#include "gUtils.h"
// For gVertex, which the 3D vertex layout below is asserted against.
#include "gVbo.h"
#include <algorithm>
#include <cstddef>
#include <iterator>


// This pool serves every long-lived texture, scene-uniform slot and cached PBR
// material. Asset-heavy applications such as Martyr legitimately exceed 1024
// sets during loading even though no descriptor is leaked. Keep enough headroom
// for a complete mobile level; descriptor storage is still bounded and created
// once, rather than allocating another pool in the render loop.
static constexpr uint32_t GVK_DESCRIPTOR_POOL_SETS = 8192;

// The stages the 2D path is built from. Each can be recompiled from its GLSL
// source in a development build, and otherwise comes from the SPIR-V committed
// in gVKShaders.h.
enum {
	GVK_STAGE_COLOR_VERT, GVK_STAGE_COLOR_FRAG,
	GVK_STAGE_IMAGE_VERT, GVK_STAGE_IMAGE_FRAG,
	GVK_STAGE_MESH3D_VERT, GVK_STAGE_MESH3D_FRAG,
	GVK_STAGE_MESH3DPBR_VERT, GVK_STAGE_MESH3DPBR_FRAG,
	GVK_STAGE_SHADOW_VERT, GVK_STAGE_SHADOW_FRAG,
	GVK_STAGE_SKYBOX_VERT, GVK_STAGE_SKYBOX_FRAG,
	GVK_STAGE_COUNT
};

struct gvkStageSource {
	const char* file;
	VkShaderStageFlagBits stage;
	const uint32_t* embedded;
	size_t embeddedsize;
};

static const gvkStageSource gvkstagesources[GVK_STAGE_COUNT] = {
	{"color2d.vert", VK_SHADER_STAGE_VERTEX_BIT, gvkspv_color2d_vert, sizeof(gvkspv_color2d_vert)},
	{"color2d.frag", VK_SHADER_STAGE_FRAGMENT_BIT, gvkspv_color2d_frag, sizeof(gvkspv_color2d_frag)},
	{"image2d.vert", VK_SHADER_STAGE_VERTEX_BIT, gvkspv_image2d_vert, sizeof(gvkspv_image2d_vert)},
	{"image2d.frag", VK_SHADER_STAGE_FRAGMENT_BIT, gvkspv_image2d_frag, sizeof(gvkspv_image2d_frag)},
	{"mesh3d.vert", VK_SHADER_STAGE_VERTEX_BIT, gvkspv_mesh3d_vert, sizeof(gvkspv_mesh3d_vert)},
	{"mesh3d.frag", VK_SHADER_STAGE_FRAGMENT_BIT, gvkspv_mesh3d_frag, sizeof(gvkspv_mesh3d_frag)},
	{"mesh3dpbr.vert", VK_SHADER_STAGE_VERTEX_BIT, gvkspv_mesh3dpbr_vert, sizeof(gvkspv_mesh3dpbr_vert)},
	{"mesh3dpbr.frag", VK_SHADER_STAGE_FRAGMENT_BIT, gvkspv_mesh3dpbr_frag, sizeof(gvkspv_mesh3dpbr_frag)},
	{"shadow3d.vert", VK_SHADER_STAGE_VERTEX_BIT, gvkspv_shadow3d_vert, sizeof(gvkspv_shadow3d_vert)},
	{"shadow3d.frag", VK_SHADER_STAGE_FRAGMENT_BIT, gvkspv_shadow3d_frag, sizeof(gvkspv_shadow3d_frag)},
	{"skybox3d.vert", VK_SHADER_STAGE_VERTEX_BIT, gvkspv_skybox3d_vert, sizeof(gvkspv_skybox3d_vert)},
	{"skybox3d.frag", VK_SHADER_STAGE_FRAGMENT_BIT, gvkspv_skybox3d_frag, sizeof(gvkspv_skybox3d_frag)},
};

// The gVertex layout, written out rather than reflected from the shader.
//
// Reflection packs attributes in location order and derives the stride from what
// the shader declares, which is right for the 2D path: there the vertex struct is
// built to match the shader. A 3D mesh is the other way round - gVertex is a fixed
// C++ struct that gVbo uploads as-is, and the shader is merely one consumer of it.
// Deriving the offsets from the shader would mean that dropping an unused attribute
// from mesh3d.vert silently shifts every following one and renders garbage. So the
// buffer's own layout is stated here, and the static_asserts below make the compiler
// check it against the struct rather than trusting this comment.
//
// Only the entries the shader actually reads are handed to the pipeline; see
// gvkBuildPipeline for how the two sources are combined.
static const VkVertexInputAttributeDescription gvkmesh3dattributes[] = {
	{0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0},    // position
	{1, 0, VK_FORMAT_R32G32B32_SFLOAT, 12},   // normal
	{2, 0, VK_FORMAT_R32G32_SFLOAT, 24},      // texcoords
	{3, 0, VK_FORMAT_R32G32B32_SFLOAT, 32},   // tangent
	{4, 0, VK_FORMAT_R32G32B32_SFLOAT, 44},   // bitangent
	{5, 0, VK_FORMAT_R32G32B32_SFLOAT, 56},   // color
	// Binding 1, the per-instance model matrix. A mat4 cannot be one attribute:
	// it arrives as four consecutive vec4 locations, each a column, 16 bytes apart.
	{6, 1, VK_FORMAT_R32G32B32A32_SFLOAT, 0},
	{7, 1, VK_FORMAT_R32G32B32A32_SFLOAT, 16},
	{8, 1, VK_FORMAT_R32G32B32A32_SFLOAT, 32},
	{9, 1, VK_FORMAT_R32G32B32A32_SFLOAT, 48},
};
static constexpr uint32_t GVK_MESH3D_VERTEX_STRIDE = 68;
static constexpr uint32_t GVK_MESH3D_INSTANCE_STRIDE = 64;   // one glm::mat4

// One vertex layout for both 2D pipelines, stated here rather than reflected for
// the same reason as the 3D one: gVKDraw's batcher packs coloured shapes and
// images into a single buffer and indexes it with firstVertex, which only works
// while every 2D draw agrees on one stride. The coloured pipeline ignores the
// texture coordinate; carrying it costs eight bytes a vertex and saves a second
// layout. See gvk2DVertex in gVKDraw.cpp, which this must match.
static const VkVertexInputAttributeDescription gvk2dattributes[] = {
	{0, 0, VK_FORMAT_R32G32B32A32_SFLOAT, 0},    // clip space position
	{1, 0, VK_FORMAT_R32G32B32A32_SFLOAT, 16},   // colour / tint
	{2, 0, VK_FORMAT_R32G32_SFLOAT, 32},         // texture coordinate
};
static constexpr uint32_t GVK_2D_VERTEX_STRIDE = 48;

static_assert(sizeof(gVertex) == GVK_MESH3D_VERTEX_STRIDE, "gVertex size no longer matches the Vulkan 3D vertex layout");
static_assert(offsetof(gVertex, position) == 0, "gVertex::position moved");
static_assert(offsetof(gVertex, normal) == 12, "gVertex::normal moved");
static_assert(offsetof(gVertex, texcoords) == 24, "gVertex::texcoords moved");
static_assert(offsetof(gVertex, tangent) == 32, "gVertex::tangent moved");
static_assert(offsetof(gVertex, bitangent) == 44, "gVertex::bitangent moved");
static_assert(offsetof(gVertex, color) == 56, "gVertex::color moved");

// What differs between the pipelines this file builds. Everything not listed here
// is shared render state and stays in gvkBuildPipeline.
struct gvkPipelineOptions {
	// A second pipeline with a line topology, for stroking unfilled 2D shapes.
	bool linevariant = false;
	// 3D needs the depth buffer; 2D is ordered by draw call and must not write it.
	bool depthtest = false;
	// 2D composites layer over layer, so it blends. 3D geometry is opaque and must
	// not: the lighting sum produces an alpha below 1 wherever a surface faces away
	// from a light, and blending that against the background would darken the
	// surface by however lit it happens to be. OpenGL never sees this because it
	// draws meshes with blending switched off.
	bool blend = true;
	VkCullModeFlags cullmode = VK_CULL_MODE_NONE;
	// Makes the cull mode and the front face dynamic, so a 3D draw can follow the
	// renderer's culling state the way it already follows the depth state. Only the
	// 3D pipelines ask for it; the 2D ones never cull, and leaving the state static
	// there means their draw calls have nothing extra to set.
	bool dynamicculling = false;
	// Builds a second pipeline with the blend flag flipped, for paths that have to
	// follow the renderer's alpha blending state rather than fix it at build time.
	bool blendvariant = false;
	// Builds a copy that adds instead of compositing, for gRenderer::BLENDMODE_ADDITIVE.
	// Blending factors cannot be dynamic state, so the mode is a choice of pipeline.
	bool additivevariant = false;
	// Offsets every depth this pipeline writes, away from the light, by a constant
	// plus a term proportional to how steeply the polygon is sloped in screen space.
	// This is where shadow acne is meant to be dealt with: the alternative is a
	// constant subtracted in the shading pass, which cannot work at both ends of the
	// scene at once - it is expressed in normalised depth, so how much world distance
	// it forgives depends on how deep the light's frustum is, and a value tuned for a
	// ten unit frustum swallows whole vehicles in a hundred unit one. The hardware
	// bias is in units of the depth buffer's own resolution and is applied per
	// polygon with its slope taken into account, so it holds regardless.
	bool depthbias = false;
	// Builds a copy of the fragment stage with specialization constant 0 set to zero,
	// which is how mesh3d.frag is told that this pipeline's draws cannot discard. See
	// the constant's comment there for why that is worth a whole second pipeline: a
	// shader containing any discard loses early depth rejection on tile based mobile
	// GPUs, for every mesh drawn through it. The default copy keeps the constant at
	// its declared value of 1, so a shader with no such constant is unaffected.
	bool nocutoutvariant = false;
	// When set, the vertex input is taken from here instead of from reflection.
	const VkVertexInputAttributeDescription* vertexattributes = nullptr;
	uint32_t vertexattributecount = 0;
	uint32_t vertexstride = 0;
	// Non-zero adds a second binding stepped per instance rather than per vertex.
	// Attributes above name binding 1 themselves.
	uint32_t instancestride = 0;
};

struct gvkShaderSet {
	std::vector<uint32_t> spirv[GVK_STAGE_COUNT];
};

// Everything one pipeline needs, all of it derived from its shaders. The handles
// come as one gVKPipelineVariants per sample count in use: index 0 is the
// single-sample build every offscreen pass records into, index 1 the build for the
// screen pass's sample count and only present while MSAA is on. Everything else
// here - layout, set layouts, reflected push block - is shared by both, because the
// variants differ only in multisample state and the pass they were built against.
struct gvkPipelineParts {
	gVKPipelineVariants variants[GVK_PIPELINE_SAMPLE_VARIANTS];
	VkPipelineLayout layout = VK_NULL_HANDLE;
	std::vector<VkDescriptorSetLayout> setlayouts;
	uint32_t pushsize = 0;
	VkShaderStageFlags pushstages = 0;
	std::vector<gVKReflectedBinding> bindings;
};

// Where one build is aimed: the render pass per sample-count variant, plus the
// multisample state the second one uses. pass[0] is the single-sample template -
// the screen pass itself while MSAA is off, otherwise its 1x twin - and pass[1] is
// the multisampled screen pass, VK_NULL_HANDLE when there is none. The shadow build
// fills in pass[0] only, which is what keeps the shadow map at 1x: multisampling a
// depth-only map that is compared against rather than displayed antialiases nothing
// and costs a multiple of its bandwidth and footprint.
struct gvkTargetPasses {
	VkRenderPass pass[GVK_PIPELINE_SAMPLE_VARIANTS] = {};
	VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT;
	// Per-sample fragment shading. See gVKContext::setSampleShadingEnabled for why
	// this is off unless an application deliberately turns it on.
	bool sampleshading = false;
	float minsampleshading = 0.0f;
};

// requireSource is set on a hot reload, where the whole point is to see the
// edit: a shader that fails to compile then has to abort the reload rather than
// quietly fall back to the SPIR-V built into the binary and look like it worked.
static bool gvkLoadShaderSet(gvkShaderSet& set, bool requireSource) {
	const bool runtime = gvkRuntimeShadersAvailable();
	for(int i = 0; i < GVK_STAGE_COUNT; i++) {
		const gvkStageSource& source = gvkstagesources[i];
		if(runtime && gvkCompileShaderFile(source.file, source.stage, set.spirv[i])) continue;
		if(requireSource) return false;
		set.spirv[i].assign(source.embedded, source.embedded + source.embeddedsize / sizeof(uint32_t));
	}
	return true;
}

static VkShaderModule gvkCreateShaderModule(VkDevice device, const std::vector<uint32_t>& code) {
	VkShaderModuleCreateInfo info{};
	info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	info.codeSize = code.size() * sizeof(uint32_t);
	info.pCode = code.data();
	VkShaderModule mod = VK_NULL_HANDLE;
	if(vkCreateShaderModule(device, &info, nullptr, &mod) != VK_SUCCESS) {
		gLoge("gVKPipeline") << "vkCreateShaderModule failed.";
		return VK_NULL_HANDLE;
	}
	return mod;
}

// Builds one pipeline out of a vertex and a fragment module. Every layout
// decision - vertex attributes, push constant range, descriptor sets - comes
// from reflecting the SPIR-V, so the shader sources are the only place those are
// written down. The fixed 2D render state (triangle list, no depth, no cull,
// dynamic viewport/scissor, straight alpha blending) is shared by both pipelines
// and stays here.
static bool gvkBuildPipeline(VkDevice device, VkPipelineCache cache, const gvkTargetPasses& passes, const char* name,
		const std::vector<uint32_t>& vertSpirv, const std::vector<uint32_t>& fragSpirv,
		const gvkPipelineOptions& options, gvkPipelineParts& parts) {
	gVKReflectedLayout reflected;
	if(!gvkReflectSpirv(vertSpirv.data(), vertSpirv.size() * sizeof(uint32_t), reflected) ||
			!gvkReflectSpirv(fragSpirv.data(), fragSpirv.size() * sizeof(uint32_t), reflected)) {
		gLoge("gVKPipeline") << "Could not reflect the " << name << " shaders.";
		return false;
	}
	parts.pushsize = reflected.pushconstantsize;
	parts.pushstages = reflected.pushconstantstages;
	parts.bindings = reflected.bindings;
	gLogi("gVKPipeline") << name << ": push " << parts.pushsize << " bytes, "
			<< reflected.bindings.size() << " descriptor bindings, vertex stride "
			<< reflected.vertexstride;

	// One descriptor set layout per set the shaders declare, in set order, so the
	// indices match the set numbers used in the shader.
	const uint32_t setcount = reflected.getSetCount();
	for(uint32_t set = 0; set < setcount; set++) {
		std::vector<VkDescriptorSetLayoutBinding> bindings = reflected.getSetBindings(set);
		VkDescriptorSetLayoutCreateInfo setlayoutinfo{};
		setlayoutinfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		setlayoutinfo.bindingCount = static_cast<uint32_t>(bindings.size());
		setlayoutinfo.pBindings = bindings.data();
		VkDescriptorSetLayout setlayout = VK_NULL_HANDLE;
		if(vkCreateDescriptorSetLayout(device, &setlayoutinfo, nullptr, &setlayout) != VK_SUCCESS) {
			gLoge("gVKPipeline") << "vkCreateDescriptorSetLayout failed for " << name << " set " << set;
			return false;
		}
		parts.setlayouts.push_back(setlayout);
	}

	VkPushConstantRange push{};
	push.stageFlags = parts.pushstages;
	push.offset = 0;
	push.size = parts.pushsize;

	VkPipelineLayoutCreateInfo layoutinfo{};
	layoutinfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	layoutinfo.setLayoutCount = static_cast<uint32_t>(parts.setlayouts.size());
	layoutinfo.pSetLayouts = parts.setlayouts.empty() ? nullptr : parts.setlayouts.data();
	layoutinfo.pushConstantRangeCount = parts.pushsize > 0 ? 1 : 0;
	layoutinfo.pPushConstantRanges = parts.pushsize > 0 ? &push : nullptr;
	if(vkCreatePipelineLayout(device, &layoutinfo, nullptr, &parts.layout) != VK_SUCCESS) {
		gLoge("gVKPipeline") << "vkCreatePipelineLayout failed for " << name;
		return false;
	}

	VkShaderModule vert = gvkCreateShaderModule(device, vertSpirv);
	VkShaderModule frag = gvkCreateShaderModule(device, fragSpirv);
	if(vert == VK_NULL_HANDLE || frag == VK_NULL_HANDLE) {
		if(vert != VK_NULL_HANDLE) vkDestroyShaderModule(device, vert, nullptr);
		if(frag != VK_NULL_HANDLE) vkDestroyShaderModule(device, frag, nullptr);
		return false;
	}

	VkPipelineShaderStageCreateInfo stages[2]{};
	stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
	stages[0].module = vert;
	stages[0].pName = "main";
	stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	stages[1].module = frag;
	stages[1].pName = "main";

	// An explicit layout wins over the reflected one; see gvkmesh3dattributes above
	// for why the 3D path supplies its own.
	//
	// The two sources are combined rather than one simply replacing the other: the
	// offsets and the stride come from the explicit table, because those describe
	// the buffer and the shader has no say in them, while *which* attributes get
	// declared comes from reflection, because that is exactly what the shader does
	// decide. Declaring one the shader never reads is legal but makes the validation
	// layers warn, and the set of attributes a mesh shader reads grows as lighting
	// and textures arrive - this way that happens on its own, with no edit here.
	std::vector<VkVertexInputAttributeDescription> selected;
	const bool explicitlayout = options.vertexattributes != nullptr;
	if(explicitlayout) {
		for(const VkVertexInputAttributeDescription& used : reflected.vertexattributes) {
			for(uint32_t i = 0; i < options.vertexattributecount; i++) {
				if(options.vertexattributes[i].location == used.location) {
					selected.push_back(options.vertexattributes[i]);
					break;
				}
			}
		}
	}

	const uint32_t attributecount = explicitlayout
			? static_cast<uint32_t>(selected.size()) : static_cast<uint32_t>(reflected.vertexattributes.size());
	const VkVertexInputAttributeDescription* attributes = explicitlayout
			? selected.data() : reflected.vertexattributes.data();

	VkVertexInputBindingDescription bindings[2]{};
	bindings[0].binding = 0;
	bindings[0].stride = explicitlayout ? options.vertexstride : reflected.vertexstride;
	bindings[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	// INSTANCE rate is what makes the attribute advance once per instance instead of
	// once per vertex - the same thing glVertexAttribDivisor(attribute, 1) does.
	bindings[1].binding = 1;
	bindings[1].stride = options.instancestride;
	bindings[1].inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;

	uint32_t bindingcount = attributecount == 0 ? 0 : 1;
	if(bindingcount == 1 && options.instancestride > 0) bindingcount = 2;

	VkPipelineVertexInputStateCreateInfo vertexinput{};
	vertexinput.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexinput.vertexBindingDescriptionCount = bindingcount;
	vertexinput.pVertexBindingDescriptions = bindings;
	vertexinput.vertexAttributeDescriptionCount = attributecount;
	vertexinput.pVertexAttributeDescriptions = attributes;

	VkPipelineInputAssemblyStateCreateInfo inputassembly{};
	inputassembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputassembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

	VkPipelineViewportStateCreateInfo viewportstate{};
	viewportstate.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportstate.viewportCount = 1;
	viewportstate.scissorCount = 1;

	VkPipelineRasterizationStateCreateInfo rasterizer{};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.cullMode = options.cullmode;
	// Careful if this is ever set to something other than VK_CULL_MODE_NONE: the
	// frame loop draws through a negative-height viewport, and mirroring an axis
	// reverses the winding a triangle appears to have on screen. Geometry the
	// OpenGL path calls counter-clockwise therefore reaches the rasteriser here as
	// clockwise, so enabling culling means picking the opposite front face to the
	// OpenGL one - not the same value. Left at NONE for now, which sidesteps it.
	rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizer.lineWidth = 1.0f;
	// See gvkPipelineOptions::depthbias. The two factors are the values that the
	// depth-only passes of most renderers settle on; they are deliberately modest,
	// because overshooting here detaches a shadow from the object casting it.
	rasterizer.depthBiasEnable = options.depthbias ? VK_TRUE : VK_FALSE;
	rasterizer.depthBiasConstantFactor = options.depthbias ? 1.25f : 0.0f;
	rasterizer.depthBiasSlopeFactor = options.depthbias ? 2.0f : 0.0f;

	// Filled per sample-count variant in the loop at the bottom of this function; a
	// pipeline's rasterizationSamples has to match the sample count of the pass it is
	// built against or it cannot be recorded into it.
	VkPipelineMultisampleStateCreateInfo multisample{};
	multisample.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisample.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	VkPipelineColorBlendAttachmentState blendattachment{};
	blendattachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
			VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	blendattachment.blendEnable = options.blend ? VK_TRUE : VK_FALSE;
	blendattachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	blendattachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	blendattachment.colorBlendOp = VK_BLEND_OP_ADD;
	blendattachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	blendattachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	blendattachment.alphaBlendOp = VK_BLEND_OP_ADD;

	VkPipelineColorBlendStateCreateInfo colorblend{};
	colorblend.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorblend.attachmentCount = 1;
	colorblend.pAttachments = &blendattachment;

	// The 3D pipeline takes its depth state per draw rather than baking it in. The
	// engine lets an app switch depth testing on and off at any point through
	// gBaseCanvas::enableDepthTest, and OpenGL honours that immediately; a pipeline
	// with the state compiled in would ignore it and quietly disagree with the
	// OpenGL backend about which surfaces are visible. These three are core in
	// Vulkan 1.3, which is the version the engine already requires.
	// PRIMITIVE_TOPOLOGY is dynamic for the same reason: a mesh carries its own draw
	// mode, and gSphere for one is a triangle strip while gBox is a triangle list.
	// Vulkan allows this to change freely within a topology class, so the triangle
	// pipeline covers list and strip and the line pipeline covers list and strip too.
	// The two culling states come last so the count below can simply take or leave
	// them; every draw recording through a pipeline that asked for them must set
	// both, because a dynamic state left unset is undefined.
	VkDynamicState dynamicstates[] = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR,
		VK_DYNAMIC_STATE_DEPTH_TEST_ENABLE,
		VK_DYNAMIC_STATE_DEPTH_WRITE_ENABLE,
		VK_DYNAMIC_STATE_DEPTH_COMPARE_OP,
		VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY,
		VK_DYNAMIC_STATE_CULL_MODE,
		VK_DYNAMIC_STATE_FRONT_FACE};
	VkPipelineDynamicStateCreateInfo dynamicstate{};
	dynamicstate.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	// The 2D pipelines keep the static state below: their geometry is expanded into
	// plain lists before it ever reaches a draw call.
	dynamicstate.dynamicStateCount = options.dynamicculling ? 8 : (options.depthtest ? 6 : 2);
	dynamicstate.pDynamicStates = dynamicstates;

	// The render pass carries a depth attachment, and a pipeline used with such a
	// pass must describe its depth state rather than leave it null.
	//
	// 3D turns both test and write on: that is what makes a near surface hide a far
	// one. 2D leaves both off, because 2D drawing is ordered by the sequence of the
	// draw calls - writing depth there would let an earlier shape reject a later one
	// that is meant to sit on top of it.
	VkPipelineDepthStencilStateCreateInfo depthstencil{};
	depthstencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthstencil.depthTestEnable = options.depthtest ? VK_TRUE : VK_FALSE;
	depthstencil.depthWriteEnable = options.depthtest ? VK_TRUE : VK_FALSE;
	// LESS, matching the engine's DEPTHTESTTYPE_LESS default. The projection
	// correction in gVKRenderEngine::setProjectionMatrix already put near at 0 and
	// far at 1, so smaller really does mean closer here.
	depthstencil.depthCompareOp = options.depthtest ? VK_COMPARE_OP_LESS : VK_COMPARE_OP_ALWAYS;
	depthstencil.depthBoundsTestEnable = VK_FALSE;
	depthstencil.stencilTestEnable = VK_FALSE;

	VkGraphicsPipelineCreateInfo pipelineinfo{};
	pipelineinfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineinfo.stageCount = 2;
	pipelineinfo.pStages = stages;
	pipelineinfo.pVertexInputState = &vertexinput;
	pipelineinfo.pInputAssemblyState = &inputassembly;
	pipelineinfo.pViewportState = &viewportstate;
	pipelineinfo.pRasterizationState = &rasterizer;
	pipelineinfo.pMultisampleState = &multisample;
	pipelineinfo.pDepthStencilState = &depthstencil;
	pipelineinfo.pColorBlendState = &colorblend;
	pipelineinfo.pDynamicState = &dynamicstate;
	pipelineinfo.layout = parts.layout;
	pipelineinfo.subpass = 0;

	// One build per sample count actually in use. This is the answer to the
	// render-pass compatibility problem multisampling creates: a pipeline is baked
	// against a pass and the sample count is part of that compatibility, so the same
	// shaders are compiled into a single-sample pipeline for the offscreen passes and
	// a multisampled one for the screen pass, and the getters in gVKContext hand out
	// whichever the pass being recorded will accept. The second build shares this
	// function's modules, layout and every state struct above and comes out of the
	// same VkPipelineCache, so it costs a fraction of the first. See gVKContext.h.
	VkResult result = VK_SUCCESS;
	for(uint32_t variant = 0; variant < GVK_PIPELINE_SAMPLE_VARIANTS && result == VK_SUCCESS; variant++) {
		if(passes.pass[variant] == VK_NULL_HANDLE) continue;
		pipelineinfo.renderPass = passes.pass[variant];
		multisample.rasterizationSamples = variant == 0 ? VK_SAMPLE_COUNT_1_BIT : passes.samples;
		// Coverage antialiasing alone runs the fragment shader once per pixel and only
		// the depth/coverage test per sample, which is why it is nearly free. Sample
		// shading runs it per sample instead - the only thing that antialiases inside a
		// triangle, and a multiplier on the most expensive stage of a mobile frame.
		// Never on the single-sample variant, where it would mean nothing.
		const bool shadepersample = variant > 0 && passes.sampleshading
				&& multisample.rasterizationSamples != VK_SAMPLE_COUNT_1_BIT;
		multisample.sampleShadingEnable = shadepersample ? VK_TRUE : VK_FALSE;
		multisample.minSampleShading = shadepersample ? passes.minsampleshading : 0.0f;
		gVKPipelineVariants& built = parts.variants[variant];

		// Every copy below puts the state it changed back before the next one, so both
		// the following copy and the next variant start from the base state the
		// options describe.
		result = vkCreateGraphicsPipelines(device, cache, 1, &pipelineinfo, nullptr, &built.pipeline);
		if(result == VK_SUCCESS && options.blendvariant) {
			// A second copy with blending flipped, so a 3D draw can follow the renderer's
			// alpha blending state. It cannot be a dynamic state - blending is baked into
			// a Vulkan pipeline - so the choice is made by binding one or the other.
			blendattachment.blendEnable = options.blend ? VK_FALSE : VK_TRUE;
			result = vkCreateGraphicsPipelines(device, cache, 1, &pipelineinfo, nullptr,
					&built.blendvariantpipeline);
			blendattachment.blendEnable = options.blend ? VK_TRUE : VK_FALSE;
		}
		if(result == VK_SUCCESS && options.additivevariant) {
			// Only the destination factor changes: the source is still scaled by alpha, so
			// a fully transparent texel contributes nothing and an opaque one contributes
			// its full colour, exactly as glBlendFunc(GL_SRC_ALPHA, GL_ONE) does. The
			// alpha channel keeps the over operator - the colour is what is being added,
			// and letting alpha accumulate as well would saturate the target's coverage.
			blendattachment.blendEnable = VK_TRUE;
			blendattachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
			result = vkCreateGraphicsPipelines(device, cache, 1, &pipelineinfo, nullptr,
					&built.additivepipeline);
			blendattachment.blendEnable = options.blend ? VK_TRUE : VK_FALSE;
			blendattachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		}
		if(result == VK_SUCCESS && options.nocutoutvariant) {
			// The same modules, compiled a second time with the cutout constant forced to
			// zero. Specialization is applied to the fragment stage only; the vertex shader
			// declares no constants, and handing it an entry that names one it does not have
			// is invalid.
			const uint32_t nocutout = 0;
			VkSpecializationMapEntry entry{};
			entry.constantID = 0;
			entry.offset = 0;
			entry.size = sizeof(nocutout);
			VkSpecializationInfo specialization{};
			specialization.mapEntryCount = 1;
			specialization.pMapEntries = &entry;
			specialization.dataSize = sizeof(nocutout);
			specialization.pData = &nocutout;
			stages[1].pSpecializationInfo = &specialization;
			result = vkCreateGraphicsPipelines(device, cache, 1, &pipelineinfo, nullptr,
					&built.nocutoutpipeline);
			stages[1].pSpecializationInfo = nullptr;
		}
		if(result == VK_SUCCESS && options.linevariant) {
			// Identical state apart from the topology: an unfilled shape is stroked as
			// separate edges, which is what the OpenGL path draws through
			// DRAWMODE_LINELOOP. A list rather than a strip so primitive restart stays
			// off - a strip would have to leave it enabled on Metal, which quietly turns
			// an index of ~0 into a break should this pipeline ever draw indexed. The
			// line width stays 1.0, the only value guaranteed without wideLines.
			inputassembly.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
			result = vkCreateGraphicsPipelines(device, cache, 1, &pipelineinfo, nullptr, &built.linepipeline);
			inputassembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		}
	}
	vkDestroyShaderModule(device, vert, nullptr);
	vkDestroyShaderModule(device, frag, nullptr);
	if(result != VK_SUCCESS) {
		gLoge("gVKPipeline") << "vkCreateGraphicsPipelines failed for " << name;
		return false;
	}
	return true;
}

// Every handle of one sample-count variant. Shared by the build's own failure path
// and by gvkDestroyGraphicsPipelines, so the two cannot drift apart as variants are
// added.
static void gvkDestroyPipelineVariants(VkDevice device, gVKPipelineVariants& variants) {
	if(variants.additivepipeline != VK_NULL_HANDLE) vkDestroyPipeline(device, variants.additivepipeline, nullptr);
	if(variants.nocutoutpipeline != VK_NULL_HANDLE) vkDestroyPipeline(device, variants.nocutoutpipeline, nullptr);
	if(variants.blendvariantpipeline != VK_NULL_HANDLE) vkDestroyPipeline(device, variants.blendvariantpipeline, nullptr);
	if(variants.linepipeline != VK_NULL_HANDLE) vkDestroyPipeline(device, variants.linepipeline, nullptr);
	if(variants.pipeline != VK_NULL_HANDLE) vkDestroyPipeline(device, variants.pipeline, nullptr);
	variants = gVKPipelineVariants{};
}

static void gvkDestroyParts(VkDevice device, gvkPipelineParts& parts) {
	for(gVKPipelineVariants& variants : parts.variants) gvkDestroyPipelineVariants(device, variants);
	if(parts.layout != VK_NULL_HANDLE) vkDestroyPipelineLayout(device, parts.layout, nullptr);
	for(VkDescriptorSetLayout setlayout : parts.setlayouts) vkDestroyDescriptorSetLayout(device, setlayout, nullptr);
	parts = gvkPipelineParts{};
}

// Sized from the descriptor types the shaders actually declare, so a shader that
// starts using a different resource kind gets a pool that can serve it.
static bool gvkCreateDescriptorPool(VkDevice device, const gvkPipelineParts& color, const gvkPipelineParts& image,
		const gvkPipelineParts& mesh3d, const gvkPipelineParts& mesh3dpbr,
		const gvkPipelineParts& skybox, VkDescriptorPool& outPool) {
	std::vector<VkDescriptorPoolSize> sizes;
	// maxSets is shared by every layout allocated from this pool. Adding every
	// pipeline's bindings and then multiplying each sum by maxSets assumes all
	// 8192 sets simultaneously have the descriptors of every pipeline. That cannot
	// happen: one allocated set has exactly one layout. On mobile drivers the old
	// calculation reserved several times the descriptor backing store actually
	// addressable by the pool.
	//
	// Find the largest number of each descriptor type used by any *single set* and
	// reserve that worst case for maxSets. This remains safe for any mixture of
	// texture, scene, PBR and skybox sets while avoiding the multiplied waste.
	auto addLayoutWorstCase = [&sizes](const std::vector<gVKReflectedBinding>& bindings) {
		std::vector<VkDescriptorPoolSize> perSet;
		uint32_t currentSet = UINT32_MAX;
		auto mergeSet = [&sizes, &perSet]() {
			for(const VkDescriptorPoolSize& candidate : perSet) {
				auto found = std::find_if(sizes.begin(), sizes.end(), [&](const VkDescriptorPoolSize& size) {
					return size.type == candidate.type;
				});
				if(found == sizes.end()) sizes.push_back(candidate);
				else found->descriptorCount = std::max(found->descriptorCount, candidate.descriptorCount);
			}
			perSet.clear();
		};
		for(const gVKReflectedBinding& b : bindings) {
			if(currentSet != UINT32_MAX && b.set != currentSet) mergeSet();
			currentSet = b.set;
			auto found = std::find_if(perSet.begin(), perSet.end(), [&](const VkDescriptorPoolSize& size) {
				return size.type == b.type;
			});
			if(found == perSet.end()) perSet.push_back({b.type, b.count});
			else found->descriptorCount += b.count;
		}
		mergeSet();
	};
	addLayoutWorstCase(color.bindings);
	addLayoutWorstCase(image.bindings);
	// The 3D path's scene uniform block is a different descriptor type to the 2D
	// path's samplers, so leaving it out here would leave nothing to allocate its
	// sets from.
	addLayoutWorstCase(mesh3d.bindings);
	// The PBR path allocates one set per material rather than one per texture, so
	// its five samplers have to be counted here too.
	addLayoutWorstCase(mesh3dpbr.bindings);
	addLayoutWorstCase(skybox.bindings);
	if(sizes.empty()) return true;   // no shader declares a descriptor; nothing to pool
	for(VkDescriptorPoolSize& size : sizes) size.descriptorCount *= GVK_DESCRIPTOR_POOL_SETS;

	VkDescriptorPoolCreateInfo poolinfo{};
	poolinfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolinfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	poolinfo.maxSets = GVK_DESCRIPTOR_POOL_SETS;
	poolinfo.poolSizeCount = static_cast<uint32_t>(sizes.size());
	poolinfo.pPoolSizes = sizes.data();
	if(vkCreateDescriptorPool(device, &poolinfo, nullptr, &outPool) != VK_SUCCESS) {
		gLoge("gVKPipeline") << "vkCreateDescriptorPool failed.";
		return false;
	}
	return true;
}

// Builds both pipelines and their pool without touching the context, so the
// caller only adopts handles once everything succeeded. On failure nothing is
// left allocated.
static bool gvkBuildAll(VkDevice device, VkPipelineCache cache, const gvkTargetPasses& passes,
		const gvkShaderSet& shaders,
		gvkPipelineParts& color, gvkPipelineParts& image, gvkPipelineParts& mesh3d,
		gvkPipelineParts& mesh3dpbr, gvkPipelineParts& skybox, VkDescriptorPool& pool) {
	gvkPipelineOptions coloropts;
	coloropts.linevariant = true;
	coloropts.additivevariant = true;
	coloropts.vertexattributes = gvk2dattributes;
	coloropts.vertexattributecount = static_cast<uint32_t>(std::size(gvk2dattributes));
	coloropts.vertexstride = GVK_2D_VERTEX_STRIDE;

	gvkPipelineOptions imageopts;
	imageopts.additivevariant = true;
	imageopts.vertexattributes = gvk2dattributes;
	imageopts.vertexattributecount = static_cast<uint32_t>(std::size(gvk2dattributes));
	imageopts.vertexstride = GVK_2D_VERTEX_STRIDE;

	gvkPipelineOptions mesh3dopts;
	mesh3dopts.depthtest = true;
	mesh3dopts.blend = false;
	// A mesh follows the renderer's culling state, which an app can change between
	// draws exactly as it changes depth testing. Alpha blending is the same kind of
	// state, but it cannot be dynamic, so it gets a second pipeline instead.
	mesh3dopts.dynamicculling = true;
	mesh3dopts.blendvariant = true;
	mesh3dopts.vertexattributes = gvkmesh3dattributes;
	mesh3dopts.vertexattributecount = static_cast<uint32_t>(std::size(gvkmesh3dattributes));
	mesh3dopts.vertexstride = GVK_MESH3D_VERTEX_STRIDE;
	mesh3dopts.instancestride = GVK_MESH3D_INSTANCE_STRIDE;
	// A mesh can be drawn as an outline too, through DRAWMODE_LINES and friends.
	mesh3dopts.linevariant = true;
	// And a copy without the cutout discard, for materials that cannot need it.
	mesh3dopts.nocutoutvariant = true;

	// The PBR path shares all of that but not the last flag: its shader has no cutout
	// branch to compile out, so it already keeps early depth rejection and a second
	// copy would only be a duplicate.
	gvkPipelineOptions mesh3dpbropts = mesh3dopts;
	mesh3dpbropts.nocutoutvariant = false;

	// The sky needs the depth buffer, so scene geometry occludes it, but never culls
	// and never blends. Its vertex layout is small enough to come from reflection.
	// The depth compare op is dynamic here as on the other 3D paths, which is what
	// lets gSkybox ask for the same EQUAL test the OpenGL path uses.
	gvkPipelineOptions skyboxopts;
	skyboxopts.depthtest = true;
	skyboxopts.blend = false;

	if(gvkBuildPipeline(device, cache, passes, "colour",
					shaders.spirv[GVK_STAGE_COLOR_VERT], shaders.spirv[GVK_STAGE_COLOR_FRAG], coloropts, color) &&
			gvkBuildPipeline(device, cache, passes, "image",
					shaders.spirv[GVK_STAGE_IMAGE_VERT], shaders.spirv[GVK_STAGE_IMAGE_FRAG], imageopts, image) &&
			gvkBuildPipeline(device, cache, passes, "mesh3d",
					shaders.spirv[GVK_STAGE_MESH3D_VERT], shaders.spirv[GVK_STAGE_MESH3D_FRAG], mesh3dopts, mesh3d) &&
			gvkBuildPipeline(device, cache, passes, "mesh3dpbr",
					shaders.spirv[GVK_STAGE_MESH3DPBR_VERT], shaders.spirv[GVK_STAGE_MESH3DPBR_FRAG],
					mesh3dpbropts, mesh3dpbr) &&
			gvkBuildPipeline(device, cache, passes, "skybox3d",
					shaders.spirv[GVK_STAGE_SKYBOX_VERT], shaders.spirv[GVK_STAGE_SKYBOX_FRAG],
					skyboxopts, skybox) &&
			gvkCreateDescriptorPool(device, color, image, mesh3d, mesh3dpbr, skybox, pool)) {
		return true;
	}
	if(pool != VK_NULL_HANDLE) { vkDestroyDescriptorPool(device, pool, nullptr); pool = VK_NULL_HANDLE; }
	gvkDestroyParts(device, color);
	gvkDestroyParts(device, image);
	gvkDestroyParts(device, mesh3d);
	gvkDestroyParts(device, mesh3dpbr);
	gvkDestroyParts(device, skybox);
	return false;
}


long long gvkShaderSourcesTimestamp() {
	if(!gvkRuntimeShadersAvailable()) return 0;
	long long newest = 0;
	for(const gvkStageSource& source : gvkstagesources) {
		newest = std::max(newest, gvkShaderFileTimestamp(source.file));
	}
	return newest;
}

bool gvkCreateGraphicsPipelines(gVKContext& ctx) {
	if(ctx.device == VK_NULL_HANDLE || ctx.renderpass == VK_NULL_HANDLE) {
		gLoge("gVKPipeline") << "Pipelines need a device and render pass first.";
		return false;
	}
	if(ctx.pipelinecache == VK_NULL_HANDLE) {
		VkPipelineCacheCreateInfo cacheinfo{};
		cacheinfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
		if(vkCreatePipelineCache(ctx.device, &cacheinfo, nullptr, &ctx.pipelinecache) != VK_SUCCESS) {
			gLoge("gVKPipeline") << "vkCreatePipelineCache failed.";
			return false;
		}
	}
	gvkShaderSet shaders;
	gvkLoadShaderSet(shaders, false);

	gvkPipelineParts color;
	gvkPipelineParts image;
	gvkPipelineParts mesh3d;
	gvkPipelineParts mesh3dpbr;
	gvkPipelineParts skybox;
	VkDescriptorPool pool = VK_NULL_HANDLE;

	// Variant 0 is built against the single-sample template - the screen pass itself
	// while MSAA is off, otherwise the 1x twin gvkCreateRenderPass keeps for exactly
	// this - and is what the per-FBO passes record. Variant 1 exists only while the
	// screen pass is multisampled and is built against that pass.
	gvkTargetPasses passes;
	passes.pass[0] = ctx.singlesamplerenderpass != VK_NULL_HANDLE ? ctx.singlesamplerenderpass : ctx.renderpass;
	passes.pass[1] = ctx.isMultiSampled() ? ctx.renderpass : VK_NULL_HANDLE;
	passes.samples = ctx.samplecount;
	// Sample shading is honoured only when the device advertises the feature; asking
	// for it without sampleRateShading is invalid pipeline state, not a slow path.
	passes.sampleshading = ctx.sampleshadingenabled && ctx.devicefeatures.sampleRateShading == VK_TRUE;
	passes.minsampleshading = ctx.minsampleshading;
	if(ctx.sampleshadingenabled && ctx.devicefeatures.sampleRateShading != VK_TRUE) {
		gLogi("gVKPipeline") << "Sample shading was requested but the device does not support"
				<< " sampleRateShading; multisampling stays coverage only.";
	}

	if(!gvkBuildAll(ctx.device, ctx.pipelinecache, passes, shaders,
			color, image, mesh3d, mesh3dpbr, skybox, pool)) return false;

	for(uint32_t v = 0; v < GVK_PIPELINE_SAMPLE_VARIANTS; v++) {
		ctx.skybox[v] = skybox.variants[v];
		ctx.mesh3dpbr[v] = mesh3dpbr.variants[v];
		ctx.mesh3d[v] = mesh3d.variants[v];
		ctx.color2d[v] = color.variants[v];
		ctx.image2d[v] = image.variants[v];
	}
	// Nothing is being recorded yet, so start on the variant the screen pass wants.
	ctx.useScreenPipelines();

	ctx.skyboxpipelinelayout = skybox.layout;
	ctx.skyboxsetlayouts = skybox.setlayouts;
	ctx.skyboxpushsize = skybox.pushsize;
	ctx.skyboxpushstages = skybox.pushstages;

	ctx.mesh3dpbrpipelinelayout = mesh3dpbr.layout;
	ctx.mesh3dpbrsetlayouts = mesh3dpbr.setlayouts;
	ctx.mesh3dpbrpushsize = mesh3dpbr.pushsize;
	ctx.mesh3dpbrpushstages = mesh3dpbr.pushstages;

	ctx.mesh3dpipelinelayout = mesh3d.layout;
	ctx.mesh3dsetlayouts = mesh3d.setlayouts;
	ctx.mesh3dpushsize = mesh3d.pushsize;
	ctx.mesh3dpushstages = mesh3d.pushstages;
	ctx.color2dpipelinelayout = color.layout;
	ctx.color2dsetlayouts = color.setlayouts;
	ctx.color2dpushsize = color.pushsize;
	ctx.color2dpushstages = color.pushstages;
	ctx.image2dpipelinelayout = image.layout;
	ctx.image2dsetlayouts = image.setlayouts;
	ctx.image2dpushsize = image.pushsize;
	ctx.image2dpushstages = image.pushstages;
	ctx.descriptorpool = pool;

	if(gvkRuntimeShadersAvailable()) {
		gLogi("gVKPipeline") << "2D and 3D graphics pipelines created from "
				<< gvkShaderSourceDir() << "; edits to those shaders reload live.";
	} else {
		gLogi("gVKPipeline") << "2D and 3D graphics pipelines created.";
	}
	if(ctx.isMultiSampled()) {
		gLogi("gVKPipeline") << "Two sample-count builds: 1x for offscreen targets and "
				<< static_cast<int>(ctx.samplecount) << "x for the screen pass"
				<< (passes.sampleshading ? ", with sample shading" : "");
	}
	return true;
}

bool gvkReloadGraphicsPipelines(gVKContext& ctx) {
	if(ctx.device == VK_NULL_HANDLE || ctx.renderpass == VK_NULL_HANDLE) return false;

	// Compile everything before touching a single Vulkan object: a shader with a
	// typo in it then leaves the running pipelines exactly as they were. The build
	// below compiles the same sources a second time, which costs a few milliseconds
	// in a development build and keeps the creation path single.
	gvkShaderSet probe;
	if(!gvkLoadShaderSet(probe, true)) return false;

	vkDeviceWaitIdle(ctx.device);
	gvkDestroyGraphicsPipelines(ctx);
	if(!gvkCreateGraphicsPipelines(ctx)) {
		gLoge("gVKPipeline") << "Shader reload failed to rebuild the pipelines.";
		return false;
	}
	gLogi("gVKPipeline") << "Shaders reloaded.";
	return true;
}

bool gvkCreateShadowPipeline(gVKContext& ctx) {
	if(ctx.device == VK_NULL_HANDLE || ctx.shadowrenderpass == VK_NULL_HANDLE) {
		gLoge("gVKPipeline") << "The shadow pipeline needs the shadow render pass first.";
		return false;
	}
	gvkDestroyShadowPipeline(ctx);

	// Built separately from the others because it belongs to a different render pass,
	// and that pass only exists once the application has asked for shadows.
	gvkShaderSet shaders;
	gvkLoadShaderSet(shaders, false);

	gvkPipelineOptions options;
	// Depth is the entire output of this pass, so the test is on and static rather
	// than dynamic: a caster that ignored depth would leave the wrong distance in the
	// map. The scene's own enableDepthTest has no say here.
	options.depthtest = true;
	options.blend = false;
	// The shadow pipeline shares the mesh vertex layout and the same dynamic state
	// set, so its draws set the culling state too. Casters are not culled - a shadow
	// wants the whole silhouette, including back faces.
	options.dynamicculling = true;
	options.vertexattributes = gvkmesh3dattributes;
	options.vertexattributecount = static_cast<uint32_t>(std::size(gvkmesh3dattributes));
	options.vertexstride = GVK_MESH3D_VERTEX_STRIDE;
	options.instancestride = GVK_MESH3D_INSTANCE_STRIDE;
	// The only pipeline that wants it: this is the pass whose depths are compared
	// against later, so this is where the offset belongs.
	options.depthbias = true;

	// One target only, and at one sample: the shadow map is a depth-only image that
	// is sampled and compared against, never displayed, so multisampling it would
	// antialias nothing while multiplying the one buffer a shadowed frame writes and
	// reads most. That is also why this pipeline is built here rather than with the
	// others - it belongs to a different pass, at a different sample count.
	gvkTargetPasses passes;
	passes.pass[0] = ctx.shadowrenderpass;

	gvkPipelineParts parts;
	if(!gvkBuildPipeline(ctx.device, ctx.pipelinecache, passes, "shadow3d",
			shaders.spirv[GVK_STAGE_SHADOW_VERT], shaders.spirv[GVK_STAGE_SHADOW_FRAG],
			options, parts)) {
		return false;
	}

	ctx.shadowpipeline = parts.variants[0].pipeline;
	ctx.shadowpipelinelayout = parts.layout;
	ctx.shadowsetlayouts = parts.setlayouts;
	ctx.shadowpushsize = parts.pushsize;
	ctx.shadowpushstages = parts.pushstages;
	return true;
}

void gvkDestroyShadowPipeline(gVKContext& ctx) {
	if(ctx.device == VK_NULL_HANDLE) return;
	if(ctx.shadowpipeline != VK_NULL_HANDLE) {
		vkDestroyPipeline(ctx.device, ctx.shadowpipeline, nullptr);
		ctx.shadowpipeline = VK_NULL_HANDLE;
	}
	if(ctx.shadowpipelinelayout != VK_NULL_HANDLE) {
		vkDestroyPipelineLayout(ctx.device, ctx.shadowpipelinelayout, nullptr);
		ctx.shadowpipelinelayout = VK_NULL_HANDLE;
	}
	for(VkDescriptorSetLayout setlayout : ctx.shadowsetlayouts) {
		vkDestroyDescriptorSetLayout(ctx.device, setlayout, nullptr);
	}
	ctx.shadowsetlayouts.clear();
	ctx.shadowpushsize = 0;
	ctx.shadowpushstages = 0;
}

void gvkDestroyGraphicsPipelines(gVKContext& ctx) {
	VkDevice device = ctx.device;
	if(device == VK_NULL_HANDLE) return;
	// Both sample-count builds, in one place: every family's variants first, then the
	// layouts and pools they shared.
	for(uint32_t v = 0; v < GVK_PIPELINE_SAMPLE_VARIANTS; v++) {
		gvkDestroyPipelineVariants(device, ctx.skybox[v]);
		gvkDestroyPipelineVariants(device, ctx.mesh3dpbr[v]);
		gvkDestroyPipelineVariants(device, ctx.mesh3d[v]);
		gvkDestroyPipelineVariants(device, ctx.image2d[v]);
		gvkDestroyPipelineVariants(device, ctx.color2d[v]);
	}
	ctx.activepipelinevariant = 0;
	if(ctx.skyboxpipelinelayout != VK_NULL_HANDLE) { vkDestroyPipelineLayout(device, ctx.skyboxpipelinelayout, nullptr); ctx.skyboxpipelinelayout = VK_NULL_HANDLE; }
	for(VkDescriptorSetLayout l : ctx.skyboxsetlayouts) vkDestroyDescriptorSetLayout(device, l, nullptr);
	ctx.skyboxsetlayouts.clear();
	if(ctx.mesh3dpbrpipelinelayout != VK_NULL_HANDLE) { vkDestroyPipelineLayout(device, ctx.mesh3dpbrpipelinelayout, nullptr); ctx.mesh3dpbrpipelinelayout = VK_NULL_HANDLE; }
	for(VkDescriptorSetLayout setlayout : ctx.mesh3dpbrsetlayouts) vkDestroyDescriptorSetLayout(device, setlayout, nullptr);
	ctx.mesh3dpbrsetlayouts.clear();
	// The sets themselves live in the descriptor pool destroyed below, so only the
	// lookup has to be dropped - keeping stale handles would hand a freed set to a
	// draw after a shader reload.
	ctx.pbrmaterialsets.clear();
	ctx.materialsets.clear();
	ctx.mesh3dpbrpushsize = 0;
	ctx.mesh3dpbrpushstages = 0;
	if(ctx.mesh3dpipelinelayout != VK_NULL_HANDLE) { vkDestroyPipelineLayout(device, ctx.mesh3dpipelinelayout, nullptr); ctx.mesh3dpipelinelayout = VK_NULL_HANDLE; }
	if(ctx.image2dpipelinelayout != VK_NULL_HANDLE) { vkDestroyPipelineLayout(device, ctx.image2dpipelinelayout, nullptr); ctx.image2dpipelinelayout = VK_NULL_HANDLE; }
	if(ctx.color2dpipelinelayout != VK_NULL_HANDLE) { vkDestroyPipelineLayout(device, ctx.color2dpipelinelayout, nullptr); ctx.color2dpipelinelayout = VK_NULL_HANDLE; }
	// Destroying the pool frees every set allocated from it, so any texture
	// descriptor sets are gone too and have to be written again afterwards.
	if(ctx.descriptorpool != VK_NULL_HANDLE) { vkDestroyDescriptorPool(device, ctx.descriptorpool, nullptr); ctx.descriptorpool = VK_NULL_HANDLE; }
	for(VkDescriptorSetLayout setlayout : ctx.mesh3dsetlayouts) vkDestroyDescriptorSetLayout(device, setlayout, nullptr);
	ctx.mesh3dsetlayouts.clear();
	ctx.mesh3dpushsize = 0;
	ctx.mesh3dpushstages = 0;
	for(VkDescriptorSetLayout setlayout : ctx.image2dsetlayouts) vkDestroyDescriptorSetLayout(device, setlayout, nullptr);
	for(VkDescriptorSetLayout setlayout : ctx.color2dsetlayouts) vkDestroyDescriptorSetLayout(device, setlayout, nullptr);
	ctx.mesh3dsetlayouts.clear();
	ctx.image2dsetlayouts.clear();
	ctx.color2dsetlayouts.clear();
	ctx.color2dpushsize = 0;
	ctx.color2dpushstages = 0;
	ctx.image2dpushsize = 0;
	ctx.image2dpushstages = 0;
	ctx.mesh3dpushsize = 0;
	ctx.mesh3dpushstages = 0;
}

void gvkDestroyPipelineCache(gVKContext& ctx) {
	if(ctx.device != VK_NULL_HANDLE && ctx.pipelinecache != VK_NULL_HANDLE) {
		vkDestroyPipelineCache(ctx.device, ctx.pipelinecache, nullptr);
		ctx.pipelinecache = VK_NULL_HANDLE;
	}
}

#endif /* GVK_VULKAN */
