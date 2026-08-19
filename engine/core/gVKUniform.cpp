/*
 * gVKUniform.cpp
 *
 * Per-frame uniform buffers of the Vulkan 3D path.
 */

#include "gVKUniform.h"

#ifdef GVK_VULKAN

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

static VkDeviceSize gvkSceneUniformStride(const gVKContext& ctx) {
	const VkDeviceSize alignment = ctx.getMinUniformBufferOffsetAlignment();
	if(alignment == 0) return sizeof(gVKSceneUniforms);
	return (sizeof(gVKSceneUniforms) + alignment - 1) & ~(alignment - 1);
}

// Appends one chunk of GVK_SCENE_UNIFORM_SLOTS scene uniform slots - one buffer
// plus its descriptor sets - to frame framei. Used both by gvkCreateUniformResources,
// to build the first chunk of every frame, and by gvkWriteSceneUniforms, to grow a
// frame past what its existing chunks hold. Leaves ctx untouched on failure.
bool gvkAppendSceneUniformChunk(gVKContext& ctx, int framei) {
	VkDevice device = *ctx.getDevice();

	// Host visible and coherent: written every frame by the CPU, read by the GPU,
	// and coherent memory means no explicit flush after each write.
	const VkDeviceSize stride = gvkSceneUniformStride(ctx);
	const VkDeviceSize buffersize = stride * GVK_SCENE_UNIFORM_SLOTS;
	VkBuffer buffer = VK_NULL_HANDLE;
	VkDeviceMemory memory = VK_NULL_HANDLE;
	if(!gvkCreateBuffer(ctx, buffersize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			buffer, memory)) {
		gLoge("gVKUniform") << "Could not create the scene uniform buffer for frame " << framei;
		return false;
	}

	void* mapped = nullptr;
	if(vkMapMemory(device, memory, 0, buffersize, 0, &mapped) != VK_SUCCESS) {
		gLoge("gVKUniform") << "vkMapMemory failed for the scene uniform buffer " << framei;
		vkDestroyBuffer(device, buffer, nullptr);
		vkFreeMemory(device, memory, nullptr);
		return false;
	}
	// A frame that draws no 3D still has its buffer bound, so it must not contain
	// whatever the allocation happened to hold.
	std::memset(mapped, 0, buffersize);

	const size_t setsbase = ctx.sceneuniformsets[framei].size();
	ctx.sceneuniformsets[framei].resize(setsbase + GVK_SCENE_UNIFORM_SLOTS, VK_NULL_HANDLE);
	VkDescriptorSetAllocateInfo allocinfo{};
	allocinfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocinfo.descriptorPool = ctx.descriptorpool;
	std::vector<VkDescriptorSetLayout> layouts(GVK_SCENE_UNIFORM_SLOTS, ctx.mesh3dsetlayouts[0]);
	allocinfo.descriptorSetCount = GVK_SCENE_UNIFORM_SLOTS;
	allocinfo.pSetLayouts = layouts.data();
	if(vkAllocateDescriptorSets(device, &allocinfo, ctx.sceneuniformsets[framei].data() + setsbase) != VK_SUCCESS) {
		gLoge("gVKUniform") << "vkAllocateDescriptorSets failed for the scene set " << framei;
		ctx.sceneuniformsets[framei].resize(setsbase);
		vkUnmapMemory(device, memory);
		vkDestroyBuffer(device, buffer, nullptr);
		vkFreeMemory(device, memory, nullptr);
		return false;
	}

	// Written once here rather than per frame: the set always points at the same
	// buffer, and only the buffer's contents change.
	for(uint32_t slot = 0; slot < GVK_SCENE_UNIFORM_SLOTS; ++slot) {
		VkDescriptorBufferInfo bufferinfo{};
		bufferinfo.buffer = buffer;
		bufferinfo.offset = stride * slot;
		bufferinfo.range = sizeof(gVKSceneUniforms);
		VkWriteDescriptorSet write{};
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.dstSet = ctx.sceneuniformsets[framei][setsbase + slot];
		write.dstBinding = 0;
		write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		write.descriptorCount = 1;
		write.pBufferInfo = &bufferinfo;
		vkUpdateDescriptorSets(device, 1, &write, 0, nullptr);
	}

	ctx.sceneuniformbuffers[framei].push_back(buffer);
	ctx.sceneuniformmemories[framei].push_back(memory);
	ctx.sceneuniformmapped[framei].push_back(mapped);
	return true;
}

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
		// The first chunk of every frame; gvkWriteSceneUniforms appends more of the
		// same shape on demand if a frame ever needs more slots than one chunk holds.
		if(!gvkAppendSceneUniformChunk(ctx, i)) {
			gvkDestroyUniformResources(ctx);
			return false;
		}
	}

	gLogi("gVKUniform") << "Scene uniform buffers ready: " << GVK_MAX_FRAMES_IN_FLIGHT
			<< " x " << GVK_SCENE_UNIFORM_SLOTS << " slots of " << sizeof(gVKSceneUniforms)
			<< " bytes, up to " << GVK_MAX_LIGHTS << " lights, growing on demand";
	return true;
}

