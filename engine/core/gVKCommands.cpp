/*
 * gVKCommands.cpp
 *
 * Skeleton created in phase 0. The bodies are filled in by the owner of this
 * module (Efe); see VULKAN_RENDERING_GOREV_DAGILIMI.md section 5.2.
 */

#include "gVKCommands.h"

#ifdef GVK_DESKTOP_GLFW

#include "gUtils.h"

bool gvkCreateCommandResources(gVKContext& ctx) {
	// TODO: create the command pool on ctx.graphicsfamily with the
	// RESET_COMMAND_BUFFER flag, then allocate GVK_MAX_FRAMES_IN_FLIGHT primary
	// command buffers in a single call.
	gLoge("gVKCommands") << "gvkCreateCommandResources is not implemented yet.";
	return false;
}

void gvkDestroyCommandResources(gVKContext& ctx) {
	// TODO: destroying the pool frees its command buffers as well, so only
	// vkDestroyCommandPool is needed here. Clear the vector afterwards.
}

#endif /* GVK_DESKTOP_GLFW */
