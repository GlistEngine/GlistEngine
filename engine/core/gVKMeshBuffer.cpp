/*
 * gVKMeshBuffer.cpp
 *
 * Static vertex and index buffers of the Vulkan backend.
 */

#include "gVKMeshBuffer.h"

#ifdef GVK_DESKTOP_GLFW

#include "gVKBuffer.h"
#include "gUtils.h"
#include <cstring>

bool gvkUploadMeshBuffer(gVKContext& ctx, gVKMeshBuffer& buf, const void* data,
		VkDeviceSize size, bool isIndex) {
	VkDevice device = *ctx.getDevice();
	if(device == VK_NULL_HANDLE) return false;

	// gVbo calls setIndexData(nullptr) to drop the index buffer of a mesh, which
	// arrives here as an empty upload.
	if(data == nullptr || size == 0) {
		gvkDestroyMeshBuffer(ctx, buf);
		return true;
	}

	// A buffer of the same size is refilled in place. A different size needs a new
	// allocation, and the old one may still be referenced by frames the GPU has not
	// finished, so the device is drained before it goes away. Resizes are rare - a
	// mesh normally uploads once - which is what makes the stall acceptable here.
	if(buf.buffer != VK_NULL_HANDLE && (buf.size != size || buf.isindex != isIndex)) {
		vkDeviceWaitIdle(device);
		gvkDestroyMeshBuffer(ctx, buf);
	}

	if(buf.buffer == VK_NULL_HANDLE) {
		VkBufferUsageFlags usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT |
				(isIndex ? VK_BUFFER_USAGE_INDEX_BUFFER_BIT : VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
		if(!gvkCreateBuffer(ctx, size, usage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
				buf.buffer, buf.memory)) {
			gLoge("gVKMeshBuffer") << "Could not create a " << (isIndex ? "index" : "vertex")
					<< " buffer of " << size << " bytes.";
			buf.buffer = VK_NULL_HANDLE;
			buf.memory = VK_NULL_HANDLE;
			buf.size = 0;
			return false;
		}
		buf.size = size;
		buf.isindex = isIndex;
	}

	// The destination is device local, so the CPU cannot write to it directly; the
	// data goes to a host visible staging buffer first and the GPU copies it over.
	VkBuffer staging = VK_NULL_HANDLE;
	VkDeviceMemory stagingmemory = VK_NULL_HANDLE;
	if(!gvkCreateBuffer(ctx, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			staging, stagingmemory)) {
		gLoge("gVKMeshBuffer") << "Could not create the staging buffer for a " << size
				<< " byte upload.";
		return false;
	}

	void* mapped = nullptr;
	if(vkMapMemory(device, stagingmemory, 0, size, 0, &mapped) != VK_SUCCESS) {
		gLoge("gVKMeshBuffer") << "vkMapMemory failed for the staging buffer.";
		vkDestroyBuffer(device, staging, nullptr);
		vkFreeMemory(device, stagingmemory, nullptr);
		return false;
	}
	std::memcpy(mapped, data, static_cast<size_t>(size));
	// Host coherent memory needs no explicit flush.
	vkUnmapMemory(device, stagingmemory);

	gvkCopyBuffer(ctx, staging, buf.buffer, size);

	// gvkCopyBuffer waits for the copy to finish, so the staging buffer is free to go.
	vkDestroyBuffer(device, staging, nullptr);
	vkFreeMemory(device, stagingmemory, nullptr);
	return true;
}

void gvkDestroyMeshBuffer(gVKContext& ctx, gVKMeshBuffer& buf) {
	VkDevice device = *ctx.getDevice();
	if(device == VK_NULL_HANDLE) return;

	if(buf.buffer != VK_NULL_HANDLE) {
		vkDestroyBuffer(device, buf.buffer, nullptr);
		buf.buffer = VK_NULL_HANDLE;
	}
	if(buf.memory != VK_NULL_HANDLE) {
		vkFreeMemory(device, buf.memory, nullptr);
		buf.memory = VK_NULL_HANDLE;
	}
	buf.size = 0;
}

#endif /* GVK_DESKTOP_GLFW */
