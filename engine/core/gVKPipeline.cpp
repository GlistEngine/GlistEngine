/*
 * gVKPipeline.cpp
 */

#include "gVKPipeline.h"

#ifdef GVK_DESKTOP_GLFW

#include "gVKReflect.h"
#include "gVKShaderCompiler.h"
#include "gVKShaders.h"
#include "gUtils.h"
// For gVertex, which the 3D vertex layout below is asserted against.
#include "gVbo.h"
#include <algorithm>
#include <cstddef>
#include <iterator>


// One combined-image-sampler descriptor per texture; 1024 is plenty of headroom
// for a 2D game and cheap to reserve.
static constexpr uint32_t GVK_DESCRIPTOR_POOL_SETS = 1024;

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

// Everything one pipeline needs, all of it derived from its shaders. linepipeline
// is the same pipeline with a line topology, used to stroke unfilled shapes; it
// stays VK_NULL_HANDLE for pipelines with no outline form.
struct gvkPipelineParts {
	VkPipeline pipeline = VK_NULL_HANDLE;
	VkPipeline linepipeline = VK_NULL_HANDLE;
	// The same pipeline with the opposite blend setting; see gvkPipelineOptions.
	VkPipeline blendvariantpipeline = VK_NULL_HANDLE;
	VkPipelineLayout layout = VK_NULL_HANDLE;
	std::vector<VkDescriptorSetLayout> setlayouts;
	uint32_t pushsize = 0;
	VkShaderStageFlags pushstages = 0;
	std::vector<gVKReflectedBinding> bindings;
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
static bool gvkBuildPipeline(VkDevice device, VkRenderPass renderpass, const char* name,
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
	pipelineinfo.renderPass = renderpass;
	pipelineinfo.subpass = 0;

	VkResult result = vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineinfo, nullptr, &parts.pipeline);
	if(result == VK_SUCCESS && options.blendvariant) {
		// A second copy with blending flipped, so a 3D draw can follow the renderer's
		// alpha blending state. It cannot be a dynamic state - blending is baked into
		// a Vulkan pipeline - so the choice is made by binding one or the other.
		blendattachment.blendEnable = options.blend ? VK_FALSE : VK_TRUE;
		result = vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineinfo, nullptr,
				&parts.blendvariantpipeline);
		blendattachment.blendEnable = options.blend ? VK_TRUE : VK_FALSE;
	}
	if(result == VK_SUCCESS && options.linevariant) {
		// Identical state apart from the topology: an unfilled shape is stroked as
		// separate edges, which is what the OpenGL path draws through
		// DRAWMODE_LINELOOP. A list rather than a strip so primitive restart stays
		// off - a strip would have to leave it enabled on Metal, which quietly turns
		// an index of ~0 into a break should this pipeline ever draw indexed. The
		// line width stays 1.0, the only value guaranteed without wideLines.
		inputassembly.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
		result = vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineinfo, nullptr, &parts.linepipeline);
	}
	vkDestroyShaderModule(device, vert, nullptr);
	vkDestroyShaderModule(device, frag, nullptr);
	if(result != VK_SUCCESS) {
		gLoge("gVKPipeline") << "vkCreateGraphicsPipelines failed for " << name;
		return false;
	}
	return true;
}

static void gvkDestroyParts(VkDevice device, gvkPipelineParts& parts) {
	if(parts.blendvariantpipeline != VK_NULL_HANDLE) vkDestroyPipeline(device, parts.blendvariantpipeline, nullptr);
	if(parts.linepipeline != VK_NULL_HANDLE) vkDestroyPipeline(device, parts.linepipeline, nullptr);
	if(parts.pipeline != VK_NULL_HANDLE) vkDestroyPipeline(device, parts.pipeline, nullptr);
	if(parts.layout != VK_NULL_HANDLE) vkDestroyPipelineLayout(device, parts.layout, nullptr);
	for(VkDescriptorSetLayout setlayout : parts.setlayouts) vkDestroyDescriptorSetLayout(device, setlayout, nullptr);
	parts = gvkPipelineParts{};
}

