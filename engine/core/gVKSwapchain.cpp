/*
 * gVKSwapchain.cpp
 *
 * Skeleton created in phase 0. The bodies are filled in by the owner of this
 * module (Burak); see VULKAN_RENDERING_GOREV_DAGILIMI.md section 5.4.
 */

#include "gVKSwapchain.h"

#ifdef GVK_DESKTOP_GLFW

#include "gVKRenderTarget.h"
#include "gVKSync.h"
#include "gUtils.h"
#include <GLFW/glfw3.h>

bool gvkCreateSwapchain(gVKContext& ctx, GLFWwindow* window) {
	// TODO: query the surface capabilities, formats and present modes, pick the
	// format, FIFO present mode and extent, create the swapchain, fetch its images
	// and build one image view per image.
	gLoge("gVKSwapchain") << "gvkCreateSwapchain is not implemented yet.";
	return false;
}

void gvkDestroySwapchain(gVKContext& ctx) {
	// TODO: destroy the image views, then the swapchain, then clear the vectors.
}

bool gvkRecreateSwapchain(gVKContext& ctx, GLFWwindow* window) {
	// TODO: bail out while the window is minimised, wait for the device to go idle,
	// tear down the framebuffers, the present semaphores and the swapchain, then
	// build them again in that order. The render pass stays valid.
	gLoge("gVKSwapchain") << "gvkRecreateSwapchain is not implemented yet.";
	return false;
}

#endif /* GVK_DESKTOP_GLFW */
