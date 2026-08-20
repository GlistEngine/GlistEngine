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

#ifdef GVK_VULKAN

struct gVKTexture {
	VkImage image = VK_NULL_HANDLE;
	VkDeviceMemory memory = VK_NULL_HANDLE;
	VkImageView view = VK_NULL_HANDLE;
	VkSampler sampler = VK_NULL_HANDLE;
	uint64_t samplerkey = 0;
	VkDescriptorSet descriptorset = VK_NULL_HANDLE;
	int width = 0;
	int height = 0;
	// Levels in the mip chain, 1 meaning no chain. Built at upload time, the same
	// way the OpenGL path calls glGenerateMipmap after its upload.
	uint32_t miplevels = 1;
	// Tracked so a render target can be moved between being written as an
	// attachment and being sampled. An uploaded texture never leaves
	// SHADER_READ_ONLY, which is why that is the default.
	VkImageLayout layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	VkImageAspectFlags aspect = VK_IMAGE_ASPECT_COLOR_BIT;
	VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
	// True for a texture created as an FBO attachment rather than uploaded from
	// pixels. Those carry no mip chain and are cleared by the render pass.
	bool isattachment = false;
	// Whether any texel in the upload is transparent enough for mesh3d.frag's cutout
	// test to discard it. Found while the pixels are being expanded to RGBA anyway,
	// so it costs nothing, and it lets a mesh drawn with this as its diffuse map take
	// the pipeline that has the discard compiled out - which is what keeps early
	// depth rejection alive on mobile. Left false for an attachment, which is never a
	// material map.
	bool hascutout = false;
	// Set after the descriptor has been recorded by a draw. Replacing a texture
	// that has never been sampled needs no device-wide wait (for example the
	// duplicate upload performed while gTexture initially sets itself up).
	bool sampled = false;
	// What the current sampler was built with, so a filtering or wrapping change
	// can be detected and only then rebuild it. The defaults match what gTexture
	// starts from.
	VkFilter minfilter = VK_FILTER_LINEAR;
	VkFilter magfilter = VK_FILTER_LINEAR;
	// Whether the sampler may walk the mip chain at all. OpenGL says so through the
	// minification filter: GL_LINEAR samples level 0 only, GL_LINEAR_MIPMAP_LINEAR
	// is trilinear. Collapsing both onto "always mip" softened textures the
	// application had asked to be sampled flat.
	bool usemipmaps = true;
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
		VkSamplerAddressMode addressU, VkSamplerAddressMode addressV,
		bool useMipmaps = true, VkSamplerMipmapMode mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR);

// Creates an empty texture to be rendered into, for gFbo's colour and depth
// attachments. Unlike an uploaded texture this carries no pixels and no mip chain:
// the render pass clears it, the draws fill it, and it is transitioned back to
// SHADER_READ_ONLY afterwards so the same object can be sampled.
gVKTexture* gvkCreateAttachmentTexture(gVKContext& ctx, int width, int height, bool depth);

void gvkDestroyTexture(gVKContext& ctx, gVKTexture* tex);

#endif /* GVK_VULKAN */

#endif /* CORE_GVKTEXTURE_H */
