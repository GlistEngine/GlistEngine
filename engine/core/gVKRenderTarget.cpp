/*
 * gVKRenderTarget.cpp
 *
 * Render pass and framebuffers of the Vulkan backend.
 */

#include "gVKRenderTarget.h"

#ifdef GVK_VULKAN

#include "gVKBuffer.h"
#include "gUtils.h"

VkFormat gvkFindDepthFormat(gVKContext& ctx) {
	if(ctx.physicaldevice == VK_NULL_HANDLE) return VK_FORMAT_UNDEFINED;

	// D32_SFLOAT first: the engine has no stencil work, so a depth-only format
	// leaves the driver free to pick the smaller layout. The two combined formats
	// follow as fallbacks, one of which every Vulkan implementation must support.
	const VkFormat candidates[] = {
			VK_FORMAT_D32_SFLOAT,
			VK_FORMAT_D32_SFLOAT_S8_UINT,
			VK_FORMAT_D24_UNORM_S8_UINT};

	for(VkFormat format : candidates) {
		VkFormatProperties properties{};
		vkGetPhysicalDeviceFormatProperties(ctx.physicaldevice, format, &properties);
		// Optimal tiling only, matching how the image below is created.
		if(properties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
			return format;
		}
	}

	return VK_FORMAT_UNDEFINED;
}

// The highest sample count at or below what was asked for that the device can give
// both a colour and a depth attachment, or 1x when it can give neither.
//
// The two limits are intersected rather than checked one at a time: a subpass whose
// attachments disagree on their sample count is invalid, so a device that offers 8x
// colour and 4x depth can only be driven at 4x. Capping instead of failing is what
// makes a request portable - an application asks for 4x and gets 4x, 2x or none
// depending on the part it landed on, and nothing else in the backend has to care.
static VkSampleCountFlagBits gvkPickSampleCount(gVKContext& ctx, uint32_t requested) {
	if(requested <= 1 || *ctx.getPhysicalDevice() == VK_NULL_HANDLE) return VK_SAMPLE_COUNT_1_BIT;

	const VkPhysicalDeviceLimits& limits = ctx.getDeviceProperties()->limits;
	const VkSampleCountFlags supported = limits.framebufferColorSampleCounts
			& limits.framebufferDepthSampleCounts;
	// Walk down from the request, so 8 on a 4x part becomes 4 and 3 becomes 2. The
	// spec guarantees VK_SAMPLE_COUNT_1_BIT is always in the mask, so the loop
	// terminates at worst with MSAA off.
	const VkSampleCountFlagBits candidates[] = {
			VK_SAMPLE_COUNT_64_BIT, VK_SAMPLE_COUNT_32_BIT, VK_SAMPLE_COUNT_16_BIT,
			VK_SAMPLE_COUNT_8_BIT, VK_SAMPLE_COUNT_4_BIT, VK_SAMPLE_COUNT_2_BIT};
	for(VkSampleCountFlagBits candidate : candidates) {
		if(static_cast<uint32_t>(candidate) > requested) continue;
		if((supported & candidate) != 0) return candidate;
	}
	return VK_SAMPLE_COUNT_1_BIT;
}

