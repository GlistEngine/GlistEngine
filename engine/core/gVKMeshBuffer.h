/*
 * gVKMeshBuffer.h
 *
 * Static vertex and index buffers of the Vulkan backend, and the mapping that
 * makes them reachable through the GLuint handles the engine passes around.
 *
 * gVbo is written against OpenGL's model: createVAO() hands out a vertex array,
 * genBuffers() hands out buffer names, and the data lands through
 * setVertexBufferData / setIndexBufferData while a vertex array is bound. Vulkan
 * has no vertex array object and no notion of a buffer "name", so the backend keeps
 * that bookkeeping itself: every id maps to a real VkBuffer here, and every vertex
 * array remembers which two ids were bound to it, which is exactly what a draw call
 * needs in order to bind the right buffers.
 *
 * Kept apart from gVKDraw's per-frame ring on purpose. That ring is host visible
 * and rewritten every frame for 2D geometry; these buffers are device local, filled
 * once through a staging copy, and live as long as the mesh does.
 *
 * Created by: Veysel Burak Eroglu.
 */

#pragma once

#ifndef CORE_GVKMESHBUFFER_H
#define CORE_GVKMESHBUFFER_H

#include "gVKContext.h"

#ifdef GVK_DESKTOP_GLFW

/*
 * One device local buffer, either vertex or index data. size is the number of bytes
 * actually uploaded, which the draw path uses to sanity check a draw against the
 * data behind it.
 */
struct gVKMeshBuffer {
	VkBuffer buffer = VK_NULL_HANDLE;
	VkDeviceMemory memory = VK_NULL_HANDLE;
	VkDeviceSize size = 0;
	// Which usage flag the buffer was created with. A GLuint name carries no type in
	// OpenGL, the first upload decides it here.
	bool isindex = false;
};

/*
 * What a vertex array id stands for lives in gVKRenderEngine.h as gVKVertexArray:
 * it holds nothing but two GLuint ids, so it does not belong behind this header's
 * Vulkan guard.
 *
 * ---------------------------------------------------------------------------
 *
 * Uploads data into buf, creating or resizing its VkBuffer as needed. The copy goes
 * through a host visible staging buffer, so the destination can stay device local:
 * mesh data is written once and read by the GPU every frame, which is the case that
 * memory type is for.
 *
 * Returns false and leaves buf empty if the buffer could not be created. Passing
 * size 0 or a null pointer frees whatever the id held and returns true.
 */
bool gvkUploadMeshBuffer(gVKContext& ctx, gVKMeshBuffer& buf, const void* data,
		VkDeviceSize size, bool isIndex);

void gvkDestroyMeshBuffer(gVKContext& ctx, gVKMeshBuffer& buf);

#endif /* GVK_DESKTOP_GLFW */

#endif /* CORE_GVKMESHBUFFER_H */
