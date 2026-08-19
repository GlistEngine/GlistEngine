/*
 * gVKReflect.cpp
 */

#include "gVKReflect.h"

#ifdef GVK_VULKAN

#include "gUtils.h"
#include <algorithm>


// Constants from the SPIR-V core specification. Spelled out rather than pulled
// from spirv.h so the reflector needs nothing beyond <vulkan/vulkan.h>.
static constexpr uint32_t GVK_SPV_MAGIC = 0x07230203;
static constexpr uint32_t GVK_SPV_HEADER_WORDS = 5;
static constexpr uint32_t GVK_NO_OFFSET = 0xffffffffu;

enum : uint32_t {
	SpvOpEntryPoint = 15, SpvOpDecorate = 71, SpvOpMemberDecorate = 72,
	SpvOpTypeInt = 21, SpvOpTypeFloat = 22, SpvOpTypeVector = 23, SpvOpTypeMatrix = 24,
	SpvOpTypeImage = 25, SpvOpTypeSampler = 26, SpvOpTypeSampledImage = 27,
	SpvOpTypeArray = 28, SpvOpTypeRuntimeArray = 29, SpvOpTypeStruct = 30,
	SpvOpTypePointer = 32, SpvOpConstant = 43, SpvOpVariable = 59,
};

enum : uint32_t {
	SpvDecorationBlock = 2, SpvDecorationBufferBlock = 3, SpvDecorationArrayStride = 6,
	SpvDecorationBuiltIn = 11, SpvDecorationLocation = 30, SpvDecorationBinding = 33,
	SpvDecorationDescriptorSet = 34, SpvDecorationOffset = 35,
};

enum : uint32_t {
	SpvStorageClassUniformConstant = 0, SpvStorageClassInput = 1, SpvStorageClassUniform = 2,
	SpvStorageClassPushConstant = 9, SpvStorageClassStorageBuffer = 12,
};

enum : uint32_t {
	SpvExecutionModelVertex = 0, SpvExecutionModelTessellationControl = 1,
	SpvExecutionModelTessellationEvaluation = 2, SpvExecutionModelGeometry = 3,
	SpvExecutionModelFragment = 4, SpvExecutionModelGLCompute = 5,
};

enum : uint32_t { SpvDimBuffer = 5 };

// Everything the reflector remembers about one SPIR-V id. A single flat record
// covers types, constants and variables alike; which fields are meaningful
// follows from opcode.
struct gvkSpvId {
	uint32_t opcode = 0;
	// OpVariable: its pointer type. OpTypePointer / OpTypeVector / OpTypeMatrix /
	// OpTypeArray / OpTypeSampledImage: the type they are built from.
	uint32_t basetype = 0;
	uint32_t storageclass = 0;
	uint32_t width = 0;         // int / float bit width
	uint32_t signedness = 0;
	uint32_t count = 0;         // vector components, matrix columns
	uint32_t lengthid = 0;      // OpTypeArray element count, as a constant id
	uint32_t constant = 0;      // OpConstant literal
	uint32_t imagesampled = 0;
	uint32_t imagedim = 0;
	uint32_t arraystride = 0;
	uint32_t location = 0;
	uint32_t binding = 0;
	uint32_t set = 0;
	bool haslocation = false, hasbinding = false, hasset = false, hasbuiltin = false;
	bool isblock = false, isbufferblock = false;
	std::vector<uint32_t> membertypes;
	std::vector<uint32_t> memberoffsets;   // GVK_NO_OFFSET where undecorated
};

static VkShaderStageFlagBits gvkStageOf(uint32_t executionModel) {
	switch(executionModel) {
	case SpvExecutionModelVertex: return VK_SHADER_STAGE_VERTEX_BIT;
	case SpvExecutionModelTessellationControl: return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
	case SpvExecutionModelTessellationEvaluation: return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
	case SpvExecutionModelGeometry: return VK_SHADER_STAGE_GEOMETRY_BIT;
	case SpvExecutionModelFragment: return VK_SHADER_STAGE_FRAGMENT_BIT;
	case SpvExecutionModelGLCompute: return VK_SHADER_STAGE_COMPUTE_BIT;
	default: return static_cast<VkShaderStageFlagBits>(0);
	}
}

