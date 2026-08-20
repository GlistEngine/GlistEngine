/*
 * gVKBuffer.h
 *
 * Small allocation helpers shared by the Vulkan draw path: creating buffers with
 * a matching memory type, and the batched upload path that carries transfer work
 * (buffer copies, image layout transitions) to the GPU. Kept separate from the
 * frame loop so both the vertex ring (gVKDraw) and textures (gVKTexture) reuse it.
 */

#pragma once

#ifndef CORE_GVKBUFFER_H
#define CORE_GVKBUFFER_H

#include "gVKContext.h"

#ifdef GVK_VULKAN

// Picks a memory type index that is allowed by typeFilter and exposes all of the
// requested property flags (e.g. host visible + coherent, or device local).
uint32_t gvkFindMemoryType(gVKContext& ctx, uint32_t typeFilter, VkMemoryPropertyFlags properties);

// Creates a buffer and backs it with freshly allocated memory. Returns false and
// leaves the handles at VK_NULL_HANDLE on failure.
//
// preferred names memory the caller would rather have; when no memory type offers
// it the allocation falls back to properties, which is what it must have. It exists
// for buffers the CPU writes and the GPU reads every frame: on a device that
// exposes device local memory as host visible those belong there rather than in
// system RAM, which the GPU can only reach across the bus.
bool gvkCreateBuffer(gVKContext& ctx, VkDeviceSize size, VkBufferUsageFlags usage,
		VkMemoryPropertyFlags properties, VkBuffer& outBuffer, VkDeviceMemory& outMemory,
		VkMemoryPropertyFlags preferred = 0);

// Transfer work - staging copies into device local buffers and images, and the
// layout transitions around them - is recorded into a shared batch rather than
// submitted on its own. gvkBeginUpload returns a command buffer that is already
// recording; record into it and hand it back with gvkEndUpload, naming the
// staging buffer the recorded commands read. The batch owns that staging buffer
// from then on and destroys it once the submission it went into has finished, so
// a caller must never destroy it itself.
//
// Returns VK_NULL_HANDLE when no batch could be opened, in which case nothing was
// recorded and gvkEndUpload must not be called.
VkCommandBuffer gvkBeginUpload(gVKContext& ctx);
void gvkEndUpload(gVKContext& ctx, VkBuffer stagingBuffer = VK_NULL_HANDLE,
		VkDeviceMemory stagingMemory = VK_NULL_HANDLE, VkDeviceSize stagingSize = 0);

// Copies size bytes into a device local buffer through staging the upload path
// owns, and leaves a barrier making the result visible to vertex and index reads.
bool gvkUploadBufferData(gVKContext& ctx, VkBuffer dst, const void* data, VkDeviceSize size);

// Submits whatever is recorded. The frame loop calls this before submitting the
// frame, which is what makes uploads recorded during that frame visible to it:
// barriers apply to everything submitted later on the same queue, so no host wait
// is involved.
void gvkFlushUploads(gVKContext& ctx);

// Frees the staging of batches the GPU has finished with. Cheap, and meant to run
// once per frame.
void gvkCollectUploads(gVKContext& ctx);

// Submits and waits for every outstanding upload. Needed before destroying
// anything a recorded-but-unsubmitted transfer names, which vkDeviceWaitIdle
// cannot cover because that work has not reached the queue yet.
void gvkWaitUploads(gVKContext& ctx);

void gvkDestroyUploadContext(gVKContext& ctx);

#endif /* GVK_VULKAN */

#endif /* CORE_GVKBUFFER_H */
