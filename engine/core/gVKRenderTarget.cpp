/*
 * gVKRenderTarget.cpp
 *
 * Render pass and framebuffers of the Vulkan backend.
 */

#include "gVKRenderTarget.h"

#ifdef GVK_DESKTOP_GLFW

#include "gVKBuffer.h"
#include "gUtils.h"

VkFormat gvkFindDepthFormat(gVKContext& ctx) {
	if(ctx.physicaldevice == VK_NULL_HANDLE) return VK_FORMAT_UNDEFINED;

	// D32_SFLOAT first: the engine has no stencil work, so a depth-only format
	// leaves the driver free to pick the smaller layout. The two combined formats
	// follow as fallbacks, one of which every Vulkan implementation must support.
	const VkFormat candidates[] = {
			VK_FORMAT_D32_SFLOAT,
			VK_FORMAT_D32_SFLOAT_S8_UINT,
			VK_FORMAT_D24_UNORM_S8_UINT};

	for(VkFormat format : candidates) {
		VkFormatProperties properties{};
		vkGetPhysicalDeviceFormatProperties(ctx.physicaldevice, format, &properties);
		// Optimal tiling only, matching how the image below is created.
		if(properties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
			return format;
		}
	}

	return VK_FORMAT_UNDEFINED;
}

bool gvkCreateDepthResources(gVKContext& ctx) {
	if(ctx.device == VK_NULL_HANDLE) {
		gLoge("gVKRenderTarget") << "Cannot create the depth buffer before the device exists.";
		return false;
	}
	if(ctx.swapchainextent.width == 0 || ctx.swapchainextent.height == 0) {
		gLoge("gVKRenderTarget") << "Cannot create the depth buffer before the swapchain extent is known.";
		return false;
	}

	// The render pass picks the format first and stores it, so a resize reuses the
	// same one and the pass stays valid.
	if(ctx.depthformat == VK_FORMAT_UNDEFINED) {
		ctx.depthformat = gvkFindDepthFormat(ctx);
		if(ctx.depthformat == VK_FORMAT_UNDEFINED) {
			gLoge("gVKRenderTarget") << "No supported depth attachment format found on this device.";
			return false;
		}
	}

	VkImageCreateInfo imageinfo{};
	imageinfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageinfo.imageType = VK_IMAGE_TYPE_2D;
	imageinfo.extent.width = ctx.swapchainextent.width;
	imageinfo.extent.height = ctx.swapchainextent.height;
	imageinfo.extent.depth = 1;
	imageinfo.mipLevels = 1;
	imageinfo.arrayLayers = 1;
	imageinfo.format = ctx.depthformat;
	imageinfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	// The render pass clears the image at the start of every frame, so whatever the
	// previous contents were does not matter.
	imageinfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageinfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	imageinfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageinfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if(vkCreateImage(ctx.device, &imageinfo, nullptr, &ctx.depthimage) != VK_SUCCESS) {
		gLoge("gVKRenderTarget") << "vkCreateImage failed for the depth buffer.";
		ctx.depthimage = VK_NULL_HANDLE;
		return false;
	}

	VkMemoryRequirements memreq{};
	vkGetImageMemoryRequirements(ctx.device, ctx.depthimage, &memreq);

	VkMemoryAllocateInfo allocinfo{};
	allocinfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocinfo.allocationSize = memreq.size;
	// Device local: the CPU never touches the depth buffer.
	allocinfo.memoryTypeIndex = gvkFindMemoryType(ctx, memreq.memoryTypeBits,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	if(vkAllocateMemory(ctx.device, &allocinfo, nullptr, &ctx.depthimagememory) != VK_SUCCESS) {
		gLoge("gVKRenderTarget") << "vkAllocateMemory failed for the depth buffer.";
		gvkDestroyDepthResources(ctx);
		return false;
	}
	vkBindImageMemory(ctx.device, ctx.depthimage, ctx.depthimagememory, 0);

	VkImageViewCreateInfo viewinfo{};
	viewinfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewinfo.image = ctx.depthimage;
	viewinfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewinfo.format = ctx.depthformat;
	// Only the depth aspect is used even when the format carries a stencil part.
	viewinfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	viewinfo.subresourceRange.baseMipLevel = 0;
	viewinfo.subresourceRange.levelCount = 1;
	viewinfo.subresourceRange.baseArrayLayer = 0;
	viewinfo.subresourceRange.layerCount = 1;

	if(vkCreateImageView(ctx.device, &viewinfo, nullptr, &ctx.depthimageview) != VK_SUCCESS) {
		gLoge("gVKRenderTarget") << "vkCreateImageView failed for the depth buffer.";
		gvkDestroyDepthResources(ctx);
		return false;
	}

	gLogi("gVKRenderTarget") << "Depth buffer created: format " << ctx.depthformat
			<< " at " << ctx.swapchainextent.width << "x" << ctx.swapchainextent.height;
	return true;
}

void gvkDestroyDepthResources(gVKContext& ctx) {
	if(ctx.device == VK_NULL_HANDLE) return;

	if(ctx.depthimageview != VK_NULL_HANDLE) {
		vkDestroyImageView(ctx.device, ctx.depthimageview, nullptr);
		ctx.depthimageview = VK_NULL_HANDLE;
	}
	if(ctx.depthimage != VK_NULL_HANDLE) {
		vkDestroyImage(ctx.device, ctx.depthimage, nullptr);
		ctx.depthimage = VK_NULL_HANDLE;
	}
	if(ctx.depthimagememory != VK_NULL_HANDLE) {
		vkFreeMemory(ctx.device, ctx.depthimagememory, nullptr);
		ctx.depthimagememory = VK_NULL_HANDLE;
	}
	// depthformat is deliberately kept: the render pass was built around it and
	// stays valid across a resize.
}

bool gvkCreateRenderPass(gVKContext& ctx) {
	if(ctx.device == VK_NULL_HANDLE || ctx.swapchainformat == VK_FORMAT_UNDEFINED) {
		gLoge("gVKRenderTarget") << "Cannot create the render pass before the swapchain exists.";
		return false;
	}

	// The attachment description below has to name a concrete format, so the choice
	// is made here rather than in gvkCreateDepthResources.
	if(ctx.depthformat == VK_FORMAT_UNDEFINED) {
		ctx.depthformat = gvkFindDepthFormat(ctx);
		if(ctx.depthformat == VK_FORMAT_UNDEFINED) {
			gLoge("gVKRenderTarget") << "No supported depth attachment format found on this device.";
			return false;
		}
	}

	VkAttachmentDescription colorattachment{};
	// The attachment has to match the images it will be used with.
	colorattachment.format = ctx.swapchainformat;
	colorattachment.samples = VK_SAMPLE_COUNT_1_BIT;
	// This is what paints the screen: the clear value handed to
	// vkCmdBeginRenderPass is written over the whole attachment by the GPU.
	colorattachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	// The result is kept, because it is what gets presented.
	colorattachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorattachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorattachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	// The previous contents are cleared anyway, so there is nothing worth keeping.
	colorattachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	// Handover point to the presentation engine.
	colorattachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentDescription depthattachment{};
	depthattachment.format = ctx.depthformat;
	depthattachment.samples = VK_SAMPLE_COUNT_1_BIT;
	// Cleared every frame for the same reason as the colour attachment.
	depthattachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	// Nothing reads depth once the frame is done, so letting the driver drop it
	// saves the write-out on tiled architectures.
	depthattachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthattachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthattachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthattachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthattachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference colorattachmentref{};
	colorattachmentref.attachment = 0;
	colorattachmentref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthattachmentref{};
	depthattachmentref.attachment = 1;
	depthattachmentref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorattachmentref;
	subpass.pDepthStencilAttachment = &depthattachmentref;

	// Without this the layout transition of the attachment is not ordered against
	// the work of the subpass, which the validation layers report as an error. The
	// early/late fragment test stages are in the masks because the depth attachment
	// is cleared and written there, not at colour output.
	VkSubpassDependency dependency{};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
			| VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
			| VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
			| VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

	VkAttachmentDescription attachments[] = {colorattachment, depthattachment};

	VkRenderPassCreateInfo renderpassinfo{};
	renderpassinfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderpassinfo.attachmentCount = 2;
	renderpassinfo.pAttachments = attachments;
	renderpassinfo.subpassCount = 1;
	renderpassinfo.pSubpasses = &subpass;
	renderpassinfo.dependencyCount = 1;
	renderpassinfo.pDependencies = &dependency;

	VkResult result = vkCreateRenderPass(ctx.device, &renderpassinfo, nullptr, &ctx.renderpass);
	if(result != VK_SUCCESS) {
		gLoge("gVKRenderTarget") << "vkCreateRenderPass failed! VkResult: " << result;
		ctx.renderpass = VK_NULL_HANDLE;
		return false;
	}

	gLogi("gVKRenderTarget") << "Render pass created with a cleared colour and depth attachment";
	return true;
}

void gvkDestroyRenderPass(gVKContext& ctx) {
	if(ctx.device == VK_NULL_HANDLE) return;

	if(ctx.renderpass != VK_NULL_HANDLE) {
		vkDestroyRenderPass(ctx.device, ctx.renderpass, nullptr);
		ctx.renderpass = VK_NULL_HANDLE;
	}
}

bool gvkCreateFramebuffers(gVKContext& ctx) {
	if(ctx.device == VK_NULL_HANDLE || ctx.renderpass == VK_NULL_HANDLE) {
		gLoge("gVKRenderTarget") << "Cannot create the framebuffers before the render pass exists.";
		return false;
	}
	if(ctx.swapchainimageviews.empty()) {
		gLoge("gVKRenderTarget") << "Cannot create the framebuffers without swapchain image views.";
		return false;
	}
	if(ctx.depthimageview == VK_NULL_HANDLE) {
		gLoge("gVKRenderTarget") << "Cannot create the framebuffers before the depth buffer exists.";
		return false;
	}

	ctx.framebuffers.resize(ctx.swapchainimageviews.size(), VK_NULL_HANDLE);

	for(size_t i = 0; i < ctx.swapchainimageviews.size(); i++) {
		// One framebuffer per swapchain image, because the frame loop does not know
		// in advance which image it will be handed. The depth view is shared by all
		// of them: only one frame is inside the render pass at a time, and the pass
		// clears depth on entry, so no frame can observe another's depth values.
		VkImageView attachments[] = {ctx.swapchainimageviews[i], ctx.depthimageview};

		VkFramebufferCreateInfo framebufferinfo{};
		framebufferinfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferinfo.renderPass = ctx.renderpass;
		framebufferinfo.attachmentCount = 2;
		framebufferinfo.pAttachments = attachments;
		framebufferinfo.width = ctx.swapchainextent.width;
		framebufferinfo.height = ctx.swapchainextent.height;
		framebufferinfo.layers = 1;

		VkResult result = vkCreateFramebuffer(ctx.device, &framebufferinfo, nullptr, &ctx.framebuffers[i]);
		if(result != VK_SUCCESS) {
			gLoge("gVKRenderTarget") << "vkCreateFramebuffer failed for swapchain image " << i
					<< "! VkResult: " << result;
			gvkDestroyFramebuffers(ctx);
			return false;
		}
	}

	gLogi("gVKRenderTarget") << "Framebuffers created: " << ctx.framebuffers.size()
			<< " at " << ctx.swapchainextent.width << "x" << ctx.swapchainextent.height;
	return true;
}

void gvkDestroyFramebuffers(gVKContext& ctx) {
	if(ctx.device == VK_NULL_HANDLE) return;

	// Kept separate from the render pass on purpose: a resize rebuilds these while
	// the render pass stays valid, since the surface format does not change.
	for(VkFramebuffer framebuffer : ctx.framebuffers) {
		if(framebuffer != VK_NULL_HANDLE) vkDestroyFramebuffer(ctx.device, framebuffer, nullptr);
	}
	ctx.framebuffers.clear();
}

#endif /* GVK_DESKTOP_GLFW */