// Sized from the descriptor types the shaders actually declare, so a shader that
// starts using a different resource kind gets a pool that can serve it.
static bool gvkCreateDescriptorPool(VkDevice device, const gvkPipelineParts& color, const gvkPipelineParts& image,
		const gvkPipelineParts& mesh3d, const gvkPipelineParts& mesh3dpbr, VkDescriptorPool& outPool) {
	std::vector<VkDescriptorPoolSize> sizes;
	auto add = [&sizes](const std::vector<gVKReflectedBinding>& bindings) {
		for(const gVKReflectedBinding& b : bindings) {
			VkDescriptorPoolSize* existing = nullptr;
			for(VkDescriptorPoolSize& s : sizes) if(s.type == b.type) { existing = &s; break; }
			if(existing != nullptr) existing->descriptorCount += b.count * GVK_DESCRIPTOR_POOL_SETS;
			else sizes.push_back({b.type, b.count * GVK_DESCRIPTOR_POOL_SETS});
		}
	};
	add(color.bindings);
	add(image.bindings);
	// The 3D path's scene uniform block is a different descriptor type to the 2D
	// path's samplers, so leaving it out here would leave nothing to allocate its
	// sets from.
	add(mesh3d.bindings);
	// The PBR path allocates one set per material rather than one per texture, so
	// its five samplers have to be counted here too.
	add(mesh3dpbr.bindings);
	if(sizes.empty()) return true;   // no shader declares a descriptor; nothing to pool

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
static bool gvkBuildAll(VkDevice device, VkRenderPass renderpass, const gvkShaderSet& shaders,
		gvkPipelineParts& color, gvkPipelineParts& image, gvkPipelineParts& mesh3d,
		gvkPipelineParts& mesh3dpbr, gvkPipelineParts& skybox, VkDescriptorPool& pool) {
	gvkPipelineOptions coloropts;
	coloropts.linevariant = true;

	gvkPipelineOptions imageopts;

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

	// The sky needs the depth buffer, so scene geometry occludes it, but never culls
	// and never blends. Its vertex layout is small enough to come from reflection.
	// The depth compare op is dynamic here as on the other 3D paths, which is what
	// lets gSkybox ask for the same EQUAL test the OpenGL path uses.
	gvkPipelineOptions skyboxopts;
	skyboxopts.depthtest = true;
	skyboxopts.blend = false;

	if(gvkBuildPipeline(device, renderpass, "colour",
					shaders.spirv[GVK_STAGE_COLOR_VERT], shaders.spirv[GVK_STAGE_COLOR_FRAG], coloropts, color) &&
			gvkBuildPipeline(device, renderpass, "image",
					shaders.spirv[GVK_STAGE_IMAGE_VERT], shaders.spirv[GVK_STAGE_IMAGE_FRAG], imageopts, image) &&
			gvkBuildPipeline(device, renderpass, "mesh3d",
					shaders.spirv[GVK_STAGE_MESH3D_VERT], shaders.spirv[GVK_STAGE_MESH3D_FRAG], mesh3dopts, mesh3d) &&
			gvkBuildPipeline(device, renderpass, "mesh3dpbr",
					shaders.spirv[GVK_STAGE_MESH3DPBR_VERT], shaders.spirv[GVK_STAGE_MESH3DPBR_FRAG],
					mesh3dopts, mesh3dpbr) &&
			gvkBuildPipeline(device, renderpass, "skybox3d",
					shaders.spirv[GVK_STAGE_SKYBOX_VERT], shaders.spirv[GVK_STAGE_SKYBOX_FRAG],
					skyboxopts, skybox) &&
			gvkCreateDescriptorPool(device, color, image, mesh3d, mesh3dpbr, pool)) {
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
	gvkShaderSet shaders;
	gvkLoadShaderSet(shaders, false);

	gvkPipelineParts color;
	gvkPipelineParts image;
	gvkPipelineParts mesh3d;
	gvkPipelineParts mesh3dpbr;
	gvkPipelineParts skybox;
	VkDescriptorPool pool = VK_NULL_HANDLE;
	if(!gvkBuildAll(ctx.device, ctx.renderpass, shaders, color, image, mesh3d, mesh3dpbr, skybox, pool)) return false;

	ctx.skyboxpipeline = skybox.pipeline;
	ctx.skyboxpipelinelayout = skybox.layout;
	ctx.skyboxsetlayouts = skybox.setlayouts;
	ctx.skyboxpushsize = skybox.pushsize;
	ctx.skyboxpushstages = skybox.pushstages;

	ctx.mesh3dpbrpipeline = mesh3dpbr.pipeline;
	ctx.mesh3dpbrblendpipeline = mesh3dpbr.blendvariantpipeline;
	ctx.mesh3dpbrpipelinelayout = mesh3dpbr.layout;
	ctx.mesh3dpbrsetlayouts = mesh3dpbr.setlayouts;
	ctx.mesh3dpbrpushsize = mesh3dpbr.pushsize;
	ctx.mesh3dpbrpushstages = mesh3dpbr.pushstages;

	ctx.mesh3dpipeline = mesh3d.pipeline;
	ctx.mesh3dblendpipeline = mesh3d.blendvariantpipeline;
	ctx.mesh3dlinepipeline = mesh3d.linepipeline;
	ctx.mesh3dpipelinelayout = mesh3d.layout;
	ctx.mesh3dsetlayouts = mesh3d.setlayouts;
	ctx.mesh3dpushsize = mesh3d.pushsize;
	ctx.mesh3dpushstages = mesh3d.pushstages;
	ctx.color2dpipeline = color.pipeline;
	ctx.color2dlinepipeline = color.linepipeline;
	ctx.color2dpipelinelayout = color.layout;
	ctx.color2dsetlayouts = color.setlayouts;
	ctx.color2dpushsize = color.pushsize;
	ctx.color2dpushstages = color.pushstages;
	ctx.image2dpipeline = image.pipeline;
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

	gvkPipelineParts parts;
	if(!gvkBuildPipeline(ctx.device, ctx.shadowrenderpass, "shadow3d",
			shaders.spirv[GVK_STAGE_SHADOW_VERT], shaders.spirv[GVK_STAGE_SHADOW_FRAG],
			options, parts)) {
		return false;
	}

	ctx.shadowpipeline = parts.pipeline;
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
	if(ctx.mesh3dpbrpipeline != VK_NULL_HANDLE) { vkDestroyPipeline(device, ctx.mesh3dpbrpipeline, nullptr); ctx.mesh3dpbrpipeline = VK_NULL_HANDLE; }
	if(ctx.skyboxpipeline != VK_NULL_HANDLE) { vkDestroyPipeline(device, ctx.skyboxpipeline, nullptr); ctx.skyboxpipeline = VK_NULL_HANDLE; }
	if(ctx.skyboxpipelinelayout != VK_NULL_HANDLE) { vkDestroyPipelineLayout(device, ctx.skyboxpipelinelayout, nullptr); ctx.skyboxpipelinelayout = VK_NULL_HANDLE; }
	for(VkDescriptorSetLayout l : ctx.skyboxsetlayouts) vkDestroyDescriptorSetLayout(device, l, nullptr);
	ctx.skyboxsetlayouts.clear();
	if(ctx.mesh3dpbrblendpipeline != VK_NULL_HANDLE) { vkDestroyPipeline(device, ctx.mesh3dpbrblendpipeline, nullptr); ctx.mesh3dpbrblendpipeline = VK_NULL_HANDLE; }
	if(ctx.mesh3dblendpipeline != VK_NULL_HANDLE) { vkDestroyPipeline(device, ctx.mesh3dblendpipeline, nullptr); ctx.mesh3dblendpipeline = VK_NULL_HANDLE; }
	if(ctx.mesh3dpbrpipelinelayout != VK_NULL_HANDLE) { vkDestroyPipelineLayout(device, ctx.mesh3dpbrpipelinelayout, nullptr); ctx.mesh3dpbrpipelinelayout = VK_NULL_HANDLE; }
	for(VkDescriptorSetLayout setlayout : ctx.mesh3dpbrsetlayouts) vkDestroyDescriptorSetLayout(device, setlayout, nullptr);
	ctx.mesh3dpbrsetlayouts.clear();
	// The sets themselves live in the descriptor pool destroyed below, so only the
	// lookup has to be dropped - keeping stale handles would hand a freed set to a
	// draw after a shader reload.
	ctx.pbrmaterialsets.clear();
	ctx.mesh3dpbrpushsize = 0;
	ctx.mesh3dpbrpushstages = 0;
	if(ctx.mesh3dlinepipeline != VK_NULL_HANDLE) { vkDestroyPipeline(device, ctx.mesh3dlinepipeline, nullptr); ctx.mesh3dlinepipeline = VK_NULL_HANDLE; }
	if(ctx.mesh3dpipeline != VK_NULL_HANDLE) { vkDestroyPipeline(device, ctx.mesh3dpipeline, nullptr); ctx.mesh3dpipeline = VK_NULL_HANDLE; }
	if(ctx.image2dpipeline != VK_NULL_HANDLE) { vkDestroyPipeline(device, ctx.image2dpipeline, nullptr); ctx.image2dpipeline = VK_NULL_HANDLE; }
	if(ctx.color2dlinepipeline != VK_NULL_HANDLE) { vkDestroyPipeline(device, ctx.color2dlinepipeline, nullptr); ctx.color2dlinepipeline = VK_NULL_HANDLE; }
	if(ctx.color2dpipeline != VK_NULL_HANDLE) { vkDestroyPipeline(device, ctx.color2dpipeline, nullptr); ctx.color2dpipeline = VK_NULL_HANDLE; }
	if(ctx.mesh3dpipelinelayout != VK_NULL_HANDLE) { vkDestroyPipelineLayout(device, ctx.mesh3dpipelinelayout, nullptr); ctx.mesh3dpipelinelayout = VK_NULL_HANDLE; }
	if(ctx.image2dpipelinelayout != VK_NULL_HANDLE) { vkDestroyPipelineLayout(device, ctx.image2dpipelinelayout, nullptr); ctx.image2dpipelinelayout = VK_NULL_HANDLE; }
	if(ctx.color2dpipelinelayout != VK_NULL_HANDLE) { vkDestroyPipelineLayout(device, ctx.color2dpipelinelayout, nullptr); ctx.color2dpipelinelayout = VK_NULL_HANDLE; }
	// Destroying the pool frees every set allocated from it, so any texture
	// descriptor sets are gone too and have to be written again afterwards.
	if(ctx.descriptorpool != VK_NULL_HANDLE) { vkDestroyDescriptorPool(device, ctx.descriptorpool, nullptr); ctx.descriptorpool = VK_NULL_HANDLE; }
	for(VkDescriptorSetLayout setlayout : ctx.mesh3dsetlayouts) vkDestroyDescriptorSetLayout(device, setlayout, nullptr);
	if(ctx.mesh3dpbrpipeline != VK_NULL_HANDLE) { vkDestroyPipeline(device, ctx.mesh3dpbrpipeline, nullptr); ctx.mesh3dpbrpipeline = VK_NULL_HANDLE; }
	if(ctx.mesh3dpbrpipelinelayout != VK_NULL_HANDLE) { vkDestroyPipelineLayout(device, ctx.mesh3dpbrpipelinelayout, nullptr); ctx.mesh3dpbrpipelinelayout = VK_NULL_HANDLE; }
	for(VkDescriptorSetLayout setlayout : ctx.mesh3dpbrsetlayouts) vkDestroyDescriptorSetLayout(device, setlayout, nullptr);
	ctx.mesh3dpbrsetlayouts.clear();
	ctx.mesh3dpbrpushsize = 0;
	ctx.mesh3dpbrpushstages = 0;
	if(ctx.mesh3dlinepipeline != VK_NULL_HANDLE) { vkDestroyPipeline(device, ctx.mesh3dlinepipeline, nullptr); ctx.mesh3dlinepipeline = VK_NULL_HANDLE; }
	if(ctx.mesh3dpipeline != VK_NULL_HANDLE) { vkDestroyPipeline(device, ctx.mesh3dpipeline, nullptr); ctx.mesh3dpipeline = VK_NULL_HANDLE; }
	if(ctx.mesh3dpipelinelayout != VK_NULL_HANDLE) { vkDestroyPipelineLayout(device, ctx.mesh3dpipelinelayout, nullptr); ctx.mesh3dpipelinelayout = VK_NULL_HANDLE; }
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

#endif /* GVK_DESKTOP_GLFW */