// Byte size of a type as the shader lays it out. Structs are measured from their
// member Offset decorations, which is what glslc emits for interface blocks, so
// the answer matches the block's declared footprint including any padding.
static uint32_t gvkTypeSize(const std::vector<gvkSpvId>& ids, uint32_t type) {
	if(type >= ids.size()) return 0;
	const gvkSpvId& t = ids[type];
	switch(t.opcode) {
	case SpvOpTypeInt:
	case SpvOpTypeFloat:
		return t.width / 8;
	case SpvOpTypeVector:
	case SpvOpTypeMatrix:
		return t.count * gvkTypeSize(ids, t.basetype);
	case SpvOpTypeArray: {
		uint32_t length = (t.lengthid < ids.size()) ? ids[t.lengthid].constant : 0;
		uint32_t stride = t.arraystride != 0 ? t.arraystride : gvkTypeSize(ids, t.basetype);
		return length * stride;
	}
	case SpvOpTypeStruct: {
		uint32_t size = 0;
		for(size_t i = 0; i < t.membertypes.size(); i++) {
			uint32_t offset = (i < t.memberoffsets.size()) ? t.memberoffsets[i] : GVK_NO_OFFSET;
			if(offset == GVK_NO_OFFSET) continue;
			size = std::max(size, offset + gvkTypeSize(ids, t.membertypes[i]));
		}
		return size;
	}
	default:
		return 0;
	}
}

// VkFormat of a vertex input. Only the scalar and vector types a vertex
// attribute can actually have are covered; anything else reports UNDEFINED and
// the caller rejects the shader.
static VkFormat gvkVertexFormat(const std::vector<gvkSpvId>& ids, uint32_t type) {
	if(type >= ids.size()) return VK_FORMAT_UNDEFINED;
	const gvkSpvId& t = ids[type];
	uint32_t components = 1;
	uint32_t scalar = type;
	if(t.opcode == SpvOpTypeVector) {
		components = t.count;
		scalar = t.basetype;
	}
	if(scalar >= ids.size() || components < 1 || components > 4) return VK_FORMAT_UNDEFINED;
	const gvkSpvId& s = ids[scalar];
	if(s.width != 32) return VK_FORMAT_UNDEFINED;
	if(s.opcode == SpvOpTypeFloat) {
		const VkFormat formats[4] = {VK_FORMAT_R32_SFLOAT, VK_FORMAT_R32G32_SFLOAT,
				VK_FORMAT_R32G32B32_SFLOAT, VK_FORMAT_R32G32B32A32_SFLOAT};
		return formats[components - 1];
	}
	if(s.opcode == SpvOpTypeInt) {
		const VkFormat sformats[4] = {VK_FORMAT_R32_SINT, VK_FORMAT_R32G32_SINT,
				VK_FORMAT_R32G32B32_SINT, VK_FORMAT_R32G32B32A32_SINT};
		const VkFormat uformats[4] = {VK_FORMAT_R32_UINT, VK_FORMAT_R32G32_UINT,
				VK_FORMAT_R32G32B32_UINT, VK_FORMAT_R32G32B32A32_UINT};
		return s.signedness != 0 ? sformats[components - 1] : uformats[components - 1];
	}
	return VK_FORMAT_UNDEFINED;
}

// Descriptor type of a resource variable, following the storage class and the
// shape of the type it points at. Arrays are unwrapped first, and their length
// becomes the descriptor count.
static bool gvkDescriptorTypeOf(const std::vector<gvkSpvId>& ids, uint32_t type, uint32_t storageClass,
		VkDescriptorType& outType, uint32_t& outCount) {
	outCount = 1;
	while(type < ids.size() && ids[type].opcode == SpvOpTypeArray) {
		const uint32_t lengthid = ids[type].lengthid;
		outCount *= (lengthid != 0 && lengthid < ids.size()) ? ids[lengthid].constant : 1;
		type = ids[type].basetype;
	}
	if(type >= ids.size()) return false;
	const gvkSpvId& t = ids[type];

	if(t.opcode == SpvOpTypeSampledImage) {
		outType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		return true;
	}
	if(t.opcode == SpvOpTypeSampler) {
		outType = VK_DESCRIPTOR_TYPE_SAMPLER;
		return true;
	}
	if(t.opcode == SpvOpTypeImage) {
		if(t.imagedim == SpvDimBuffer) {
			outType = t.imagesampled == 2 ? VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER
					: VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
		} else {
			outType = t.imagesampled == 2 ? VK_DESCRIPTOR_TYPE_STORAGE_IMAGE
					: VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
		}
		return true;
	}
	if(t.opcode == SpvOpTypeStruct) {
		// A Block in the Uniform class is a uniform buffer; a BufferBlock there, or
		// any block in the StorageBuffer class, is a storage buffer.
		if(storageClass == SpvStorageClassStorageBuffer || t.isbufferblock) {
			outType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		} else {
			outType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		}
		return true;
	}
	return false;
}