void gvkDestroyUniformResources(gVKContext& ctx) {
	VkDevice device = *ctx.getDevice();
	if(device == VK_NULL_HANDLE) return;

	for(int i = 0; i < GVK_MAX_FRAMES_IN_FLIGHT; i++) {
		// The sets are not freed one by one: they were allocated from the descriptor
		// pool, which gvkDestroyGraphicsPipelines destroys as a whole.
		ctx.sceneuniformsets[i].clear();

		for(size_t chunk = 0; chunk < ctx.sceneuniformbuffers[i].size(); ++chunk) {
			if(ctx.sceneuniformmapped[i][chunk] != nullptr) {
				vkUnmapMemory(device, ctx.sceneuniformmemories[i][chunk]);
			}
			if(ctx.sceneuniformbuffers[i][chunk] != VK_NULL_HANDLE) {
				vkDestroyBuffer(device, ctx.sceneuniformbuffers[i][chunk], nullptr);
			}
			if(ctx.sceneuniformmemories[i][chunk] != VK_NULL_HANDLE) {
				vkFreeMemory(device, ctx.sceneuniformmemories[i][chunk], nullptr);
			}
		}
		ctx.sceneuniformbuffers[i].clear();
		ctx.sceneuniformmemories[i].clear();
		ctx.sceneuniformmapped[i].clear();
	}
}

bool gvkWriteSceneUniforms(gVKContext& ctx, const gVKSceneUniforms& data) {
	if(ctx.sceneuniformmapped[ctx.currentframe].empty()) return false;
	if(ctx.sceneuniformslotcount >= ctx.sceneuniformsets[ctx.currentframe].size()) {
		// Every chunk allocated so far for this frame is full; append another rather
		// than capping how many scene lighting changes a frame can make.
		if(!gvkAppendSceneUniformChunk(ctx, ctx.currentframe)) {
			gLoge("gVKUniform") << "Could not grow the scene uniform buffer for frame " << ctx.currentframe;
			return false;
		}
	}
	ctx.currentsceneuniformslot = ctx.sceneuniformslotcount++;
	const uint32_t chunk = ctx.currentsceneuniformslot / GVK_SCENE_UNIFORM_SLOTS;
	const uint32_t index = ctx.currentsceneuniformslot % GVK_SCENE_UNIFORM_SLOTS;
	auto* destination = static_cast<unsigned char*>(ctx.sceneuniformmapped[ctx.currentframe][chunk])
			+ gvkSceneUniformStride(ctx) * index;
	std::memcpy(destination, &data, sizeof(gVKSceneUniforms));
	return true;
}

#endif /* GVK_VULKAN */
