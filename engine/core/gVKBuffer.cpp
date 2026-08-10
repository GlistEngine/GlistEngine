/*
 * gVKBuffer.cpp
 */

#include "gVKBuffer.h"

#ifdef GVK_DESKTOP_GLFW

#include "gUtils.h"

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

VkCommandBuffer gvkBeginSingleTimeCommands(gVKContext& ctx) {
	VkDevice device = *ctx.getDevice();

	VkCommandBufferAllocateInfo allocinfo{};
	allocinfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocinfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocinfo.commandPool = *ctx.getCommandPool();
	allocinfo.commandBufferCount = 1;

	VkCommandBuffer commandbuffer = VK_NULL_HANDLE;
	vkAllocateCommandBuffers(device, &allocinfo, &commandbuffer);

	VkCommandBufferBeginInfo begininfo{};
	begininfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	begininfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	vkBeginCommandBuffer(commandbuffer, &begininfo);
	return commandbuffer;
}

void gvkEndSingleTimeCommands(gVKContext& ctx, VkCommandBuffer commandBuffer) {
	VkDevice device = *ctx.getDevice();
	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitinfo{};
	submitinfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitinfo.commandBufferCount = 1;
	submitinfo.pCommandBuffers = &commandBuffer;
	// A transient transfer; waiting on the queue is simpler than a fence here.
	vkQueueSubmit(*ctx.getGraphicsQueue(), 1, &submitinfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(*ctx.getGraphicsQueue());
	vkFreeCommandBuffers(device, *ctx.getCommandPool(), 1, &commandBuffer);
}

void gvkCopyBuffer(gVKContext& ctx, VkBuffer src, VkBuffer dst, VkDeviceSize size) {
	VkCommandBuffer commandbuffer = gvkBeginSingleTimeCommands(ctx);
	VkBufferCopy region{};
	region.size = size;
	vkCmdCopyBuffer(commandbuffer, src, dst, 1, &region);
	gvkEndSingleTimeCommands(ctx, commandbuffer);
}

#endif /* GVK_DESKTOP_GLFW */
