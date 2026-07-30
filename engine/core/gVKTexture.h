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
};

// Uploads tightly-packed RGBA8 pixels (4 bytes per pixel, top row first) into a
// new device-local sampled image and allocates its descriptor set from the
// context's pool. Returns nullptr on failure.
gVKTexture* gvkCreateTextureRGBA8(gVKContext& ctx, const void* rgbaPixels, int width, int height);

void gvkDestroyTexture(gVKContext& ctx, gVKTexture* tex);

#endif /* GVK_DESKTOP_GLFW */

#endif /* CORE_GVKTEXTURE_H */
