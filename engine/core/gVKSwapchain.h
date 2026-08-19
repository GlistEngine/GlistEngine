/*
 * gVKSwapchain.h
 *
 * Swapchain, its image views and the resize path of the Vulkan backend.
 * Created by: Veysel Burak Eroglu.
 */

#pragma once

#ifndef CORE_GVKSWAPCHAIN_H
#define CORE_GVKSWAPCHAIN_H

#include "gVKContext.h"

#ifdef GVK_VULKAN

class gBaseWindow;

/*
 * Creates the swapchain together with its images and image views, and fills in
 * ctx.swapchainformat and ctx.swapchainextent. Present mode is FIFO, which the
 * specification guarantees on every driver.
 */
bool gvkCreateSwapchain(gVKContext& ctx, gBaseWindow* window);

/*
 * Destroys the image views and the swapchain itself. Safe to call twice.
 */
void gvkDestroySwapchain(gVKContext& ctx);

/*
 * Rebuilds everything that depends on the window size after a resize or after the
 * swapchain went out of date. Returns false when the window is minimised, in which
 * case nothing is rebuilt and the caller has to skip the frame.
 */
bool gvkRecreateSwapchain(gVKContext& ctx, gBaseWindow* window);

// Rebuilds the presentation surface and everything that hangs off it. Needed when
// the platform replaces the native window the surface was created from - Android
// does this on rotation and when the activity returns to the foreground - or when
// the driver reports the surface lost. Returns false when there is nothing to
// build onto yet, in which case the caller skips the frame and tries again.
bool gvkRecreateSurface(gVKContext& ctx, gBaseWindow* window);

#endif /* GVK_VULKAN */

#endif /* CORE_GVKSWAPCHAIN_H */
