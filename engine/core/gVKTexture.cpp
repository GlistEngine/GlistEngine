/*
 * gVKTexture.cpp
 */

#include "gVKTexture.h"

#include <algorithm>

#ifdef GVK_VULKAN

#include "gVKBuffer.h"
#include "gUtils.h"
#include <cstring>

// Records a layout transition for a colour image with a full pipeline barrier.
// Kept simple (the transfer path is one-off), so the stage masks are the broad
// TRANSFER / FRAGMENT ones the upload actually needs.
// Number of mip levels a texture of this size carries: halving the larger side
// until it reaches 1, counting level 0. A 667x80 image gives 10.
static uint32_t gvkMipLevelsFor(int width, int height) {
	uint32_t levels = 1;
	int largest = width > height ? width : height;
	while(largest > 1) {
		largest /= 2;
		levels++;
	}
	return levels;
}

// baseLevel / levelCount select which part of the mip chain moves; the defaults
// cover a single-level image, which is every caller that predates mipmapping.
static void gvkTransitionImageLayout(VkCommandBuffer cmd, VkImage image,
		VkImageLayout oldLayout, VkImageLayout newLayout,
		VkAccessFlags srcAccess, VkAccessFlags dstAccess,
		VkPipelineStageFlags srcStage, VkPipelineStageFlags dstStage,
		uint32_t baseLevel = 0, uint32_t levelCount = 1) {
	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = image;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = baseLevel;
	barrier.subresourceRange.levelCount = levelCount;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;
	barrier.srcAccessMask = srcAccess;
	barrier.dstAccessMask = dstAccess;
	vkCmdPipelineBarrier(cmd, srcStage, dstStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
}

// Builds a sampler for the given filtering and wrapping. Only one mip level is ever
// uploaded, so the mipmap mode follows the minification filter and stays consistent
// with it rather than being a third setting.
static uint64_t gvkSamplerKey(VkFilter minFilter, VkFilter magFilter,
		VkSamplerAddressMode addressU, VkSamplerAddressMode addressV,
		bool useMipmaps, VkSamplerMipmapMode mipmapMode) {
	return static_cast<uint64_t>(minFilter)
			| (static_cast<uint64_t>(magFilter) << 8)
			| (static_cast<uint64_t>(addressU) << 16)
			| (static_cast<uint64_t>(addressV) << 24)
			| (static_cast<uint64_t>(useMipmaps ? 1 : 0) << 32)
			| (static_cast<uint64_t>(mipmapMode) << 40);
}

static VkSampler gvkAcquireSampler(gVKContext& ctx, VkFilter minFilter, VkFilter magFilter,
		VkSamplerAddressMode addressU, VkSamplerAddressMode addressV,
		bool useMipmaps, VkSamplerMipmapMode mipmapMode, uint64_t& outKey) {
	outKey = gvkSamplerKey(minFilter, magFilter, addressU, addressV, useMipmaps, mipmapMode);
	auto& cache = ctx.getSamplerCache();
	auto found = cache.find(outKey);
	if(found != cache.end()) {
		++found->second.second;
		return found->second.first;
	}
	VkSamplerCreateInfo samplerinfo{};
	samplerinfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerinfo.magFilter = magFilter;
	samplerinfo.minFilter = minFilter;
	samplerinfo.addressModeU = addressU;
	samplerinfo.addressModeV = addressV;
	samplerinfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerinfo.mipmapMode = mipmapMode;
	// maxLod defaults to 0, which pins sampling to level 0 and would make the mip
	// chain built at upload time dead weight. CLAMP_NONE lets every level be
	// reached - but only when the application asked for a mipmapping filter, since
	// plain GL_LINEAR means level 0 alone.
	samplerinfo.minLod = 0.0f;
	samplerinfo.maxLod = useMipmaps ? VK_LOD_CLAMP_NONE : 0.0f;
	samplerinfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	// Anisotropy is a rule for choosing among mip levels, so it is only asked for
	// where there is a chain to choose from; a sampler pinned to level 0 would gain
	// nothing from it. Capped at 8 rather than taken at the device maximum, which is
	// usually 16: the difference between the two is not visible at these texture
	// sizes and the extra taps are bandwidth a mobile tiler would rather keep.
	// gVKRenderEngine enables the feature when the device offers it, and leaves it
	// off otherwise, which is why this checks before asking.
	const VkPhysicalDeviceFeatures* features = ctx.getDeviceFeatures();
	if(useMipmaps && features != nullptr && features->samplerAnisotropy == VK_TRUE) {
		samplerinfo.anisotropyEnable = VK_TRUE;
		samplerinfo.maxAnisotropy = std::min(8.0f, ctx.getDeviceProperties()->limits.maxSamplerAnisotropy);
	}
	VkSampler sampler = VK_NULL_HANDLE;
	vkCreateSampler(*ctx.getDevice(), &samplerinfo, nullptr, &sampler);
	if(sampler != VK_NULL_HANDLE) cache.emplace(outKey, std::make_pair(sampler, 1u));
	return sampler;
}

static void gvkReleaseSampler(gVKContext& ctx, uint64_t key, VkSampler sampler) {
	if(sampler == VK_NULL_HANDLE) return;
	auto& cache = ctx.getSamplerCache();
	auto found = cache.find(key);
	if(found == cache.end()) {
		vkDestroySampler(*ctx.getDevice(), sampler, nullptr);
		return;
	}
	if(--found->second.second == 0) {
		vkDestroySampler(*ctx.getDevice(), found->second.first, nullptr);
		cache.erase(found);
	}
}

gVKTexture* gvkCreateTextureRGBA8(gVKContext& ctx, const void* rgbaPixels, int width, int height) {
	if(width <= 0 || height <= 0 || rgbaPixels == nullptr) return nullptr;
	VkDevice device = *ctx.getDevice();
	VkDeviceSize imagesize = static_cast<VkDeviceSize>(width) * height * 4;

	// Staging buffer holds the pixels while they are copied to the device-local image.
	VkBuffer staging = VK_NULL_HANDLE;
	VkDeviceMemory stagingmem = VK_NULL_HANDLE;
	if(!gvkCreateBuffer(ctx, imagesize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, staging, stagingmem)) {
		gLoge("gVKTexture") << "Could not create the texture staging buffer.";
		return nullptr;
	}
	void* mapped = nullptr;
	if(vkMapMemory(device, stagingmem, 0, imagesize, 0, &mapped) != VK_SUCCESS) {
		gLoge("gVKTexture") << "Could not map the texture staging memory.";
		vkDestroyBuffer(device, staging, nullptr);
		vkFreeMemory(device, stagingmem, nullptr);
		return nullptr;
	}
	std::memcpy(mapped, rgbaPixels, static_cast<size_t>(imagesize));
	vkUnmapMemory(device, stagingmem);

	gVKTexture* tex = new gVKTexture();
	tex->width = width;
	tex->height = height;
	// A full mip chain, the same as the OpenGL path's glGenerateMipmap. This only
	// shows up in 3D: a 2D image is drawn at its own size, so level 0 is all that is
	// ever sampled, but a textured mesh seen at an angle or in the distance samples
	// far below level 0 and without a chain aliases badly against the OpenGL result.
	tex->miplevels = gvkMipLevelsFor(width, height);

	VkImageCreateInfo imageinfo{};
	imageinfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageinfo.imageType = VK_IMAGE_TYPE_2D;
	imageinfo.extent = {static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1};
	imageinfo.mipLevels = tex->miplevels;
	imageinfo.arrayLayers = 1;
	// Keep file pixels as display-ready values, matching the OpenGL texture path
	// and the UNORM swapchain. No implicit sRGB decode is performed while sampling.
	imageinfo.format = VK_FORMAT_R8G8B8A8_UNORM;
	imageinfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageinfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	// TRANSFER_SRC as well, because each mip level is produced by blitting from the
	// level above it.
	imageinfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
			VK_IMAGE_USAGE_SAMPLED_BIT;
	imageinfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageinfo.samples = VK_SAMPLE_COUNT_1_BIT;
	if(vkCreateImage(device, &imageinfo, nullptr, &tex->image) != VK_SUCCESS) {
		gLoge("gVKTexture") << "vkCreateImage failed.";
		vkDestroyBuffer(device, staging, nullptr);
		vkFreeMemory(device, stagingmem, nullptr);
		delete tex;
		return nullptr;
	}

	VkMemoryRequirements memreq;
	vkGetImageMemoryRequirements(device, tex->image, &memreq);
	VkMemoryAllocateInfo allocinfo{};
	allocinfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocinfo.allocationSize = memreq.size;
	allocinfo.memoryTypeIndex = gvkFindMemoryType(ctx, memreq.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	if(vkAllocateMemory(device, &allocinfo, nullptr, &tex->memory) != VK_SUCCESS) {
		gLoge("gVKTexture") << "vkAllocateMemory failed for an uploaded texture.";
		vkDestroyBuffer(device, staging, nullptr);
		vkFreeMemory(device, stagingmem, nullptr);
		gvkDestroyTexture(ctx, tex);
		return nullptr;
	}
	if(vkBindImageMemory(device, tex->image, tex->memory, 0) != VK_SUCCESS) {
		gLoge("gVKTexture") << "vkBindImageMemory failed for an uploaded texture.";
		vkDestroyBuffer(device, staging, nullptr);
		vkFreeMemory(device, stagingmem, nullptr);
		gvkDestroyTexture(ctx, tex);
		return nullptr;
	}

	// Upload: undefined -> transfer dst, copy into level 0, build the rest of the
	// chain from it, then the whole chain -> shader read. This is recorded into the
	// shared upload batch, so it is not submitted here and the staging buffer stays
	// alive until the batch that reads it has finished on the GPU.
	VkCommandBuffer cmd = gvkBeginUpload(ctx);
	if(cmd == VK_NULL_HANDLE) {
		gLoge("gVKTexture") << "Could not open an upload batch for a texture.";
		vkDestroyBuffer(device, staging, nullptr);
		vkFreeMemory(device, stagingmem, nullptr);
		gvkDestroyTexture(ctx, tex);
		return nullptr;
	}
	gvkTransitionImageLayout(cmd, tex->image,
			VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			0, VK_ACCESS_TRANSFER_WRITE_BIT,
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
			0, tex->miplevels);

	VkBufferImageCopy region{};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;
	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;
	region.imageOffset = {0, 0, 0};
	region.imageExtent = {static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1};
	vkCmdCopyBufferToImage(cmd, staging, tex->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

	// Each level is half the previous one, produced by a linear blit. The source
	// level has to be in TRANSFER_SRC while the destination stays TRANSFER_DST, so
	// levels are moved one at a time rather than as a whole.
	int32_t mipwidth = width;
	int32_t mipheight = height;
	for(uint32_t level = 1; level < tex->miplevels; level++) {
		gvkTransitionImageLayout(cmd, tex->image,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_TRANSFER_READ_BIT,
				VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
				level - 1, 1);

		// A dimension that has already reached 1 stays there; only the other one
		// keeps halving, which is what makes a non-square texture end at 1x1.
		const int32_t nextwidth = mipwidth > 1 ? mipwidth / 2 : 1;
		const int32_t nextheight = mipheight > 1 ? mipheight / 2 : 1;

		VkImageBlit blit{};
		blit.srcOffsets[0] = {0, 0, 0};
		blit.srcOffsets[1] = {mipwidth, mipheight, 1};
		blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.srcSubresource.mipLevel = level - 1;
		blit.srcSubresource.baseArrayLayer = 0;
		blit.srcSubresource.layerCount = 1;
		blit.dstOffsets[0] = {0, 0, 0};
		blit.dstOffsets[1] = {nextwidth, nextheight, 1};
		blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.dstSubresource.mipLevel = level;
		blit.dstSubresource.baseArrayLayer = 0;
		blit.dstSubresource.layerCount = 1;
		vkCmdBlitImage(cmd,
				tex->image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				tex->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				1, &blit, VK_FILTER_LINEAR);

		// This level is finished as a source; hand it to the shader now so only the
		// still-unwritten levels remain in TRANSFER_DST.
		gvkTransitionImageLayout(cmd, tex->image,
				VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
				VK_ACCESS_TRANSFER_READ_BIT, VK_ACCESS_SHADER_READ_BIT,
				VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
				level - 1, 1);

		mipwidth = nextwidth;
		mipheight = nextheight;
	}

	// The last level was never a blit source, so it is still in TRANSFER_DST.
	gvkTransitionImageLayout(cmd, tex->image,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
			tex->miplevels - 1, 1);
	// The batch owns the staging buffer from here on and destroys it once its
	// submission has finished; destroying it here would free memory the GPU is
	// about to read.
	gvkEndUpload(ctx, staging, stagingmem, imagesize);

	VkImageViewCreateInfo viewinfo{};
	viewinfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewinfo.image = tex->image;
	viewinfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewinfo.format = VK_FORMAT_R8G8B8A8_UNORM;
	viewinfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	viewinfo.subresourceRange.baseMipLevel = 0;
	viewinfo.subresourceRange.levelCount = tex->miplevels;
	viewinfo.subresourceRange.baseArrayLayer = 0;
	viewinfo.subresourceRange.layerCount = 1;
	if(vkCreateImageView(device, &viewinfo, nullptr, &tex->view) != VK_SUCCESS) {
		gLoge("gVKTexture") << "vkCreateImageView failed for an uploaded texture.";
		gvkDestroyTexture(ctx, tex);
		return nullptr;
	}

	// The sampler starts from gTexture's own defaults; setFiltering / setWrapping
	// rebuild it through gvkSetTextureSampler when the texture asks for something
	// else, which the upload path does right after this returns.
	tex->sampler = gvkAcquireSampler(ctx, tex->minfilter, tex->magfilter, tex->addressu, tex->addressv,
			tex->usemipmaps, tex->mipmapmode, tex->samplerkey);
	if(tex->sampler == VK_NULL_HANDLE) {
		gLoge("gVKTexture") << "vkCreateSampler failed for an uploaded texture.";
		gvkDestroyTexture(ctx, tex);
		return nullptr;
	}

	if(!gvkWriteTextureDescriptorSet(ctx, tex)) {
		gvkDestroyTexture(ctx, tex);
		return nullptr;
	}
	return tex;
}

bool gvkSetTextureSampler(gVKContext& ctx, gVKTexture* tex, VkFilter minFilter, VkFilter magFilter,
		VkSamplerAddressMode addressU, VkSamplerAddressMode addressV,
		bool useMipmaps, VkSamplerMipmapMode mipmapMode) {
	if(tex == nullptr) return false;
	if(tex->minfilter == minFilter && tex->magfilter == magFilter
			&& tex->addressu == addressU && tex->addressv == addressV
			&& tex->usemipmaps == useMipmaps && tex->mipmapmode == mipmapMode) {
		return false;
	}
	VkDevice device = *ctx.getDevice();
	if(device == VK_NULL_HANDLE || tex->sampler == VK_NULL_HANDLE) return false;
	// A frame that is still in flight may sample through the old sampler. Newly
	// loaded textures have not reached a draw yet, however, and draining the whole
	// device for every filter/wrap setter turns a large level load into hundreds of
	// serial GPU stalls (especially expensive through MoltenVK). Only synchronize
	// when this texture has actually been submitted before.
	uint64_t newsamplerkey = 0;
	VkSampler newsampler = gvkAcquireSampler(ctx, minFilter, magFilter, addressU, addressV,
			useMipmaps, mipmapMode, newsamplerkey);
	if(newsampler == VK_NULL_HANDLE) {
		gLoge("gVKTexture") << "vkCreateSampler failed while changing texture sampling.";
		return false;
	}
	if(tex->sampled) vkDeviceWaitIdle(device);
	gvkReleaseSampler(ctx, tex->samplerkey, tex->sampler);
	tex->sampler = newsampler;
	tex->samplerkey = newsamplerkey;
	tex->minfilter = minFilter;
	tex->magfilter = magFilter;
	tex->addressu = addressU;
	tex->addressv = addressV;
	tex->usemipmaps = useMipmaps;
	tex->mipmapmode = mipmapMode;
	if(tex->descriptorset != VK_NULL_HANDLE) {
		VkDescriptorImageInfo imginfo{};
		imginfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imginfo.imageView = tex->view;
		imginfo.sampler = tex->sampler;
		VkWriteDescriptorSet write{};
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.dstSet = tex->descriptorset;
		write.dstBinding = 0;
		write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		write.descriptorCount = 1;
		write.pImageInfo = &imginfo;
		vkUpdateDescriptorSets(device, 1, &write, 0, nullptr);
	}
	return true;
}

bool gvkWriteTextureDescriptorSet(gVKContext& ctx, gVKTexture* tex) {
	if(tex == nullptr || tex->view == VK_NULL_HANDLE) return false;
	VkDevice device = *ctx.getDevice();
	VkDescriptorSetLayout layout = ctx.getImageDescriptorSetLayout();
	if(device == VK_NULL_HANDLE || layout == VK_NULL_HANDLE || ctx.getDescriptorPool() == VK_NULL_HANDLE) {
		return false;
	}

	VkDescriptorSetAllocateInfo dsalloc{};
	dsalloc.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	dsalloc.descriptorPool = ctx.getDescriptorPool();
	dsalloc.descriptorSetCount = 1;
	dsalloc.pSetLayouts = &layout;
	const VkResult allocresult = vkAllocateDescriptorSets(device, &dsalloc, &tex->descriptorset);
	if(allocresult != VK_SUCCESS) {
		gLoge("gVKTexture") << "vkAllocateDescriptorSets failed, VkResult="
				<< static_cast<int>(allocresult) << ".";
		tex->descriptorset = VK_NULL_HANDLE;
		return false;
	}

	VkDescriptorImageInfo imginfo{};
	imginfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imginfo.imageView = tex->view;
	imginfo.sampler = tex->sampler;
	VkWriteDescriptorSet write{};
	write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write.dstSet = tex->descriptorset;
	write.dstBinding = 0;
	write.dstArrayElement = 0;
	write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	write.descriptorCount = 1;
	write.pImageInfo = &imginfo;
	vkUpdateDescriptorSets(device, 1, &write, 0, nullptr);
	return true;
}

gVKTexture* gvkCreateAttachmentTexture(gVKContext& ctx, int width, int height, bool depth) {
	VkDevice device = *ctx.getDevice();
	if(device == VK_NULL_HANDLE || width <= 0 || height <= 0) return nullptr;

	gVKTexture* tex = new gVKTexture();
	tex->width = width;
	tex->height = height;
	tex->miplevels = 1;
	tex->isattachment = true;
	// The swapchain's own formats, not this file's usual R8G8B8A8. Vulkan pipelines
	// are built against a render pass, and a pipeline can only be recorded into a
	// pass its attachments are compatible with - so an offscreen target has to carry
	// the same formats as the screen or none of the existing pipelines could draw
	// into it. The depth one is what the shadow map already settled on.
	tex->format = depth ? gvkFindDepthFormat(ctx) : *ctx.getSwapchainFormat();
	tex->aspect = depth ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
	tex->layout = VK_IMAGE_LAYOUT_UNDEFINED;
	if(tex->format == VK_FORMAT_UNDEFINED) { delete tex; return nullptr; }

	VkImageCreateInfo imageinfo{};
	imageinfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageinfo.imageType = VK_IMAGE_TYPE_2D;
	imageinfo.extent = {static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1};
	imageinfo.mipLevels = 1;
	imageinfo.arrayLayers = 1;
	imageinfo.format = tex->format;
	imageinfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageinfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	// SAMPLED as well as the attachment usage: the whole point of an FBO here is to
	// draw into it and then read it back as a texture.
	imageinfo.usage = (depth ? VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT
			: VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) | VK_IMAGE_USAGE_SAMPLED_BIT;
	imageinfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageinfo.samples = VK_SAMPLE_COUNT_1_BIT;
	if(vkCreateImage(device, &imageinfo, nullptr, &tex->image) != VK_SUCCESS) {
		gLoge("gVKTexture") << "vkCreateImage failed for a framebuffer attachment.";
		delete tex;
		return nullptr;
	}

	VkMemoryRequirements requirements{};
	vkGetImageMemoryRequirements(device, tex->image, &requirements);
	VkMemoryAllocateInfo allocinfo{};
	allocinfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocinfo.allocationSize = requirements.size;
	allocinfo.memoryTypeIndex = gvkFindMemoryType(ctx, requirements.memoryTypeBits,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	if(vkAllocateMemory(device, &allocinfo, nullptr, &tex->memory) != VK_SUCCESS) {
		gLoge("gVKTexture") << "vkAllocateMemory failed for a framebuffer attachment.";
		gvkDestroyTexture(ctx, tex);
		return nullptr;
	}
	if(vkBindImageMemory(device, tex->image, tex->memory, 0) != VK_SUCCESS) {
		gLoge("gVKTexture") << "vkBindImageMemory failed for a framebuffer attachment.";
		gvkDestroyTexture(ctx, tex);
		return nullptr;
	}

	VkImageViewCreateInfo viewinfo{};
	viewinfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewinfo.image = tex->image;
	viewinfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewinfo.format = tex->format;
	viewinfo.subresourceRange.aspectMask = tex->aspect;
	viewinfo.subresourceRange.baseMipLevel = 0;
	viewinfo.subresourceRange.levelCount = 1;
	viewinfo.subresourceRange.baseArrayLayer = 0;
	viewinfo.subresourceRange.layerCount = 1;
	if(vkCreateImageView(device, &viewinfo, nullptr, &tex->view) != VK_SUCCESS) {
		gLoge("gVKTexture") << "vkCreateImageView failed for a framebuffer attachment.";
		gvkDestroyTexture(ctx, tex);
		return nullptr;
	}

	// Clamped rather than repeating: a render target sampled outside its own edge
	// should not fold the opposite side back in.
	tex->addressu = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	tex->addressv = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	tex->sampler = gvkAcquireSampler(ctx, tex->minfilter, tex->magfilter, tex->addressu, tex->addressv,
			tex->usemipmaps, tex->mipmapmode, tex->samplerkey);
	if(tex->sampler == VK_NULL_HANDLE) {
		gLoge("gVKTexture") << "vkCreateSampler failed for a framebuffer attachment.";
		gvkDestroyTexture(ctx, tex);
		return nullptr;
	}
	if(!gvkWriteTextureDescriptorSet(ctx, tex)) {
		gvkDestroyTexture(ctx, tex);
		return nullptr;
	}
	return tex;
}

void gvkDestroyTexture(gVKContext& ctx, gVKTexture* tex) {
	if(tex == nullptr) return;
	VkDevice device = *ctx.getDevice();
	if(device != VK_NULL_HANDLE) {
		// This image may be the destination of a transfer that is recorded but not
		// yet submitted, which vkDeviceWaitIdle cannot see. The first destruction
		// after a load drains the upload path; the rest of the level's textures then
		// find nothing outstanding and return immediately.
		gvkWaitUploads(ctx);
		if(tex->descriptorset != VK_NULL_HANDLE && ctx.getDescriptorPool() != VK_NULL_HANDLE) {
			vkFreeDescriptorSets(device, ctx.getDescriptorPool(), 1, &tex->descriptorset);
		}
		gvkReleaseSampler(ctx, tex->samplerkey, tex->sampler);
		if(tex->view != VK_NULL_HANDLE) vkDestroyImageView(device, tex->view, nullptr);
		if(tex->image != VK_NULL_HANDLE) vkDestroyImage(device, tex->image, nullptr);
		if(tex->memory != VK_NULL_HANDLE) vkFreeMemory(device, tex->memory, nullptr);
	}
	delete tex;
}

#endif /* GVK_VULKAN */
