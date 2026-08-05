/*
 * gVKRenderTarget.cpp
 *
 * Formats and depth targets used by Vulkan Dynamic Rendering.
 */

#include "gVKRenderTarget.h"

#ifdef GVK_DESKTOP_GLFW

#include "gUtils.h"

static VkFormat gvkFindDepthFormat(VkPhysicalDevice physicaldevice) {
	const VkFormat candidates[] = {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D24_UNORM_S8_UINT,
			VK_FORMAT_D32_SFLOAT_S8_UINT};
	for(VkFormat format : candidates) {
		VkFormatProperties properties{};
		vkGetPhysicalDeviceFormatProperties(physicaldevice, format, &properties);
		if(properties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) return format;
	}
	return VK_FORMAT_UNDEFINED;
}

bool gvkSelectRenderingFormats(gVKContext& ctx) {
	if(ctx.device == VK_NULL_HANDLE || ctx.swapchainformat == VK_FORMAT_UNDEFINED) {
		gLoge("gVKRenderTarget") << "Cannot select rendering formats before the swapchain exists.";
		return false;
	}

	ctx.depthformat = gvkFindDepthFormat(ctx.physicaldevice);
	if(ctx.depthformat == VK_FORMAT_UNDEFINED) {
		gLoge("gVKRenderTarget") << "No supported depth attachment format was found.";
		return false;
	}
	gLogi("gVKRenderTarget") << "Dynamic rendering formats selected: colour "
			<< ctx.swapchainformat << ", depth " << ctx.depthformat;
	return true;
}

void gvkResetRenderingFormats(gVKContext& ctx) {
	if(ctx.device == VK_NULL_HANDLE) return;

	ctx.depthformat = VK_FORMAT_UNDEFINED;
}

bool gvkCreateDepthTargets(gVKContext& ctx) {
	if(ctx.device == VK_NULL_HANDLE || ctx.depthformat == VK_FORMAT_UNDEFINED) {
		gLoge("gVKRenderTarget") << "Cannot create depth images before formats are selected.";
		return false;
	}
	if(ctx.swapchainimageviews.empty()) {
		gLoge("gVKRenderTarget") << "Cannot create depth targets without swapchain image views.";
		return false;
	}

	ctx.depthimages.resize(ctx.swapchainimageviews.size(), VK_NULL_HANDLE);
	ctx.depthmemories.resize(ctx.swapchainimageviews.size(), VK_NULL_HANDLE);
	ctx.depthimageviews.resize(ctx.swapchainimageviews.size(), VK_NULL_HANDLE);

	for(size_t i = 0; i < ctx.swapchainimageviews.size(); i++) {
		VkImageCreateInfo imageinfo{};
		imageinfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageinfo.imageType = VK_IMAGE_TYPE_2D;
		imageinfo.extent = {ctx.swapchainextent.width, ctx.swapchainextent.height, 1};
		imageinfo.mipLevels = 1;
		imageinfo.arrayLayers = 1;
		imageinfo.format = ctx.depthformat;
		imageinfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageinfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageinfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		imageinfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageinfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		if(vkCreateImage(ctx.device, &imageinfo, nullptr, &ctx.depthimages[i]) != VK_SUCCESS) {
			gvkDestroyDepthTargets(ctx); return false;
		}
		VkMemoryRequirements requirements{};
		vkGetImageMemoryRequirements(ctx.device, ctx.depthimages[i], &requirements);
		uint32_t memorytype = UINT32_MAX;
		for(uint32_t type = 0; type < ctx.devicememoryproperties.memoryTypeCount; type++) {
			if((requirements.memoryTypeBits & (1u << type)) &&
					(ctx.devicememoryproperties.memoryTypes[type].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)) {
				memorytype = type; break;
			}
		}
		if(memorytype == UINT32_MAX) { gvkDestroyDepthTargets(ctx); return false; }
		VkMemoryAllocateInfo allocinfo{};
		allocinfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocinfo.allocationSize = requirements.size;
		allocinfo.memoryTypeIndex = memorytype;
		if(vkAllocateMemory(ctx.device, &allocinfo, nullptr, &ctx.depthmemories[i]) != VK_SUCCESS ||
				vkBindImageMemory(ctx.device, ctx.depthimages[i], ctx.depthmemories[i], 0) != VK_SUCCESS) {
			gvkDestroyDepthTargets(ctx); return false;
		}
		VkImageViewCreateInfo viewinfo{};
		viewinfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewinfo.image = ctx.depthimages[i];
		viewinfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewinfo.format = ctx.depthformat;
		viewinfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		viewinfo.subresourceRange.levelCount = 1;
		viewinfo.subresourceRange.layerCount = 1;
		if(vkCreateImageView(ctx.device, &viewinfo, nullptr, &ctx.depthimageviews[i]) != VK_SUCCESS) {
			gvkDestroyDepthTargets(ctx); return false;
		}
	}

	gLogi("gVKRenderTarget") << "Dynamic rendering depth images created: " << ctx.depthimages.size()
			<< " at " << ctx.swapchainextent.width << "x" << ctx.swapchainextent.height;
	return true;
}

