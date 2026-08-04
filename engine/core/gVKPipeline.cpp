/*
 * gVKPipeline.cpp
 */

#include "gVKPipeline.h"

#ifdef GVK_DESKTOP_GLFW

#include "gVKReflect.h"
#include "gVKShaderCompiler.h"
#include "gVKShaders.h"
#include "gUtils.h"
#include <algorithm>


// One combined-image-sampler descriptor per texture; 1024 is plenty of headroom
// for a 2D game and cheap to reserve.
static constexpr uint32_t GVK_DESCRIPTOR_POOL_SETS = 1024;

// The stages the 2D path is built from. Each can be recompiled from its GLSL
// source in a development build, and otherwise comes from the SPIR-V committed
// in gVKShaders.h.
enum {
	GVK_STAGE_COLOR_VERT, GVK_STAGE_COLOR_FRAG,
	GVK_STAGE_IMAGE_VERT, GVK_STAGE_IMAGE_FRAG,
	GVK_STAGE_COLOR3D_VERT, GVK_STAGE_COLOR3D_FRAG,
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
	{"color3d.vert", VK_SHADER_STAGE_VERTEX_BIT, gvkspv_color3d_vert, sizeof(gvkspv_color3d_vert)},
	{"color3d.frag", VK_SHADER_STAGE_FRAGMENT_BIT, gvkspv_color3d_frag, sizeof(gvkspv_color3d_frag)},
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
static bool gvkBuildPipeline(VkDevice device, VkFormat colorFormat, VkFormat depthFormat, const char* name,
		const std::vector<uint32_t>& vertSpirv, const std::vector<uint32_t>& fragSpirv,
		bool lineVariant, bool depthEnabled, bool dynamic3DState, gvkPipelineParts& parts) {
	gVKReflectedLayout reflected;
	if(!gvkReflectSpirv(vertSpirv.data(), vertSpirv.size() * sizeof(uint32_t), reflected) ||
			!gvkReflectSpirv(fragSpirv.data(), fragSpirv.size() * sizeof(uint32_t), reflected)) {
		gLoge("gVKPipeline") << "Could not reflect the " << name << " shaders.";
		return false;
	}
	parts.pushsize = reflected.pushconstantsize;
	parts.pushstages = reflected.pushconstantstages;
	parts.bindings = reflected.bindings;

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

	VkVertexInputBindingDescription binding{};
	binding.binding = 0;
	binding.stride = reflected.vertexstride;
	binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	VkPipelineVertexInputStateCreateInfo vertexinput{};
	vertexinput.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexinput.vertexBindingDescriptionCount = reflected.vertexattributes.empty() ? 0 : 1;
	vertexinput.pVertexBindingDescriptions = &binding;
	vertexinput.vertexAttributeDescriptionCount = static_cast<uint32_t>(reflected.vertexattributes.size());
	vertexinput.pVertexAttributeDescriptions = reflected.vertexattributes.data();

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
	rasterizer.cullMode = VK_CULL_MODE_NONE;
	rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizer.lineWidth = 1.0f;

	VkPipelineMultisampleStateCreateInfo multisample{};
	multisample.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisample.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	VkPipelineDepthStencilStateCreateInfo depthstate{};
	depthstate.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthstate.depthTestEnable = depthEnabled ? VK_TRUE : VK_FALSE;
	depthstate.depthWriteEnable = depthEnabled ? VK_TRUE : VK_FALSE;
	depthstate.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;

	VkPipelineColorBlendAttachmentState blendattachment{};
	blendattachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
			VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	blendattachment.blendEnable = VK_TRUE;
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

	VkDynamicState dynamicstates[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR,
			VK_DYNAMIC_STATE_CULL_MODE, VK_DYNAMIC_STATE_FRONT_FACE,
			VK_DYNAMIC_STATE_DEPTH_TEST_ENABLE, VK_DYNAMIC_STATE_DEPTH_WRITE_ENABLE,
			VK_DYNAMIC_STATE_DEPTH_COMPARE_OP};
	VkPipelineDynamicStateCreateInfo dynamicstate{};
	dynamicstate.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicstate.dynamicStateCount = dynamic3DState ? 7 : 2;
	dynamicstate.pDynamicStates = dynamicstates;

	VkGraphicsPipelineCreateInfo pipelineinfo{};
	pipelineinfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineinfo.stageCount = 2;
	pipelineinfo.pStages = stages;
	pipelineinfo.pVertexInputState = &vertexinput;
	pipelineinfo.pInputAssemblyState = &inputassembly;
	pipelineinfo.pViewportState = &viewportstate;
	pipelineinfo.pRasterizationState = &rasterizer;
	pipelineinfo.pMultisampleState = &multisample;
	pipelineinfo.pDepthStencilState = &depthstate;
	pipelineinfo.pColorBlendState = &colorblend;
	pipelineinfo.pDynamicState = &dynamicstate;
	pipelineinfo.layout = parts.layout;
	VkPipelineRenderingCreateInfo renderinginfo{};
	renderinginfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
	renderinginfo.colorAttachmentCount = 1;
	renderinginfo.pColorAttachmentFormats = &colorFormat;
	renderinginfo.depthAttachmentFormat = depthFormat;
	pipelineinfo.pNext = &renderinginfo;
	pipelineinfo.renderPass = VK_NULL_HANDLE;
	pipelineinfo.subpass = 0;

	VkResult result = vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineinfo, nullptr, &parts.pipeline);
	if(result == VK_SUCCESS && lineVariant) {
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
	if(parts.linepipeline != VK_NULL_HANDLE) vkDestroyPipeline(device, parts.linepipeline, nullptr);
	if(parts.pipeline != VK_NULL_HANDLE) vkDestroyPipeline(device, parts.pipeline, nullptr);
	if(parts.layout != VK_NULL_HANDLE) vkDestroyPipelineLayout(device, parts.layout, nullptr);
	for(VkDescriptorSetLayout setlayout : parts.setlayouts) vkDestroyDescriptorSetLayout(device, setlayout, nullptr);
	parts = gvkPipelineParts{};
}

// Sized from the descriptor types the shaders actually declare, so a shader that
// starts using a different resource kind gets a pool that can serve it.
static bool gvkCreateDescriptorPool(VkDevice device, const gvkPipelineParts& color, const gvkPipelineParts& image,
		const gvkPipelineParts& color3d, VkDescriptorPool& outPool) {
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
	add(color3d.bindings);
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
static bool gvkBuildAll(VkDevice device, VkFormat colorFormat, VkFormat depthFormat, const gvkShaderSet& shaders,
		gvkPipelineParts& color, gvkPipelineParts& image, gvkPipelineParts& color3d, VkDescriptorPool& pool) {
	if(gvkBuildPipeline(device, colorFormat, depthFormat, "colour",
					shaders.spirv[GVK_STAGE_COLOR_VERT], shaders.spirv[GVK_STAGE_COLOR_FRAG], true, false, false, color) &&
			gvkBuildPipeline(device, colorFormat, depthFormat, "image",
					shaders.spirv[GVK_STAGE_IMAGE_VERT], shaders.spirv[GVK_STAGE_IMAGE_FRAG], false, false, false, image) &&
			gvkBuildPipeline(device, colorFormat, depthFormat, "3D material",
					shaders.spirv[GVK_STAGE_COLOR3D_VERT], shaders.spirv[GVK_STAGE_COLOR3D_FRAG], false, true, true, color3d) &&
			gvkCreateDescriptorPool(device, color, image, color3d, pool)) {
		return true;
	}
	if(pool != VK_NULL_HANDLE) { vkDestroyDescriptorPool(device, pool, nullptr); pool = VK_NULL_HANDLE; }
	gvkDestroyParts(device, color);
	gvkDestroyParts(device, image);
	gvkDestroyParts(device, color3d);
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
	if(ctx.device == VK_NULL_HANDLE || ctx.swapchainformat == VK_FORMAT_UNDEFINED ||
			ctx.depthformat == VK_FORMAT_UNDEFINED) {
		gLoge("gVKPipeline") << "Pipelines need dynamic rendering formats first.";
		return false;
	}
	gvkShaderSet shaders;
	gvkLoadShaderSet(shaders, false);

	gvkPipelineParts color;
	gvkPipelineParts image;
	gvkPipelineParts color3d;
	VkDescriptorPool pool = VK_NULL_HANDLE;
	if(!gvkBuildAll(ctx.device, ctx.swapchainformat, ctx.depthformat, shaders, color, image, color3d, pool)) return false;

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
	ctx.color3dpipeline = color3d.pipeline;
	ctx.color3dpipelinelayout = color3d.layout;
	ctx.color3dpushsize = color3d.pushsize;
	ctx.color3dpushstages = color3d.pushstages;
	ctx.color3dsetlayouts = color3d.setlayouts;
	ctx.descriptorpool = pool;

	if(gvkRuntimeShadersAvailable()) {
		gLogi("gVKPipeline") << "2D graphics pipelines created from "
				<< gvkShaderSourceDir() << "; edits to those shaders reload live.";
	} else {
		gLogi("gVKPipeline") << "2D graphics pipelines created.";
	}
	return true;
}

bool gvkReloadGraphicsPipelines(gVKContext& ctx) {
	if(ctx.device == VK_NULL_HANDLE || ctx.swapchainformat == VK_FORMAT_UNDEFINED) return false;

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

void gvkDestroyGraphicsPipelines(gVKContext& ctx) {
	VkDevice device = ctx.device;
	if(device == VK_NULL_HANDLE) return;
	if(ctx.image2dpipeline != VK_NULL_HANDLE) { vkDestroyPipeline(device, ctx.image2dpipeline, nullptr); ctx.image2dpipeline = VK_NULL_HANDLE; }
	if(ctx.color3dpipeline != VK_NULL_HANDLE) { vkDestroyPipeline(device, ctx.color3dpipeline, nullptr); ctx.color3dpipeline = VK_NULL_HANDLE; }
	if(ctx.color2dlinepipeline != VK_NULL_HANDLE) { vkDestroyPipeline(device, ctx.color2dlinepipeline, nullptr); ctx.color2dlinepipeline = VK_NULL_HANDLE; }
	if(ctx.color2dpipeline != VK_NULL_HANDLE) { vkDestroyPipeline(device, ctx.color2dpipeline, nullptr); ctx.color2dpipeline = VK_NULL_HANDLE; }
	if(ctx.image2dpipelinelayout != VK_NULL_HANDLE) { vkDestroyPipelineLayout(device, ctx.image2dpipelinelayout, nullptr); ctx.image2dpipelinelayout = VK_NULL_HANDLE; }
	if(ctx.color3dpipelinelayout != VK_NULL_HANDLE) { vkDestroyPipelineLayout(device, ctx.color3dpipelinelayout, nullptr); ctx.color3dpipelinelayout = VK_NULL_HANDLE; }
	if(ctx.color2dpipelinelayout != VK_NULL_HANDLE) { vkDestroyPipelineLayout(device, ctx.color2dpipelinelayout, nullptr); ctx.color2dpipelinelayout = VK_NULL_HANDLE; }
	// Destroying the pool frees every set allocated from it, so any texture
	// descriptor sets are gone too and have to be written again afterwards.
	if(ctx.descriptorpool != VK_NULL_HANDLE) { vkDestroyDescriptorPool(device, ctx.descriptorpool, nullptr); ctx.descriptorpool = VK_NULL_HANDLE; }
	for(VkDescriptorSetLayout setlayout : ctx.image2dsetlayouts) vkDestroyDescriptorSetLayout(device, setlayout, nullptr);
	for(VkDescriptorSetLayout setlayout : ctx.color2dsetlayouts) vkDestroyDescriptorSetLayout(device, setlayout, nullptr);
	for(VkDescriptorSetLayout setlayout : ctx.color3dsetlayouts) vkDestroyDescriptorSetLayout(device, setlayout, nullptr);
	ctx.image2dsetlayouts.clear();
	ctx.color2dsetlayouts.clear();
	ctx.color3dsetlayouts.clear();
	ctx.color2dpushsize = 0;
	ctx.color2dpushstages = 0;
	ctx.image2dpushsize = 0;
	ctx.image2dpushstages = 0;
	ctx.color3dpushsize = 0;
	ctx.color3dpushstages = 0;
}

#endif /* GVK_DESKTOP_GLFW */
