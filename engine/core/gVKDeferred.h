/*
 * gVKDeferred.h
 *
 * Internal Vulkan G-buffer resources used by gDeferredLight. The public renderer
 * API stays backend-neutral; this module owns the Vulkan images, render pass and
 * per-frame framebuffers behind that API.
 */

#pragma once

#ifndef CORE_GVKDEFERRED_H
#define CORE_GVKDEFERRED_H

#include "gVKContext.h"

#ifdef GVK_VULKAN

struct gVKDeferredResources;

// Creates a single-sample position/normal/albedo/depth G-buffer with one private
// attachment set per frame in flight. Failure destroys every partially-created
// object and returns null.
gVKDeferredResources* gvkCreateDeferredResources(gVKContext& ctx,
		uint32_t width, uint32_t height);
void gvkDestroyDeferredResources(gVKContext& ctx, gVKDeferredResources*& resources);

// Allocates one three-sampler set per frame slot after the lighting pipeline has
// supplied its reflected set-1 layout. These sets are recreated when the shared
// descriptor pool is rebuilt by shader reload or an MSAA change.
bool gvkCreateDeferredDescriptors(gVKContext& ctx, gVKDeferredResources& resources,
		VkDescriptorSetLayout layout);
void gvkReleaseDeferredDescriptors(gVKContext& ctx, gVKDeferredResources& resources,
		bool freeSets = true);
VkDescriptorSet gvkGetDeferredDescriptorSet(const gVKDeferredResources& resources,
		uint32_t frame);

// The geometry pass may interrupt the lazily-opened screen pass. Ending it leaves
// the colour attachments shader-readable for the lighting pass implemented next.
bool gvkBeginDeferredPass(gVKContext& ctx, gVKDeferredResources& resources);
void gvkEndDeferredPass(gVKContext& ctx);

// Ends the fullscreen lighting pass and copies the G-buffer depth into the
// single-sampled default target. The next ordinary draw reopens the screen pass
// with LOAD operations, so forward 3D geometry keeps the deferred scene's
// occlusion just like the OpenGL depth blit.
bool gvkRestoreDeferredDepth(gVKContext& ctx, gVKDeferredResources& resources);

// Narrow internal accessors for the pipeline and lighting stages. Keeping the
// resource layout opaque prevents those stages from taking ownership of handles.
VkRenderPass gvkGetDeferredRenderPass(const gVKDeferredResources& resources);
VkImageView gvkGetDeferredColorView(const gVKDeferredResources& resources,
		uint32_t frame, uint32_t attachment);
VkExtent2D gvkGetDeferredExtent(const gVKDeferredResources& resources);

#endif /* GVK_VULKAN */

#endif /* CORE_GVKDEFERRED_H */
