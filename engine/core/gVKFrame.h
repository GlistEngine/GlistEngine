/*
 * gVKFrame.h
 *
 * The Vulkan frame loop: acquire an image, record the command buffer, submit it
 * and present the result. This is the module the engine's main loop drives.
 * Owner: Mehmet.
 */

#pragma once

#ifndef CORE_GVKFRAME_H
#define CORE_GVKFRAME_H

#include "gVKContext.h"

#ifdef GVK_DESKTOP_GLFW

struct GLFWwindow;

/*
 * Waits for the previous use of this frame slot, acquires a swapchain image and
 * begins recording the render pass with ctx.clearvalue. Returns false when the
 * frame has to be skipped, for example because the swapchain was rebuilt; the
 * caller must not call gvkEndFrame in that case.
 */
bool gvkBeginFrame(gVKContext& ctx, GLFWwindow* window);

/*
 * Ends the render pass, submits the command buffer and presents the image, then
 * advances to the next frame slot.
 */
bool gvkEndFrame(gVKContext& ctx, GLFWwindow* window);

#endif /* GVK_DESKTOP_GLFW */

#endif /* CORE_GVKFRAME_H */
