/*
 * gVKSwapchain.cpp
 *
 * Swapchain, its image views and the resize path. Created by: Veysel Burak Eroglu.
 */

#include "gVKSwapchain.h"

#ifdef GVK_VULKAN

#include "gVKRenderTarget.h"
#include "gVKSync.h"
#include "gUtils.h"
#include "gBaseWindow.h"
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
static VkExtent2D gvkPickExtent(const VkSurfaceCapabilitiesKHR& caps, gBaseWindow* window) {
	if(caps.currentExtent.width != UINT32_MAX) {
		return caps.currentExtent;
	}
	int width = 0, height = 0;
	width = window->getWidth();
	height = window->getHeight();
	VkExtent2D extent{};
	extent.width = std::clamp(static_cast<uint32_t>(width), caps.minImageExtent.width, caps.maxImageExtent.width);
	extent.height = std::clamp(static_cast<uint32_t>(height), caps.minImageExtent.height, caps.maxImageExtent.height);
	return extent;
}

// Which present mode to ask for, from what the surface reported when the device
// was chosen.
//
// This is where vsync lives on Vulkan. There is no glfwSwapInterval to call:
// presentation pacing is a property of the swapchain, so the choice is made here
// and a change to it costs a swapchain rebuild.
//
// FIFO waits for the display and is the only mode the specification guarantees
// everywhere, so it is both the vsynced choice and the fallback.
//
// With vsync off, IMMEDIATE comes before MAILBOX. MAILBOX drops stale frames
// rather than queueing them, which avoids tearing and reads like the better mode,
// but the application still cannot get ahead of the refresh: with the image count
// drivers hand out here it measured a hard 144.0 fps on a 144 Hz screen, frame
// period 6.944 ms with no variance, all of it spent waiting inside
// vkAcquireNextImageKHR, while OpenGL with glfwSwapInterval(0) ran the same scene
// at 523. Vsync off is a request not to be paced by the display and IMMEDIATE is
// the mode that means that; MAILBOX stays as the fallback for surfaces without it.
static VkPresentModeKHR gvkPickPresentMode(const std::vector<VkPresentModeKHR>& modes, bool vsyncenabled) {
	if(vsyncenabled) return VK_PRESENT_MODE_FIFO_KHR;
	for(VkPresentModeKHR mode : modes) {
		if(mode == VK_PRESENT_MODE_IMMEDIATE_KHR) return mode;
	}
	for(VkPresentModeKHR mode : modes) {
		if(mode == VK_PRESENT_MODE_MAILBOX_KHR) return mode;
	}
	return VK_PRESENT_MODE_FIFO_KHR;
}

bool gvkCreateSwapchain(gVKContext& ctx, gBaseWindow* window) {
	if(ctx.device == VK_NULL_HANDLE || ctx.surface == VK_NULL_HANDLE || window == nullptr) {
		gLoge("gVKSwapchain") << "Cannot create the swapchain before the device and the surface exist.";
		return false;
	}

	VkSurfaceCapabilitiesKHR caps{};
	if(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(ctx.physicaldevice, ctx.surface, &caps) != VK_SUCCESS) {
		gLoge("gVKSwapchain") << "Could not query the surface capabilities.";
		return false;
	}
	// Keep the capabilities that produced this swapchain. On platforms such as
	// Android, currentExtent is fixed in physical surface pixels while the window
	// can intentionally expose a smaller logical design size to the game.
	ctx.surfacecapabilities = caps;

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
	if((caps.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT) != 0)
		createinfo.imageUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

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

	// ANativeWindow dimensions are already expressed in the activity's current
	// coordinates. Applying Android's natural-display transform again rotates a
	// landscape render target inside the landscape window.
	createinfo.preTransform = (caps.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
			? VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR
			: caps.currentTransform;
	createinfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createinfo.presentMode = gvkPickPresentMode(ctx.surfacepresentmodes, ctx.vsyncenabled);
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
			<< ", transform " << createinfo.preTransform
			<< ", present mode " << (createinfo.presentMode == VK_PRESENT_MODE_FIFO_KHR ? "FIFO (vsync)"
					: createinfo.presentMode == VK_PRESENT_MODE_MAILBOX_KHR ? "MAILBOX"
					: createinfo.presentMode == VK_PRESENT_MODE_IMMEDIATE_KHR ? "IMMEDIATE" : "other");
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
}

bool gvkRecreateSwapchain(gVKContext& ctx, gBaseWindow* window) {
	if(window == nullptr || ctx.device == VK_NULL_HANDLE) return false;

	int width = 0, height = 0;
	width = window->getWidth();
	height = window->getHeight();
	if(width == 0 || height == 0) {
		// Minimised: there is nothing to size the swapchain to. The frame is skipped
		// and the loop tries again once the window comes back.
		return false;
	}

	// Everything below is still referenced by work the GPU may not have finished.
	vkDeviceWaitIdle(ctx.device);

	// Reverse dependency order: the framebuffers point at the image views, at the
	// depth view and - with MSAA on - at the multisampled colour view, and the
	// present semaphores are one per image, so all of them go before the swapchain.
	gvkDestroyFramebuffers(ctx);
	gvkDestroyMsaaColorResources(ctx);
	gvkDestroyDepthResources(ctx);
	gvkDestroyPresentSemaphores(ctx);
	gvkDestroySwapchain(ctx);

	if(!gvkCreateSwapchain(ctx, window)) return false;
	// The depth buffer is sized to the swapchain, so it is rebuilt here; its format
	// is kept, which is what lets the render pass survive.
	if(!gvkCreateDepthResources(ctx)) return false;
	// So is the multisampled colour attachment, for the same reason. Its sample
	// count and format are unchanged, so this too leaves the render pass valid; it
	// does nothing at all while MSAA is off.
	if(!gvkCreateMsaaColorResources(ctx)) return false;
	if(!gvkCreateFramebuffers(ctx)) return false;
	// The render pass survives: neither the surface format nor the depth format
	// changes with the size.
	if(!gvkCreatePresentSemaphores(ctx, static_cast<uint32_t>(ctx.swapchainimages.size()))) return false;

	gLogi("gVKSwapchain") << "Swapchain recreated for " << ctx.swapchainextent.width
			<< "x" << ctx.swapchainextent.height;
	return true;
}

#endif /* GVK_VULKAN */
