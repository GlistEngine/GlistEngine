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

#endif /* GVK_DESKTOP_GLFW */
