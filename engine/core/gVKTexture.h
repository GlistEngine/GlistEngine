/*
 * gVKTexture.h
 *
 * A GPU texture for the Vulkan backend: a sampled image plus the view, sampler
 * and descriptor set the image pipeline binds. gVKRenderEngine keeps a registry
 * mapping the GLuint ids gTexture/gImage hand out to these objects, so the
 * existing load path (createTextures -> texImage2D) allocates a real Vulkan
 * texture without gTexture knowing anything about Vulkan.
 */

#pragma once

#ifndef CORE_GVKTEXTURE_H
#define CORE_GVKTEXTURE_H

#include "gVKContext.h"

#ifdef GVK_DESKTOP_GLFW

struct gVKTexture {
	VkImage image = VK_NULL_HANDLE;
	VkDeviceMemory memory = VK_NULL_HANDLE;
	VkImageView view = VK_NULL_HANDLE;
	VkSampler sampler = VK_NULL_HANDLE;
	VkDescriptorSet descriptorset = VK_NULL_HANDLE;
	int width = 0;
	int height = 0;
	uint32_t miplevels = 1;
	bool usemipmaps = true;
	// What the current sampler was built with, so a filtering or wrapping change
	// can be detected and only then rebuild it. The defaults match what gTexture
	// starts from.
	VkFilter minfilter = VK_FILTER_LINEAR;
	VkFilter magfilter = VK_FILTER_LINEAR;
	VkSamplerMipmapMode mipmapmode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	VkSamplerAddressMode addressu = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	VkSamplerAddressMode addressv = VK_SAMPLER_ADDRESS_MODE_REPEAT;
};

// Uploads tightly-packed RGBA8 pixels (4 bytes per pixel, top row first) into a
// new device-local sampled image and allocates its descriptor set from the
// context's pool. Returns nullptr on failure.
gVKTexture* gvkCreateTextureRGBA8(gVKContext& ctx, const void* rgbaPixels, int width, int height);

// Allocates the texture's descriptor set from the context's pool and points it at
// the image. Called once at creation, and again after a shader reload, which
// destroys the pool and with it every set allocated from it.
bool gvkWriteTextureDescriptorSet(gVKContext& ctx, gVKTexture* tex);

// Rebuilds the texture's sampler when the requested filtering or wrapping differs
// from what it already has, and repoints the descriptor set at the new one. This is
// how gTexture::setFiltering / setWrapping reach Vulkan. Returns false when nothing
// had to change.
bool gvkSetTextureSampler(gVKContext& ctx, gVKTexture* tex, VkFilter minFilter, VkFilter magFilter,
		VkSamplerAddressMode addressU, VkSamplerAddressMode addressV, bool useMipmaps = true,
		VkSamplerMipmapMode mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR);

void gvkDestroyTexture(gVKContext& ctx, gVKTexture* tex);

#endif /* GVK_DESKTOP_GLFW */

#endif /* CORE_GVKTEXTURE_H */