void gvkDestroyDepthTargets(gVKContext& ctx) {
	if(ctx.device == VK_NULL_HANDLE) return;

	// Kept separate from the render pass on purpose: a resize rebuilds these while
	// the render pass stays valid, since the surface format does not change.
	for(VkImageView view : ctx.depthimageviews) if(view != VK_NULL_HANDLE) vkDestroyImageView(ctx.device, view, nullptr);
	for(VkImage image : ctx.depthimages) if(image != VK_NULL_HANDLE) vkDestroyImage(ctx.device, image, nullptr);
	for(VkDeviceMemory memory : ctx.depthmemories) if(memory != VK_NULL_HANDLE) vkFreeMemory(ctx.device, memory, nullptr);
	ctx.depthimageviews.clear();
	ctx.depthimages.clear();
	ctx.depthmemories.clear();
}

bool gvkCreateShadowTarget(gVKContext& ctx, uint32_t width, uint32_t height) {
	gvkDestroyShadowTarget(ctx);
	if(ctx.device == VK_NULL_HANDLE || ctx.depthformat == VK_FORMAT_UNDEFINED || width == 0 || height == 0) return false;
	VkImageCreateInfo imageinfo{};
	imageinfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageinfo.imageType = VK_IMAGE_TYPE_2D;
	imageinfo.extent = {width, height, 1};
	imageinfo.mipLevels = 1; imageinfo.arrayLayers = 1;
	imageinfo.format = ctx.depthformat;
	imageinfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageinfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageinfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	imageinfo.samples = VK_SAMPLE_COUNT_1_BIT;
	if(vkCreateImage(ctx.device, &imageinfo, nullptr, &ctx.shadowimage) != VK_SUCCESS) return false;
	VkMemoryRequirements req{}; vkGetImageMemoryRequirements(ctx.device, ctx.shadowimage, &req);
	uint32_t memorytype = UINT32_MAX;
	for(uint32_t i = 0; i < ctx.devicememoryproperties.memoryTypeCount; i++) {
		if((req.memoryTypeBits & (1u << i)) && (ctx.devicememoryproperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)) { memorytype = i; break; }
	}
	if(memorytype == UINT32_MAX) { gvkDestroyShadowTarget(ctx); return false; }
	VkMemoryAllocateInfo alloc{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
	alloc.allocationSize = req.size; alloc.memoryTypeIndex = memorytype;
	if(vkAllocateMemory(ctx.device, &alloc, nullptr, &ctx.shadowmemory) != VK_SUCCESS ||
			vkBindImageMemory(ctx.device, ctx.shadowimage, ctx.shadowmemory, 0) != VK_SUCCESS) { gvkDestroyShadowTarget(ctx); return false; }
	VkImageViewCreateInfo view{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
	view.image = ctx.shadowimage; view.viewType = VK_IMAGE_VIEW_TYPE_2D; view.format = ctx.depthformat;
	view.subresourceRange = {VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 1};
	if(vkCreateImageView(ctx.device, &view, nullptr, &ctx.shadowimageview) != VK_SUCCESS) { gvkDestroyShadowTarget(ctx); return false; }
	VkSamplerCreateInfo sampler{VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO};
	sampler.magFilter = VK_FILTER_LINEAR; sampler.minFilter = VK_FILTER_LINEAR;
	sampler.addressModeU = sampler.addressModeV = sampler.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	sampler.maxLod = 1.0f;
	if(vkCreateSampler(ctx.device, &sampler, nullptr, &ctx.shadowsampler) != VK_SUCCESS) { gvkDestroyShadowTarget(ctx); return false; }
	VkDescriptorSetLayout layout = ctx.getShadowDescriptorSetLayout();
	if(layout == VK_NULL_HANDLE || ctx.descriptorpool == VK_NULL_HANDLE) { gvkDestroyShadowTarget(ctx); return false; }
	VkDescriptorSetAllocateInfo ds{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
	ds.descriptorPool = ctx.descriptorpool; ds.descriptorSetCount = 1; ds.pSetLayouts = &layout;
	if(vkAllocateDescriptorSets(ctx.device, &ds, &ctx.shadowdescriptorset) != VK_SUCCESS) { gvkDestroyShadowTarget(ctx); return false; }
	VkDescriptorImageInfo info{ctx.shadowsampler, ctx.shadowimageview, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL};
	VkWriteDescriptorSet write{VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
	write.dstSet = ctx.shadowdescriptorset; write.dstBinding = 0;
	write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER; write.descriptorCount = 1; write.pImageInfo = &info;
	vkUpdateDescriptorSets(ctx.device, 1, &write, 0, nullptr);
	VkCommandBufferAllocateInfo cmdalloc{VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
	cmdalloc.commandPool = ctx.commandpool; cmdalloc.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY; cmdalloc.commandBufferCount = 1;
	VkCommandBuffer cmd = VK_NULL_HANDLE;
	if(vkAllocateCommandBuffers(ctx.device, &cmdalloc, &cmd) != VK_SUCCESS) { gvkDestroyShadowTarget(ctx); return false; }
	VkCommandBufferBeginInfo begin{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO}; begin.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	vkBeginCommandBuffer(cmd, &begin);
	VkImageMemoryBarrier barrier{VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
	barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED; barrier.newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
	barrier.srcQueueFamilyIndex = barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED; barrier.image = ctx.shadowimage;
	barrier.subresourceRange = {VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 1}; barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
			0, 0, nullptr, 0, nullptr, 1, &barrier);
	vkEndCommandBuffer(cmd);
	VkSubmitInfo submit{VK_STRUCTURE_TYPE_SUBMIT_INFO}; submit.commandBufferCount = 1; submit.pCommandBuffers = &cmd;
	vkQueueSubmit(ctx.graphicsqueue, 1, &submit, VK_NULL_HANDLE); vkQueueWaitIdle(ctx.graphicsqueue);
	vkFreeCommandBuffers(ctx.device, ctx.commandpool, 1, &cmd);
	ctx.shadowextent = {width, height}; ctx.shadowlayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
	return true;
}

void gvkDestroyShadowTarget(gVKContext& ctx) {
	if(ctx.device == VK_NULL_HANDLE) return;
	if(ctx.shadowsampler != VK_NULL_HANDLE) vkDestroySampler(ctx.device, ctx.shadowsampler, nullptr);
	if(ctx.shadowimageview != VK_NULL_HANDLE) vkDestroyImageView(ctx.device, ctx.shadowimageview, nullptr);
	if(ctx.shadowimage != VK_NULL_HANDLE) vkDestroyImage(ctx.device, ctx.shadowimage, nullptr);
	if(ctx.shadowmemory != VK_NULL_HANDLE) vkFreeMemory(ctx.device, ctx.shadowmemory, nullptr);
	ctx.shadowsampler = VK_NULL_HANDLE; ctx.shadowimageview = VK_NULL_HANDLE; ctx.shadowimage = VK_NULL_HANDLE;
	ctx.shadowmemory = VK_NULL_HANDLE; ctx.shadowdescriptorset = VK_NULL_HANDLE; ctx.shadowextent = {0,0};
	ctx.shadowlayout = VK_IMAGE_LAYOUT_UNDEFINED;
}

#endif /* GVK_DESKTOP_GLFW */
