/*
 * gVKUniform.cpp
 *
 * Per-frame uniform buffers of the Vulkan 3D path.
 */

#include "gVKUniform.h"

#ifdef GVK_DESKTOP_GLFW

#include "gVKBuffer.h"
#include "gRenderer.h"
#include "gUtils.h"
#include <cstring>

// The shader's light array is a fixed size, so a mismatch here would quietly drop
// lights or read past the end of the block.
static_assert(GVK_MAX_LIGHTS == GLIST_MAX_LIGHTS,
		"GVK_MAX_LIGHTS must match GLIST_MAX_LIGHTS and the array size in mesh3d.frag");
// The Vulkan light layout is meant to be the OpenGL one; if the shared struct is
// ever reshaped, this catches it at compile time rather than as wrong shading.
static_assert(sizeof(gVKLightData) == sizeof(gRenderer::gSceneLightData),
		"gVKLightData no longer matches gRenderer::gSceneLightData");

bool gvkCreateUniformResources(gVKContext& ctx) {
	VkDevice device = *ctx.getDevice();
	if(device == VK_NULL_HANDLE) return false;
	if(ctx.mesh3dsetlayouts.empty()) {
		// No descriptor set in the mesh3d shaders means nothing to allocate. Not an
		// error: a build whose shaders declare no uniform block simply has no scene
		// data to bind.
		return true;
	}
	if(ctx.descriptorpool == VK_NULL_HANDLE) {
		gLoge("gVKUniform") << "The descriptor pool has to exist before the uniform sets.";
		return false;
	}

	for(int i = 0; i < GVK_MAX_FRAMES_IN_FLIGHT; i++) {
		// Host visible and coherent: written every frame by the CPU, read by the GPU,
		// and coherent memory means no explicit flush after each write.
		if(!gvkCreateBuffer(ctx, sizeof(gVKSceneUniforms), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				ctx.sceneuniformbuffers[i], ctx.sceneuniformmemories[i])) {
			gLoge("gVKUniform") << "Could not create the scene uniform buffer for frame " << i;
			gvkDestroyUniformResources(ctx);
			return false;
		}

		if(vkMapMemory(device, ctx.sceneuniformmemories[i], 0, sizeof(gVKSceneUniforms), 0,
				&ctx.sceneuniformmapped[i]) != VK_SUCCESS) {
			gLoge("gVKUniform") << "vkMapMemory failed for the scene uniform buffer " << i;
			gvkDestroyUniformResources(ctx);
			return false;
		}
		// A frame that draws no 3D still has its buffer bound, so it must not contain
		// whatever the allocation happened to hold.
		std::memset(ctx.sceneuniformmapped[i], 0, sizeof(gVKSceneUniforms));

		VkDescriptorSetAllocateInfo allocinfo{};
		allocinfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocinfo.descriptorPool = ctx.descriptorpool;
		allocinfo.descriptorSetCount = 1;
		allocinfo.pSetLayouts = &ctx.mesh3dsetlayouts[0];
		if(vkAllocateDescriptorSets(device, &allocinfo, &ctx.sceneuniformsets[i]) != VK_SUCCESS) {
			gLoge("gVKUniform") << "vkAllocateDescriptorSets failed for the scene set " << i;
			ctx.sceneuniformsets[i] = VK_NULL_HANDLE;
			gvkDestroyUniformResources(ctx);
			return false;
		}

		// Written once here rather than per frame: the set always points at the same
		// buffer, and only the buffer's contents change.
		VkDescriptorBufferInfo bufferinfo{};
		bufferinfo.buffer = ctx.sceneuniformbuffers[i];
		bufferinfo.offset = 0;
		bufferinfo.range = sizeof(gVKSceneUniforms);

		VkWriteDescriptorSet write{};
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.dstSet = ctx.sceneuniformsets[i];
		write.dstBinding = 0;
		write.dstArrayElement = 0;
		write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		write.descriptorCount = 1;
		write.pBufferInfo = &bufferinfo;
		vkUpdateDescriptorSets(device, 1, &write, 0, nullptr);
	}

	gLogi("gVKUniform") << "Scene uniform buffers ready: " << GVK_MAX_FRAMES_IN_FLIGHT
			<< " x " << sizeof(gVKSceneUniforms) << " bytes, up to " << GVK_MAX_LIGHTS << " lights";
	return true;
}

void gvkDestroyUniformResources(gVKContext& ctx) {
	VkDevice device = *ctx.getDevice();
	if(device == VK_NULL_HANDLE) return;

	for(int i = 0; i < GVK_MAX_FRAMES_IN_FLIGHT; i++) {
		// The sets are not freed one by one: they were allocated from the descriptor
		// pool, which gvkDestroyGraphicsPipelines destroys as a whole.
		ctx.sceneuniformsets[i] = VK_NULL_HANDLE;

		if(ctx.sceneuniformmapped[i] != nullptr) {
			vkUnmapMemory(device, ctx.sceneuniformmemories[i]);
			ctx.sceneuniformmapped[i] = nullptr;
		}
		if(ctx.sceneuniformbuffers[i] != VK_NULL_HANDLE) {
			vkDestroyBuffer(device, ctx.sceneuniformbuffers[i], nullptr);
			ctx.sceneuniformbuffers[i] = VK_NULL_HANDLE;
		}
		if(ctx.sceneuniformmemories[i] != VK_NULL_HANDLE) {
			vkFreeMemory(device, ctx.sceneuniformmemories[i], nullptr);
			ctx.sceneuniformmemories[i] = VK_NULL_HANDLE;
		}
	}
}

void gvkWriteSceneUniforms(gVKContext& ctx, const gVKSceneUniforms& data) {
	void* mapped = ctx.sceneuniformmapped[ctx.currentframe];
	if(mapped == nullptr) return;
	std::memcpy(mapped, &data, sizeof(gVKSceneUniforms));
}

#endif /* GVK_DESKTOP_GLFW */
