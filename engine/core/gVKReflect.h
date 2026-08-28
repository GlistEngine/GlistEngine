/*
 * gVKReflect.h
 *
 * Reads a compiled SPIR-V module back and reports what the shader expects from
 * the pipeline: its vertex inputs, its push constant block and its descriptor
 * bindings. gVKPipeline uses this instead of repeating those declarations in
 * C++, so editing a shader under graphics/shaders is enough - the
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
#include <string>
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
	// The name the shader gave the resource: the sampler's variable name, or the
	// block's type name for a uniform buffer. Empty when the module carries no
	// debug names, which is what glslc emits under -g0. gShader looks a texture up
	// by this name, so a stripped module can still be drawn with - it just cannot
	// be addressed by name.
	std::string name;
	// Byte footprint of a uniform or storage block, 0 for images and samplers.
	uint32_t blocksize = 0;
};

// What a block member is made of, which is what a setter needs in order to turn
// a packed glm value into the block's own layout. A glm::mat3 is nine floats
// with no gaps; the same mat3 in a std140 block is three columns sixteen bytes
// apart, so writing one over the other would corrupt two thirds of it.
enum gVKMemberComponent {
	GVK_MEMBER_UNKNOWN = 0,
	GVK_MEMBER_FLOAT,
	GVK_MEMBER_INT,
	GVK_MEMBER_UINT,
	GVK_MEMBER_BOOL,
};

// One addressable scalar, vector, matrix or array leaf inside a uniform block or
// a push constant block, named the way the shader source names it. Nested
// structs and arrays of structs are flattened with dots and indices, so
// "lights[2].position" addresses exactly what it reads like.
struct gVKReflectedMember {
	std::string name;
	uint32_t set = 0;
	uint32_t binding = 0;
	// Byte offset from the start of the block.
	uint32_t offset = 0;
	// Bytes the member occupies in the block, padding between matrix columns and
	// array elements included.
	uint32_t size = 0;
	gVKMemberComponent component = GVK_MEMBER_UNKNOWN;
	// Vector components, or rows of a matrix. 1 for a scalar.
	uint32_t rows = 1;
	// Matrix columns. 1 for scalars and vectors.
	uint32_t columns = 1;
	// Bytes between two matrix columns; 0 when the member is not a matrix.
	uint32_t matrixstride = 0;
	// Elements of an array, 1 when the member is not one.
	uint32_t arraylength = 1;
	// Bytes between two array elements; 0 when the member is not an array.
	uint32_t arraystride = 0;
	// Push constant members live in the push constant block rather than in a
	// descriptor, so set and binding say nothing about them.
	bool pushconstant = false;
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
	// Every named leaf of every uniform and push constant block, in declaration
	// order. Empty when the modules were compiled without debug names.
	std::vector<gVKReflectedMember> members;

	// The member with this exact name, or nullptr. Names are the shader's own.
	const gVKReflectedMember* findMember(const std::string& name) const;
	// The member an application-facing name addresses, which is not always one the
	// reflector stored: "kernel[3]" names an element of a member the shader
	// declared once, so the element is resolved into `out` with its own offset.
	// Names without an index resolve to a copy of the stored member. Returns false
	// when the shader declares nothing by that name.
	//
	// Everything that answers a question about a name has to come through here,
	// including the "does this shader have it" test - a name resolvable by one and
	// not the other means the setter is never reached.
	bool resolveMember(const std::string& name, gVKReflectedMember& out) const;
	// The binding with this exact name, or nullptr.
	const gVKReflectedBinding* findBinding(const std::string& name) const;

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