uint32_t gVKReflectedLayout::getSetCount() const {
	uint32_t count = 0;
	for(const gVKReflectedBinding& b : bindings) count = std::max(count, b.set + 1);
	return count;
}

std::vector<VkDescriptorSetLayoutBinding> gVKReflectedLayout::getSetBindings(uint32_t set) const {
	std::vector<VkDescriptorSetLayoutBinding> result;
	for(const gVKReflectedBinding& b : bindings) {
		if(b.set != set) continue;
		VkDescriptorSetLayoutBinding entry{};
		entry.binding = b.binding;
		entry.descriptorType = b.type;
		entry.descriptorCount = b.count;
		entry.stageFlags = b.stages;
		result.push_back(entry);
	}
	return result;
}

bool gvkReflectSpirv(const uint32_t* spirv, size_t sizeBytes, gVKReflectedLayout& layout) {
	const size_t wordcount = sizeBytes / sizeof(uint32_t);
	if(spirv == nullptr || wordcount < GVK_SPV_HEADER_WORDS || spirv[0] != GVK_SPV_MAGIC) {
		gLoge("gVKReflect") << "Not a SPIR-V module.";
		return false;
	}

	// Word 3 of the header is the id bound, so ids index straight into a vector.
	const uint32_t idbound = spirv[3];
	if(idbound == 0 || idbound > (1u << 22)) {
		gLoge("gVKReflect") << "SPIR-V id bound looks wrong: " << idbound;
		return false;
	}
	std::vector<gvkSpvId> ids(idbound);
	VkShaderStageFlagBits stage = static_cast<VkShaderStageFlagBits>(0);

	// Single linear pass: declarations always precede their uses in SPIR-V except
	// for decorations, which come first, so one walk is enough to fill the table.
	for(size_t i = GVK_SPV_HEADER_WORDS; i < wordcount;) {
		const uint32_t opcode = spirv[i] & 0xffffu;
		const uint32_t length = spirv[i] >> 16;
		if(length == 0 || i + length > wordcount) {
			gLoge("gVKReflect") << "Malformed SPIR-V instruction stream.";
			return false;
		}
		const uint32_t* ops = &spirv[i + 1];
		const uint32_t opcount = length - 1;

		auto at = [&ids, idbound](uint32_t id) -> gvkSpvId* {
			return id < idbound ? &ids[id] : nullptr;
		};

		switch(opcode) {
		case SpvOpEntryPoint:
			if(opcount >= 1) stage = gvkStageOf(ops[0]);
			break;
		case SpvOpDecorate: {
			gvkSpvId* target = opcount >= 2 ? at(ops[0]) : nullptr;
			if(target == nullptr) break;
			switch(ops[1]) {
			case SpvDecorationBlock: target->isblock = true; break;
			case SpvDecorationBufferBlock: target->isbufferblock = true; break;
			case SpvDecorationBuiltIn: target->hasbuiltin = true; break;
			case SpvDecorationArrayStride: if(opcount >= 3) target->arraystride = ops[2]; break;
			case SpvDecorationLocation: if(opcount >= 3) { target->location = ops[2]; target->haslocation = true; } break;
			case SpvDecorationBinding: if(opcount >= 3) { target->binding = ops[2]; target->hasbinding = true; } break;
			case SpvDecorationDescriptorSet: if(opcount >= 3) { target->set = ops[2]; target->hasset = true; } break;
			default: break;
			}
			break;
		}
		case SpvOpMemberDecorate: {
			gvkSpvId* target = opcount >= 3 ? at(ops[0]) : nullptr;
			if(target == nullptr) break;
			const uint32_t member = ops[1];
			if(ops[2] == SpvDecorationOffset && opcount >= 4) {
				if(target->memberoffsets.size() <= member) target->memberoffsets.resize(member + 1, GVK_NO_OFFSET);
				target->memberoffsets[member] = ops[3];
			} else if(ops[2] == SpvDecorationBuiltIn) {
				// A struct with built-in members is gl_PerVertex, never an interface
				// block the pipeline layout cares about.
				target->hasbuiltin = true;
			}
			break;
		}
		case SpvOpTypeInt: {
			gvkSpvId* t = opcount >= 3 ? at(ops[0]) : nullptr;
			if(t != nullptr) { t->opcode = opcode; t->width = ops[1]; t->signedness = ops[2]; }
			break;
		}
		case SpvOpTypeFloat: {
			gvkSpvId* t = opcount >= 2 ? at(ops[0]) : nullptr;
			if(t != nullptr) { t->opcode = opcode; t->width = ops[1]; }
			break;
		}
		case SpvOpTypeVector:
		case SpvOpTypeMatrix: {
			gvkSpvId* t = opcount >= 3 ? at(ops[0]) : nullptr;
			if(t != nullptr) { t->opcode = opcode; t->basetype = ops[1]; t->count = ops[2]; }
			break;
		}
		case SpvOpTypeArray: {
			gvkSpvId* t = opcount >= 3 ? at(ops[0]) : nullptr;
			if(t != nullptr) { t->opcode = opcode; t->basetype = ops[1]; t->lengthid = ops[2]; }
			break;
		}
		case SpvOpTypeRuntimeArray: {
			gvkSpvId* t = opcount >= 2 ? at(ops[0]) : nullptr;
			if(t != nullptr) { t->opcode = opcode; t->basetype = ops[1]; }
			break;
		}
		case SpvOpTypeStruct: {
			gvkSpvId* t = opcount >= 1 ? at(ops[0]) : nullptr;
			if(t != nullptr) {
				t->opcode = opcode;
				t->membertypes.assign(ops + 1, ops + opcount);
			}
			break;
		}
		case SpvOpTypePointer: {
			gvkSpvId* t = opcount >= 3 ? at(ops[0]) : nullptr;
			if(t != nullptr) { t->opcode = opcode; t->storageclass = ops[1]; t->basetype = ops[2]; }
			break;
		}
		case SpvOpTypeImage: {
			gvkSpvId* t = opcount >= 8 ? at(ops[0]) : nullptr;
			if(t != nullptr) { t->opcode = opcode; t->imagedim = ops[2]; t->imagesampled = ops[6]; }
			break;
		}
		case SpvOpTypeSampler: {
			gvkSpvId* t = opcount >= 1 ? at(ops[0]) : nullptr;
			if(t != nullptr) t->opcode = opcode;
			break;
		}
		case SpvOpTypeSampledImage: {
			gvkSpvId* t = opcount >= 2 ? at(ops[0]) : nullptr;
			if(t != nullptr) { t->opcode = opcode; t->basetype = ops[1]; }
			break;
		}
		case SpvOpConstant: {
			// ops[0] is the result type, ops[1] the id, ops[2] the low literal word.
			gvkSpvId* c = opcount >= 3 ? at(ops[1]) : nullptr;
			if(c != nullptr) { c->opcode = opcode; c->basetype = ops[0]; c->constant = ops[2]; }
			break;
		}
		case SpvOpVariable: {
			gvkSpvId* v = opcount >= 3 ? at(ops[1]) : nullptr;
			if(v != nullptr) { v->opcode = opcode; v->basetype = ops[0]; v->storageclass = ops[2]; }
			break;
		}
		default:
			break;
		}
		i += length;
	}

	if(stage == 0) {
		gLoge("gVKReflect") << "SPIR-V module has no entry point.";
		return false;
	}
	layout.stages |= stage;

	// Vertex inputs are only meaningful on the vertex stage; the Input variables of
	// a fragment shader are interpolated varyings, not pipeline attributes.
	std::vector<VkVertexInputAttributeDescription> attributes;

	for(uint32_t id = 0; id < idbound; id++) {
		const gvkSpvId& var = ids[id];
		if(var.opcode != SpvOpVariable) continue;
		const gvkSpvId* pointer = var.basetype < idbound ? &ids[var.basetype] : nullptr;
		if(pointer == nullptr || pointer->opcode != SpvOpTypePointer) continue;
		const uint32_t pointee = pointer->basetype;

		if(var.storageclass == SpvStorageClassInput) {
			if(stage != VK_SHADER_STAGE_VERTEX_BIT || !var.haslocation || var.hasbuiltin) continue;

			// A matrix attribute is not one input but one per column, at consecutive
			// locations - a mat4 declared at location 6 occupies 6, 7, 8 and 9. That
			// is how an instanced model matrix arrives, so it has to be unrolled here
			// rather than rejected.
			uint32_t columns = 1;
			uint32_t columntype = pointee;
			if(pointee < idbound && ids[pointee].opcode == SpvOpTypeMatrix) {
				columns = ids[pointee].count;
				columntype = ids[pointee].basetype;
			}

			VkFormat format = gvkVertexFormat(ids, columntype);
			if(format == VK_FORMAT_UNDEFINED || columns < 1 || columns > 4) {
				gLoge("gVKReflect") << "Vertex input at location " << var.location
						<< " has a type this reflector cannot map to a VkFormat.";
				return false;
			}
			for(uint32_t column = 0; column < columns; column++) {
				VkVertexInputAttributeDescription attribute{};
				attribute.location = var.location + column;
				attribute.binding = 0;
				attribute.format = format;
				attribute.offset = 0;   // filled in below, once they are in location order
				attributes.push_back(attribute);
			}
			continue;
		}

		if(var.storageclass == SpvStorageClassPushConstant) {
			uint32_t size = gvkTypeSize(ids, pointee);
			if(size == 0) {
				gLoge("gVKReflect") << "Push constant block has no measurable size.";
				return false;
			}
			layout.pushconstantsize = std::max(layout.pushconstantsize, size);
			layout.pushconstantstages |= stage;
			continue;
		}

		if(var.storageclass == SpvStorageClassUniformConstant ||
				var.storageclass == SpvStorageClassUniform ||
				var.storageclass == SpvStorageClassStorageBuffer) {
			if(!var.hasset && !var.hasbinding) continue;
			VkDescriptorType type;
			uint32_t count = 1;
			if(!gvkDescriptorTypeOf(ids, pointee, var.storageclass, type, count)) {
				gLoge("gVKReflect") << "Resource at set " << var.set << " binding " << var.binding
						<< " has a type this reflector does not understand.";
				return false;
			}
			// Merge into an existing entry when another stage already declared it.
			gVKReflectedBinding* existing = nullptr;
			for(gVKReflectedBinding& b : layout.bindings) {
				if(b.set == var.set && b.binding == var.binding) { existing = &b; break; }
			}
			if(existing != nullptr) {
				existing->stages |= stage;
				existing->count = std::max(existing->count, count);
			} else {
				gVKReflectedBinding entry{};
				entry.set = var.set;
				entry.binding = var.binding;
				entry.count = count;
				entry.type = type;
				entry.stages = stage;
				layout.bindings.push_back(entry);
			}
		}
	}

	if(stage == VK_SHADER_STAGE_VERTEX_BIT) {
		// Pack the attributes in location order. The draw path builds its vertex
		// structs the same way, so declaration order in the shader is the layout.
		std::sort(attributes.begin(), attributes.end(),
				[](const VkVertexInputAttributeDescription& a, const VkVertexInputAttributeDescription& b) {
					return a.location < b.location;
				});
		uint32_t offset = 0;
		for(VkVertexInputAttributeDescription& attribute : attributes) {
			attribute.offset = offset;
			switch(attribute.format) {
			case VK_FORMAT_R32_SFLOAT: case VK_FORMAT_R32_SINT: case VK_FORMAT_R32_UINT: offset += 4; break;
			case VK_FORMAT_R32G32_SFLOAT: case VK_FORMAT_R32G32_SINT: case VK_FORMAT_R32G32_UINT: offset += 8; break;
			case VK_FORMAT_R32G32B32_SFLOAT: case VK_FORMAT_R32G32B32_SINT: case VK_FORMAT_R32G32B32_UINT: offset += 12; break;
			default: offset += 16; break;
			}
		}
		layout.vertexattributes = attributes;
		layout.vertexstride = offset;
	}

	std::sort(layout.bindings.begin(), layout.bindings.end(),
			[](const gVKReflectedBinding& a, const gVKReflectedBinding& b) {
				return a.set != b.set ? a.set < b.set : a.binding < b.binding;
			});
	return true;
}

#endif /* GVK_VULKAN */
