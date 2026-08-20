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

#ifdef GVK_VULKAN

/*
 * Picks a depth format the physical device supports as a depth-stencil attachment,
 * preferring the cheapest one that is enough: D32_SFLOAT, then D32_SFLOAT_S8_UINT,
 * then D24_UNORM_S8_UINT. Returns VK_FORMAT_UNDEFINED if the device supports none
 * of them, which no desktop driver does in practice.
 */
VkFormat gvkFindDepthFormat(gVKContext& ctx);

/*
 * Creates one depth image per frame in flight, sized to the current swapchain
 * extent. Private frame slots avoid overlapping writes without allocating one
 * depth image for every swapchain colour image. The images carry the context's
 * sample count: every attachment of a subpass has to agree on it, so switching the
 * screen pass to MSAA multisamples depth as well.
 */
bool gvkCreateDepthResources(gVKContext& ctx);
void gvkDestroyDepthResources(gVKContext& ctx);

/*
 * Creates the multisampled colour attachment the screen pass renders into while
 * MSAA is on, one per frame in flight alongside the depth images. A no-op that
 * simply reports success when the context's sample count is 1, where the swapchain
 * image is rendered into directly. Must be called after gvkCreateRenderPass, which
 * is where the sample count is resolved, and before gvkCreateFramebuffers.
 */
bool gvkCreateMsaaColorResources(gVKContext& ctx);
void gvkDestroyMsaaColorResources(gVKContext& ctx);

/*
 * Creates the single subpass render pass used by the backend. Depends on
 * ctx.swapchainformat, so the swapchain has to exist first. The pass carries a
 * colour and a depth attachment; the depth one is cleared on load and discarded on
 * store, since nothing reads it after the frame.
 *
 * This is also where the requested sample count is resolved against what the device
 * supports and written to the context. With MSAA on, the pass gains a third
 * attachment - the acquired swapchain image as the resolve target - and builds a
 * second, single-sampled pass as the compatibility template the offscreen pipelines
 * are built against. See gVKContext.h for why the two exist.
 */
bool gvkCreateRenderPass(gVKContext& ctx);
void gvkDestroyRenderPass(gVKContext& ctx);

/*
 * Creates one framebuffer per frame-slot/swapchain-image pair, pairing any acquired
 * colour image with that frame slot's private depth view. A resize
 * rebuilds the framebuffers while the render pass stays valid, because neither the
 * surface format nor the depth format changes.
 */
bool gvkCreateFramebuffers(gVKContext& ctx);
void gvkDestroyFramebuffers(gVKContext& ctx);

#endif /* GVK_VULKAN */

#endif /* CORE_GVKRENDERTARGET_H */
