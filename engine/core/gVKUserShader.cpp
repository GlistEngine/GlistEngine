/*
 * gVKUserShader.cpp
 */

#include "gVKUserShader.h"

#ifdef GVK_VULKAN

#include "gVKBuffer.h"
#include "gVKPipeline.h"
#include "gVKShaderCompiler.h"
#include "gVKShaders.h"
#include "gVKTexture.h"
#include "gUtils.h"

#include <cstring>
#include <fstream>
#include <vector>

// How many draws one shader can make per frame before its uniform ring has to
// grow. A block written once and drawn once per frame is the common case; a
// post-process chain runs a handful. Growing is a reallocation, not a failure.
static constexpr uint32_t GVK_USER_SHADER_SLOTS = 16;

// Where growing stops. A shader wanting more than this in a single frame is
// doing something the ring was never the right structure for, and silently
// allocating without bound would hide it.
static constexpr uint32_t GVK_MAX_USER_SHADER_SLOTS = 256;

// Texture units a user shader may sample from. The OpenGL path allows more, but a
// Vulkan pipeline is limited by maxPerStageDescriptorSampledImages, and eight
// covers every sampler count the engine's own shaders use.
static constexpr int GVK_USER_SHADER_MAX_UNITS = 8;

// Descriptor sets one user shader may bind. Vulkan guarantees four, and Adreno
// and Mali offer exactly four, so a shader wanting more would not be portable
// even where it built.
static constexpr size_t GVK_MAX_USER_SHADER_SETS = 4;

/*
 * One uniform block the shader declared, with the memory behind it.
 *
 * The buffer is a ring rather than one allocation because the values may change
 * between two draws of the same frame - a post-process chain sets a different
 * radius per pass - and the first draw is still queued when the second is
 * recorded. Overwriting the buffer would retroactively change what the first draw
 * reads. Each bind takes the next slot, and the ring restarts when the frame
 * does, by which point the fence has proved the GPU is done with it.
 */
struct gvkUserBlock {
	uint32_t set = 0;
	uint32_t binding = 0;
	// The block's own footprint, and the stride between ring slots, which is the
	// footprint rounded up to the device's uniform buffer offset alignment.
	uint32_t size = 0;
	VkDeviceSize slotstride = 0;
	uint32_t slotcount = 0;
	// What the setters write into. Copied into a ring slot at bind time.
	std::vector<uint8_t> shadow;
	VkBuffer buffers[GVK_MAX_FRAMES_IN_FLIGHT] = {};
	VkDeviceMemory memories[GVK_MAX_FRAMES_IN_FLIGHT] = {};
	void* mapped[GVK_MAX_FRAMES_IN_FLIGHT] = {};
};

// One sampler the shader declared, and the texture unit it was told to read.
struct gvkUserSampler {
	uint32_t set = 0;
	uint32_t binding = 0;
	std::string name;
	int unit = 0;
};

struct gvkUserShader {
	bool live = false;
	std::string name;
	gVKUserPipeline pipeline;
	std::vector<gvkUserBlock> blocks;
	std::vector<gvkUserSampler> samplers;
	// Push constants are written straight into the command buffer, so they need no
	// buffer of their own - only a shadow copy to accumulate into.
	std::vector<uint8_t> pushshadow;
	// Descriptor sets, [set index][frame][slot]. Flat per set index: sets.size() is
	// the pipeline's set count, and every entry holds slotcount * frames handles.
	std::vector<std::vector<VkDescriptorSet>> sets;
	uint32_t slotcount = 0;
	// Advances once per bind and resets with the frame, indexing both the uniform
	// ring and the descriptor ring.
	uint32_t currentslot = 0;
	// The frame the ring was last rewound on, as a sequence number rather than the
	// cycling frame index: a shader drawn on alternate frames sees the same index
	// twice running and would never rewind.
	uint64_t lastframesequence = 0xffffffffffffffffull;
	// The SPIR-V, kept so the pipeline can be rebuilt after a swapchain or render
	// pass change without going back to the source.
	std::vector<uint32_t> vertspirv;
	std::vector<uint32_t> fragspirv;
	// A shader drawn with a sampler left unbound, or drawn past what its ring can
	// hold, is drawn that way every frame - so each complaint is made once per
	// shader rather than once per draw.
	bool warnedmissingtexture = false;
	bool warnedslotlimit = false;
};

static std::vector<gvkUserShader> gvkusershaders;
static gVKUserShaderId gvkbounduser = GVK_NO_USER_SHADER;
static gVKUserShaderTextureResolver gvkuserresolver = nullptr;

static void gvkDestroyUserBlock(gVKContext& ctx, gvkUserBlock& block);

