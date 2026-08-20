/*
 * gVKMeshBuffer.cpp
 *
 * Static vertex and index buffers of the Vulkan backend.
 */

#include "gVKMeshBuffer.h"

#ifdef GVK_VULKAN

#include "gVKBuffer.h"
#include "gUtils.h"
#include <algorithm>
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
// Each entry remembers the frame it was retired in, and is freed once enough
// frames have passed that no command buffer can still reference it. Holding them
// all until shutdown was the first version of this, on the reasoning that one
// buffer per mesh is a small bounded amount - which is true only while meshes are
// bounded. Measured on game_martyr the list reached two thousand entries in a
// minute of play, each holding a VkBuffer and its VkDeviceMemory, because the game
// creates meshes it never destroys and every one of them promotes.
struct gVKRetiredBuffer {
	VkBuffer buffer = VK_NULL_HANDLE;
	VkDeviceMemory memory = VK_NULL_HANDLE;
	uint64_t frame = 0;
};
static std::vector<gVKRetiredBuffer> gvkretiredmeshbuffers;

// A buffer retired while frame N was being recorded can still be read by frame N
// and by the frames already in flight behind it. Once the frame counter has moved
// on by more than the number of frames in flight, every submission that could name
// it has finished - the frame loop waits on that slot's fence before reusing it.
void gvkCollectRetiredMeshBuffers(gVKContext& ctx) {
	if(gvkretiredmeshbuffers.empty()) return;
	VkDevice device = *ctx.getDevice();
	if(device == VK_NULL_HANDLE) return;
	const uint64_t now = ctx.getMeshGeneration();
	const uint64_t keepfor = GVK_MAX_FRAMES_IN_FLIGHT + 1;
	size_t kept = 0;
	for(size_t i = 0; i < gvkretiredmeshbuffers.size(); i++) {
		gVKRetiredBuffer& entry = gvkretiredmeshbuffers[i];
		if(now < entry.frame + keepfor) {
			gvkretiredmeshbuffers[kept++] = entry;
			continue;
		}
		if(entry.buffer != VK_NULL_HANDLE) vkDestroyBuffer(device, entry.buffer, nullptr);
		if(entry.memory != VK_NULL_HANDLE) vkFreeMemory(device, entry.memory, nullptr);
	}
	gvkretiredmeshbuffers.resize(kept);
}

void gvkDestroyRetiredMeshBuffers(gVKContext& ctx) {
	VkDevice device = *ctx.getDevice();
	if(device == VK_NULL_HANDLE) {
		gvkretiredmeshbuffers.clear();
		return;
	}
	for(auto& retired : gvkretiredmeshbuffers) {
		if(retired.buffer != VK_NULL_HANDLE) vkDestroyBuffer(device, retired.buffer, nullptr);
		if(retired.memory != VK_NULL_HANDLE) vkFreeMemory(device, retired.memory, nullptr);
	}
	gvkretiredmeshbuffers.clear();
}

