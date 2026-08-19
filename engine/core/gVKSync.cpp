/*
 * gVKSync.cpp
 *
 * Synchronisation primitives of the Vulkan frame loop.
 */

#include "gVKSync.h"

#ifdef GVK_VULKAN

#include "gUtils.h"

bool gvkCreateFrameSyncObjects(gVKContext& ctx) {
	if(ctx.device == VK_NULL_HANDLE) {
		gLoge("gVKSync") << "Cannot create the frame synchronisation objects before the device exists.";
		return false;
	}

	ctx.imageavailablesemaphores.resize(GVK_MAX_FRAMES_IN_FLIGHT, VK_NULL_HANDLE);
	ctx.inflightfences.resize(GVK_MAX_FRAMES_IN_FLIGHT, VK_NULL_HANDLE);

	VkSemaphoreCreateInfo semaphoreinfo{};
	semaphoreinfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceinfo{};
	fenceinfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	// Created already signalled: the first frame waits on this fence before it has
	// ever submitted anything, and an unsignalled fence would block forever there.
	fenceinfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for(int i = 0; i < GVK_MAX_FRAMES_IN_FLIGHT; i++) {
		VkResult result = vkCreateSemaphore(ctx.device, &semaphoreinfo, nullptr, &ctx.imageavailablesemaphores[i]);
		if(result != VK_SUCCESS) {
			gLoge("gVKSync") << "vkCreateSemaphore failed for the image available semaphore of frame "
					<< i << "! VkResult: " << result;
			gvkDestroyFrameSyncObjects(ctx);
			return false;
		}
		result = vkCreateFence(ctx.device, &fenceinfo, nullptr, &ctx.inflightfences[i]);
		if(result != VK_SUCCESS) {
			gLoge("gVKSync") << "vkCreateFence failed for frame " << i << "! VkResult: " << result;
			gvkDestroyFrameSyncObjects(ctx);
			return false;
		}
	}

	gLogi("gVKSync") << "Frame synchronisation objects created for "
			<< GVK_MAX_FRAMES_IN_FLIGHT << " frames in flight";
	return true;
}

void gvkDestroyFrameSyncObjects(gVKContext& ctx) {
	if(ctx.device == VK_NULL_HANDLE) return;

	for(VkSemaphore semaphore : ctx.imageavailablesemaphores) {
		if(semaphore != VK_NULL_HANDLE) vkDestroySemaphore(ctx.device, semaphore, nullptr);
	}
	ctx.imageavailablesemaphores.clear();

	for(VkFence fence : ctx.inflightfences) {
		if(fence != VK_NULL_HANDLE) vkDestroyFence(ctx.device, fence, nullptr);
	}
	ctx.inflightfences.clear();
}

bool gvkCreatePresentSemaphores(gVKContext& ctx, uint32_t imagecount) {
	if(ctx.device == VK_NULL_HANDLE) {
		gLoge("gVKSync") << "Cannot create the present semaphores before the device exists.";
		return false;
	}
	if(imagecount == 0) {
		gLoge("gVKSync") << "Cannot create present semaphores for an empty swapchain.";
		return false;
	}

	ctx.renderfinishedsemaphores.resize(imagecount, VK_NULL_HANDLE);

	VkSemaphoreCreateInfo semaphoreinfo{};
	semaphoreinfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	for(uint32_t i = 0; i < imagecount; i++) {
		VkResult result = vkCreateSemaphore(ctx.device, &semaphoreinfo, nullptr, &ctx.renderfinishedsemaphores[i]);
		if(result != VK_SUCCESS) {
			gLoge("gVKSync") << "vkCreateSemaphore failed for the render finished semaphore of image "
					<< i << "! VkResult: " << result;
			gvkDestroyPresentSemaphores(ctx);
			return false;
		}
	}

	gLogi("gVKSync") << "Present semaphores created: " << imagecount << " (one per swapchain image)";
	return true;
}

void gvkDestroyPresentSemaphores(gVKContext& ctx) {
	if(ctx.device == VK_NULL_HANDLE) return;

	for(VkSemaphore semaphore : ctx.renderfinishedsemaphores) {
		if(semaphore != VK_NULL_HANDLE) vkDestroySemaphore(ctx.device, semaphore, nullptr);
	}
	ctx.renderfinishedsemaphores.clear();
}

#endif /* GVK_VULKAN */
