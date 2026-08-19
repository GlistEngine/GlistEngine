/*
 * gVKReflect.h
 *
 * Reads a compiled SPIR-V module back and reports what the shader expects from
 * the pipeline: its vertex inputs, its push constant block and its descriptor
 * bindings. gVKPipeline uses this instead of repeating those declarations in
 * C++, so editing a .vert/.frag under graphics/shaders/vk is enough - the
 * pipeline layout follows the shader on its own.
 *
 * This is a deliberately small reflector: it understands the subset of SPIR-V
 * that glslc emits for the engine's 2D shaders (scalars, vectors, matrices,
 * arrays, structs, images and samplers) rather than the whole specification.
 * Anything it does not recognise is skipped with a warning instead of guessed
 * at, so a shader using an unsupported construct fails loudly at pipeline
 * creation rather than silently rendering wrong.
 */

#pragma once

#ifndef CORE_GVKREFLECT_H
#define CORE_GVKREFLECT_H

#include "gVKContext.h"

#ifdef GVK_VULKAN

#include <cstddef>
#include <cstdint>
#include <vector>

// One descriptor a shader declares, in the shape vkCreateDescriptorSetLayout
// wants. stages accumulates while several stages of the same pipeline are
// reflected, so a sampler read by both the vertex and fragment shader ends up
// with both bits set.
struct gVKReflectedBinding {
	uint32_t set = 0;
	uint32_t binding = 0;
	uint32_t count = 1;
	VkDescriptorType type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	VkShaderStageFlags stages = 0;
};

// The merged interface of every stage of one pipeline. Reflect the vertex and
// the fragment module into the same instance to get the whole picture: vertex
// inputs come from the vertex stage only, while push constants and descriptors
// are unioned across stages.
struct gVKReflectedLayout {
	// Which stages have been reflected into this layout so far.
	VkShaderStageFlags stages = 0;
	// Sorted by location, with offsets packed in that order (which is how the
	// draw path lays its vertex structs out) and stride set to the total.
	std::vector<VkVertexInputAttributeDescription> vertexattributes;
	uint32_t vertexstride = 0;
	// 0 when the pipeline uses no push constants.
	uint32_t pushconstantsize = 0;
	VkShaderStageFlags pushconstantstages = 0;
	// Sorted by (set, binding).
	std::vector<gVKReflectedBinding> bindings;

	// Highest descriptor set index used, plus one; 0 when there are none.
	uint32_t getSetCount() const;
	// The bindings belonging to one set, in binding order.
	std::vector<VkDescriptorSetLayoutBinding> getSetBindings(uint32_t set) const;
};

// Merges one SPIR-V module's interface into layout. Returns false when the
// module is not valid SPIR-V or uses something this reflector cannot describe.
bool gvkReflectSpirv(const uint32_t* spirv, size_t sizeBytes, gVKReflectedLayout& layout);

#endif /* GVK_VULKAN */

#endif /* CORE_GVKREFLECT_H */
