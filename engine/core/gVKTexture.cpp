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
static void gvkTransitionImageLayout(VkCommandBuffer cmd, VkImage image,
		VkImageLayout oldLayout, VkImageLayout newLayout,
		VkAccessFlags srcAccess, VkAccessFlags dstAccess,
		VkPipelineStageFlags srcStage, VkPipelineStageFlags dstStage) {
	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = image;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;
	barrier.srcAccessMask = srcAccess;
	barrier.dstAccessMask = dstAccess;
	vkCmdPipelineBarrier(cmd, srcStage, dstStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
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

	VkImageCreateInfo imageinfo{};
	imageinfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageinfo.imageType = VK_IMAGE_TYPE_2D;
	imageinfo.extent = {static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1};
	imageinfo.mipLevels = 1;
	imageinfo.arrayLayers = 1;
	// sRGB: the pixels come from ordinary image files (sRGB-encoded), so the sampler
	// decodes them to linear for shading, and the sRGB swapchain re-encodes on write.
	// Using a UNORM format here would double-encode and wash the image out.
	imageinfo.format = VK_FORMAT_R8G8B8A8_SRGB;
	imageinfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageinfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageinfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
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

	// Upload: undefined -> transfer dst, copy, transfer dst -> shader read.
	VkCommandBuffer cmd = gvkBeginSingleTimeCommands(ctx);
	gvkTransitionImageLayout(cmd, tex->image,
			VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			0, VK_ACCESS_TRANSFER_WRITE_BIT,
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);

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

	gvkTransitionImageLayout(cmd, tex->image,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
	gvkEndSingleTimeCommands(ctx, cmd);

	vkDestroyBuffer(device, staging, nullptr);
	vkFreeMemory(device, stagingmem, nullptr);

	VkImageViewCreateInfo viewinfo{};
	viewinfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewinfo.image = tex->image;
	viewinfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewinfo.format = VK_FORMAT_R8G8B8A8_SRGB;
	viewinfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	viewinfo.subresourceRange.baseMipLevel = 0;
	viewinfo.subresourceRange.levelCount = 1;
	viewinfo.subresourceRange.baseArrayLayer = 0;
	viewinfo.subresourceRange.layerCount = 1;
	vkCreateImageView(device, &viewinfo, nullptr, &tex->view);

	VkSamplerCreateInfo samplerinfo{};
	samplerinfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerinfo.magFilter = VK_FILTER_LINEAR;
	samplerinfo.minFilter = VK_FILTER_LINEAR;
	samplerinfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerinfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerinfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerinfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerinfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	vkCreateSampler(device, &samplerinfo, nullptr, &tex->sampler);

	VkDescriptorSetLayout layout = ctx.getImageDescriptorSetLayout();
	VkDescriptorSetAllocateInfo dsalloc{};
	dsalloc.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	dsalloc.descriptorPool = ctx.getDescriptorPool();
	dsalloc.descriptorSetCount = 1;
	dsalloc.pSetLayouts = &layout;
	if(vkAllocateDescriptorSets(device, &dsalloc, &tex->descriptorset) != VK_SUCCESS) {
		gLoge("gVKTexture") << "vkAllocateDescriptorSets failed (pool exhausted?).";
	} else {
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
	}

	return tex;
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
