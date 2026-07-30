/*
 * gVKBuffer.h
 *
 * Small allocation helpers shared by the Vulkan draw path: creating buffers with
 * a matching memory type, and running one-off transfer commands (buffer copies,
 * image layout transitions) on a throwaway command buffer. Kept separate from the
 * frame loop so both the vertex ring (gVKDraw) and textures (gVKTexture) reuse it.
 */

#pragma once

#ifndef CORE_GVKBUFFER_H
#define CORE_GVKBUFFER_H

#include "gVKContext.h"

#ifdef GVK_DESKTOP_GLFW

// Picks a memory type index that is allowed by typeFilter and exposes all of the
// requested property flags (e.g. host visible + coherent, or device local).
uint32_t gvkFindMemoryType(gVKContext& ctx, uint32_t typeFilter, VkMemoryPropertyFlags properties);

// Creates a buffer and backs it with freshly allocated memory. Returns false and
// leaves the handles at VK_NULL_HANDLE on failure.
bool gvkCreateBuffer(gVKContext& ctx, VkDeviceSize size, VkBufferUsageFlags usage,
		VkMemoryPropertyFlags properties, VkBuffer& outBuffer, VkDeviceMemory& outMemory);

// Allocates and begins a single-use command buffer from the context's pool. Pair
// every call with gvkEndSingleTimeCommands, which submits it and waits for the GPU.
VkCommandBuffer gvkBeginSingleTimeCommands(gVKContext& ctx);
void gvkEndSingleTimeCommands(gVKContext& ctx, VkCommandBuffer commandBuffer);

void gvkCopyBuffer(gVKContext& ctx, VkBuffer src, VkBuffer dst, VkDeviceSize size);

#endif /* GVK_DESKTOP_GLFW */

#endif /* CORE_GVKBUFFER_H */
