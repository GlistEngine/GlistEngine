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

// Buffers replaced by a promotion, kept alive rather than destroyed on the spot.
//
// A promotion happens in the middle of a draw - the second upload arrives while
// the frame that will use it is being recorded - and by then that command buffer
// may already hold a bind of the buffer being replaced. vkDeviceWaitIdle does not
// help: it waits for submitted work, and this command buffer has not been
// submitted yet, so destroying the buffer here leaves a recorded reference to
// freed memory. That wedged the device hard enough that every later
// vkAcquireNextImageKHR failed and the window stopped updating altogether.
//
// One buffer per mesh that ever turns dynamic is a bounded, small amount to hold
// until shutdown, and it is the version of this that is obviously correct.
static std::vector<std::pair<VkBuffer, VkDeviceMemory>> gvkretiredmeshbuffers;

void gvkDestroyRetiredMeshBuffers(gVKContext& ctx) {
	VkDevice device = *ctx.getDevice();
	if(device == VK_NULL_HANDLE) {
		gvkretiredmeshbuffers.clear();
		return;
	}
	for(auto& retired : gvkretiredmeshbuffers) {
		if(retired.first != VK_NULL_HANDLE) vkDestroyBuffer(device, retired.first, nullptr);
		if(retired.second != VK_NULL_HANDLE) vkFreeMemory(device, retired.second, nullptr);
	}
	gvkretiredmeshbuffers.clear();
}

