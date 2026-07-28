/*
 * gVKSwapchain.h
 *
 * Swapchain, its image views and the resize path of the Vulkan backend.
 * Owner: Burak.
 */

#pragma once

#ifndef CORE_GVKSWAPCHAIN_H
#define CORE_GVKSWAPCHAIN_H

#include "gVKContext.h"

#ifdef GVK_DESKTOP_GLFW

struct GLFWwindow;

/*
 * Creates the swapchain together with its images and image views, and fills in
 * ctx.swapchainformat and ctx.swapchainextent. Present mode is FIFO, which the
 * specification guarantees on every driver.
 */
bool gvkCreateSwapchain(gVKContext& ctx, GLFWwindow* window);

/*
 * Destroys the image views and the swapchain itself. Safe to call twice.
 */
void gvkDestroySwapchain(gVKContext& ctx);

/*
 * Rebuilds everything that depends on the window size after a resize or after the
 * swapchain went out of date. Returns false when the window is minimised, in which
 * case nothing is rebuilt and the caller has to skip the frame.
 */
bool gvkRecreateSwapchain(gVKContext& ctx, GLFWwindow* window);

#endif /* GVK_DESKTOP_GLFW */

#endif /* CORE_GVKSWAPCHAIN_H */
