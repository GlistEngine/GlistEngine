/*
 * gVKReflect.cpp
 */

#include "gVKReflect.h"

#ifdef GVK_VULKAN

#include "gUtils.h"
#include <algorithm>
#include <string>


// Constants from the SPIR-V core specification. Spelled out rather than pulled
// from spirv.h so the reflector needs nothing beyond <vulkan/vulkan.h>.
static constexpr uint32_t GVK_SPV_MAGIC = 0x07230203;
static constexpr uint32_t GVK_SPV_HEADER_WORDS = 5;
static constexpr uint32_t GVK_NO_OFFSET = 0xffffffffu;

enum : uint32_t {
	SpvOpName = 5, SpvOpMemberName = 6,
	SpvOpEntryPoint = 15, SpvOpDecorate = 71, SpvOpMemberDecorate = 72,
	SpvOpTypeInt = 21, SpvOpTypeFloat = 22, SpvOpTypeVector = 23, SpvOpTypeMatrix = 24,
	SpvOpTypeImage = 25, SpvOpTypeSampler = 26, SpvOpTypeSampledImage = 27,
	SpvOpTypeArray = 28, SpvOpTypeRuntimeArray = 29, SpvOpTypeStruct = 30,
	SpvOpTypePointer = 32, SpvOpConstant = 43, SpvOpVariable = 59,
};

enum : uint32_t {
	SpvDecorationBlock = 2, SpvDecorationBufferBlock = 3, SpvDecorationRowMajor = 4,
	SpvDecorationColMajor = 5, SpvDecorationArrayStride = 6, SpvDecorationMatrixStride = 7,
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
	// OpName / OpMemberName, present only while the module keeps its debug names.
	std::string name;
	std::vector<std::string> membernames;
	// MatrixStride is decorated on the member, not on the matrix type, so it is
	// kept alongside the offsets and read when that member turns out to be one.
	std::vector<uint32_t> membermatrixstrides;
	std::vector<bool> memberrowmajor;
};

// A SPIR-V literal string: UTF-8 packed four bytes to a word, low byte first,
// null terminated and padded out to the end of the word.
static std::string gvkSpvString(const uint32_t* words, uint32_t count) {
	std::string text;
	for(uint32_t w = 0; w < count; w++) {
		for(int b = 0; b < 4; b++) {
			const char c = static_cast<char>((words[w] >> (b * 8)) & 0xffu);
			if(c == '\0') return text;
			text.push_back(c);
		}
	}
	return text;
}

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

static uint32_t gvkTypeSize(const std::vector<gvkSpvId>& ids, uint32_t type);

