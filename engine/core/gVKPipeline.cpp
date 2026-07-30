/*
 * gVKPipeline.cpp
 */

#include "gVKPipeline.h"

#ifdef GVK_DESKTOP_GLFW

#include "gVKShaders.h"
#include "gUtils.h"

// mat4 mvp + vec4 colour/tint. Same size for both pipelines; the colour pipeline
// reads it in the vertex stage, the image pipeline reads the tint in the fragment
// stage, so the range covers both stages.
static constexpr uint32_t GVK_PUSH_CONSTANT_SIZE = sizeof(float) * (16 + 4);

static VkShaderModule gvkCreateShaderModule(VkDevice device, const uint32_t* code, size_t sizeBytes) {
	VkShaderModuleCreateInfo info{};
	info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	info.codeSize = sizeBytes;
	info.pCode = code;
	VkShaderModule mod = VK_NULL_HANDLE;
	if(vkCreateShaderModule(device, &info, nullptr, &mod) != VK_SUCCESS) {
		gLoge("gVKPipeline") << "vkCreateShaderModule failed.";
		return VK_NULL_HANDLE;
	}
	return mod;
}

// Builds a pipeline that shares all the 2D state (triangle list, no depth, no
// cull, dynamic viewport/scissor, straight alpha blending) and differs only in
// its shaders, vertex layout and pipeline layout.
static VkPipeline gvkBuildPipeline(VkDevice device, VkRenderPass renderpass,
		VkShaderModule vert, VkShaderModule frag,
		const VkVertexInputBindingDescription& binding,
		const VkVertexInputAttributeDescription* attributes, uint32_t attributeCount,
		VkPipelineLayout layout) {
	VkPipelineShaderStageCreateInfo stages[2]{};
	stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
	stages[0].module = vert;
	stages[0].pName = "main";
	stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	stages[1].module = frag;
	stages[1].pName = "main";

	VkPipelineVertexInputStateCreateInfo vertexinput{};
	vertexinput.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexinput.vertexBindingDescriptionCount = 1;
	vertexinput.pVertexBindingDescriptions = &binding;
	vertexinput.vertexAttributeDescriptionCount = attributeCount;
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
	rasterizer.cullMode = VK_CULL_MODE_NONE;
	rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizer.lineWidth = 1.0f;

	VkPipelineMultisampleStateCreateInfo multisample{};
	multisample.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisample.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

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

	VkDynamicState dynamicstates[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
	VkPipelineDynamicStateCreateInfo dynamicstate{};
	dynamicstate.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicstate.dynamicStateCount = 2;
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
	// No depth attachment in the render pass, so no depth-stencil state.
	pipelineinfo.pColorBlendState = &colorblend;
	pipelineinfo.pDynamicState = &dynamicstate;
	pipelineinfo.layout = layout;
	pipelineinfo.renderPass = renderpass;
	pipelineinfo.subpass = 0;

	VkPipeline pipeline = VK_NULL_HANDLE;
	if(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineinfo, nullptr, &pipeline) != VK_SUCCESS) {
		gLoge("gVKPipeline") << "vkCreateGraphicsPipelines failed.";
		return VK_NULL_HANDLE;
	}
	return pipeline;
}

