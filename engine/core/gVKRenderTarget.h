/*
 * gVKRenderTarget.h
 *
 * Render pass and framebuffers of the Vulkan backend. The render pass is what
 * actually clears the screen: its colour attachment uses a CLEAR load operation,
 * so the clear value handed to vkCmdBeginRenderPass is written by the GPU.
 * Created by: Anil Emre Akkoc.
 */

#pragma once

#ifndef CORE_GVKRENDERTARGET_H
#define CORE_GVKRENDERTARGET_H

#include "gVKContext.h"

#ifdef GVK_DESKTOP_GLFW

/*
 * Creates the single subpass render pass used by the backend. Depends on
 * ctx.swapchainformat, so the swapchain has to exist first.
 */
bool gvkCreateRenderPass(gVKContext& ctx);
void gvkDestroyRenderPass(gVKContext& ctx);

/*
 * Creates one framebuffer per swapchain image view. Kept separate from the render
 * pass on purpose: a resize rebuilds the framebuffers while the render pass stays
 * valid, because the surface format does not change.
 */
bool gvkCreateFramebuffers(gVKContext& ctx);
void gvkDestroyFramebuffers(gVKContext& ctx);

#endif /* GVK_DESKTOP_GLFW */

#endif /* CORE_GVKRENDERTARGET_H */
