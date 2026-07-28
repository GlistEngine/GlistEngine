/*
 * gVKSync.cpp
 *
 * Skeleton created in phase 0. The bodies are filled in by the owner of this
 * module (Ozlem); see VULKAN_RENDERING_GOREV_DAGILIMI.md section 5.1.
 */

#include "gVKSync.h"

#ifdef GVK_DESKTOP_GLFW

#include "gUtils.h"

bool gvkCreateFrameSyncObjects(gVKContext& ctx) {
	// TODO: GVK_MAX_FRAMES_IN_FLIGHT image available semaphores and as many
	// fences. The fences must use VK_FENCE_CREATE_SIGNALED_BIT.
	gLoge("gVKSync") << "gvkCreateFrameSyncObjects is not implemented yet.";
	return false;
}

void gvkDestroyFrameSyncObjects(gVKContext& ctx) {
	// TODO: destroy the semaphores and fences, then clear both vectors.
}

bool gvkCreatePresentSemaphores(gVKContext& ctx, uint32_t imagecount) {
	// TODO: one semaphore per swapchain image, not per frame in flight.
	gLoge("gVKSync") << "gvkCreatePresentSemaphores is not implemented yet.";
	return false;
}

void gvkDestroyPresentSemaphores(gVKContext& ctx) {
	// TODO: destroy the semaphores and clear the vector.
}

#endif /* GVK_DESKTOP_GLFW */
