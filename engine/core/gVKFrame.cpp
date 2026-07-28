/*
 * gVKFrame.cpp
 *
 * Skeleton created in phase 0. The bodies are filled in by the owner of this
 * module (Mehmet); see VULKAN_RENDERING_GOREV_DAGILIMI.md section 5.5.
 */

#include "gVKFrame.h"

#ifdef GVK_DESKTOP_GLFW

#include "gVKSwapchain.h"
#include "gUtils.h"
#include <GLFW/glfw3.h>

bool gvkBeginFrame(gVKContext& ctx, GLFWwindow* window) {
	// TODO: wait on the fence of this frame slot, acquire the next swapchain
	// image, reset the fence and the command buffer, then begin recording and
	// begin the render pass with ctx.clearvalue. Rebuild the swapchain and skip
	// the frame when the acquire reports that it is out of date.
	return false;
}

bool gvkEndFrame(gVKContext& ctx, GLFWwindow* window) {
	// TODO: end the render pass and the recording, submit the command buffer with
	// the image available semaphore as the wait and the render finished semaphore
	// of the acquired image as the signal, present, then advance ctx.currentframe.
	return false;
}

#endif /* GVK_DESKTOP_GLFW */
