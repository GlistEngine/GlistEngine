/*
 * gVKSwapchain.cpp
 *
 * Swapchain, its image views and the resize path. Created by: Veysel Burak Eroglu.
 */

#include "gVKSwapchain.h"

#ifdef GVK_DESKTOP_GLFW

#include "gVKRenderTarget.h"
#include "gVKSync.h"
#include "gUtils.h"
#include <GLFW/glfw3.h>
#include <algorithm>

// The OpenGL backend treats the engine's colour values as display-ready values.
// Prefer an UNORM swapchain so Vulkan follows the same path without an implicit
// linear-to-sRGB conversion on every clear and fragment output.
static VkSurfaceFormatKHR gvkPickSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats) {
	if(formats.size() == 1 && formats[0].format == VK_FORMAT_UNDEFINED) {
		return {VK_FORMAT_B8G8R8A8_UNORM, formats[0].colorSpace};
	}
	for(const auto& format : formats) {
		if(format.format == VK_FORMAT_B8G8R8A8_UNORM &&
				format.colorSpace == VK_COLOR_SPACE_PASS_THROUGH_EXT) {
			return format;
		}
	}
	for(const auto& format : formats) {
		if(format.format == VK_FORMAT_R8G8B8A8_UNORM &&
				format.colorSpace == VK_COLOR_SPACE_PASS_THROUGH_EXT) {
			return format;
		}
	}
	for(const auto& format : formats) {
		if(format.format == VK_FORMAT_B8G8R8A8_UNORM &&
				format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			return format;
		}
	}
	for(const auto& format : formats) {
		if(format.format == VK_FORMAT_R8G8B8A8_UNORM &&
				format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			return format;
		}
	}
	return formats[0];
}

// Resolves the size of the swapchain images. Most drivers dictate it through
// currentExtent; the special value UINT32_MAX means the window system lets us
// choose, in which case the framebuffer size is used and clamped to what the
// surface supports.
static VkExtent2D gvkPickExtent(const VkSurfaceCapabilitiesKHR& caps, GLFWwindow* window) {
	if(caps.currentExtent.width != UINT32_MAX) {
		return caps.currentExtent;
	}
	int width = 0, height = 0;
	glfwGetFramebufferSize(window, &width, &height);
	VkExtent2D extent{};
	extent.width = std::clamp(static_cast<uint32_t>(width), caps.minImageExtent.width, caps.maxImageExtent.width);
	extent.height = std::clamp(static_cast<uint32_t>(height), caps.minImageExtent.height, caps.maxImageExtent.height);
	return extent;
}

