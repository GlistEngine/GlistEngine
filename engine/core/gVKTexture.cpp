/*
 * gVKTexture.cpp
 */

#include "gVKTexture.h"

#ifdef GVK_DESKTOP_GLFW

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
static VkSampler gvkCreateSampler(VkDevice device, VkFilter minFilter, VkFilter magFilter,
		VkSamplerAddressMode addressU, VkSamplerAddressMode addressV) {
	VkSamplerCreateInfo samplerinfo{};
	samplerinfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerinfo.magFilter = magFilter;
	samplerinfo.minFilter = minFilter;
	samplerinfo.addressModeU = addressU;
	samplerinfo.addressModeV = addressV;
	samplerinfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerinfo.mipmapMode = minFilter == VK_FILTER_NEAREST
			? VK_SAMPLER_MIPMAP_MODE_NEAREST : VK_SAMPLER_MIPMAP_MODE_LINEAR;
	// Without this maxLod defaults to 0, which pins sampling to level 0 and makes
	// the mip chain built at upload time dead weight. CLAMP_NONE lets every level
	// be reached, so the texture is filtered the way glGenerateMipmap leaves it.
	samplerinfo.minLod = 0.0f;
	samplerinfo.maxLod = VK_LOD_CLAMP_NONE;
	samplerinfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	VkSampler sampler = VK_NULL_HANDLE;
	vkCreateSampler(device, &samplerinfo, nullptr, &sampler);
	return sampler;
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
	vkMapMemory(device, stagingmem, 0, imagesize, 0, &mapped);
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
	vkAllocateMemory(device, &allocinfo, nullptr, &tex->memory);
	vkBindImageMemory(device, tex->image, tex->memory, 0);

	// Upload: undefined -> transfer dst, copy into level 0, build the rest of the
	// chain from it, then the whole chain -> shader read.
	VkCommandBuffer cmd = gvkBeginSingleTimeCommands(ctx);
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
	gvkEndSingleTimeCommands(ctx, cmd);

	vkDestroyBuffer(device, staging, nullptr);
	vkFreeMemory(device, stagingmem, nullptr);

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
	vkCreateImageView(device, &viewinfo, nullptr, &tex->view);

	// The sampler starts from gTexture's own defaults; setFiltering / setWrapping
	// rebuild it through gvkSetTextureSampler when the texture asks for something
	// else, which the upload path does right after this returns.
	tex->sampler = gvkCreateSampler(device, tex->minfilter, tex->magfilter, tex->addressu, tex->addressv);

	gvkWriteTextureDescriptorSet(ctx, tex);
	return tex;
}

bool gvkSetTextureSampler(gVKContext& ctx, gVKTexture* tex, VkFilter minFilter, VkFilter magFilter,
		VkSamplerAddressMode addressU, VkSamplerAddressMode addressV) {
	if(tex == nullptr) return false;
	if(tex->minfilter == minFilter && tex->magfilter == magFilter
			&& tex->addressu == addressU && tex->addressv == addressV) {
		return false;
	}
	tex->minfilter = minFilter;
	tex->magfilter = magFilter;
	tex->addressu = addressU;
	tex->addressv = addressV;

	VkDevice device = *ctx.getDevice();
	if(device == VK_NULL_HANDLE || tex->sampler == VK_NULL_HANDLE) return false;
	// A frame that is still in flight may sample through the old sampler, so the
	// device is drained before it is destroyed. Sampler changes happen while a
	// texture is being set up, not per frame, so this costs nothing in a running
	// scene.
	vkDeviceWaitIdle(device);
	vkDestroySampler(device, tex->sampler, nullptr);
	tex->sampler = gvkCreateSampler(device, minFilter, magFilter, addressU, addressV);
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
	if(vkAllocateDescriptorSets(device, &dsalloc, &tex->descriptorset) != VK_SUCCESS) {
		gLoge("gVKTexture") << "vkAllocateDescriptorSets failed (pool exhausted?).";
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

void gvkDestroyTexture(gVKContext& ctx, gVKTexture* tex) {
	if(tex == nullptr) return;
	VkDevice device = *ctx.getDevice();
	if(device != VK_NULL_HANDLE) {
		if(tex->descriptorset != VK_NULL_HANDLE && ctx.getDescriptorPool() != VK_NULL_HANDLE) {
			vkFreeDescriptorSets(device, ctx.getDescriptorPool(), 1, &tex->descriptorset);
		}
		if(tex->sampler != VK_NULL_HANDLE) vkDestroySampler(device, tex->sampler, nullptr);
		if(tex->view != VK_NULL_HANDLE) vkDestroyImageView(device, tex->view, nullptr);
		if(tex->image != VK_NULL_HANDLE) vkDestroyImage(device, tex->image, nullptr);
		if(tex->memory != VK_NULL_HANDLE) vkFreeMemory(device, tex->memory, nullptr);
	}
	delete tex;
}

#endif /* GVK_DESKTOP_GLFW */