// Byte size of one member of a struct, as the block reserves it rather than as
// the type packs it. The two differ for matrices, and only the struct can tell
// them apart: MatrixStride is decorated on the member, not on the matrix type, so
// a mat3 - 36 bytes of floats - reserves 48 with its columns 16 bytes apart. Read
// from the type alone it measures 36, and a block ending in one is then allocated
// eight bytes short of what the shader reads.
static uint32_t gvkStructMemberSize(const std::vector<gvkSpvId>& ids, const gvkSpvId& block, size_t member) {
	if(member >= block.membertypes.size()) return 0;
	const uint32_t type = block.membertypes[member];
	if(type >= ids.size()) return 0;
	const uint32_t matrixstride = (member < block.membermatrixstrides.size())
			? block.membermatrixstrides[member] : 0;
	// An array of matrices is spaced by its ArrayStride, which already covers the
	// per-column padding, so only a lone matrix needs the correction.
	if(matrixstride != 0 && ids[type].opcode == SpvOpTypeMatrix) {
		return ids[type].count * matrixstride;
	}
	return gvkTypeSize(ids, type);
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
			size = std::max(size, offset + gvkStructMemberSize(ids, t, i));
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


// Upper bound on how many leaves one shader may expose. A block declaring a huge
// array of structs would otherwise flatten into an entry per element per field;
// the cap turns that into a warning rather than a stall.
static constexpr size_t GVK_MAX_REFLECTED_MEMBERS = 4096;

// Fills in the shape of a leaf: the component type, how many rows and columns it
// has, and its array footprint. Returns false for a type this reflector has no
// way to describe, which the caller reports rather than guesses at.
static bool gvkClassifyLeaf(const std::vector<gvkSpvId>& ids, uint32_t type, gVKReflectedMember& member) {
	if(type >= ids.size()) return false;

	// Arrays first: the element type is what carries the shape, and the array only
	// contributes its length and stride.
	if(ids[type].opcode == SpvOpTypeArray) {
		const gvkSpvId& array = ids[type];
		const uint32_t length = (array.lengthid < ids.size()) ? ids[array.lengthid].constant : 0;
		if(length == 0) return false;
		member.arraylength = length;
		member.arraystride = array.arraystride != 0 ? array.arraystride : gvkTypeSize(ids, array.basetype);
		type = array.basetype;
		if(type >= ids.size()) return false;
	}

	if(ids[type].opcode == SpvOpTypeMatrix) {
		member.columns = ids[type].count;
		type = ids[type].basetype;
		if(type >= ids.size()) return false;
	}
	if(ids[type].opcode == SpvOpTypeVector) {
		member.rows = ids[type].count;
		type = ids[type].basetype;
		if(type >= ids.size()) return false;
	}

	const gvkSpvId& scalar = ids[type];
	if(scalar.opcode == SpvOpTypeFloat) {
		member.component = GVK_MEMBER_FLOAT;
	} else if(scalar.opcode == SpvOpTypeInt) {
		// A GLSL bool is a 32 bit unsigned integer in a block, which is why setBool
		// and setUnsignedInt end up writing the same bytes.
		member.component = scalar.signedness != 0 ? GVK_MEMBER_INT : GVK_MEMBER_UINT;
	} else {
		return false;
	}
	return true;
}

// Walks one block's struct type and appends an entry for every addressable leaf.
// Nested structs extend the name with a dot and arrays of structs with an index,
// so what comes out reads the way the shader source spells it.
static void gvkFlattenBlock(const std::vector<gvkSpvId>& ids, uint32_t structType,
		const std::string& prefix, uint32_t baseOffset, uint32_t set, uint32_t binding,
		bool pushConstant, std::vector<gVKReflectedMember>& out) {
	if(structType >= ids.size() || ids[structType].opcode != SpvOpTypeStruct) return;
	const gvkSpvId& block = ids[structType];

	for(size_t i = 0; i < block.membertypes.size(); i++) {
		if(out.size() >= GVK_MAX_REFLECTED_MEMBERS) {
			gLogw("gVKReflect") << "Block has more members than this reflector exposes by name; the rest are unreachable from gShader.";
			return;
		}
		const uint32_t offset = (i < block.memberoffsets.size()) ? block.memberoffsets[i] : GVK_NO_OFFSET;
		if(offset == GVK_NO_OFFSET) continue;
		const uint32_t type = block.membertypes[i];
		if(type >= ids.size()) continue;

		std::string name = (i < block.membernames.size()) ? block.membernames[i] : std::string();
		if(name.empty()) name = "member" + gToStr(static_cast<int>(i));
		const std::string fullname = prefix.empty() ? name : prefix + "." + name;
		const uint32_t absoluteoffset = baseOffset + offset;

		// An array of structs is expanded per element, which is what makes
		// "lights[3].position" resolvable the way the OpenGL path resolves it.
		uint32_t elementtype = type;
		uint32_t length = 1;
		uint32_t stride = 0;
		if(ids[type].opcode == SpvOpTypeArray) {
			elementtype = ids[type].basetype;
			length = (ids[type].lengthid < ids.size()) ? ids[ids[type].lengthid].constant : 0;
			stride = ids[type].arraystride != 0 ? ids[type].arraystride : gvkTypeSize(ids, elementtype);
		}
		if(elementtype < ids.size() && ids[elementtype].opcode == SpvOpTypeStruct) {
			for(uint32_t e = 0; e < length; e++) {
				const std::string elementname = (ids[type].opcode == SpvOpTypeArray)
						? fullname + "[" + gToStr(static_cast<int>(e)) + "]"
						: fullname;
				gvkFlattenBlock(ids, elementtype, elementname, absoluteoffset + e * stride,
						set, binding, pushConstant, out);
			}
			continue;
		}

		gVKReflectedMember member;
		member.name = fullname;
		member.set = set;
		member.binding = binding;
		member.offset = absoluteoffset;
		member.pushconstant = pushConstant;
		if(i < block.membermatrixstrides.size()) member.matrixstride = block.membermatrixstrides[i];
		if(!gvkClassifyLeaf(ids, type, member)) {
			// Not fatal: the shader still works, this one member just cannot be
			// addressed by name. Rejecting the whole module would be worse.
			gLogw("gVKReflect") << "Block member '" << fullname << "' has a type that cannot be set by name.";
			continue;
		}
		member.size = gvkTypeSize(ids, type);
		// gvkTypeSize measures a matrix as its columns packed tight, which is what
		// it occupies in memory but not what it occupies in a block: std140 spaces
		// the columns matrixstride apart, so a mat3 declared as 36 bytes of floats
		// actually reserves 48. Anything reading size to know where the member ends
		// needs the reserved figure.
		if(member.arraylength <= 1 && member.columns > 1 && member.matrixstride != 0) {
			member.size = member.columns * member.matrixstride;
		}
		out.push_back(member);
	}
}

const gVKReflectedMember* gVKReflectedLayout::findMember(const std::string& name) const {
	for(const gVKReflectedMember& member : members) {
		if(member.name == name) return &member;
	}
	// A block declared with an instance name addresses its members through it -
	// "params.intensity" - while an anonymous block addresses them bare. The
	// shader source decides which, and the caller should not have to know, so a
	// trailing-component match stands in for the qualified form.
	for(const gVKReflectedMember& member : members) {
		const size_t dot = member.name.rfind('.');
		if(dot != std::string::npos && member.name.compare(dot + 1, std::string::npos, name) == 0) return &member;
		if(dot == std::string::npos && name.size() > member.name.size()) {
			const size_t namedot = name.rfind('.');
			if(namedot != std::string::npos && name.compare(namedot + 1, std::string::npos, member.name) == 0) return &member;
		}
	}
	return nullptr;
}

bool gVKReflectedLayout::resolveMember(const std::string& name, gVKReflectedMember& out) const {
	const gVKReflectedMember* member = findMember(name);
	if(member != nullptr) {
		out = *member;
		return true;
	}
	// "offsets[2]" addresses one element of a plain array. Arrays of structs are
	// already flattened per element by gvkFlattenBlock, because their leaves need
	// names of their own; an array of scalars, vectors or matrices stays one member
	// with a stride, so the index is resolved against that stride here.
	if(name.size() < 4 || name.back() != ']') return false;
	const size_t bracket = name.rfind('[');
	if(bracket == std::string::npos || bracket + 1 >= name.size() - 1) return false;
	const std::string digits = name.substr(bracket + 1, name.size() - bracket - 2);
	if(digits.empty() || digits.find_first_not_of("0123456789") != std::string::npos) return false;
	const gVKReflectedMember* array = findMember(name.substr(0, bracket));
	if(array == nullptr || array->arraylength == 0) return false;

	const unsigned long element = std::stoul(digits);
	if(element >= array->arraylength) return false;
	out = *array;
	out.name = name;
	out.offset += static_cast<uint32_t>(element) * array->arraystride;
	// One element, measured as itself rather than as the array: the stride between
	// elements is 16 bytes even for a float, and taking that as the size would let
	// a write spill into the padding a neighbouring member may sit in.
	out.arraylength = 1;
	out.size = (out.columns > 1 && out.matrixstride != 0)
			? out.columns * out.matrixstride
			: out.rows * 4u;
	return true;
}

const gVKReflectedBinding* gVKReflectedLayout::findBinding(const std::string& name) const {
	for(const gVKReflectedBinding& binding : bindings) {
		if(binding.name == name) return &binding;
	}
	return nullptr;
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
		case SpvOpName: {
			gvkSpvId* target = opcount >= 2 ? at(ops[0]) : nullptr;
			if(target != nullptr) target->name = gvkSpvString(ops + 1, opcount - 1);
			break;
		}
		case SpvOpMemberName: {
			gvkSpvId* target = opcount >= 3 ? at(ops[0]) : nullptr;
			if(target == nullptr) break;
			const uint32_t member = ops[1];
			if(target->membernames.size() <= member) target->membernames.resize(member + 1);
			target->membernames[member] = gvkSpvString(ops + 2, opcount - 2);
			break;
		}
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
			} else if(ops[2] == SpvDecorationMatrixStride && opcount >= 4) {
				if(target->membermatrixstrides.size() <= member) target->membermatrixstrides.resize(member + 1, 0);
				target->membermatrixstrides[member] = ops[3];
			} else if(ops[2] == SpvDecorationRowMajor) {
				if(target->memberrowmajor.size() <= member) target->memberrowmajor.resize(member + 1, false);
				target->memberrowmajor[member] = true;
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
	// A pipeline has one push constant block shared by its stages, so it is
	// flattened once however many stages declare it.
	bool pushconstantsflattened = false;
	for(const gVKReflectedMember& member : layout.members) {
		if(member.pushconstant) { pushconstantsflattened = true; break; }
	}

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
			// Only the first stage to declare the block contributes its members: the
			// other stages see the same block, and reflecting it twice would leave
			// two entries per name.
			if(!pushconstantsflattened) {
				gvkFlattenBlock(ids, pointee, "", 0, 0, 0, true, layout.members);
				pushconstantsflattened = true;
			}
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
			// A sampler is addressed by its variable name; a block by the name of its
			// struct type, which is what the source writes before the braces.
			std::string resourcename = var.name;
			const bool isblock = (type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER ||
					type == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
			if(isblock && pointee < idbound && !ids[pointee].name.empty()) resourcename = ids[pointee].name;

			if(existing != nullptr) {
				existing->stages |= stage;
				existing->count = std::max(existing->count, count);
				if(existing->name.empty()) existing->name = resourcename;
			} else {
				gVKReflectedBinding entry{};
				entry.set = var.set;
				entry.binding = var.binding;
				entry.count = count;
				entry.type = type;
				entry.stages = stage;
				entry.name = resourcename;
				if(isblock) {
					// Rounded up to sixteen: std140 pads a block out to a multiple of
					// its largest alignment, and the buffer range bound for it has to
					// cover what the shader may read, not only what the last member
					// occupies.
					entry.blocksize = (gvkTypeSize(ids, pointee) + 15u) & ~15u;
				}
				layout.bindings.push_back(entry);
				// Flattened once per binding, on whichever stage declares it first:
				// a block read by both stages is still one block.
				if(type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) {
					gvkFlattenBlock(ids, pointee, "", 0, var.set, var.binding, false, layout.members);
				}
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