bool gvkCreateSwapchain(gVKContext& ctx, GLFWwindow* window) {
	if(ctx.device == VK_NULL_HANDLE || ctx.surface == VK_NULL_HANDLE || window == nullptr) {
		gLoge("gVKSwapchain") << "Cannot create the swapchain before the device and the surface exist.";
		return false;
	}

	VkSurfaceCapabilitiesKHR caps{};
	if(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(ctx.physicaldevice, ctx.surface, &caps) != VK_SUCCESS) {
		gLoge("gVKSwapchain") << "Could not query the surface capabilities.";
		return false;
	}

	uint32_t formatcount = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(ctx.physicaldevice, ctx.surface, &formatcount, nullptr);
	if(formatcount == 0) {
		gLoge("gVKSwapchain") << "The surface reports no usable format.";
		return false;
	}
	std::vector<VkSurfaceFormatKHR> formats(formatcount);
	vkGetPhysicalDeviceSurfaceFormatsKHR(ctx.physicaldevice, ctx.surface, &formatcount, formats.data());
	const VkSurfaceFormatKHR surfaceformat = gvkPickSurfaceFormat(formats);

	const VkExtent2D extent = gvkPickExtent(caps, window);
	if(extent.width == 0 || extent.height == 0) {
		// The window is minimised. Nothing can be presented, so the caller has to
		// try again once it is restored.
		return false;
	}

	// One image more than the driver's minimum, so the application can work on the
	// next frame while an image is still being presented.
	uint32_t imagecount = caps.minImageCount + 1;
	if(caps.maxImageCount > 0 && imagecount > caps.maxImageCount) {
		imagecount = caps.maxImageCount;
	}

	VkSwapchainCreateInfoKHR createinfo{};
	createinfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createinfo.surface = ctx.surface;
	createinfo.minImageCount = imagecount;
	createinfo.imageFormat = surfaceformat.format;
	createinfo.imageColorSpace = surfaceformat.colorSpace;
	createinfo.imageExtent = extent;
	createinfo.imageArrayLayers = 1;
	createinfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	// When one family renders and another presents, the images are used by both, so
	// they have to be shared. On the usual single family setup exclusive ownership
	// is both valid and faster.
	const uint32_t families[] = {ctx.graphicsfamily, ctx.presentfamily};
	if(ctx.graphicsfamily != ctx.presentfamily) {
		createinfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createinfo.queueFamilyIndexCount = 2;
		createinfo.pQueueFamilyIndices = families;
	} else {
		createinfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	}

	createinfo.preTransform = caps.currentTransform;
	createinfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	// FIFO is the only present mode the specification guarantees everywhere, and it
	// is vsynced, so no tearing.
	createinfo.presentMode = VK_PRESENT_MODE_FIFO_KHR;
	createinfo.clipped = VK_TRUE;
	createinfo.oldSwapchain = VK_NULL_HANDLE;

	VkResult result = vkCreateSwapchainKHR(ctx.device, &createinfo, nullptr, &ctx.swapchain);
	if(result != VK_SUCCESS) {
		gLoge("gVKSwapchain") << "vkCreateSwapchainKHR failed! VkResult: " << result;
		ctx.swapchain = VK_NULL_HANDLE;
		return false;
	}

	// The driver decides how many images it actually creates, so the count is read
	// back instead of assumed.
	uint32_t createdcount = 0;
	vkGetSwapchainImagesKHR(ctx.device, ctx.swapchain, &createdcount, nullptr);
	ctx.swapchainimages.resize(createdcount);
	vkGetSwapchainImagesKHR(ctx.device, ctx.swapchain, &createdcount, ctx.swapchainimages.data());
	ctx.swapchainimagelayouts.assign(createdcount, VK_IMAGE_LAYOUT_UNDEFINED);

	ctx.swapchainformat = surfaceformat.format;
	ctx.swapchainextent = extent;

	// A framebuffer cannot reference an image directly; it needs a view that tells
	// Vulkan how the image is interpreted.
	ctx.swapchainimageviews.resize(createdcount, VK_NULL_HANDLE);
	for(uint32_t i = 0; i < createdcount; i++) {
		VkImageViewCreateInfo viewinfo{};
		viewinfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewinfo.image = ctx.swapchainimages[i];
		viewinfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewinfo.format = ctx.swapchainformat;
		viewinfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewinfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewinfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewinfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewinfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		viewinfo.subresourceRange.baseMipLevel = 0;
		viewinfo.subresourceRange.levelCount = 1;
		viewinfo.subresourceRange.baseArrayLayer = 0;
		viewinfo.subresourceRange.layerCount = 1;

		result = vkCreateImageView(ctx.device, &viewinfo, nullptr, &ctx.swapchainimageviews[i]);
		if(result != VK_SUCCESS) {
			gLoge("gVKSwapchain") << "vkCreateImageView failed for swapchain image " << i
					<< "! VkResult: " << result;
			gvkDestroySwapchain(ctx);
			return false;
		}
	}

	gLogi("gVKSwapchain") << "Swapchain created: " << createdcount << " images, "
			<< ctx.swapchainextent.width << "x" << ctx.swapchainextent.height
			<< ", format " << ctx.swapchainformat << ", color space " << surfaceformat.colorSpace
			<< ", present mode FIFO";
	return true;
}

void gvkDestroySwapchain(gVKContext& ctx) {
	if(ctx.device == VK_NULL_HANDLE) return;

	for(VkImageView view : ctx.swapchainimageviews) {
		if(view != VK_NULL_HANDLE) vkDestroyImageView(ctx.device, view, nullptr);
	}
	ctx.swapchainimageviews.clear();

	if(ctx.swapchain != VK_NULL_HANDLE) {
		vkDestroySwapchainKHR(ctx.device, ctx.swapchain, nullptr);
		ctx.swapchain = VK_NULL_HANDLE;
	}
	// The images belong to the swapchain and are freed with it, so only the handles
	// are dropped here.
	ctx.swapchainimages.clear();
	ctx.swapchainimagelayouts.clear();
}

bool gvkRecreateSwapchain(gVKContext& ctx, GLFWwindow* window) {
	if(window == nullptr || ctx.device == VK_NULL_HANDLE) return false;

	int width = 0, height = 0;
	glfwGetFramebufferSize(window, &width, &height);
	if(width == 0 || height == 0) {
		// Minimised: there is nothing to size the swapchain to. The frame is skipped
		// and the loop tries again once the window comes back.
		return false;
	}

	// Everything below is still referenced by work the GPU may not have finished.
	vkDeviceWaitIdle(ctx.device);

	// Reverse dependency order: the framebuffers point at the image views, and the
	// present semaphores are one per image, so both go before the swapchain.
	gvkDestroyDepthTargets(ctx);
	gvkDestroyPresentSemaphores(ctx);
	gvkDestroySwapchain(ctx);

	if(!gvkCreateSwapchain(ctx, window)) return false;
	if(!gvkCreateDepthTargets(ctx)) return false;
	// The render pass survives: the surface format does not change with the size.
	if(!gvkCreatePresentSemaphores(ctx, static_cast<uint32_t>(ctx.swapchainimages.size()))) return false;

	gLogi("gVKSwapchain") << "Swapchain recreated for " << ctx.swapchainextent.width
			<< "x" << ctx.swapchainextent.height;
	return true;
}

#endif /* GVK_DESKTOP_GLFW */
