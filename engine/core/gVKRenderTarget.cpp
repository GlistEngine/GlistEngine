/*
 * gVKRenderTarget.cpp
 *
 * Render pass and framebuffers of the Vulkan backend.
 * See VULKAN_RENDERING_GOREV_DAGILIMI.md section 5.3 for the locked decisions.
 */

#include "gVKRenderTarget.h"

#ifdef GVK_DESKTOP_GLFW

#include "gUtils.h"

bool gvkCreateRenderPass(gVKContext& ctx) {
	if(ctx.device == VK_NULL_HANDLE || ctx.swapchainformat == VK_FORMAT_UNDEFINED) {
		gLoge("gVKRenderTarget") << "Cannot create the render pass before the swapchain exists.";
		return false;
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

	VkAttachmentReference colorattachmentref{};
	colorattachmentref.attachment = 0;
	colorattachmentref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorattachmentref;

	// Without this the layout transition of the attachment is not ordered against
	// the work of the subpass, which the validation layers report as an error.
	VkSubpassDependency dependency{};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkRenderPassCreateInfo renderpassinfo{};
	renderpassinfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderpassinfo.attachmentCount = 1;
	renderpassinfo.pAttachments = &colorattachment;
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

	gLogi("gVKRenderTarget") << "Render pass created with one cleared colour attachment";
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

	ctx.framebuffers.resize(ctx.swapchainimageviews.size(), VK_NULL_HANDLE);

	for(size_t i = 0; i < ctx.swapchainimageviews.size(); i++) {
		// One framebuffer per swapchain image, because the frame loop does not know
		// in advance which image it will be handed.
		VkImageView attachments[] = {ctx.swapchainimageviews[i]};

		VkFramebufferCreateInfo framebufferinfo{};
		framebufferinfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferinfo.renderPass = ctx.renderpass;
		framebufferinfo.attachmentCount = 1;
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