bool gvkCreateGraphicsPipelines(gVKContext& ctx) {
	if(ctx.device == VK_NULL_HANDLE || ctx.renderpass == VK_NULL_HANDLE) {
		gLoge("gVKPipeline") << "Pipelines need a device and render pass first.";
		return false;
	}
	VkDevice device = ctx.device;

	/* ---------------- colour pipeline (triangle / rectangle) ---------------- */
	VkShaderModule colorvert = gvkCreateShaderModule(device, gvkspv_color2d_vert, sizeof(gvkspv_color2d_vert));
	VkShaderModule colorfrag = gvkCreateShaderModule(device, gvkspv_color2d_frag, sizeof(gvkspv_color2d_frag));

	VkPushConstantRange colorpush{};
	colorpush.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	colorpush.offset = 0;
	colorpush.size = GVK_PUSH_CONSTANT_SIZE;

	VkPipelineLayoutCreateInfo colorlayoutinfo{};
	colorlayoutinfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	colorlayoutinfo.pushConstantRangeCount = 1;
	colorlayoutinfo.pPushConstantRanges = &colorpush;
	vkCreatePipelineLayout(device, &colorlayoutinfo, nullptr, &ctx.color2dpipelinelayout);

	VkVertexInputBindingDescription colorbinding{};
	colorbinding.binding = 0;
	colorbinding.stride = sizeof(float) * 2;
	colorbinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	VkVertexInputAttributeDescription colorattr{};
	colorattr.location = 0;
	colorattr.binding = 0;
	colorattr.format = VK_FORMAT_R32G32_SFLOAT;
	colorattr.offset = 0;

	ctx.color2dpipeline = gvkBuildPipeline(device, ctx.renderpass, colorvert, colorfrag, colorbinding, &colorattr, 1,
			ctx.color2dpipelinelayout);

	/* ---------------- image descriptor layout + pool ---------------- */
	VkDescriptorSetLayoutBinding samplerbinding{};
	samplerbinding.binding = 0;
	samplerbinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerbinding.descriptorCount = 1;
	samplerbinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	VkDescriptorSetLayoutCreateInfo setlayoutinfo{};
	setlayoutinfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	setlayoutinfo.bindingCount = 1;
	setlayoutinfo.pBindings = &samplerbinding;
	vkCreateDescriptorSetLayout(device, &setlayoutinfo, nullptr, &ctx.image2ddescriptorsetlayout);

	// One combined-image-sampler descriptor per texture; 1024 is plenty of headroom
	// for a 2D game and cheap to reserve.
	VkDescriptorPoolSize poolsize{};
	poolsize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolsize.descriptorCount = 1024;
	VkDescriptorPoolCreateInfo poolinfo{};
	poolinfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolinfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	poolinfo.maxSets = 1024;
	poolinfo.poolSizeCount = 1;
	poolinfo.pPoolSizes = &poolsize;
	vkCreateDescriptorPool(device, &poolinfo, nullptr, &ctx.descriptorpool);

	/* ---------------- image pipeline (textured quad) ---------------- */
	VkShaderModule imagevert = gvkCreateShaderModule(device, gvkspv_image2d_vert, sizeof(gvkspv_image2d_vert));
	VkShaderModule imagefrag = gvkCreateShaderModule(device, gvkspv_image2d_frag, sizeof(gvkspv_image2d_frag));

	VkPushConstantRange imagepush{};
	imagepush.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
	imagepush.offset = 0;
	imagepush.size = GVK_PUSH_CONSTANT_SIZE;

	VkPipelineLayoutCreateInfo imagelayoutinfo{};
	imagelayoutinfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	imagelayoutinfo.setLayoutCount = 1;
	imagelayoutinfo.pSetLayouts = &ctx.image2ddescriptorsetlayout;
	imagelayoutinfo.pushConstantRangeCount = 1;
	imagelayoutinfo.pPushConstantRanges = &imagepush;
	vkCreatePipelineLayout(device, &imagelayoutinfo, nullptr, &ctx.image2dpipelinelayout);

	VkVertexInputBindingDescription imagebinding{};
	imagebinding.binding = 0;
	imagebinding.stride = sizeof(float) * 4;
	imagebinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	VkVertexInputAttributeDescription imageattrs[2]{};
	imageattrs[0].location = 0;
	imageattrs[0].binding = 0;
	imageattrs[0].format = VK_FORMAT_R32G32_SFLOAT;
	imageattrs[0].offset = 0;
	imageattrs[1].location = 1;
	imageattrs[1].binding = 0;
	imageattrs[1].format = VK_FORMAT_R32G32_SFLOAT;
	imageattrs[1].offset = sizeof(float) * 2;

	ctx.image2dpipeline = gvkBuildPipeline(device, ctx.renderpass, imagevert, imagefrag, imagebinding, imageattrs, 2,
			ctx.image2dpipelinelayout);

	// Modules are only needed while the pipelines are being created.
	vkDestroyShaderModule(device, colorvert, nullptr);
	vkDestroyShaderModule(device, colorfrag, nullptr);
	vkDestroyShaderModule(device, imagevert, nullptr);
	vkDestroyShaderModule(device, imagefrag, nullptr);

	bool ok = ctx.color2dpipeline != VK_NULL_HANDLE && ctx.image2dpipeline != VK_NULL_HANDLE;
	if(ok) gLogi("gVKPipeline") << "2D graphics pipelines created.";
	return ok;
}

void gvkDestroyGraphicsPipelines(gVKContext& ctx) {
	VkDevice device = ctx.device;
	if(device == VK_NULL_HANDLE) return;
	if(ctx.image2dpipeline != VK_NULL_HANDLE) { vkDestroyPipeline(device, ctx.image2dpipeline, nullptr); ctx.image2dpipeline = VK_NULL_HANDLE; }
	if(ctx.color2dpipeline != VK_NULL_HANDLE) { vkDestroyPipeline(device, ctx.color2dpipeline, nullptr); ctx.color2dpipeline = VK_NULL_HANDLE; }
	if(ctx.image2dpipelinelayout != VK_NULL_HANDLE) { vkDestroyPipelineLayout(device, ctx.image2dpipelinelayout, nullptr); ctx.image2dpipelinelayout = VK_NULL_HANDLE; }
	if(ctx.color2dpipelinelayout != VK_NULL_HANDLE) { vkDestroyPipelineLayout(device, ctx.color2dpipelinelayout, nullptr); ctx.color2dpipelinelayout = VK_NULL_HANDLE; }
	if(ctx.descriptorpool != VK_NULL_HANDLE) { vkDestroyDescriptorPool(device, ctx.descriptorpool, nullptr); ctx.descriptorpool = VK_NULL_HANDLE; }
	if(ctx.image2ddescriptorsetlayout != VK_NULL_HANDLE) { vkDestroyDescriptorSetLayout(device, ctx.image2ddescriptorsetlayout, nullptr); ctx.image2ddescriptorsetlayout = VK_NULL_HANDLE; }
}

#endif /* GVK_DESKTOP_GLFW */
