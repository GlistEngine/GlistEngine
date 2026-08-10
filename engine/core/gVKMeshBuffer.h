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

#include <cstdint>
#include <utility>
#include <vector>

/*
 * One buffer, either vertex or index data. size is the number of bytes actually
 * uploaded, which the draw path uses to sanity check a draw against the data
 * behind it.
 *
 * There are two shapes here, and which one a buffer takes is decided by how it is
 * used rather than by anything the caller says:
 *
 *  - Uploaded once: a device local buffer filled through a staging copy. This is
 *    the overwhelming majority - a mesh loads and is then drawn unchanged - and
 *    device local is the right memory for data the GPU reads every frame.
 *
 *  - Uploaded again: the buffer is rebuilt as one host visible copy per frame in
 *    flight, and every later upload is a plain memcpy into the copy belonging to
 *    the frame being recorded. This is what a CPU-skinned mesh does; it rewrites
 *    its vertices on every animation frame, and the staging path made that cost a
 *    full pipeline stall each time.
 *
 * The per-frame copies are what makes writing safe without any stall: the frame
 * loop waits on frame f's fence before recording frame f, so by the time the CPU
 * writes slot f, every submission that read slot f has finished.
 */
struct gVKMeshBuffer {
	// The buffer a draw should bind. For a static buffer this is the only one; for
	// a dynamic one gvkResolveMeshBuffer keeps it pointed at the current frame's
	// slot, so every call site can go on reading this field.
	VkBuffer buffer = VK_NULL_HANDLE;
	VkDeviceMemory memory = VK_NULL_HANDLE;
	VkDeviceSize size = 0;
	// Which usage flag the buffer was created with. A GLuint name carries no type in
	// OpenGL, the first upload decides it here.
	bool isindex = false;

	// Everything below is unused until a second upload promotes the buffer.
	bool isdynamic = false;
	VkBuffer slotbuffers[GVK_MAX_FRAMES_IN_FLIGHT] = {};
	VkDeviceMemory slotmemories[GVK_MAX_FRAMES_IN_FLIGHT] = {};
	void* slotmapped[GVK_MAX_FRAMES_IN_FLIGHT] = {};
	// Which version of the data each slot currently holds, against the newest
	// version in shadow. A frame that draws the mesh without a fresh upload finds
	// its slot behind and refreshes it, so no frame ever draws stale vertices.
	uint64_t slotversion[GVK_MAX_FRAMES_IN_FLIGHT] = {};
	uint64_t version = 0;
	std::vector<unsigned char> shadow;

	// Where this mesh's newest data sits inside the per-frame arena, and which
	// frame that slice belongs to. A mesh drawn without a fresh upload keeps
	// using its slice for the rest of that frame; once the arena has been rewound
	// the generation no longer matches and the data is pushed again.
	VkBuffer arenabuffer = VK_NULL_HANDLE;
	VkDeviceSize arenaoffset = 0;
	uint64_t arenageneration = 0;
	// The frame of this buffer's last upload. A second upload inside the same
	// frame is what makes an arena slice necessary; a mesh uploaded once a frame
	// is served by its own per-frame copy, which costs nothing to keep.
	uint64_t lastuploadgeneration = ~0ull;
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

/*
 * The VkBuffer a draw recorded right now should bind, and the only supported way
 * to get one: a dynamic buffer has a copy per frame in flight, and this picks the
 * one belonging to the frame being recorded, refreshing it from the newest data
 * first if that frame has not seen the latest upload.
 *
 * outOffset is the byte offset to bind the vertex buffer from - zero for a
 * static mesh, and the mesh's slice inside the arena for a dynamic one.
 *
 * A static buffer is returned as it is. Returns VK_NULL_HANDLE if there is nothing
 * to draw from, which callers already check for.
 */
VkBuffer gvkResolveMeshBuffer(gVKContext& ctx, gVKMeshBuffer& buf, VkDeviceSize& outOffset);

/*
 * Frees the device local buffers that promotions replaced. They are held rather
 * than destroyed at promotion time because the frame being recorded may already
 * reference them; call this once at shutdown, after the device is idle.
 */
void gvkDestroyRetiredMeshBuffers(gVKContext& ctx);

void gvkDestroyMeshBuffer(gVKContext& ctx, gVKMeshBuffer& buf);

#endif /* GVK_DESKTOP_GLFW */

#endif /* CORE_GVKMESHBUFFER_H */