/*
 * What a ring growth replaced, held until the draws that named it are done.
 *
 * Growing happens in the middle of a frame, after draws that bound the old
 * descriptor sets and read the old buffers have already been recorded. Handing
 * either straight back to the driver is undefined while a command buffer still
 * refers to it, and the growth path cannot wait for idle - it is called from
 * inside the recording it would be waiting on. So the old ring is parked and
 * released a full cycle of frames later, by which point the frame fence has
 * proved the queue is done with it.
 */
struct gvkRetiredRing {
	std::vector<gvkUserBlock> blocks;
	std::vector<std::vector<VkDescriptorSet>> sets;
	int framesleft = 0;
};
static std::vector<gvkRetiredRing> gvkretiredrings;

static void gvkDrainRetiredRings(gVKContext& ctx, bool force) {
	if(gvkretiredrings.empty()) return;
	VkDevice device = *ctx.getDevice();
	VkDescriptorPool pool = ctx.getDescriptorPool();
	for(size_t i = gvkretiredrings.size(); i-- > 0;) {
		gvkRetiredRing& retired = gvkretiredrings[i];
		if(!force && --retired.framesleft > 0) continue;
		if(pool != VK_NULL_HANDLE) {
			for(std::vector<VkDescriptorSet>& sets : retired.sets) {
				if(!sets.empty()) vkFreeDescriptorSets(device, pool, static_cast<uint32_t>(sets.size()), sets.data());
			}
		}
		for(gvkUserBlock& block : retired.blocks) gvkDestroyUserBlock(ctx, block);
		gvkretiredrings.erase(gvkretiredrings.begin() + i);
	}
}

static gvkUserShader* gvkUserShaderAt(gVKUserShaderId id) {
	if(id == GVK_NO_USER_SHADER || id > gvkusershaders.size()) return nullptr;
	gvkUserShader& shader = gvkusershaders[id - 1];
	return shader.live ? &shader : nullptr;
}

void gvkSetUserShaderTextureResolver(gVKUserShaderTextureResolver resolver) {
	gvkuserresolver = resolver;
}

void gvkUseUserShader(gVKUserShaderId id) {
	gvkbounduser = (gvkUserShaderAt(id) != nullptr) ? id : GVK_NO_USER_SHADER;
}

gVKUserShaderId gvkBoundUserShader() {
	return gvkbounduser;
}

uint32_t gvkBoundUserShaderVertexStride() {
	gvkUserShader* shader = gvkUserShaderAt(gvkbounduser);
	return shader != nullptr ? shader->pipeline.vertexstride : 0;
}

bool gvkBoundUserShaderUsesMeshLayout() {
	gvkUserShader* shader = gvkUserShaderAt(gvkbounduser);
	return shader != nullptr && shader->pipeline.usesmeshlayout;
}


// ---------------------------------------------------------------- SPIR-V input

// Reads a whole file as SPIR-V words. Returns false when the file is missing or
// is not a plausible module, which is how a missing .spv beside a shader source
// is told apart from a corrupt one.
static bool gvkReadSpirvFile(const std::string& path, std::vector<uint32_t>& spirv) {
	std::ifstream file(path, std::ios::binary | std::ios::ate);
	if(!file.is_open()) return false;
	const std::streamsize size = file.tellg();
	if(size <= 0 || (size % sizeof(uint32_t)) != 0) {
		gLoge("gVKUserShader") << path << " is not a whole number of SPIR-V words.";
		return false;
	}
	file.seekg(0);
	spirv.resize(static_cast<size_t>(size) / sizeof(uint32_t));
	if(!file.read(reinterpret_cast<char*>(spirv.data()), size)) {
		gLoge("gVKUserShader") << "Could not read " << path;
		spirv.clear();
		return false;
	}
	return true;
}

// GLSL in, SPIR-V out, by whichever route this build has.
//
// Compiling comes first where a compiler is linked in, and the file beside the
// source is the fallback. That is the opposite of what shipping the same bytes
// the players get would suggest, and it is deliberate: `source` is not the file
// on disk, it is what gShader's preprocessor made of it - one dialect selected
// out of the #if VULKAN / #if GLES / #if GLCORE blocks the file carries. A .spv
// built from the raw file by glslc alone would have no dialect selected at all,
// so preferring it would mean preferring the wrong module, and an edit to the
// source would go unnoticed for as long as a stale .spv sat next to it.
static bool gvkObtainSpirv(const std::string& source, const std::string& path,
		VkShaderStageFlagBits stage, const std::string& debugName, std::vector<uint32_t>& spirv,
		const uint32_t* builtinSpirv, size_t builtinBytes) {
	if(gvkCompileShaderSource(source, stage, debugName, spirv)) return true;
	if(!path.empty() && gvkReadSpirvFile(path + ".spv", spirv)) return true;
	// Built at the same time as the engine, from the Vulkan branch of the same
	// source, so this is the module the compiler would have produced.
	if(builtinSpirv != nullptr && builtinBytes > 0) {
		spirv.assign(builtinSpirv, builtinSpirv + builtinBytes / sizeof(uint32_t));
		return true;
	}

	if(path.empty()) {
		gLoge("gVKUserShader") << debugName << " could not be compiled: this build has no shader"
				<< " compiler, and a shader given as source rather than as a file has no .spv to fall back on.";
	} else {
		gLoge("gVKUserShader") << debugName << " could not be compiled and " << path << ".spv"
				<< " does not exist. A build with no shader compiler needs the SPIR-V ahead of"
				<< " time, compiled from the Vulkan branch of the source - glslc on the file as"
				<< " it stands sees no #version, because the dialect blocks are resolved by the"
				<< " engine rather than by the GLSL preprocessor.";
	}
	return false;
}