// Rebuilds a buffer as one host visible copy per frame in flight and leaves them
// permanently mapped. Called the moment a buffer is uploaded to a second time,
// which is what marks it as data the CPU intends to keep rewriting.
static bool gvkMakeMeshBufferDynamic(gVKContext& ctx, gVKMeshBuffer& buf,
		VkDeviceSize size, bool isIndex) {
	VkDevice device = *ctx.getDevice();

	// Set aside rather than freed; see gvkretiredmeshbuffers above for why. The same
	// applies to slots being replaced by larger ones: the frame being recorded may
	// already have bound them.
	if(buf.buffer != VK_NULL_HANDLE && !buf.isdynamic) {
		// The retired list counts frames, which is the right measure for a command
		// buffer that still names this buffer - but a staging copy into it may not
		// have been submitted at all yet, and no number of frames makes unsubmitted
		// work safe. Drain the upload path once, here, where a static buffer stops
		// being one.
		gvkWaitUploads(ctx);
		gvkretiredmeshbuffers.push_back({buf.buffer, buf.memory, ctx.getMeshGeneration()});
		buf.buffer = VK_NULL_HANDLE;
		buf.memory = VK_NULL_HANDLE;
	}
	for(int i = 0; i < GVK_MAX_FRAMES_IN_FLIGHT; i++) {
		if(buf.slotbuffers[i] == VK_NULL_HANDLE) continue;
		if(buf.slotmapped[i] != nullptr) vkUnmapMemory(device, buf.slotmemories[i]);
		gvkretiredmeshbuffers.push_back({buf.slotbuffers[i], buf.slotmemories[i],
				ctx.getMeshGeneration()});
		buf.slotbuffers[i] = VK_NULL_HANDLE;
		buf.slotmemories[i] = VK_NULL_HANDLE;
		buf.slotmapped[i] = nullptr;
	}
	gvkDestroyMeshBuffer(ctx, buf);

	// Half again as much as asked for, so a mesh whose size moves a little between
	// draws settles after one allocation instead of reallocating on each. Without it
	// a gRectangle alternating between filled and outlined - four indices then five -
	// rebuilt both its buffers twice a frame, and every rebuild drained the device.
	const VkDeviceSize capacity = std::max<VkDeviceSize>(size + size / 2, 256);
	const VkBufferUsageFlags usage = isIndex ? VK_BUFFER_USAGE_INDEX_BUFFER_BIT
			: VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	for(int i = 0; i < GVK_MAX_FRAMES_IN_FLIGHT; i++) {
		if(!gvkCreateBuffer(ctx, capacity, usage,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				buf.slotbuffers[i], buf.slotmemories[i],
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
						| VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) {
			gLoge("gVKMeshBuffer") << "Could not create a host visible " << capacity
					<< " byte buffer for a mesh that updates every frame.";
			gvkDestroyMeshBuffer(ctx, buf);
			return false;
		}
		if(vkMapMemory(device, buf.slotmemories[i], 0, capacity, 0, &buf.slotmapped[i]) != VK_SUCCESS) {
			gLoge("gVKMeshBuffer") << "vkMapMemory failed for a per-frame mesh buffer.";
			gvkDestroyMeshBuffer(ctx, buf);
			return false;
		}
		// Nothing written yet, so no slot holds any version of the data.
		buf.slotversion[i] = 0;
	}

	buf.isdynamic = true;
	buf.size = size;
	buf.capacity = capacity;
	buf.isindex = isIndex;
	buf.version = 0;
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
	const bool rolechanged = wasallocated && buf.isindex != isIndex;
	const bool shapechanged = wasallocated && (buf.size != size || buf.isindex != isIndex);

	// The second upload is the signal that this mesh rewrites its own vertices -
	// CPU skinning does it on every animation frame. Staging such a buffer means a
	// device local copy per update, and because the copy is submitted to the
	// graphics queue and waited on, each one drains everything already queued: the
	// frame being recorded ends up serialised against the one before it. Moving the
	// buffer to host visible memory removes the copy, the submit and the wait, and
	// leaves an upload costing one memcpy.
	//
	// A second upload of a different size counts just as much. It used to be excluded
	// - a resize looked like a different mesh rather than the same one changing - and
	// so it fell through to the staging path below, which drains the device on every
	// call. gRectangle is one buffer drawn filled and then outlined, four indices then
	// five, and that alone cost about a millisecond a frame in game_martyr's HUD. It
	// also destroyed buffers the command buffer being recorded still referenced.
	if(!buf.isdynamic && wasallocated && !rolechanged) {
		if(!gvkMakeMeshBufferDynamic(ctx, buf, size, isIndex)) return false;
	}

	if(buf.isdynamic) {
		if(rolechanged || size > buf.capacity) {
			// Only a mesh outgrowing its slots needs new ones; anything that still
			// fits is written in place and simply records its new length.
			if(!gvkMakeMeshBufferDynamic(ctx, buf, size, isIndex)) return false;
		}
		buf.size = size;
		buf.version++;

		const uint64_t generation = ctx.getMeshGeneration();
		// Uploads that arrive while no frame is being recorded all share one
		// generation, so the second and later ones look like re-poses inside a single
		// frame and would take an arena slice. The arena is rewound before the first
		// real frame, and the slice takes the newest data with it: a mesh built in
		// setup() through more than two uploads - gCylinder writing its positions and
		// the canvas then filling in normals and texture coordinates - ended up drawing
		// whichever version happened to land in a slot, which is why the spinning
		// cylinder and cone came out untextured while everything else was fine.
		const bool inframe = ctx.isFrameActive();
		const bool againthisframe = inframe && buf.lastuploadgeneration == generation;
		buf.lastuploadgeneration = inframe ? generation : ~0ull;

		// Only a second upload inside one frame needs an arena slice, and that is the
		// case worth spending memory on: the mesh is being re-posed between draws - a
		// game posing a single soldier model once per enemy - and each of those draws
		// has to read what it was recorded with, because they all execute after the
		// frame is submitted.
		//
		// The first upload of a frame goes to the buffer's own per-frame copy instead.
		// That copy is what a later draw finds when the mesh is not uploaded again,
		// and keeping it current here is what stops the resolve below from having to
		// copy the whole mesh back out of another slot. Doing it the other way round -
		// arena first, always - meant re-pushing every dynamic mesh that held still
		// for a frame, which measured 18 to 22 MB of memcpy per frame in a scene with
		// a hundred of them, and was the single largest cost in the whole draw path.
		//
		// Index data never takes a slice: what makes one necessary is re-posing, and
		// that rewrites positions rather than the indices joining them.
		if(againthisframe && !buf.isindex) {
			const VkDeviceSize arenaoffset = ctx.pushMeshData(data, size);
			if(arenaoffset != VK_WHOLE_SIZE) {
				buf.arenabuffer = ctx.getCurrentMeshArena();
				buf.arenaoffset = arenaoffset;
				buf.arenageneration = generation;
				buf.buffer = buf.arenabuffer;
				return true;
			}
			// The arena is full; it grows at the next frame start. Falling through to
			// the per-frame copy is correct for everything except this mesh's earlier
			// draw in this frame, which will show this pose instead of its own.
		}

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
	// The upload path owns that staging buffer and the copy shares a batch with
	// every other transfer around it, so a level of static meshes costs one submit
	// per batch rather than one submit and one host wait per mesh.
	return gvkUploadBufferData(ctx, buf.buffer, data, size);
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

	// No slice for this frame, so the mesh's own per-frame copy is what it draws
	// from. Pushing the data into the arena again would work as well and used to be
	// what happened here, but it copies the whole mesh for a mesh that has not
	// changed at all - and a scene full of animations that hold still for a frame
	// pays that on nearly every draw.
	const uint32_t slot = ctx.getCurrentFrame() % GVK_MAX_FRAMES_IN_FLIGHT;
	if(buf.slotmapped[slot] == nullptr) return VK_NULL_HANDLE;

	// This frame's slot is behind whenever the mesh was last uploaded while a
	// different frame was being recorded - a mesh that animated for a while and then
	// held still does exactly that. The draw binds whichever slot holds the newest
	// data instead of bringing this one up to date.
	//
	// Binding another frame's slot is safe. The reason each frame has its own copy is
	// that the CPU writes one while the GPU may still be reading the other, and that
	// is a write-versus-read hazard; two draws reading the same buffer is not a hazard
	// at all. The slot the CPU writes is still only ever the current frame's, and the
	// frame loop has waited on that frame's fence before any of this runs.
	//
	// Copying instead - which is what this did - is what makes it expensive, and the
	// cost is not the copy itself but where it reads from. These slots live in memory
	// that is device local as well as host visible, so the GPU reads vertices without
	// crossing the bus; such memory is write combined, and reading it back is roughly
	// 25 MB/s. On game_martyr that turned 400 KB of catch-up per frame into 20 ms of
	// frame time, and only in the parts of the map where meshes skip frames - which is
	// why it looked like the backend randomly getting slower rather than a copy.
	uint32_t freshest = slot;
	for(uint32_t i = 0; i < GVK_MAX_FRAMES_IN_FLIGHT; i++) {
		if(buf.slotmapped[i] != nullptr && buf.slotversion[i] > buf.slotversion[freshest]) freshest = i;
	}

	buf.buffer = buf.slotbuffers[freshest];
	return buf.buffer;
}

void gvkDestroyMeshBuffer(gVKContext& ctx, gVKMeshBuffer& buf) {
	VkDevice device = *ctx.getDevice();
	if(device == VK_NULL_HANDLE) return;

	if(buf.buffer != VK_NULL_HANDLE && !buf.isdynamic) {
		// A static buffer is the destination of a staging copy that may be recorded
		// and not yet submitted, which vkDeviceWaitIdle - what the callers that need
		// it already do - cannot cover, because that work has not reached the queue.
		// Only this branch can name such a buffer, so nothing else here has to wait.
		gvkWaitUploads(ctx);
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
	buf.size = 0;
	buf.capacity = 0;
}

#endif /* GVK_VULKAN */