// Rebuilds a buffer as one host visible copy per frame in flight and leaves them
// permanently mapped. Called the moment a buffer is uploaded to a second time,
// which is what marks it as data the CPU intends to keep rewriting.
static bool gvkMakeMeshBufferDynamic(gVKContext& ctx, gVKMeshBuffer& buf,
		VkDeviceSize size, bool isIndex) {
	VkDevice device = *ctx.getDevice();

	// Set aside rather than freed; see gvkretiredmeshbuffers above for why.
	if(!buf.isdynamic && buf.buffer != VK_NULL_HANDLE) {
		gvkretiredmeshbuffers.emplace_back(buf.buffer, buf.memory);
		buf.buffer = VK_NULL_HANDLE;
		buf.memory = VK_NULL_HANDLE;
	}
	gvkDestroyMeshBuffer(ctx, buf);

	const VkBufferUsageFlags usage = isIndex ? VK_BUFFER_USAGE_INDEX_BUFFER_BIT
			: VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	for(int i = 0; i < GVK_MAX_FRAMES_IN_FLIGHT; i++) {
		if(!gvkCreateBuffer(ctx, size, usage,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				buf.slotbuffers[i], buf.slotmemories[i])) {
			gLoge("gVKMeshBuffer") << "Could not create a host visible " << size
					<< " byte buffer for a mesh that updates every frame.";
			gvkDestroyMeshBuffer(ctx, buf);
			return false;
		}
		if(vkMapMemory(device, buf.slotmemories[i], 0, size, 0, &buf.slotmapped[i]) != VK_SUCCESS) {
			gLoge("gVKMeshBuffer") << "vkMapMemory failed for a per-frame mesh buffer.";
			gvkDestroyMeshBuffer(ctx, buf);
			return false;
		}
		// Nothing written yet, so no slot holds any version of the data.
		buf.slotversion[i] = 0;
	}

	buf.isdynamic = true;
	buf.size = size;
	buf.isindex = isIndex;
	buf.version = 0;
	buf.shadow.resize(static_cast<size_t>(size));
	return true;
}

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

	const bool wasallocated = buf.isdynamic || buf.buffer != VK_NULL_HANDLE;
	const bool shapechanged = wasallocated && (buf.size != size || buf.isindex != isIndex);

	// The second upload is the signal that this mesh rewrites its own vertices -
	// CPU skinning does it on every animation frame. Staging such a buffer means a
	// device local copy per update, and because the copy is submitted to the
	// graphics queue and waited on, each one drains everything already queued: the
	// frame being recorded ends up serialised against the one before it. Moving the
	// buffer to host visible memory removes the copy, the submit and the wait, and
	// leaves an upload costing one memcpy.
	if(!buf.isdynamic && wasallocated && !shapechanged) {
		if(!gvkMakeMeshBufferDynamic(ctx, buf, size, isIndex)) return false;
	}

	if(buf.isdynamic) {
		if(shapechanged) {
			// A resize cannot reuse the mapped slots. Rebuilding them keeps the
			// buffer dynamic, which is what it has already shown itself to be.
			if(!gvkMakeMeshBufferDynamic(ctx, buf, size, isIndex)) return false;
		}
		std::memcpy(buf.shadow.data(), data, static_cast<size_t>(size));
		buf.version++;

		// A slice of its own for this upload. The mesh may be uploaded to and drawn
		// several times in one frame - a game posing a single soldier model once per
		// enemy does exactly that - and every one of those draws has to read the data
		// it was recorded with. One buffer per mesh cannot do that: the draws execute
		// after the frame is submitted, so they would all read whatever was written
		// last and every soldier would strike the same pose.
		// Vertices only. The arena is created as a vertex buffer, and index data has
		// no reason to be there anyway: what makes a slice per draw necessary is the
		// same mesh being re-posed between draws, and re-posing rewrites positions,
		// never the indices that join them.
		const VkDeviceSize arenaoffset = buf.isindex
				? VK_WHOLE_SIZE : ctx.pushMeshData(data, size);
		if(arenaoffset != VK_WHOLE_SIZE) {
			buf.arenabuffer = ctx.getCurrentMeshArena();
			buf.arenaoffset = arenaoffset;
			buf.arenageneration = ctx.getMeshGeneration();
			buf.buffer = buf.arenabuffer;
			return true;
		}

		// The arena is full for this frame. It grows at the next frame start, so this
		// is the one frame where the mesh's own per-frame buffer stands in - correct
		// against anything drawn in another frame, and only wrong if the same mesh is
		// posed more than once inside this one.
		const uint32_t slot = ctx.getCurrentFrame() % GVK_MAX_FRAMES_IN_FLIGHT;
		std::memcpy(buf.slotmapped[slot], data, static_cast<size_t>(size));
		buf.slotversion[slot] = buf.version;
		buf.arenabuffer = VK_NULL_HANDLE;
		buf.buffer = buf.slotbuffers[slot];
		return true;
	}

	// A different size needs a new allocation, and the old one may still be
	// referenced by frames the GPU has not finished, so the device is drained
	// before it goes away.
	if(shapechanged) {
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

VkBuffer gvkResolveMeshBuffer(gVKContext& ctx, gVKMeshBuffer& buf, VkDeviceSize& outOffset) {
	outOffset = 0;
	if(!buf.isdynamic) return buf.buffer;

	// The slice this mesh was last uploaded into, as long as it belongs to the frame
	// being recorded. Two enemies posed to the same animation frame skip the second
	// upload entirely, and this is what lets the second draw reuse the first's data
	// rather than push an identical copy.
	if(!buf.isindex && buf.arenabuffer != VK_NULL_HANDLE && buf.arenageneration == ctx.getMeshGeneration()) {
		outOffset = buf.arenaoffset;
		buf.buffer = buf.arenabuffer;
		return buf.arenabuffer;
	}

	// A new frame has rewound the arena, so the slice is gone. Push the newest data
	// back in and carry on; falling through to the per-frame buffer below would work
	// too, but this keeps every draw of the frame reading from the same place.
	if(!buf.isindex && !buf.shadow.empty()) {
		const VkDeviceSize offset = ctx.pushMeshData(buf.shadow.data(), buf.shadow.size());
		if(offset != VK_WHOLE_SIZE) {
			buf.arenabuffer = ctx.getCurrentMeshArena();
			buf.arenaoffset = offset;
			buf.arenageneration = ctx.getMeshGeneration();
			buf.buffer = buf.arenabuffer;
			outOffset = offset;
			return buf.arenabuffer;
		}
	}

	const uint32_t slot = ctx.getCurrentFrame() % GVK_MAX_FRAMES_IN_FLIGHT;
	if(buf.slotmapped[slot] == nullptr) return VK_NULL_HANDLE;

	// This frame's slot is behind whenever the mesh was uploaded while a different
	// frame was being recorded - a skinned mesh holding the same animation frame
	// across two renders does exactly that. Catching it up costs one memcpy and is
	// what keeps the two slots from showing alternating poses.
	if(buf.slotversion[slot] != buf.version && !buf.shadow.empty()) {
		std::memcpy(buf.slotmapped[slot], buf.shadow.data(), buf.shadow.size());
		buf.slotversion[slot] = buf.version;
	}

	buf.buffer = buf.slotbuffers[slot];
	return buf.buffer;
}

void gvkDestroyMeshBuffer(gVKContext& ctx, gVKMeshBuffer& buf) {
	VkDevice device = *ctx.getDevice();
	if(device == VK_NULL_HANDLE) return;

	if(buf.buffer != VK_NULL_HANDLE && !buf.isdynamic) {
		vkDestroyBuffer(device, buf.buffer, nullptr);
	}
	buf.buffer = VK_NULL_HANDLE;
	if(buf.memory != VK_NULL_HANDLE) {
		vkFreeMemory(device, buf.memory, nullptr);
		buf.memory = VK_NULL_HANDLE;
	}

	// The mapped slots are freed rather than unmapped first: freeing the memory
	// they belong to unmaps them, and the pointers go away with it.
	for(int i = 0; i < GVK_MAX_FRAMES_IN_FLIGHT; i++) {
		if(buf.slotbuffers[i] != VK_NULL_HANDLE) {
			vkDestroyBuffer(device, buf.slotbuffers[i], nullptr);
			buf.slotbuffers[i] = VK_NULL_HANDLE;
		}
		if(buf.slotmemories[i] != VK_NULL_HANDLE) {
			vkFreeMemory(device, buf.slotmemories[i], nullptr);
			buf.slotmemories[i] = VK_NULL_HANDLE;
		}
		buf.slotmapped[i] = nullptr;
		buf.slotversion[i] = 0;
	}
	buf.isdynamic = false;
	buf.version = 0;
	buf.shadow.clear();
	buf.size = 0;
}

#endif /* GVK_DESKTOP_GLFW */
