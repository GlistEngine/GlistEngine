/*
 * gVKBuffer.cpp
 */

#include "gVKBuffer.h"

#ifdef GVK_VULKAN

#include "gUtils.h"
#include <cstring>
#include <vector>

// Returns UINT32_MAX rather than logging when nothing matches, so a caller can ask
// for memory it would like and fall back to memory it needs.
static uint32_t gvkTryFindMemoryType(gVKContext& ctx, uint32_t typeFilter,
		VkMemoryPropertyFlags properties) {
	VkPhysicalDeviceMemoryProperties* memprops = ctx.getDeviceMemoryProperties();
	for(uint32_t i = 0; i < memprops->memoryTypeCount; i++) {
		if((typeFilter & (1u << i)) &&
				(memprops->memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}
	return UINT32_MAX;
}

uint32_t gvkFindMemoryType(gVKContext& ctx, uint32_t typeFilter, VkMemoryPropertyFlags properties) {
	const uint32_t index = gvkTryFindMemoryType(ctx, typeFilter, properties);
	if(index != UINT32_MAX) return index;
	gLoge("gVKBuffer") << "No suitable memory type found.";
	return 0;
}

bool gvkCreateBuffer(gVKContext& ctx, VkDeviceSize size, VkBufferUsageFlags usage,
		VkMemoryPropertyFlags properties, VkBuffer& outBuffer, VkDeviceMemory& outMemory,
		VkMemoryPropertyFlags preferred) {
	VkDevice device = *ctx.getDevice();

	VkBufferCreateInfo bufferinfo{};
	bufferinfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferinfo.size = size;
	bufferinfo.usage = usage;
	bufferinfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	if(vkCreateBuffer(device, &bufferinfo, nullptr, &outBuffer) != VK_SUCCESS) {
		gLoge("gVKBuffer") << "vkCreateBuffer failed.";
		outBuffer = VK_NULL_HANDLE;
		return false;
	}

	VkMemoryRequirements memreq;
	vkGetBufferMemoryRequirements(device, outBuffer, &memreq);

	// preferred is memory the caller would rather have but can do without - device
	// local memory that the CPU can also write, which is where a buffer the CPU
	// rewrites every frame belongs: plain host visible memory lives in system RAM and
	// the GPU reads it across the bus on every draw.
	uint32_t memtype = UINT32_MAX;
	if(preferred != 0) memtype = gvkTryFindMemoryType(ctx, memreq.memoryTypeBits, preferred);
	if(memtype == UINT32_MAX) memtype = gvkFindMemoryType(ctx, memreq.memoryTypeBits, properties);

	VkMemoryAllocateInfo allocinfo{};
	allocinfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocinfo.allocationSize = memreq.size;
	allocinfo.memoryTypeIndex = memtype;
	if(vkAllocateMemory(device, &allocinfo, nullptr, &outMemory) != VK_SUCCESS) {
		gLoge("gVKBuffer") << "vkAllocateMemory failed.";
		vkDestroyBuffer(device, outBuffer, nullptr);
		outBuffer = VK_NULL_HANDLE;
		outMemory = VK_NULL_HANDLE;
		return false;
	}
	vkBindBufferMemory(device, outBuffer, outMemory, 0);
	return true;
}

// ---------------------------------------------------------------------------
// Batched uploads
//
// Every texture and every static mesh used to own its transfer end to end: its
// own command buffer, its own fence, its own queue submit and a host wait for
// that submit to finish. Loading a level meant several hundred round trips to
// the GPU, each costing far more than the copy it carried - and on Android's
// gfxstream and on MoltenVK each one also crosses a translation layer.
//
// Transfers now share a batch. The batch owns the command buffer, the fence and,
// which is the part that has to be exact, the staging buffers: those cannot be
// freed until the submission reading them has finished. Freeing them early is
// what an earlier attempt at batching got wrong, and gfxstream reported it as
// "Failed to unbox VkBuffer" from inside an unrelated later call.
//
// No host wait is involved in the common path. A batch is submitted before the
// frame that could use what it uploaded, and a barrier recorded in one submission
// applies to everything submitted after it on the same queue, so the frame sees
// finished uploads without the CPU ever blocking on them.

struct gVKUploadStaging {
	VkBuffer buffer = VK_NULL_HANDLE;
	VkDeviceMemory memory = VK_NULL_HANDLE;
};

struct gVKUploadBatch {
	VkCommandBuffer cmd = VK_NULL_HANDLE;
	VkFence fence = VK_NULL_HANDLE;
	std::vector<gVKUploadStaging> staging;
	// Recording: commands have been written and the batch has not been submitted.
	// Pending: submitted, and its staging is still being read by the GPU.
	bool recording = false;
	bool pending = false;
};

// Four slots: three submissions can be in flight while a fourth is recorded. This
// is what bounds staging memory - opening a fifth batch waits for the oldest one,
// so the upload path holds at most four batches' worth at any time.
static const size_t gvkuploadbatchcount = 4;
static gVKUploadBatch gvkuploadbatches[gvkuploadbatchcount];
static size_t gvkuploadcurrent = 0;

// Staging bytes recorded into the open batch. A batch is submitted once it passes
// this, so a level loading a hundred textures does not hold all of them in host
// memory waiting for a frame boundary that only comes after the load.
static VkDeviceSize gvkuploadbytes = 0;
static const VkDeviceSize gvkuploadflushbytes = 8 * 1024 * 1024;

static void gvkReleaseUploadStaging(gVKContext& ctx, gVKUploadBatch& batch) {
	VkDevice device = *ctx.getDevice();
	if(device != VK_NULL_HANDLE) {
		for(auto& entry : batch.staging) {
			if(entry.buffer != VK_NULL_HANDLE) vkDestroyBuffer(device, entry.buffer, nullptr);
			if(entry.memory != VK_NULL_HANDLE) vkFreeMemory(device, entry.memory, nullptr);
		}
	}
	// Cleared rather than freed: the same slot stages the next batch, and a level
	// load cycles through these four vectors thousands of times.
	batch.staging.clear();
	batch.pending = false;
}

VkCommandBuffer gvkBeginUpload(gVKContext& ctx) {
	VkDevice device = *ctx.getDevice();
	if(device == VK_NULL_HANDLE || *ctx.getCommandPool() == VK_NULL_HANDLE) return VK_NULL_HANDLE;

	gVKUploadBatch& batch = gvkuploadbatches[gvkuploadcurrent];
	if(batch.recording) return batch.cmd;

	// This slot's previous submission may still be running, and neither its command
	// buffer nor its staging can be reused until it is done.
	if(batch.pending) {
		vkWaitForFences(device, 1, &batch.fence, VK_TRUE, UINT64_MAX);
		gvkReleaseUploadStaging(ctx, batch);
	}

	if(batch.fence == VK_NULL_HANDLE) {
		VkFenceCreateInfo fenceinfo{};
		fenceinfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		if(vkCreateFence(device, &fenceinfo, nullptr, &batch.fence) != VK_SUCCESS) {
			gLoge("gVKBuffer") << "vkCreateFence failed for an upload batch.";
			batch.fence = VK_NULL_HANDLE;
			return VK_NULL_HANDLE;
		}
	}
	if(batch.cmd == VK_NULL_HANDLE) {
		VkCommandBufferAllocateInfo allocinfo{};
		allocinfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocinfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocinfo.commandPool = *ctx.getCommandPool();
		allocinfo.commandBufferCount = 1;
		if(vkAllocateCommandBuffers(device, &allocinfo, &batch.cmd) != VK_SUCCESS) {
			gLoge("gVKBuffer") << "vkAllocateCommandBuffers failed for an upload batch.";
			batch.cmd = VK_NULL_HANDLE;
			return VK_NULL_HANDLE;
		}
	}

	VkCommandBufferBeginInfo begininfo{};
	begininfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	begininfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	if(vkBeginCommandBuffer(batch.cmd, &begininfo) != VK_SUCCESS) {
		gLoge("gVKBuffer") << "vkBeginCommandBuffer failed for an upload batch.";
		return VK_NULL_HANDLE;
	}
	batch.recording = true;
	return batch.cmd;
}

void gvkEndUpload(gVKContext& ctx, VkBuffer stagingBuffer, VkDeviceMemory stagingMemory,
		VkDeviceSize stagingSize) {
	gVKUploadBatch& batch = gvkuploadbatches[gvkuploadcurrent];
	if(!batch.recording) return;
	if(stagingBuffer != VK_NULL_HANDLE || stagingMemory != VK_NULL_HANDLE) {
		batch.staging.push_back({stagingBuffer, stagingMemory});
		gvkuploadbytes += stagingSize;
	}
	if(gvkuploadbytes >= gvkuploadflushbytes) gvkFlushUploads(ctx);
}

void gvkFlushUploads(gVKContext& ctx) {
	VkDevice device = *ctx.getDevice();
	gVKUploadBatch& batch = gvkuploadbatches[gvkuploadcurrent];
	if(!batch.recording || device == VK_NULL_HANDLE) return;
	batch.recording = false;
	gvkuploadbytes = 0;

	if(vkEndCommandBuffer(batch.cmd) != VK_SUCCESS) {
		gLoge("gVKBuffer") << "vkEndCommandBuffer failed for an upload batch.";
		// Nothing was submitted, so nothing can still be reading the staging.
		gvkReleaseUploadStaging(ctx, batch);
		return;
	}

	VkSubmitInfo submitinfo{};
	submitinfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitinfo.commandBufferCount = 1;
	submitinfo.pCommandBuffers = &batch.cmd;

	if(vkResetFences(device, 1, &batch.fence) != VK_SUCCESS) {
		gLoge("gVKBuffer") << "vkResetFences failed for an upload batch.";
		gvkReleaseUploadStaging(ctx, batch);
		return;
	}
	const VkResult result = vkQueueSubmit(*ctx.getGraphicsQueue(), 1, &submitinfo, batch.fence);
	if(result != VK_SUCCESS) {
		gLoge("gVKBuffer") << "vkQueueSubmit failed for an upload batch! VkResult: " << result;
		gvkReleaseUploadStaging(ctx, batch);
		return;
	}
	batch.pending = true;

	// The next transfer records into the following slot, so this one keeps its
	// staging alive for as long as the GPU needs it.
	gvkuploadcurrent = (gvkuploadcurrent + 1) % gvkuploadbatchcount;
}

void gvkCollectUploads(gVKContext& ctx) {
	VkDevice device = *ctx.getDevice();
	if(device == VK_NULL_HANDLE) return;
	for(size_t i = 0; i < gvkuploadbatchcount; i++) {
		gVKUploadBatch& batch = gvkuploadbatches[i];
		if(!batch.pending) continue;
		if(vkGetFenceStatus(device, batch.fence) != VK_SUCCESS) continue;
		gvkReleaseUploadStaging(ctx, batch);
	}
}

void gvkWaitUploads(gVKContext& ctx) {
	VkDevice device = *ctx.getDevice();
	if(device == VK_NULL_HANDLE) return;
	gvkFlushUploads(ctx);
	for(size_t i = 0; i < gvkuploadbatchcount; i++) {
		gVKUploadBatch& batch = gvkuploadbatches[i];
		if(!batch.pending) continue;
		vkWaitForFences(device, 1, &batch.fence, VK_TRUE, UINT64_MAX);
		gvkReleaseUploadStaging(ctx, batch);
	}
}

void gvkDestroyUploadContext(gVKContext& ctx) {
	VkDevice device = *ctx.getDevice();
	if(device != VK_NULL_HANDLE) gvkWaitUploads(ctx);
	for(size_t i = 0; i < gvkuploadbatchcount; i++) {
		gVKUploadBatch& batch = gvkuploadbatches[i];
		// A device that is already gone takes its objects with it; only the staging
		// bookkeeping still has to be dropped so a later context starts clean.
		if(device != VK_NULL_HANDLE) {
			if(batch.cmd != VK_NULL_HANDLE && *ctx.getCommandPool() != VK_NULL_HANDLE) {
				vkFreeCommandBuffers(device, *ctx.getCommandPool(), 1, &batch.cmd);
			}
			if(batch.fence != VK_NULL_HANDLE) vkDestroyFence(device, batch.fence, nullptr);
		}
		batch.cmd = VK_NULL_HANDLE;
		batch.fence = VK_NULL_HANDLE;
		batch.staging.clear();
		batch.recording = false;
		batch.pending = false;
	}
	gvkuploadcurrent = 0;
	gvkuploadbytes = 0;
}

bool gvkUploadBufferData(gVKContext& ctx, VkBuffer dst, const void* data, VkDeviceSize size) {
	VkDevice device = *ctx.getDevice();
	if(device == VK_NULL_HANDLE || dst == VK_NULL_HANDLE || data == nullptr || size == 0) return false;

	VkBuffer staging = VK_NULL_HANDLE;
	VkDeviceMemory stagingmemory = VK_NULL_HANDLE;
	if(!gvkCreateBuffer(ctx, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			staging, stagingmemory)) {
		gLoge("gVKBuffer") << "Could not create the staging buffer for a " << size << " byte upload.";
		return false;
	}
	void* mapped = nullptr;
	if(vkMapMemory(device, stagingmemory, 0, size, 0, &mapped) != VK_SUCCESS) {
		gLoge("gVKBuffer") << "vkMapMemory failed for a staging buffer.";
		vkDestroyBuffer(device, staging, nullptr);
		vkFreeMemory(device, stagingmemory, nullptr);
		return false;
	}
	std::memcpy(mapped, data, static_cast<size_t>(size));
	// Host coherent memory needs no explicit flush.
	vkUnmapMemory(device, stagingmemory);

	VkCommandBuffer cmd = gvkBeginUpload(ctx);
	if(cmd == VK_NULL_HANDLE) {
		vkDestroyBuffer(device, staging, nullptr);
		vkFreeMemory(device, stagingmemory, nullptr);
		return false;
	}
	VkBufferCopy region{};
	region.size = size;
	vkCmdCopyBuffer(cmd, staging, dst, 1, &region);

	// The copy used to be followed by a host wait, which is what made its result
	// visible. A batch is not waited for, so the dependency has to be recorded:
	// without this barrier the draw reading these vertices is only ordered against
	// the copy, not against its writes landing in memory.
	VkBufferMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
	barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT | VK_ACCESS_INDEX_READ_BIT;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.buffer = dst;
	barrier.offset = 0;
	barrier.size = size;
	vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
			0, 0, nullptr, 1, &barrier, 0, nullptr);

	gvkEndUpload(ctx, staging, stagingmemory, size);
	return true;
}

#endif /* GVK_VULKAN */