// ------------------------------------------------------------ resource creation

// Allocates one block's ring: a buffer per frame in flight, each holding
// slotcount slots, permanently mapped because it is rewritten every frame.
static bool gvkCreateUserBlockBuffers(gVKContext& ctx, gvkUserBlock& block, uint32_t slotCount) {
	VkDevice device = *ctx.getDevice();
	const VkDeviceSize alignment = ctx.getMinUniformBufferOffsetAlignment();
	block.slotstride = alignment > 0 ? ((block.size + alignment - 1) & ~(alignment - 1)) : block.size;
	block.slotcount = slotCount;
	const VkDeviceSize total = block.slotstride * slotCount;

	for(int frame = 0; frame < GVK_MAX_FRAMES_IN_FLIGHT; frame++) {
		// The preferred set is searched as a whole, not added to what is required, so
		// it has to name everything: device local memory the CPU can also write is
		// where a buffer rewritten every frame belongs, but naming device local
		// alone would find GPU-private memory that cannot be mapped at all.
		if(!gvkCreateBuffer(ctx, total, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				block.buffers[frame], block.memories[frame],
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
						| VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) {
			gLoge("gVKUserShader") << "Could not allocate a uniform buffer for a user shader.";
			return false;
		}
		if(vkMapMemory(device, block.memories[frame], 0, total, 0, &block.mapped[frame]) != VK_SUCCESS) {
			gLoge("gVKUserShader") << "Could not map a user shader's uniform buffer.";
			block.mapped[frame] = nullptr;
			return false;
		}
	}
	return true;
}

static void gvkDestroyUserBlock(gVKContext& ctx, gvkUserBlock& block) {
	VkDevice device = *ctx.getDevice();
	for(int frame = 0; frame < GVK_MAX_FRAMES_IN_FLIGHT; frame++) {
		if(block.mapped[frame] != nullptr) vkUnmapMemory(device, block.memories[frame]);
		if(block.buffers[frame] != VK_NULL_HANDLE) vkDestroyBuffer(device, block.buffers[frame], nullptr);
		if(block.memories[frame] != VK_NULL_HANDLE) vkFreeMemory(device, block.memories[frame], nullptr);
	}
	block = gvkUserBlock{};
}

// Allocates every descriptor set the shader needs - one per set index, per frame,
// per ring slot - and points the uniform bindings at their slice of the ring.
// Sampler bindings are written at bind time, once it is known which texture the
// unit holds.
static bool gvkAllocateUserSets(gVKContext& ctx, gvkUserShader& shader) {
	VkDevice device = *ctx.getDevice();
	const uint32_t setcount = static_cast<uint32_t>(shader.pipeline.setlayouts.size());
	shader.sets.assign(setcount, {});
	if(setcount == 0) return true;

	const uint32_t perset = shader.slotcount * GVK_MAX_FRAMES_IN_FLIGHT;
	for(uint32_t set = 0; set < setcount; set++) {
		std::vector<VkDescriptorSetLayout> layouts(perset, shader.pipeline.setlayouts[set]);
		VkDescriptorSetAllocateInfo allocinfo{};
		allocinfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocinfo.descriptorPool = ctx.getDescriptorPool();
		allocinfo.descriptorSetCount = perset;
		allocinfo.pSetLayouts = layouts.data();

		shader.sets[set].assign(perset, VK_NULL_HANDLE);
		if(vkAllocateDescriptorSets(device, &allocinfo, shader.sets[set].data()) != VK_SUCCESS) {
			gLoge("gVKUserShader") << "The descriptor pool could not supply " << perset
					<< " sets for the shader '" << shader.name << "'.";
			shader.sets[set].clear();
			// The sets allocated for the earlier set indices go back to the pool: the
			// caller throws the shader away, and a pool hands out a fixed number.
			VkDescriptorPool pool = ctx.getDescriptorPool();
			if(pool != VK_NULL_HANDLE) {
				for(std::vector<VkDescriptorSet>& allocated : shader.sets) {
					if(!allocated.empty()) {
						vkFreeDescriptorSets(device, pool, static_cast<uint32_t>(allocated.size()), allocated.data());
						allocated.clear();
					}
				}
			}
			return false;
		}
	}

	// Uniform bindings never change what they point at, so they are written once
	// here rather than per draw.
	std::vector<VkDescriptorBufferInfo> bufferinfos;
	std::vector<VkWriteDescriptorSet> writes;
	bufferinfos.reserve(shader.blocks.size() * perset);
	writes.reserve(shader.blocks.size() * perset);
	for(const gvkUserBlock& block : shader.blocks) {
		if(block.set >= setcount) continue;
		for(int frame = 0; frame < GVK_MAX_FRAMES_IN_FLIGHT; frame++) {
			for(uint32_t slot = 0; slot < shader.slotcount; slot++) {
				VkDescriptorBufferInfo info{};
				info.buffer = block.buffers[frame];
				info.offset = block.slotstride * slot;
				info.range = block.size;
				bufferinfos.push_back(info);

				VkWriteDescriptorSet write{};
				write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				write.dstSet = shader.sets[block.set][frame * shader.slotcount + slot];
				write.dstBinding = block.binding;
				write.descriptorCount = 1;
				write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				writes.push_back(write);
			}
		}
	}
	// The infos are pointed at only now: reserving above keeps the vector from
	// reallocating mid-loop, and taking the addresses afterwards keeps that from
	// being a silent requirement.
	for(size_t i = 0; i < writes.size(); i++) writes[i].pBufferInfo = &bufferinfos[i];
	if(!writes.empty()) {
		vkUpdateDescriptorSets(device, static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);
	}
	return true;
}

// Turns a reflected interface into the blocks and samplers the setters address.
static bool gvkBuildUserResources(gVKContext& ctx, gvkUserShader& shader) {
	const gVKReflectedLayout& reflected = shader.pipeline.reflected;
	shader.pushshadow.assign(shader.pipeline.pushsize, 0);
	shader.slotcount = GVK_USER_SHADER_SLOTS;

	for(const gVKReflectedBinding& binding : reflected.bindings) {
		if(binding.type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) {
			if(binding.blocksize == 0) {
				gLoge("gVKUserShader") << "Uniform block '" << binding.name << "' of shader '"
						<< shader.name << "' has no measurable size.";
				return false;
			}
			gvkUserBlock block;
			block.set = binding.set;
			block.binding = binding.binding;
			block.size = binding.blocksize;
			block.shadow.assign(binding.blocksize, 0);
			if(!gvkCreateUserBlockBuffers(ctx, block, shader.slotcount)) {
				gvkDestroyUserBlock(ctx, block);
				return false;
			}
			shader.blocks.push_back(std::move(block));
			continue;
		}
		if(binding.type == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) {
			gvkUserSampler sampler;
			sampler.set = binding.set;
			sampler.binding = binding.binding;
			sampler.name = binding.name;
			// Until told otherwise a sampler reads the unit its binding index names,
			// which is what an application that never calls setInt would expect.
			sampler.unit = static_cast<int>(binding.binding);
			shader.samplers.push_back(sampler);
			continue;
		}
		gLoge("gVKUserShader") << "Shader '" << shader.name << "' declares a resource at set "
				<< binding.set << " binding " << binding.binding
				<< " of a kind this path does not support yet (only uniform blocks and samplers).";
		return false;
	}
	return gvkAllocateUserSets(ctx, shader);
}


// ----------------------------------------------------------------- lifetime

gVKUserShaderId gvkCreateUserShader(gVKContext& ctx, const std::string& vertexSource,
		const std::string& fragmentSource, const std::string& vertexPath,
		const std::string& fragmentPath, int builtin) {
	const std::string name = vertexPath.empty() ? std::string("user shader") : vertexPath;

	const uint32_t* builtinvert = nullptr;
	const uint32_t* builtinfrag = nullptr;
	size_t builtinvertbytes = 0;
	size_t builtinfragbytes = 0;
	if(builtin == GVK_BUILTIN_FBO) {
		builtinvert = gvkspv_fbo_vert;
		builtinvertbytes = sizeof(gvkspv_fbo_vert);
		builtinfrag = gvkspv_fbo_frag;
		builtinfragbytes = sizeof(gvkspv_fbo_frag);
	} else if(builtin == GVK_BUILTIN_MAGNIFIER) {
		// gMagnifier uses the same fullscreen-quad vertex layout as gFbo.
		builtinvert = gvkspv_fbo_vert;
		builtinvertbytes = sizeof(gvkspv_fbo_vert);
		builtinfrag = gvkspv_magnifier_frag;
		builtinfragbytes = sizeof(gvkspv_magnifier_frag);
	}

	std::vector<uint32_t> vertspirv;
	std::vector<uint32_t> fragspirv;
	if(!gvkObtainSpirv(vertexSource, vertexPath, VK_SHADER_STAGE_VERTEX_BIT, name + " (vertex)", vertspirv,
			builtinvert, builtinvertbytes)) {
		return GVK_NO_USER_SHADER;
	}
	if(!gvkObtainSpirv(fragmentSource, fragmentPath, VK_SHADER_STAGE_FRAGMENT_BIT, name + " (fragment)", fragspirv,
			builtinfrag, builtinfragbytes)) {
		return GVK_NO_USER_SHADER;
	}

	// Reuse a slot whose shader was destroyed rather than growing without bound.
	size_t index = gvkusershaders.size();
	for(size_t i = 0; i < gvkusershaders.size(); i++) {
		if(!gvkusershaders[i].live) { index = i; break; }
	}
	if(index == gvkusershaders.size()) gvkusershaders.emplace_back();

	gvkUserShader& shader = gvkusershaders[index];
	shader = gvkUserShader{};
	shader.name = name;
	shader.vertspirv = vertspirv;
	shader.fragspirv = fragspirv;

	if(!gvkBuildUserPipeline(ctx, vertspirv, fragspirv, name.c_str(), shader.pipeline)) {
		shader = gvkUserShader{};
		return GVK_NO_USER_SHADER;
	}
	if(!gvkBuildUserResources(ctx, shader)) {
		for(gvkUserBlock& block : shader.blocks) gvkDestroyUserBlock(ctx, block);
		gvkDestroyUserPipeline(ctx, shader.pipeline);
		shader = gvkUserShader{};
		return GVK_NO_USER_SHADER;
	}

	shader.live = true;
	return static_cast<gVKUserShaderId>(index + 1);
}

void gvkDestroyUserShader(gVKContext& ctx, gVKUserShaderId id) {
	gvkUserShader* shader = gvkUserShaderAt(id);
	if(shader == nullptr) return;
	if(gvkbounduser == id) gvkbounduser = GVK_NO_USER_SHADER;

	for(gvkUserBlock& block : shader->blocks) gvkDestroyUserBlock(ctx, block);
	gvkDestroyUserPipeline(ctx, shader->pipeline);
	// The pool is created with FREE_DESCRIPTOR_SET, so the sets go back to it. A
	// shader reloaded during development would otherwise take its allocation with
	// it every time, and a pool has a fixed number of sets to give.
	VkDevice device = *ctx.getDevice();
	VkDescriptorPool pool = ctx.getDescriptorPool();
	if(pool != VK_NULL_HANDLE) {
		for(std::vector<VkDescriptorSet>& sets : shader->sets) {
			if(sets.empty()) continue;
			vkFreeDescriptorSets(device, pool, static_cast<uint32_t>(sets.size()), sets.data());
		}
	}
	*shader = gvkUserShader{};
}

void gvkDestroyAllUserShaders(gVKContext& ctx) {
	for(size_t i = 0; i < gvkusershaders.size(); i++) {
		if(gvkusershaders[i].live) gvkDestroyUserShader(ctx, static_cast<gVKUserShaderId>(i + 1));
	}
	gvkusershaders.clear();
	gvkbounduser = GVK_NO_USER_SHADER;
	// Nothing will reach another frame boundary now, and the caller has waited for
	// the device before tearing the backend down.
	gvkDrainRetiredRings(ctx, true);
}

bool gvkRebuildUserShaders(gVKContext& ctx) {
	bool allrebuilt = true;
	for(gvkUserShader& shader : gvkusershaders) {
		if(!shader.live) continue;

		// The pipeline goes because the render pass it was built against did; the
		// descriptor sets go because the pool did. The buffers behind the blocks are
		// untouched by either, so they are kept and only re-pointed at.
		gvkDestroyUserPipeline(ctx, shader.pipeline);
		// Only the handles are dropped here: the pool itself was destroyed by
		// whatever triggered the rebuild, which frees every set it held.
		shader.sets.clear();

		if(!gvkBuildUserPipeline(ctx, shader.vertspirv, shader.fragspirv, shader.name.c_str(), shader.pipeline)
				|| !gvkAllocateUserSets(ctx, shader)) {
			gLoge("gVKUserShader") << "Could not rebuild '" << shader.name << "'; it will not draw.";
			// Emptied, not merely marked dead: the slot is handed to the next shader
			// that loads, and clearing it there would drop these buffers on the floor
			// with nothing left holding their handles.
			for(gvkUserBlock& block : shader.blocks) gvkDestroyUserBlock(ctx, block);
			gvkDestroyUserPipeline(ctx, shader.pipeline);
			shader = gvkUserShader{};
			allrebuilt = false;
		}
	}
	return allrebuilt;
}


// ------------------------------------------------------------------- setters

// Copies one value into a block or push constant shadow, converting the packed
// glm form into the layout the shader declared.
static void gvkWriteMember(uint8_t* base, const gVKReflectedMember& member, const void* data,
		uint32_t rows, uint32_t columns) {
	const uint8_t* src = static_cast<const uint8_t*>(data);
	uint8_t* dst = base + member.offset;

	// A matrix is columns of rows, and the block spaces its columns matrixstride
	// bytes apart while glm packs them tight. mat4 in std140 happens to have a
	// stride of 16, which is exactly its column size, so the two agree - mat3 and
	// mat2 do not, and copying them wholesale is what silently corrupts them.
	if(columns > 1) {
		const uint32_t columnbytes = rows * sizeof(float);
		const uint32_t stride = member.matrixstride != 0 ? member.matrixstride : columnbytes;
		for(uint32_t column = 0; column < columns; column++) {
			std::memcpy(dst + column * stride, src + column * columnbytes, columnbytes);
		}
		return;
	}

	// A scalar or vector is contiguous in both, so its own footprint is the limit -
	// a vec3 written over a vec3 must not spill into the four bytes of padding the
	// block leaves after it, in case another member was packed there.
	uint32_t bytes = rows * sizeof(float);
	if(member.size != 0 && member.size < bytes) bytes = member.size;
	std::memcpy(dst, src, bytes);
}

bool gvkSetUserShaderValue(gVKUserShaderId id, const std::string& name, const void* data,
		gVKMemberComponent component, uint32_t rows, uint32_t columns) {
	gvkUserShader* shader = gvkUserShaderAt(id);
	if(shader == nullptr || data == nullptr) return false;

	// Resolved rather than looked up, so that an indexed name like "kernel[3]"
	// reaches the element it addresses. The same call answers gvkUserShaderHasUniform,
	// which is what getUniformLocation asks before a setter is ever called: a name
	// only one of the two could resolve would be silently dropped.
	gVKReflectedMember resolved;
	if(!shader->pipeline.reflected.resolveMember(name, resolved)) return false;
	const gVKReflectedMember* member = &resolved;

	// Writing a float over an int would reinterpret the bits rather than convert
	// them, so a mismatch is refused instead of producing a plausible wrong number.
	// bool and uint share a representation in a block and are deliberately not
	// separated here.
	const bool integersource = (component == GVK_MEMBER_INT || component == GVK_MEMBER_UINT ||
			component == GVK_MEMBER_BOOL);
	const bool integertarget = (member->component == GVK_MEMBER_INT || member->component == GVK_MEMBER_UINT ||
			member->component == GVK_MEMBER_BOOL);
	if(integersource != integertarget) {
		gLoge("gVKUserShader") << "'" << name << "' is declared as a "
				<< (integertarget ? "integer" : "floating point") << " value in '" << shader->name
				<< "' but was set with a " << (integersource ? "integer" : "floating point") << " one.";
		return false;
	}

	if(member->pushconstant) {
		if(shader->pushshadow.empty()) return false;
		gvkWriteMember(shader->pushshadow.data(), *member, data, rows, columns);
		return true;
	}
	for(gvkUserBlock& block : shader->blocks) {
		if(block.set != member->set || block.binding != member->binding) continue;
		gvkWriteMember(block.shadow.data(), *member, data, rows, columns);
		return true;
	}
	return false;
}

bool gvkSetUserShaderTexture(gVKUserShaderId id, const std::string& name, int unit) {
	gvkUserShader* shader = gvkUserShaderAt(id);
	if(shader == nullptr) return false;
	if(unit < 0 || unit >= GVK_USER_SHADER_MAX_UNITS) {
		gLoge("gVKUserShader") << "Texture unit " << unit << " is outside the range this backend binds (0.."
				<< (GVK_USER_SHADER_MAX_UNITS - 1) << ").";
		return false;
	}
	for(gvkUserSampler& sampler : shader->samplers) {
		if(sampler.name != name) continue;
		sampler.unit = unit;
		return true;
	}
	return false;
}

bool gvkUserShaderHasUniform(gVKUserShaderId id, const std::string& name) {
	gvkUserShader* shader = gvkUserShaderAt(id);
	if(shader == nullptr) return false;
	gVKReflectedMember resolved;
	if(shader->pipeline.reflected.resolveMember(name, resolved)) return true;
	for(const gvkUserSampler& sampler : shader->samplers) {
		if(sampler.name == name) return true;
	}
	return false;
}


// ------------------------------------------------------------------- drawing

// Doubles a shader's ring - its uniform buffers and its descriptor sets - keeping
// what is already recorded in this frame intact. Everything below the old slot
// count keeps its handles and its contents, so a draw already queued still reads
// what it was given.
static bool gvkGrowUserShaderRing(gVKContext& ctx, gvkUserShader& shader) {
	const uint32_t wanted = shader.slotcount * 2;
	if(wanted > GVK_MAX_USER_SHADER_SLOTS) {
		gLogw("gVKUserShader") << "'" << shader.name << "' asked for " << wanted
				<< " uniform slots in one frame, past the " << GVK_MAX_USER_SHADER_SLOTS << " limit.";
		return false;
	}

	// New buffers first: a failure here leaves the shader exactly as it was.
	std::vector<gvkUserBlock> grown;
	grown.reserve(shader.blocks.size());
	for(const gvkUserBlock& block : shader.blocks) {
		gvkUserBlock bigger;
		bigger.set = block.set;
		bigger.binding = block.binding;
		bigger.size = block.size;
		bigger.shadow = block.shadow;
		if(!gvkCreateUserBlockBuffers(ctx, bigger, wanted)) {
			gvkDestroyUserBlock(ctx, bigger);
			for(gvkUserBlock& partial : grown) gvkDestroyUserBlock(ctx, partial);
			return false;
		}
		// Carry over what this frame has already written, so the draws recorded
		// before the growth and those after it agree.
		for(int frame = 0; frame < GVK_MAX_FRAMES_IN_FLIGHT; frame++) {
			if(block.mapped[frame] == nullptr || bigger.mapped[frame] == nullptr) continue;
			for(uint32_t slot = 0; slot < block.slotcount; slot++) {
				std::memcpy(static_cast<uint8_t*>(bigger.mapped[frame]) + bigger.slotstride * slot,
						static_cast<const uint8_t*>(block.mapped[frame]) + block.slotstride * slot,
						block.size);
			}
		}
		grown.push_back(std::move(bigger));
	}

	// The sets point at the old buffers, so they are reallocated wholesale. The
	// pool is created with FREE_DESCRIPTOR_SET, so the old ones go back to it.
	std::vector<std::vector<VkDescriptorSet>> oldsets;
	oldsets.swap(shader.sets);
	std::vector<gvkUserBlock> oldblocks;
	oldblocks.swap(shader.blocks);
	shader.blocks = std::move(grown);
	const uint32_t oldslotcount = shader.slotcount;
	shader.slotcount = wanted;

	if(!gvkAllocateUserSets(ctx, shader)) {
		// Put everything back rather than leave the shader half-grown.
		for(gvkUserBlock& block : shader.blocks) gvkDestroyUserBlock(ctx, block);
		shader.blocks = std::move(oldblocks);
		shader.sets = std::move(oldsets);
		shader.slotcount = oldslotcount;
		return false;
	}

	// Parked, not freed: the draws recorded earlier in this very frame still name
	// the old sets and read the old buffers. See gvkRetiredRing.
	gvkRetiredRing retired;
	retired.blocks = std::move(oldblocks);
	retired.sets = std::move(oldsets);
	retired.framesleft = GVK_MAX_FRAMES_IN_FLIGHT + 1;
	gvkretiredrings.push_back(std::move(retired));
	return true;
}

bool gvkBindUserShaderForDraw(gVKContext& ctx, VkCommandBuffer cmd, bool blending, bool lines) {
	gvkUserShader* shader = gvkUserShaderAt(gvkbounduser);
	if(shader == nullptr || cmd == VK_NULL_HANDLE) return false;
	VkDevice device = *ctx.getDevice();

	const uint32_t frame = ctx.getCurrentFrame();
	// A new frame means the GPU has finished with the previous use of this ring -
	// the frame fence has already been waited on by the time a draw is recorded -
	// so the slots start over.
	if(ctx.getFrameSequence() != shader->lastframesequence) {
		shader->lastframesequence = ctx.getFrameSequence();
		shader->currentslot = 0;
		// A frame boundary is also when a ring grown earlier becomes releasable.
		gvkDrainRetiredRings(ctx, false);
	}
	// A shader drawn more times in a frame than the ring has slots grows the ring
	// rather than overwriting a slot a queued draw is still reading. A bloom
	// pyramid does eight or more passes with different values each time, so a fixed
	// ceiling would be a ceiling on what an effect may do.
	if(shader->currentslot >= shader->slotcount && !gvkGrowUserShaderRing(ctx, *shader)) {
		// Once per shader: a shader drawing past the ceiling does so on every frame,
		// and the warning would then outnumber the draws it is about.
		if(!shader->warnedslotlimit) {
			shader->warnedslotlimit = true;
			gLogw("gVKUserShader") << "'" << shader->name << "' needs more uniform slots than"
					<< " could be allocated; this draw reuses the last one and may show stale values.";
		}
		shader->currentslot = shader->slotcount - 1;
	}
	const uint32_t slot = shader->currentslot;

	const gVKPipelineVariants& variants = shader->pipeline.variants[ctx.getActivePipelineVariant()];
	VkPipeline pipeline = lines ? variants.linepipeline
			: (blending ? variants.blendvariantpipeline : variants.pipeline);
	if(pipeline == VK_NULL_HANDLE) return false;

	// The shadow copies become this slot's contents. Copied rather than written in
	// place because the setters run whenever the application calls them, which is
	// not necessarily between two draws.
	for(gvkUserBlock& block : shader->blocks) {
		if(block.mapped[frame] == nullptr) continue;
		std::memcpy(static_cast<uint8_t*>(block.mapped[frame]) + block.slotstride * slot,
				block.shadow.data(), block.size);
	}

	// Samplers are pointed at whatever each unit holds right now. This is the one
	// part that cannot be prepared ahead of time: the unit's texture is a property
	// of the renderer at the moment of the draw, not of the shader.
	if(!shader->samplers.empty()) {
		std::vector<VkDescriptorImageInfo> imageinfos;
		std::vector<VkWriteDescriptorSet> writes;
		imageinfos.reserve(shader->samplers.size());
		writes.reserve(shader->samplers.size());
		for(const gvkUserSampler& sampler : shader->samplers) {
			gVKTexture* texture = gvkuserresolver != nullptr ? gvkuserresolver(sampler.unit) : nullptr;
			if(texture == nullptr || texture->view == VK_NULL_HANDLE || texture->sampler == VK_NULL_HANDLE) {
				if(!shader->warnedmissingtexture) {
					shader->warnedmissingtexture = true;
					gLogw("gVKUserShader") << "'" << shader->name << "' samples '" << sampler.name
							<< "' from texture unit " << sampler.unit << ", which has no texture bound.";
				}
				return false;
			}
			texture->sampled = true;

			VkDescriptorImageInfo info{};
			// Whatever layout the texture rests in between passes. A mip-chained
			// render target rests in GENERAL, because a level of it is an attachment
			// while the rest are being read.
			info.imageLayout = texture->layout != VK_IMAGE_LAYOUT_UNDEFINED
					? texture->layout : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			info.imageView = texture->view;
			info.sampler = texture->sampler;
			imageinfos.push_back(info);

			VkWriteDescriptorSet write{};
			write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			write.dstSet = shader->sets[sampler.set][frame * shader->slotcount + slot];
			write.dstBinding = sampler.binding;
			write.descriptorCount = 1;
			write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			writes.push_back(write);
		}
		for(size_t i = 0; i < writes.size(); i++) writes[i].pImageInfo = &imageinfos[i];
		vkUpdateDescriptorSets(device, static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);
	}

	// Through the context's own cache, so the engine's next draw sees that its
	// pipeline is no longer the bound one and rebinds rather than trusting a stale
	// handle.
	if(ctx.shouldBindPipeline(pipeline)) vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

	if(!shader->sets.empty()) {
		// The cache compares against a fixed-size array, and Vulkan guarantees only
		// four bound sets anyway, so a shader wanting more is refused rather than
		// silently half-bound.
		if(shader->sets.size() > GVK_MAX_USER_SHADER_SETS) {
			gLoge("gVKUserShader") << "'" << shader->name << "' uses " << shader->sets.size()
					<< " descriptor sets; at most " << GVK_MAX_USER_SHADER_SETS << " can be bound.";
			return false;
		}
		VkDescriptorSet bound[GVK_MAX_USER_SHADER_SETS] = {};
		uint32_t boundcount = 0;
		for(size_t set = 0; set < shader->sets.size(); set++) {
			if(shader->sets[set].empty()) { boundcount = 0; break; }
			bound[boundcount++] = shader->sets[set][frame * shader->slotcount + slot];
		}
		if(boundcount > 0 && ctx.shouldBindDescriptorSets(shader->pipeline.layout, bound, boundcount)) {
			vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, shader->pipeline.layout,
					0, boundcount, bound, 0, nullptr);
		}
	}

	if(shader->pipeline.pushsize > 0 && !shader->pushshadow.empty()) {
		vkCmdPushConstants(cmd, shader->pipeline.layout, shader->pipeline.pushstages, 0,
				shader->pipeline.pushsize, shader->pushshadow.data());
	}

	shader->currentslot++;
	return true;
}

#endif /* GVK_VULKAN */
