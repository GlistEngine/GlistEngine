/*
 * gVKCommands.cpp
 *
 * Command pool and command buffers of the Vulkan backend.
 */

#include "gVKCommands.h"

#ifdef GVK_DESKTOP_GLFW

#include "gUtils.h"
#include <algorithm>
#include <thread>

bool gvkCreateCommandResources(gVKContext& ctx) {
	if(ctx.device == VK_NULL_HANDLE) {
		gLoge("gVKCommands") << "Cannot create the command resources before the device exists.";
		return false;
	}

	VkCommandPoolCreateInfo poolinfo{};
	poolinfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	// Drawing commands are submitted on the graphics queue, so the pool has to
	// belong to that family.
	poolinfo.queueFamilyIndex = ctx.graphicsfamily;
	// Every frame records its command buffer again from scratch. Without this flag
	// resetting an individual buffer is not allowed and the whole pool would have
	// to be reset instead.
	poolinfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

	VkResult result = vkCreateCommandPool(ctx.device, &poolinfo, nullptr, &ctx.commandpool);
	if(result != VK_SUCCESS) {
		gLoge("gVKCommands") << "vkCreateCommandPool failed! VkResult: " << result;
		ctx.commandpool = VK_NULL_HANDLE;
		return false;
	}

	// One buffer per frame in flight, so the frame being recorded never touches the
	// buffer the GPU is still reading from.
	ctx.commandbuffers.resize(GVK_MAX_FRAMES_IN_FLIGHT, VK_NULL_HANDLE);

	VkCommandBufferAllocateInfo allocinfo{};
	allocinfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocinfo.commandPool = ctx.commandpool;
	// Primary: submitted to a queue directly, as opposed to being called from
	// another command buffer.
	allocinfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocinfo.commandBufferCount = static_cast<uint32_t>(ctx.commandbuffers.size());

	result = vkAllocateCommandBuffers(ctx.device, &allocinfo, ctx.commandbuffers.data());
	if(result != VK_SUCCESS) {
		gLoge("gVKCommands") << "vkAllocateCommandBuffers failed! VkResult: " << result;
		gvkDestroyCommandResources(ctx);
		return false;
	}

	// Secondary command buffers are recorded from worker threads once the renderer
	// has collected a complete render-pass command list.  Command pools themselves
	// are not thread safe, hence one pool per worker and one secondary buffer per
	// frame slot. Keep the number bounded: recording competes with the application's
	// update thread, and a handful of large mesh batches scales better than dozens
	// of tiny workers.
	const unsigned int hardwarethreads = std::thread::hardware_concurrency();
	const unsigned int workercount = hardwarethreads > 1
			? std::min(3u, hardwarethreads - 1) : 0u;
	ctx.workercommandpools.assign(workercount, VK_NULL_HANDLE);
	ctx.workercommandbuffers.assign(workercount,
			std::vector<VkCommandBuffer>(GVK_MAX_FRAMES_IN_FLIGHT, VK_NULL_HANDLE));
	for(unsigned int worker = 0; worker < workercount; ++worker) {
		if(vkCreateCommandPool(ctx.device, &poolinfo, nullptr, &ctx.workercommandpools[worker]) != VK_SUCCESS) {
			gLoge("gVKCommands") << "Could not create Vulkan worker command pool " << worker << ".";
			gvkDestroyCommandResources(ctx);
			return false;
		}
		VkCommandBufferAllocateInfo workeralloc{};
		workeralloc.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		workeralloc.commandPool = ctx.workercommandpools[worker];
		workeralloc.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
		workeralloc.commandBufferCount = GVK_MAX_FRAMES_IN_FLIGHT;
		if(vkAllocateCommandBuffers(ctx.device, &workeralloc, ctx.workercommandbuffers[worker].data()) != VK_SUCCESS) {
			gLoge("gVKCommands") << "Could not allocate Vulkan worker command buffers.";
			gvkDestroyCommandResources(ctx);
			return false;
		}
	}

	gLogi("gVKCommands") << "Command pool created with " << ctx.commandbuffers.size()
			<< " primary command buffers and " << workercount << " worker pools on queue family "
			<< ctx.graphicsfamily;
	return true;
}

void gvkDestroyCommandResources(gVKContext& ctx) {
	if(ctx.device == VK_NULL_HANDLE) return;

	for(VkCommandPool& pool : ctx.workercommandpools) {
		if(pool != VK_NULL_HANDLE) vkDestroyCommandPool(ctx.device, pool, nullptr);
		pool = VK_NULL_HANDLE;
	}
	ctx.workercommandbuffers.clear();
	ctx.workercommandpools.clear();

	// Destroying the pool frees every command buffer allocated from it, so calling
	// vkFreeCommandBuffers beforehand would be redundant.
	if(ctx.commandpool != VK_NULL_HANDLE) {
		vkDestroyCommandPool(ctx.device, ctx.commandpool, nullptr);
		ctx.commandpool = VK_NULL_HANDLE;
	}
	ctx.commandbuffers.clear();
}

#endif /* GVK_DESKTOP_GLFW */