// Finds a memory type for a transient attachment, preferring one the device only
// backs with real storage if it ever has to.
//
// This is the whole mobile story in one function. On a tile-based GPU (Adreno,
// Mali, Apple) the multisampled colour and depth attachments are written, read and
// resolved entirely inside the tile's on-chip memory, and never need to exist in
// main memory at all - provided the driver is told so. LAZILY_ALLOCATED memory is
// how it is told: the allocation commits no physical pages until something forces
// the contents out to RAM, which a LOAD_OP_CLEAR + STORE_OP_DONT_CARE attachment
// never does. That is the difference between 4x MSAA costing a few per cent (the
// resolve, on chip) and costing 60% (four times the colour and depth traffic across
// the memory bus, plus four times the footprint). Desktop parts do not offer the
// property and simply get device-local memory, where MSAA costs what it costs.
static uint32_t gvkFindTransientMemoryType(gVKContext& ctx, uint32_t typebits, bool& outlazy) {
	const VkPhysicalDeviceMemoryProperties& memory = *ctx.getDeviceMemoryProperties();
	for(uint32_t i = 0; i < memory.memoryTypeCount; ++i) {
		const VkMemoryPropertyFlags flags = memory.memoryTypes[i].propertyFlags;
		if((typebits & (1u << i)) &&
				(flags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) &&
				(flags & VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT)) {
			outlazy = true;
			return i;
		}
	}
	return gvkFindMemoryType(ctx, typebits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
}

bool gvkCreateDepthResources(gVKContext& ctx) {
	if(ctx.device == VK_NULL_HANDLE) {
		gLoge("gVKRenderTarget") << "Cannot create the depth buffer before the device exists.";
		return false;
	}
	if(ctx.swapchainextent.width == 0 || ctx.swapchainextent.height == 0) {
		gLoge("gVKRenderTarget") << "Cannot create the depth buffer before the swapchain extent is known.";
		return false;
	}

	// The render pass picks the format first and stores it, so a resize reuses the
	// same one and the pass stays valid.
	if(ctx.depthformat == VK_FORMAT_UNDEFINED) {
		ctx.depthformat = gvkFindDepthFormat(ctx);
		if(ctx.depthformat == VK_FORMAT_UNDEFINED) {
			gLoge("gVKRenderTarget") << "No supported depth attachment format found on this device.";
			return false;
		}
	}

	gvkDestroyDepthResources(ctx);
	ctx.depthimages.assign(GVK_MAX_FRAMES_IN_FLIGHT, VK_NULL_HANDLE);
	ctx.depthimagememories.assign(GVK_MAX_FRAMES_IN_FLIGHT, VK_NULL_HANDLE);
	ctx.depthimageviews.assign(GVK_MAX_FRAMES_IN_FLIGHT, VK_NULL_HANDLE);

	VkImageCreateInfo imageinfo{};
	imageinfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageinfo.imageType = VK_IMAGE_TYPE_2D;
	imageinfo.extent.width = ctx.swapchainextent.width;
	imageinfo.extent.height = ctx.swapchainextent.height;
	imageinfo.extent.depth = 1;
	imageinfo.mipLevels = 1;
	imageinfo.arrayLayers = 1;
	imageinfo.format = ctx.depthformat;
	imageinfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	// The render pass clears the image at the start of every frame, so whatever the
	// previous contents were does not matter.
	imageinfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageinfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT |
			VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT;
	// Every attachment of a subpass must agree on the sample count, so the depth
	// buffer follows the colour one. Nothing samples it and the pass discards it, so
	// a multisampled depth buffer stays in tile memory exactly like the 1x one.
	imageinfo.samples = ctx.samplecount;
	imageinfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	bool lazy = false;
	for(int frame = 0; frame < GVK_MAX_FRAMES_IN_FLIGHT; ++frame) {
		VkImage& image = ctx.depthimages[frame];
		VkDeviceMemory& memory = ctx.depthimagememories[frame];
		VkImageView& view = ctx.depthimageviews[frame];
		if(vkCreateImage(ctx.device, &imageinfo, nullptr, &image) != VK_SUCCESS) {
			gLoge("gVKRenderTarget") << "vkCreateImage failed for depth slot " << frame << ".";
			gvkDestroyDepthResources(ctx);
			return false;
		}
		VkMemoryRequirements memreq{};
		vkGetImageMemoryRequirements(ctx.device, image, &memreq);
		const uint32_t memorytype = gvkFindTransientMemoryType(ctx, memreq.memoryTypeBits, lazy);
		VkMemoryAllocateInfo allocinfo{};
		allocinfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocinfo.allocationSize = memreq.size;
		allocinfo.memoryTypeIndex = memorytype;
		if(vkAllocateMemory(ctx.device, &allocinfo, nullptr, &memory) != VK_SUCCESS) {
			gLoge("gVKRenderTarget") << "vkAllocateMemory failed for depth slot " << frame << ".";
			gvkDestroyDepthResources(ctx);
			return false;
		}
		vkBindImageMemory(ctx.device, image, memory, 0);
		VkImageViewCreateInfo viewinfo{};
		viewinfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewinfo.image = image;
		viewinfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewinfo.format = ctx.depthformat;
		viewinfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		viewinfo.subresourceRange.levelCount = 1;
		viewinfo.subresourceRange.layerCount = 1;
		if(vkCreateImageView(ctx.device, &viewinfo, nullptr, &view) != VK_SUCCESS) {
			gLoge("gVKRenderTarget") << "vkCreateImageView failed for depth slot " << frame << ".";
			gvkDestroyDepthResources(ctx);
			return false;
		}
	}

	gLogi("gVKRenderTarget") << GVK_MAX_FRAMES_IN_FLIGHT << " depth buffers created: format "
			<< ctx.depthformat << " at " << ctx.swapchainextent.width << "x" << ctx.swapchainextent.height
			<< ", " << static_cast<int>(ctx.samplecount) << "x samples"
			<< (lazy ? ", lazy transient memory" : ", device memory");
	return true;
}

bool gvkCreateMsaaColorResources(gVKContext& ctx) {
	gvkDestroyMsaaColorResources(ctx);
	// Without MSAA the render pass writes straight into the acquired swapchain
	// image, so there is nothing to allocate here at all - which is what keeps the
	// default path byte for byte what it was before multisampling existed.
	if(!ctx.isMultiSampled()) return true;
	if(ctx.device == VK_NULL_HANDLE || ctx.swapchainformat == VK_FORMAT_UNDEFINED) {
		gLoge("gVKRenderTarget") << "Cannot create the MSAA colour buffer before the swapchain exists.";
		return false;
	}

	ctx.msaacolorimages.assign(GVK_MAX_FRAMES_IN_FLIGHT, VK_NULL_HANDLE);
	ctx.msaacolormemories.assign(GVK_MAX_FRAMES_IN_FLIGHT, VK_NULL_HANDLE);
	ctx.msaacolorviews.assign(GVK_MAX_FRAMES_IN_FLIGHT, VK_NULL_HANDLE);

	VkImageCreateInfo imageinfo{};
	imageinfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageinfo.imageType = VK_IMAGE_TYPE_2D;
	imageinfo.extent.width = ctx.swapchainextent.width;
	imageinfo.extent.height = ctx.swapchainextent.height;
	imageinfo.extent.depth = 1;
	imageinfo.mipLevels = 1;
	imageinfo.arrayLayers = 1;
	// The resolve writes into the swapchain image, so the two have to share a format.
	imageinfo.format = ctx.swapchainformat;
	imageinfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageinfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	// TRANSIENT is the promise that goes with the pass's CLEAR/DONT_CARE pair: this
	// image is written and resolved within one render pass and never loaded from or
	// stored to memory, which is what lets a tiler keep all of it on chip. See
	// gvkFindTransientMemoryType above for why that decides the entire cost of MSAA.
	imageinfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT;
	imageinfo.samples = ctx.samplecount;
	imageinfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	bool lazy = false;
	for(int frame = 0; frame < GVK_MAX_FRAMES_IN_FLIGHT; ++frame) {
		VkImage& image = ctx.msaacolorimages[frame];
		VkDeviceMemory& memory = ctx.msaacolormemories[frame];
		VkImageView& view = ctx.msaacolorviews[frame];
		if(vkCreateImage(ctx.device, &imageinfo, nullptr, &image) != VK_SUCCESS) {
			gLoge("gVKRenderTarget") << "vkCreateImage failed for MSAA colour slot " << frame << ".";
			gvkDestroyMsaaColorResources(ctx);
			return false;
		}
		VkMemoryRequirements memreq{};
		vkGetImageMemoryRequirements(ctx.device, image, &memreq);
		const uint32_t memorytype = gvkFindTransientMemoryType(ctx, memreq.memoryTypeBits, lazy);
		VkMemoryAllocateInfo allocinfo{};
		allocinfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocinfo.allocationSize = memreq.size;
		allocinfo.memoryTypeIndex = memorytype;
		if(vkAllocateMemory(ctx.device, &allocinfo, nullptr, &memory) != VK_SUCCESS) {
			gLoge("gVKRenderTarget") << "vkAllocateMemory failed for MSAA colour slot " << frame << ".";
			gvkDestroyMsaaColorResources(ctx);
			return false;
		}
		vkBindImageMemory(ctx.device, image, memory, 0);
		VkImageViewCreateInfo viewinfo{};
		viewinfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewinfo.image = image;
		viewinfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewinfo.format = ctx.swapchainformat;
		viewinfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		viewinfo.subresourceRange.levelCount = 1;
		viewinfo.subresourceRange.layerCount = 1;
		if(vkCreateImageView(ctx.device, &viewinfo, nullptr, &view) != VK_SUCCESS) {
			gLoge("gVKRenderTarget") << "vkCreateImageView failed for MSAA colour slot " << frame << ".";
			gvkDestroyMsaaColorResources(ctx);
			return false;
		}
	}

	gLogi("gVKRenderTarget") << GVK_MAX_FRAMES_IN_FLIGHT << " MSAA colour buffers created: "
			<< static_cast<int>(ctx.samplecount) << "x at " << ctx.swapchainextent.width
			<< "x" << ctx.swapchainextent.height
			<< (lazy ? ", lazy transient memory (stays in tile memory)" : ", device memory");
	return true;
}

void gvkDestroyMsaaColorResources(gVKContext& ctx) {
	if(ctx.device == VK_NULL_HANDLE) return;

	for(VkImageView view : ctx.msaacolorviews) if(view != VK_NULL_HANDLE) vkDestroyImageView(ctx.device, view, nullptr);
	for(VkImage image : ctx.msaacolorimages) if(image != VK_NULL_HANDLE) vkDestroyImage(ctx.device, image, nullptr);
	for(VkDeviceMemory memory : ctx.msaacolormemories) if(memory != VK_NULL_HANDLE) vkFreeMemory(ctx.device, memory, nullptr);
	ctx.msaacolorviews.clear();
	ctx.msaacolorimages.clear();
	ctx.msaacolormemories.clear();
}

void gvkDestroyDepthResources(gVKContext& ctx) {
	if(ctx.device == VK_NULL_HANDLE) return;

	for(VkImageView view : ctx.depthimageviews) if(view != VK_NULL_HANDLE) vkDestroyImageView(ctx.device, view, nullptr);
	for(VkImage image : ctx.depthimages) if(image != VK_NULL_HANDLE) vkDestroyImage(ctx.device, image, nullptr);
	for(VkDeviceMemory memory : ctx.depthimagememories) if(memory != VK_NULL_HANDLE) vkFreeMemory(ctx.device, memory, nullptr);
	ctx.depthimageviews.clear();
	ctx.depthimages.clear();
	ctx.depthimagememories.clear();
	// depthformat is deliberately kept: the render pass was built around it and
	// stays valid across a resize.
}

// Builds one screen-shaped render pass at the given sample count.
//
// At one sample this is the plain colour + depth pass the backend has always used:
// the swapchain image is rendered into directly and stored, because it is what gets
// presented. Above one sample the colour attachment becomes the multisampled image
// and the swapchain image joins as attachment 2, the resolve target - the subpass
// resolves into it as it ends, and only that resolved single-sample result is ever
// written to memory.
//
// The load/store pairs are the point of the whole exercise on mobile. The
// multisampled colour attachment is CLEAR on load and DONT_CARE on store, and so is
// depth: nothing outside the pass ever reads either, so a tile-based GPU can keep
// all N samples in tile memory, resolve them there, and write out one pixel per
// tile position. Storing the samples instead - which is what STORE_OP_STORE on the
// multisampled attachment would ask for - forces N times the colour traffic across
// the memory bus and turns 4x MSAA from a few per cent into most of the frame.
//
// The attachment order (colour, depth, resolve) is deliberate: it keeps the colour
// and depth clear values at indices 0 and 1 in both forms, so the frame loop hands
// over the same two-entry array either way. The resolve attachment loads DONT_CARE
// and therefore needs no clear value at all.
static bool gvkBuildScreenRenderPass(gVKContext& ctx, VkSampleCountFlagBits samples, VkRenderPass& outPass) {
	const bool multisampled = samples != VK_SAMPLE_COUNT_1_BIT;

	VkAttachmentDescription colorattachment{};
	// The attachment has to match the images it will be used with.
	colorattachment.format = *ctx.getSwapchainFormat();
	colorattachment.samples = samples;
	// This is what paints the screen: the clear value handed to
	// vkCmdBeginRenderPass is written over the whole attachment by the GPU.
	colorattachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	// Single-sampled, the result is kept because it is what gets presented.
	// Multisampled, the samples are thrown away and the resolve is kept instead.
	colorattachment.storeOp = multisampled ? VK_ATTACHMENT_STORE_OP_DONT_CARE : VK_ATTACHMENT_STORE_OP_STORE;
	colorattachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorattachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	// The previous contents are cleared anyway, so there is nothing worth keeping.
	colorattachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	// Handover point to the presentation engine - unless this is the multisampled
	// image, which no one outside the pass ever sees.
	colorattachment.finalLayout = multisampled
			? VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentDescription depthattachment{};
	depthattachment.format = ctx.getDepthFormat();
	depthattachment.samples = samples;
	// Cleared every frame for the same reason as the colour attachment.
	depthattachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	// Nothing reads depth once the frame is done, so letting the driver drop it
	// saves the write-out on tiled architectures.
	depthattachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthattachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthattachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthattachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthattachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	// The acquired swapchain image, when the colour attachment above is not it.
	VkAttachmentDescription resolveattachment{};
	resolveattachment.format = *ctx.getSwapchainFormat();
	resolveattachment.samples = VK_SAMPLE_COUNT_1_BIT;
	// Every texel is written by the resolve, so loading the previous frame's would
	// be pure bandwidth. This is stored: it is the image that gets presented.
	resolveattachment.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	resolveattachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	resolveattachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	resolveattachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	resolveattachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	resolveattachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference colorattachmentref{};
	colorattachmentref.attachment = 0;
	colorattachmentref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthattachmentref{};
	depthattachmentref.attachment = 1;
	depthattachmentref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference resolveattachmentref{};
	resolveattachmentref.attachment = 2;
	resolveattachmentref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorattachmentref;
	subpass.pDepthStencilAttachment = &depthattachmentref;
	// One resolve reference per colour reference, resolved by the GPU as the subpass
	// ends. Doing it here rather than with a vkCmdResolveImage afterwards is what
	// keeps the samples on chip; an explicit resolve would have to store them first.
	subpass.pResolveAttachments = multisampled ? &resolveattachmentref : nullptr;

	// Without this the layout transition of the attachment is not ordered against
	// the work of the subpass, which the validation layers report as an error. The
	// early/late fragment test stages are in the masks because the depth attachment
	// is cleared and written there, not at colour output.
	VkSubpassDependency dependency{};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
			| VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
			| VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
			| VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

	VkAttachmentDescription attachments[] = {colorattachment, depthattachment, resolveattachment};

	VkRenderPassCreateInfo renderpassinfo{};
	renderpassinfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderpassinfo.attachmentCount = multisampled ? 3 : 2;
	renderpassinfo.pAttachments = attachments;
	renderpassinfo.subpassCount = 1;
	renderpassinfo.pSubpasses = &subpass;
	renderpassinfo.dependencyCount = 1;
	renderpassinfo.pDependencies = &dependency;

	VkResult result = vkCreateRenderPass(*ctx.getDevice(), &renderpassinfo, nullptr, &outPass);
	if(result != VK_SUCCESS) {
		gLoge("gVKRenderTarget") << "vkCreateRenderPass failed! VkResult: " << result;
		outPass = VK_NULL_HANDLE;
		return false;
	}
	return true;
}

bool gvkCreateRenderPass(gVKContext& ctx) {
	if(ctx.device == VK_NULL_HANDLE || ctx.swapchainformat == VK_FORMAT_UNDEFINED) {
		gLoge("gVKRenderTarget") << "Cannot create the render pass before the swapchain exists.";
		return false;
	}

	// The attachment description below has to name a concrete format, so the choice
	// is made here rather than in gvkCreateDepthResources.
	if(ctx.depthformat == VK_FORMAT_UNDEFINED) {
		ctx.depthformat = gvkFindDepthFormat(ctx);
		if(ctx.depthformat == VK_FORMAT_UNDEFINED) {
			gLoge("gVKRenderTarget") << "No supported depth attachment format found on this device.";
			return false;
		}
	}

	// And the sample count, for the same reason: it is baked into the attachments
	// here, into the images gvkCreateMsaaColorResources / gvkCreateDepthResources
	// allocate next, and into every pipeline built afterwards. This is the one place
	// the application's request meets what the hardware will actually do.
	const VkSampleCountFlagBits samples = gvkPickSampleCount(ctx, ctx.requestedsamples);
	if(ctx.requestedsamples > 1 && samples == VK_SAMPLE_COUNT_1_BIT) {
		gLogi("gVKRenderTarget") << "MSAA " << ctx.requestedsamples
				<< "x was requested but this device supports no multisampled colour+depth"
				<< " combination; continuing without it.";
	}
	ctx.samplecount = samples;

	if(!gvkBuildScreenRenderPass(ctx, ctx.samplecount, ctx.renderpass)) return false;

	// While the screen pass is multisampled, the pipelines that record into the
	// per-FBO passes still have to be built against *something* single-sampled, and
	// those passes do not exist yet - gFbo creates one on first bind, long after the
	// pipelines are up. So a 1x twin of the screen pass is kept purely as the
	// compatibility template to build them against. It is never begun and owns no
	// framebuffer. See the design note in gVKContext.h.
	if(ctx.isMultiSampled() && !gvkBuildScreenRenderPass(ctx, VK_SAMPLE_COUNT_1_BIT, ctx.singlesamplerenderpass)) {
		gvkDestroyRenderPass(ctx);
		return false;
	}

	gLogi("gVKRenderTarget") << "Render pass created with a cleared colour and depth attachment"
			<< (ctx.isMultiSampled()
					? ", " + std::to_string(static_cast<int>(ctx.samplecount)) + "x MSAA resolved to the swapchain image"
					: "");
	return true;
}

void gvkDestroyRenderPass(gVKContext& ctx) {
	if(ctx.device == VK_NULL_HANDLE) return;

	if(ctx.singlesamplerenderpass != VK_NULL_HANDLE) {
		vkDestroyRenderPass(ctx.device, ctx.singlesamplerenderpass, nullptr);
		ctx.singlesamplerenderpass = VK_NULL_HANDLE;
	}
	if(ctx.renderpass != VK_NULL_HANDLE) {
		vkDestroyRenderPass(ctx.device, ctx.renderpass, nullptr);
		ctx.renderpass = VK_NULL_HANDLE;
	}
}

bool gvkCreateFramebuffers(gVKContext& ctx) {
	if(ctx.device == VK_NULL_HANDLE || ctx.renderpass == VK_NULL_HANDLE) {
		gLoge("gVKRenderTarget") << "Cannot create the framebuffers before the render pass exists.";
		return false;
	}
	if(ctx.swapchainimageviews.empty()) {
		gLoge("gVKRenderTarget") << "Cannot create the framebuffers without swapchain image views.";
		return false;
	}
	if(ctx.depthimageviews.size() != GVK_MAX_FRAMES_IN_FLIGHT) {
		gLoge("gVKRenderTarget") << "Cannot create the framebuffers before the depth buffer exists.";
		return false;
	}
	if(ctx.isMultiSampled() && ctx.msaacolorviews.size() != GVK_MAX_FRAMES_IN_FLIGHT) {
		gLoge("gVKRenderTarget") << "Cannot create the framebuffers before the MSAA colour buffer exists.";
		return false;
	}

	const size_t imagecount = ctx.swapchainimageviews.size();
	ctx.framebuffers.resize(imagecount * GVK_MAX_FRAMES_IN_FLIGHT, VK_NULL_HANDLE);

	for(size_t frame = 0; frame < GVK_MAX_FRAMES_IN_FLIGHT; ++frame) {
	for(size_t i = 0; i < imagecount; i++) {
		// Same order as the render pass declares them. Multisampled, attachment 0 is
		// this frame slot's private MSAA image and the acquired swapchain image comes
		// last as the resolve target; single-sampled, the swapchain image is drawn
		// into directly and there is no third attachment.
		VkImageView attachments[3];
		uint32_t attachmentcount = 0;
		if(ctx.isMultiSampled()) attachments[attachmentcount++] = ctx.msaacolorviews[frame];
		else attachments[attachmentcount++] = ctx.swapchainimageviews[i];
		attachments[attachmentcount++] = ctx.depthimageviews[frame];
		if(ctx.isMultiSampled()) attachments[attachmentcount++] = ctx.swapchainimageviews[i];

		VkFramebufferCreateInfo framebufferinfo{};
		framebufferinfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferinfo.renderPass = ctx.renderpass;
		framebufferinfo.attachmentCount = attachmentcount;
		framebufferinfo.pAttachments = attachments;
		framebufferinfo.width = ctx.swapchainextent.width;
		framebufferinfo.height = ctx.swapchainextent.height;
		framebufferinfo.layers = 1;

		const size_t framebufferindex = frame * imagecount + i;
		VkResult result = vkCreateFramebuffer(ctx.device, &framebufferinfo, nullptr,
				&ctx.framebuffers[framebufferindex]);
		if(result != VK_SUCCESS) {
			gLoge("gVKRenderTarget") << "vkCreateFramebuffer failed for swapchain image " << i
					<< "! VkResult: " << result;
			gvkDestroyFramebuffers(ctx);
			return false;
		}
	}
	}

	gLogi("gVKRenderTarget") << "Framebuffers created: " << ctx.framebuffers.size()
			<< " at " << ctx.swapchainextent.width << "x" << ctx.swapchainextent.height
			<< " across " << GVK_MAX_FRAMES_IN_FLIGHT << " frame slots";
	return true;
}

void gvkDestroyFramebuffers(gVKContext& ctx) {
	if(ctx.device == VK_NULL_HANDLE) return;

	// Kept separate from the render pass on purpose: a resize rebuilds these while
	// the render pass stays valid, since the surface format does not change.
	for(VkFramebuffer framebuffer : ctx.framebuffers) {
		if(framebuffer != VK_NULL_HANDLE) vkDestroyFramebuffer(ctx.device, framebuffer, nullptr);
	}
	ctx.framebuffers.clear();
}

#endif /* GVK_VULKAN */
