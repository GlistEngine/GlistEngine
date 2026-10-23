/*
 * gVKDeferred.cpp
 *
 * Vulkan G-buffer allocation and geometry render-pass lifetime.
 */

#include "gVKDeferred.h"

#ifdef GVK_VULKAN

#include "gVKBuffer.h"
#include "gVKRenderTarget.h"
#include "gUtils.h"

#include <array>
#include <vector>

namespace {

struct gVKDeferredAttachment {
	VkImage image = VK_NULL_HANDLE;
	VkDeviceMemory memory = VK_NULL_HANDLE;
	VkImageView view = VK_NULL_HANDLE;
};

struct gVKDeferredFrame {
	std::array<gVKDeferredAttachment, 3> colors;
	gVKDeferredAttachment depth;
	VkFramebuffer framebuffer = VK_NULL_HANDLE;
};

bool supportsGBufferFormat(gVKContext& ctx, VkFormat format) {
	VkFormatProperties properties{};
	vkGetPhysicalDeviceFormatProperties(*ctx.getPhysicalDevice(), format, &properties);
	const VkFormatFeatureFlags required = VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT
			| VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT;
	return (properties.optimalTilingFeatures & required) == required;
}

bool createAttachment(gVKContext& ctx, uint32_t width, uint32_t height,
		VkFormat format, VkImageUsageFlags usage, VkImageAspectFlags aspect,
		gVKDeferredAttachment& attachment) {
	const VkDevice device = *ctx.getDevice();
	VkImageCreateInfo imageinfo{};
	imageinfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageinfo.imageType = VK_IMAGE_TYPE_2D;
	imageinfo.extent = {width, height, 1};
	imageinfo.mipLevels = 1;
	imageinfo.arrayLayers = 1;
	imageinfo.format = format;
	imageinfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageinfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageinfo.usage = usage;
	imageinfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageinfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	if(vkCreateImage(device, &imageinfo, nullptr, &attachment.image) != VK_SUCCESS) return false;

	VkMemoryRequirements requirements{};
	vkGetImageMemoryRequirements(device, attachment.image, &requirements);
	VkMemoryAllocateInfo allocation{};
	allocation.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocation.allocationSize = requirements.size;
	allocation.memoryTypeIndex = gvkFindMemoryType(ctx, requirements.memoryTypeBits,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	if(vkAllocateMemory(device, &allocation, nullptr, &attachment.memory) != VK_SUCCESS) return false;
	if(vkBindImageMemory(device, attachment.image, attachment.memory, 0) != VK_SUCCESS) return false;

	VkImageViewCreateInfo viewinfo{};
	viewinfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewinfo.image = attachment.image;
	viewinfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewinfo.format = format;
	viewinfo.subresourceRange.aspectMask = aspect;
	viewinfo.subresourceRange.levelCount = 1;
	viewinfo.subresourceRange.layerCount = 1;
	return vkCreateImageView(device, &viewinfo, nullptr, &attachment.view) == VK_SUCCESS;
}

void destroyAttachment(VkDevice device, gVKDeferredAttachment& attachment) {
	if(attachment.view != VK_NULL_HANDLE) vkDestroyImageView(device, attachment.view, nullptr);
	if(attachment.image != VK_NULL_HANDLE) vkDestroyImage(device, attachment.image, nullptr);
	if(attachment.memory != VK_NULL_HANDLE) vkFreeMemory(device, attachment.memory, nullptr);
	attachment = {};
}

} // namespace

struct gVKDeferredResources {
	VkExtent2D extent = {0, 0};
	// Match the existing OpenGL G-buffer: position is RGBA32F, normal/specular is
	// RGBA16F, and albedo is four normalized 8-bit channels.
	VkFormat positionformat = VK_FORMAT_R32G32B32A32_SFLOAT;
	VkFormat normalformat = VK_FORMAT_R16G16B16A16_SFLOAT;
	VkFormat albedoformat = VK_FORMAT_R8G8B8A8_UNORM;
	VkFormat depthformat = VK_FORMAT_UNDEFINED;
	VkRenderPass renderpass = VK_NULL_HANDLE;
	std::vector<gVKDeferredFrame> frames;
	VkSampler sampler = VK_NULL_HANDLE;
	std::vector<VkDescriptorSet> descriptorsets;
};

static bool createDeferredRenderPass(gVKContext& ctx, gVKDeferredResources& resources) {
	VkAttachmentDescription attachments[4]{};
	const VkFormat colorformats[3] = {
		resources.positionformat, resources.normalformat, resources.albedoformat
	};
	VkAttachmentReference colorrefs[3]{};
	for(uint32_t i = 0; i < 3; ++i) {
		attachments[i].format = colorformats[i];
		attachments[i].samples = VK_SAMPLE_COUNT_1_BIT;
		attachments[i].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachments[i].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachments[i].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachments[i].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachments[i].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attachments[i].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		colorrefs[i].attachment = i;
		colorrefs[i].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	}

	attachments[3].format = resources.depthformat;
	attachments[3].samples = VK_SAMPLE_COUNT_1_BIT;
	attachments[3].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachments[3].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachments[3].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[3].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[3].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachments[3].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	VkAttachmentReference depthref{3, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};

	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 3;
	subpass.pColorAttachments = colorrefs;
	subpass.pDepthStencilAttachment = &depthref;

	VkSubpassDependency dependencies[2]{};
	dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[0].dstSubpass = 0;
	dependencies[0].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	dependencies[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
	dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
			| VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
			| VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
	dependencies[1].srcSubpass = 0;
	dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	VkRenderPassCreateInfo passinfo{};
	passinfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	passinfo.attachmentCount = 4;
	passinfo.pAttachments = attachments;
	passinfo.subpassCount = 1;
	passinfo.pSubpasses = &subpass;
	passinfo.dependencyCount = 2;
	passinfo.pDependencies = dependencies;
	if(vkCreateRenderPass(*ctx.getDevice(), &passinfo, nullptr, &resources.renderpass) != VK_SUCCESS) {
		gLoge("gVKDeferred") << "vkCreateRenderPass failed for the G-buffer.";
		return false;
	}
	return true;
}

gVKDeferredResources* gvkCreateDeferredResources(gVKContext& ctx,
		uint32_t width, uint32_t height) {
	if(*ctx.getDevice() == VK_NULL_HANDLE || width == 0 || height == 0) return nullptr;
	auto* resources = new gVKDeferredResources();
	resources->extent = {width, height};
	resources->depthformat = gvkFindDepthFormat(ctx);
	if(resources->depthformat == VK_FORMAT_UNDEFINED
			|| !supportsGBufferFormat(ctx, resources->positionformat)
			|| !supportsGBufferFormat(ctx, resources->normalformat)
			|| !supportsGBufferFormat(ctx, resources->albedoformat)) {
		gLoge("gVKDeferred") << "This device does not support the required G-buffer formats.";
		delete resources;
		return nullptr;
	}
	if(!createDeferredRenderPass(ctx, *resources)) {
		gvkDestroyDeferredResources(ctx, resources);
		return nullptr;
	}

	resources->frames.resize(GVK_MAX_FRAMES_IN_FLIGHT);
	const VkFormat colorformats[3] = {
		resources->positionformat, resources->normalformat, resources->albedoformat
	};
	for(size_t frameindex = 0; frameindex < resources->frames.size(); ++frameindex) {
		gVKDeferredFrame& frame = resources->frames[frameindex];
		for(uint32_t colorindex = 0; colorindex < 3; ++colorindex) {
			if(!createAttachment(ctx, width, height, colorformats[colorindex],
					VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
					VK_IMAGE_ASPECT_COLOR_BIT, frame.colors[colorindex])) {
				gLoge("gVKDeferred") << "Could not create G-buffer color attachment "
						<< colorindex << " for frame slot " << frameindex << ".";
				gvkDestroyDeferredResources(ctx, resources);
				return nullptr;
			}
		}
		if(!createAttachment(ctx, width, height, resources->depthformat,
				VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
				VK_IMAGE_ASPECT_DEPTH_BIT, frame.depth)) {
			gLoge("gVKDeferred") << "Could not create G-buffer depth attachment for frame slot "
					<< frameindex << ".";
			gvkDestroyDeferredResources(ctx, resources);
			return nullptr;
		}

		VkImageView views[4] = {
			frame.colors[0].view, frame.colors[1].view, frame.colors[2].view, frame.depth.view
		};
		VkFramebufferCreateInfo framebufferinfo{};
		framebufferinfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferinfo.renderPass = resources->renderpass;
		framebufferinfo.attachmentCount = 4;
		framebufferinfo.pAttachments = views;
		framebufferinfo.width = width;
		framebufferinfo.height = height;
		framebufferinfo.layers = 1;
		if(vkCreateFramebuffer(*ctx.getDevice(), &framebufferinfo, nullptr,
				&frame.framebuffer) != VK_SUCCESS) {
			gLoge("gVKDeferred") << "Could not create G-buffer framebuffer for frame slot "
					<< frameindex << ".";
			gvkDestroyDeferredResources(ctx, resources);
			return nullptr;
		}
	}
	return resources;
}

void gvkDestroyDeferredResources(gVKContext& ctx, gVKDeferredResources*& resources) {
	if(resources == nullptr) return;
	const VkDevice device = *ctx.getDevice();
	if(device != VK_NULL_HANDLE) {
		gvkReleaseDeferredDescriptors(ctx, *resources);
		for(gVKDeferredFrame& frame : resources->frames) {
			if(frame.framebuffer != VK_NULL_HANDLE) {
				vkDestroyFramebuffer(device, frame.framebuffer, nullptr);
				frame.framebuffer = VK_NULL_HANDLE;
			}
			for(gVKDeferredAttachment& color : frame.colors) destroyAttachment(device, color);
			destroyAttachment(device, frame.depth);
		}
		if(resources->renderpass != VK_NULL_HANDLE) {
			vkDestroyRenderPass(device, resources->renderpass, nullptr);
			resources->renderpass = VK_NULL_HANDLE;
		}
	}
	delete resources;
	resources = nullptr;
}

bool gvkCreateDeferredDescriptors(gVKContext& ctx, gVKDeferredResources& resources,
		VkDescriptorSetLayout layout) {
	const VkDevice device = *ctx.getDevice();
	if(device == VK_NULL_HANDLE || layout == VK_NULL_HANDLE
			|| ctx.getDescriptorPool() == VK_NULL_HANDLE || resources.frames.empty()) return false;
	gvkReleaseDeferredDescriptors(ctx, resources);

	VkSamplerCreateInfo samplerinfo{};
	samplerinfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerinfo.magFilter = VK_FILTER_NEAREST;
	samplerinfo.minFilter = VK_FILTER_NEAREST;
	samplerinfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
	samplerinfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerinfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerinfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerinfo.minLod = 0.0f;
	samplerinfo.maxLod = 0.0f;
	if(vkCreateSampler(device, &samplerinfo, nullptr, &resources.sampler) != VK_SUCCESS) {
		gLoge("gVKDeferred") << "Could not create the G-buffer sampler.";
		return false;
	}

	resources.descriptorsets.resize(resources.frames.size(), VK_NULL_HANDLE);
	std::vector<VkDescriptorSetLayout> layouts(resources.frames.size(), layout);
	VkDescriptorSetAllocateInfo allocinfo{};
	allocinfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocinfo.descriptorPool = ctx.getDescriptorPool();
	allocinfo.descriptorSetCount = static_cast<uint32_t>(layouts.size());
	allocinfo.pSetLayouts = layouts.data();
	if(vkAllocateDescriptorSets(device, &allocinfo, resources.descriptorsets.data()) != VK_SUCCESS) {
		gLoge("gVKDeferred") << "Could not allocate the G-buffer descriptor sets.";
		// A failed allocation does not produce descriptor-set handles that may be
		// passed to vkFreeDescriptorSets; only release the sampler and local vector.
		gvkReleaseDeferredDescriptors(ctx, resources, false);
		return false;
	}

	for(size_t frameindex = 0; frameindex < resources.frames.size(); ++frameindex) {
		VkDescriptorImageInfo images[3]{};
		VkWriteDescriptorSet writes[3]{};
		for(uint32_t attachment = 0; attachment < 3; ++attachment) {
			images[attachment].sampler = resources.sampler;
			images[attachment].imageView = resources.frames[frameindex].colors[attachment].view;
			images[attachment].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			writes[attachment].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			writes[attachment].dstSet = resources.descriptorsets[frameindex];
			writes[attachment].dstBinding = attachment;
			writes[attachment].descriptorCount = 1;
			writes[attachment].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			writes[attachment].pImageInfo = &images[attachment];
		}
		vkUpdateDescriptorSets(device, 3, writes, 0, nullptr);
	}
	return true;
}

void gvkReleaseDeferredDescriptors(gVKContext& ctx, gVKDeferredResources& resources,
		bool freeSets) {
	const VkDevice device = *ctx.getDevice();
	if(device == VK_NULL_HANDLE) return;
	if(freeSets && !resources.descriptorsets.empty()
			&& ctx.getDescriptorPool() != VK_NULL_HANDLE) {
		vkFreeDescriptorSets(device, ctx.getDescriptorPool(),
				static_cast<uint32_t>(resources.descriptorsets.size()),
				resources.descriptorsets.data());
	}
	resources.descriptorsets.clear();
	if(resources.sampler != VK_NULL_HANDLE) {
		vkDestroySampler(device, resources.sampler, nullptr);
		resources.sampler = VK_NULL_HANDLE;
	}
}

VkDescriptorSet gvkGetDeferredDescriptorSet(const gVKDeferredResources& resources,
		uint32_t frame) {
	return frame < resources.descriptorsets.size()
			? resources.descriptorsets[frame] : VK_NULL_HANDLE;
}

bool gvkBeginDeferredPass(gVKContext& ctx, gVKDeferredResources& resources) {
	if(!ctx.frameactive || ctx.shadowpassactive || ctx.deferredpassactive) return false;
	if(ctx.currentframe >= resources.frames.size()) return false;
	VkCommandBuffer commandbuffer = ctx.commandbuffers[ctx.currentframe];
	if(commandbuffer == VK_NULL_HANDLE) return false;

	// Deferred geometry may follow ordinary screen rendering. Close that pass but
	// keep screenpassbegun set, so the later lighting pass reopens without clearing.
	if(ctx.renderpassactive) {
		vkCmdEndRenderPass(commandbuffer);
		ctx.renderpassactive = false;
	}

	VkClearValue clearvalues[4]{};
	clearvalues[3].depthStencil = {1.0f, 0};
	VkRenderPassBeginInfo begininfo{};
	begininfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	begininfo.renderPass = resources.renderpass;
	begininfo.framebuffer = resources.frames[ctx.currentframe].framebuffer;
	begininfo.renderArea.offset = {0, 0};
	begininfo.renderArea.extent = resources.extent;
	begininfo.clearValueCount = 4;
	begininfo.pClearValues = clearvalues;
	vkCmdBeginRenderPass(commandbuffer, &begininfo, VK_SUBPASS_CONTENTS_INLINE);

	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(resources.extent.width);
	viewport.height = static_cast<float>(resources.extent.height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(commandbuffer, 0, 1, &viewport);
	VkRect2D scissor{{0, 0}, resources.extent};
	vkCmdSetScissor(commandbuffer, 0, 1, &scissor);

	ctx.currentpassextent = resources.extent;
	// Like every other offscreen target, keep OpenGL's bottom-left texture
	// convention. The fullscreen lighting pass can then use the same UV table as
	// the existing post-process path.
	ctx.passflipsy = false;
	ctx.renderpassactive = true;
	ctx.deferredpassactive = true;
	ctx.useOffscreenPipelines();
	ctx.resetRecordedDrawState();
	return true;
}

void gvkEndDeferredPass(gVKContext& ctx) {
	if(!ctx.deferredpassactive) return;
	vkCmdEndRenderPass(ctx.commandbuffers[ctx.currentframe]);
	ctx.deferredpassactive = false;
	ctx.renderpassactive = false;
	ctx.passflipsy = true;
}

bool gvkRestoreDeferredDepth(gVKContext& ctx, gVKDeferredResources& resources) {
	if(!ctx.frameactive || ctx.deferredpassactive || ctx.shadowpassactive
			|| !ctx.renderpassactive || ctx.isMultiSampled()) return false;
	if(ctx.currentframe >= resources.frames.size()
			|| ctx.currentframe >= ctx.depthimages.size()) return false;
	if(resources.extent.width != ctx.swapchainextent.width
			|| resources.extent.height != ctx.swapchainextent.height) {
		static bool reported = false;
		if(!reported) {
			reported = true;
			gLogw("gVKDeferred") << "Cannot preserve deferred depth because the G-buffer and"
					<< " default target sizes differ.";
		}
		return false;
	}
	VkFormatProperties depthproperties{};
	vkGetPhysicalDeviceFormatProperties(ctx.physicaldevice, resources.depthformat,
			&depthproperties);
	const VkFormatFeatureFlags blitfeatures = VK_FORMAT_FEATURE_BLIT_SRC_BIT
			| VK_FORMAT_FEATURE_BLIT_DST_BIT;
	if((depthproperties.optimalTilingFeatures & blitfeatures) != blitfeatures) {
		static bool reported = false;
		if(!reported) {
			reported = true;
			gLogw("gVKDeferred") << "This device cannot blit the selected depth format;"
					<< " forward geometry after deferred lighting will not share its depth.";
		}
		return false;
	}

	VkCommandBuffer cmd = ctx.commandbuffers[ctx.currentframe];
	VkImage source = resources.frames[ctx.currentframe].depth.image;
	VkImage destination = ctx.depthimages[ctx.currentframe];
	if(cmd == VK_NULL_HANDLE || source == VK_NULL_HANDLE || destination == VK_NULL_HANDLE) return false;

	// The fullscreen draw has finished writing colour. Close its pass so the
	// destination depth attachment can become a transfer destination; colour lands
	// in PRESENT_SRC_KHR and will be loaded by the next screen-pass reopen.
	vkCmdEndRenderPass(cmd);
	ctx.renderpassactive = false;

	VkImageMemoryBarrier barriers[2]{};
	for(VkImageMemoryBarrier& barrier : barriers) {
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.layerCount = 1;
		barrier.oldLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	}
	barriers[0].srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	barriers[0].dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
	barriers[0].newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
	barriers[0].image = source;
	barriers[1].srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	barriers[1].dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barriers[1].newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barriers[1].image = destination;
	vkCmdPipelineBarrier(cmd,
			VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT
					| VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			0, 0, nullptr, 0, nullptr, 2, barriers);

	// G-buffer rendering keeps the engine's OpenGL-style offscreen orientation,
	// while the default Vulkan target uses a negative-height viewport. The
	// fullscreen lighting quad accounts for that in its UVs; mirror Y here as well
	// so its colour and the restored depth describe the same screen pixels.
	VkImageBlit blit{};
	blit.srcSubresource = {VK_IMAGE_ASPECT_DEPTH_BIT, 0, 0, 1};
	blit.srcOffsets[1] = {
			static_cast<int32_t>(resources.extent.width),
			static_cast<int32_t>(resources.extent.height), 1};
	blit.dstSubresource = {VK_IMAGE_ASPECT_DEPTH_BIT, 0, 0, 1};
	blit.dstOffsets[0] = {0, static_cast<int32_t>(resources.extent.height), 0};
	blit.dstOffsets[1] = {static_cast<int32_t>(resources.extent.width), 0, 1};
	vkCmdBlitImage(cmd, source, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			destination, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit, VK_FILTER_NEAREST);

	barriers[0].srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
	barriers[0].dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT
			| VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	barriers[0].oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
	barriers[0].newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	barriers[1].srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barriers[1].dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT
			| VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	barriers[1].oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barriers[1].newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	vkCmdPipelineBarrier(cmd,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT
					| VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
			0, 0, nullptr, 0, nullptr, 2, barriers);

	ctx.resetRecordedDrawState();
	return true;
}

VkRenderPass gvkGetDeferredRenderPass(const gVKDeferredResources& resources) {
	return resources.renderpass;
}

VkImageView gvkGetDeferredColorView(const gVKDeferredResources& resources,
		uint32_t frame, uint32_t attachment) {
	if(frame >= resources.frames.size() || attachment >= 3) return VK_NULL_HANDLE;
	return resources.frames[frame].colors[attachment].view;
}

VkExtent2D gvkGetDeferredExtent(const gVKDeferredResources& resources) {
	return resources.extent;
}

#endif /* GVK_VULKAN */
