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
 * Picks a depth format the physical device supports as a depth-stencil attachment,
 * preferring the cheapest one that is enough: D32_SFLOAT, then D32_SFLOAT_S8_UINT,
 * then D24_UNORM_S8_UINT. Returns VK_FORMAT_UNDEFINED if the device supports none
 * of them, which no desktop driver does in practice.
 */
VkFormat gvkFindDepthFormat(gVKContext& ctx);

/*
 * Creates the depth image, its memory and its view, sized to the current swapchain
 * extent. Called before the framebuffers, and again after a resize, because the
 * depth buffer has to match the colour attachment pixel for pixel.
 */
bool gvkCreateDepthResources(gVKContext& ctx);
void gvkDestroyDepthResources(gVKContext& ctx);

/*
 * Creates the single subpass render pass used by the backend. Depends on
 * ctx.swapchainformat, so the swapchain has to exist first. The pass carries a
 * colour and a depth attachment; the depth one is cleared on load and discarded on
 * store, since nothing reads it after the frame.
 */
bool gvkCreateRenderPass(gVKContext& ctx);
void gvkDestroyRenderPass(gVKContext& ctx);

/*
 * Creates one framebuffer per swapchain image view, each pairing that view with the
 * shared depth view. Kept separate from the render pass on purpose: a resize
 * rebuilds the framebuffers while the render pass stays valid, because neither the
 * surface format nor the depth format changes.
 */
bool gvkCreateFramebuffers(gVKContext& ctx);
void gvkDestroyFramebuffers(gVKContext& ctx);

#endif /* GVK_DESKTOP_GLFW */

#endif /* CORE_GVKRENDERTARGET_H */
