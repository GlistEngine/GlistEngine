/*
 * gVKRenderTarget.cpp
 *
 * Skeleton created in phase 0. The bodies are filled in by the owner of this
 * module (Anil); see VULKAN_RENDERING_GOREV_DAGILIMI.md section 5.3.
 */

#include "gVKRenderTarget.h"

#ifdef GVK_DESKTOP_GLFW

#include "gUtils.h"

bool gvkCreateRenderPass(gVKContext& ctx) {
	// TODO: one colour attachment in ctx.swapchainformat with a CLEAR load
	// operation, a STORE store operation and PRESENT_SRC_KHR as its final layout,
	// a single graphics subpass, and one external subpass dependency.
	gLoge("gVKRenderTarget") << "gvkCreateRenderPass is not implemented yet.";
	return false;
}

void gvkDestroyRenderPass(gVKContext& ctx) {
	// TODO: destroy the render pass and reset the handle.
}

bool gvkCreateFramebuffers(gVKContext& ctx) {
	// TODO: one framebuffer per entry of ctx.swapchainimageviews, sized with
	// ctx.swapchainextent and attached to ctx.renderpass.
	gLoge("gVKRenderTarget") << "gvkCreateFramebuffers is not implemented yet.";
	return false;
}

void gvkDestroyFramebuffers(gVKContext& ctx) {
	// TODO: destroy every framebuffer and clear the vector.
}

#endif /* GVK_DESKTOP_GLFW */
