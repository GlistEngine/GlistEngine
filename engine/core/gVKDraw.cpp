/*
 * gVKDraw.cpp
 */

#include "gVKDraw.h"

#ifdef GVK_VULKAN

#include "gVKBuffer.h"
#include "gUtils.h"
#include <algorithm>
#include <vector>

// 4 MB of vertices per frame in flight. A batched 2D frame is not the handful of
// triangles the unbatched path recorded: every quad, glyph and outline the frame
// draws now lives here at once, at 48 bytes a vertex, until the batch it belongs
// to is flushed. Overflow is dropped with a warning rather than growing the buffer
// mid-frame, and a dropped draw is a hole in the HUD, so the headroom is real.
static constexpr VkDeviceSize GVK_DYNAMIC_VERTEX_CAPACITY = 4u << 20;

// Enough for a few thousand animated vertices on the first frame; it grows from
// here the moment a frame needs more.
static constexpr VkDeviceSize GVK_MESH_ARENA_INITIAL_CAPACITY = 8 * 1024 * 1024;

// One host visible arena per frame in flight, persistently mapped. Recreated
// rather than resized, which is why the caller drains the device first: the
// buffers being replaced may still be referenced by work in flight.
bool gvkEnsureMeshArena(gVKContext& ctx, VkDeviceSize capacity) {
	if(ctx.device == VK_NULL_HANDLE || capacity == 0) return false;
	if(capacity <= ctx.mesharenacapacity) return true;

	gvkDestroyMeshArena(ctx);

	const int frames = GVK_MAX_FRAMES_IN_FLIGHT;
	ctx.mesharenacapacity = capacity;
	ctx.mesharenabuffers.assign(frames, VK_NULL_HANDLE);
	ctx.mesharenamemories.assign(frames, VK_NULL_HANDLE);
	ctx.mesharenamapped.assign(frames, nullptr);
	ctx.mesharenaoffsets.assign(frames, 0);
	for(int i = 0; i < frames; i++) {
		if(!gvkCreateBuffer(ctx, capacity, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				ctx.mesharenabuffers[i], ctx.mesharenamemories[i],
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
						| VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) {
			gLoge("gVKDraw") << "Could not create the " << capacity
					<< " byte mesh arena; animated meshes fall back to one buffer each.";
			gvkDestroyMeshArena(ctx);
			return false;
		}
		vkMapMemory(ctx.device, ctx.mesharenamemories[i], 0, capacity, 0, &ctx.mesharenamapped[i]);
	}
	gLogi("gVKDraw") << "Mesh arena ready: " << frames << " x " << capacity << " bytes";
	return true;
}

void gvkDestroyMeshArena(gVKContext& ctx) {
	if(ctx.device == VK_NULL_HANDLE) return;
	for(size_t i = 0; i < ctx.mesharenabuffers.size(); i++) {
		if(ctx.mesharenamemories[i] != VK_NULL_HANDLE) {
			vkUnmapMemory(ctx.device, ctx.mesharenamemories[i]);
			vkFreeMemory(ctx.device, ctx.mesharenamemories[i], nullptr);
		}
		if(ctx.mesharenabuffers[i] != VK_NULL_HANDLE) {
			vkDestroyBuffer(ctx.device, ctx.mesharenabuffers[i], nullptr);
		}
	}
	ctx.mesharenabuffers.clear();
	ctx.mesharenamemories.clear();
	ctx.mesharenamapped.clear();
	ctx.mesharenaoffsets.clear();
	ctx.mesharenacapacity = 0;
}

bool gvkCreateDrawResources(gVKContext& ctx) {
	if(ctx.device == VK_NULL_HANDLE) return false;

	const int frames = GVK_MAX_FRAMES_IN_FLIGHT;
	ctx.dynvertexcapacity = GVK_DYNAMIC_VERTEX_CAPACITY;
	ctx.dynvertexbuffers.assign(frames, VK_NULL_HANDLE);
	ctx.dynvertexmemories.assign(frames, VK_NULL_HANDLE);
	ctx.dynvertexmapped.assign(frames, nullptr);
	ctx.dynvertexoffsets.assign(frames, 0);

	for(int i = 0; i < frames; i++) {
		if(!gvkCreateBuffer(ctx, ctx.dynvertexcapacity, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				ctx.dynvertexbuffers[i], ctx.dynvertexmemories[i],
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
						| VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) {
			gLoge("gVKDraw") << "Could not create the dynamic vertex buffer.";
			return false;
		}
		// Host coherent and persistently mapped: no flush and no per-frame remap.
		vkMapMemory(ctx.device, ctx.dynvertexmemories[i], 0, ctx.dynvertexcapacity, 0, &ctx.dynvertexmapped[i]);
	}
	// The mesh arena starts modest and grows to whatever a frame actually asks
	// for; a scene that re-poses one model per enemy needs a slice per draw, and
	// how many that is only the running game knows.
	if(!gvkEnsureMeshArena(ctx, GVK_MESH_ARENA_INITIAL_CAPACITY)) return false;
	gLogi("gVKDraw") << "Dynamic vertex buffers ready.";
	return true;
}

void gvkDestroyDrawResources(gVKContext& ctx) {
	gvkDestroyMeshArena(ctx);
	if(ctx.device == VK_NULL_HANDLE) return;
	for(size_t i = 0; i < ctx.dynvertexbuffers.size(); i++) {
		if(ctx.dynvertexmapped[i] != nullptr) vkUnmapMemory(ctx.device, ctx.dynvertexmemories[i]);
		if(ctx.dynvertexbuffers[i] != VK_NULL_HANDLE) vkDestroyBuffer(ctx.device, ctx.dynvertexbuffers[i], nullptr);
		if(ctx.dynvertexmemories[i] != VK_NULL_HANDLE) vkFreeMemory(ctx.device, ctx.dynvertexmemories[i], nullptr);
	}
	ctx.dynvertexbuffers.clear();
	ctx.dynvertexmemories.clear();
	ctx.dynvertexmapped.clear();
	ctx.dynvertexoffsets.clear();
	ctx.dynvertexcapacity = 0;
}

// The 2D vertex, matching gvk2dattributes in gVKPipeline.cpp. Position is already
// in clip space: the transform is applied here rather than in the shader, and the
// tint travels with the vertex rather than in a push constant. Both are what let
// consecutive 2D draws share one vkCmdDraw - a push constant cannot vary within a
// draw, so as long as the matrix and the colour lived there, every shape and every
// image had to be its own draw call.
//
// The position is a full vec4 rather than the vec2 the old layout used, so an
// arbitrary transform survives: the 2D projection is orthographic today, but a
// caller handing in a perspective matrix would otherwise lose z and w.
struct gvk2DVertex {
	glm::vec4 pos;
	glm::vec4 color;
	glm::vec2 uv;
	glm::vec2 pad;
};
static_assert(sizeof(gvk2DVertex) == 48, "gvk2DVertex no longer matches GVK_2D_VERTEX_STRIDE");

// What the image shaders still push: which sampler the mask branch reads. It
// cannot move into the vertex, so it is part of the batch key instead.
struct gvk2DPush {
	int masking = 0;
};

// The open 2D batch. One at a time is enough: 2D is ordered by draw call and has
// to stay in the order the game issued it, so a draw that cannot join the batch
// ends it rather than starting a second one alongside.
//
// The vertices themselves are already in the frame's dynamic buffer - a draw
// appends them the moment it arrives. What the batch holds is only where its run
// starts and how long it has grown, so flushing is one vkCmdDraw over a range
// that is contiguous by construction.
struct gVK2DBatch {
	VkPipeline pipeline = VK_NULL_HANDLE;
	VkPipelineLayout layout = VK_NULL_HANDLE;
	VkDescriptorSet sets[2] = {VK_NULL_HANDLE, VK_NULL_HANDLE};
	uint32_t setcount = 0;
	uint32_t pushsize = 0;
	VkShaderStageFlags pushstages = 0;
	int masking = 0;
	VkDeviceSize beginoffset = 0;
	VkDeviceSize endoffset = 0;
	uint32_t vertexcount = 0;
};
static gVK2DBatch gvk2dbatch;

void gvkReset2DBatch() {
	gvk2dbatch.vertexcount = 0;
}

void gvkFlush2DBatch(gVKContext& ctx) {
	if(gvk2dbatch.vertexcount == 0) return;
	VkCommandBuffer cmd = ctx.getCurrentCommandBuffer();
	// Dropped rather than recorded if the pass closed underneath it. That only
	// happens on a frame that failed somewhere earlier, and recording into a
	// finished pass would be worse than losing the geometry.
	if(cmd == VK_NULL_HANDLE || !ctx.isRenderPassActive()) {
		gvk2dbatch.vertexcount = 0;
		return;
	}

	if(ctx.shouldBindPipeline(gvk2dbatch.pipeline)) {
		vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, gvk2dbatch.pipeline);
	}
	// Bound at zero and reached through firstVertex rather than at the run's own
	// offset, so the binding is identical for every 2D draw in the frame and the
	// cache collapses them into one bind. Exact arithmetic, not an approximation:
	// pushDynamicVertices hands out 16 byte aligned slices and the 2D stride is a
	// multiple of 16.
	VkBuffer vbuf = ctx.getCurrentDynamicVertexBuffer();
	const VkDeviceSize bindoffset = 0;
	if(ctx.shouldBindVertexBuffers(&vbuf, &bindoffset, 1)) {
		vkCmdBindVertexBuffers(cmd, 0, 1, &vbuf, &bindoffset);
	}
	if(gvk2dbatch.setcount > 0
			&& ctx.shouldBindDescriptorSets(gvk2dbatch.layout, gvk2dbatch.sets, gvk2dbatch.setcount)) {
		vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, gvk2dbatch.layout,
				0, gvk2dbatch.setcount, gvk2dbatch.sets, 0, nullptr);
	}
	if(gvk2dbatch.pushsize > 0) {
		const gvk2DPush push{gvk2dbatch.masking};
		vkCmdPushConstants(cmd, gvk2dbatch.layout, gvk2dbatch.pushstages, 0,
				std::min<uint32_t>(sizeof(push), gvk2dbatch.pushsize), &push);
	}
	vkCmdDraw(cmd, gvk2dbatch.vertexcount, 1,
			static_cast<uint32_t>(gvk2dbatch.beginoffset / sizeof(gvk2DVertex)), 0);
	gvk2dbatch.vertexcount = 0;
}

// Appends one draw's vertices to the open batch, or ends that batch and starts a
// new one when this draw cannot share its state. The vertices go into the frame's
// buffer first: the offset they land at is itself part of what decides whether the
// run is still contiguous, and pushing does not disturb the batch already open.
static bool gvkAppend2D(gVKContext& ctx, VkPipeline pipeline, VkPipelineLayout layout,
		const VkDescriptorSet* sets, uint32_t setcount, uint32_t pushsize, VkShaderStageFlags pushstages,
		int masking, const gvk2DVertex* vertices, uint32_t count) {
	if(pipeline == VK_NULL_HANDLE || count == 0) return false;
	const VkDeviceSize offset = ctx.pushDynamicVertices(vertices, sizeof(gvk2DVertex) * count,
			sizeof(gvk2DVertex));
	if(offset == VK_WHOLE_SIZE) {
		gLogw("gVKDraw") << "Dynamic vertex buffer full; dropping a 2D draw.";
		return false;
	}

	const bool joins = gvk2dbatch.vertexcount > 0
			&& gvk2dbatch.pipeline == pipeline
			&& gvk2dbatch.layout == layout
			&& gvk2dbatch.setcount == setcount
			&& (setcount == 0 || std::equal(sets, sets + setcount, gvk2dbatch.sets))
			&& gvk2dbatch.masking == masking
			&& gvk2dbatch.endoffset == offset;
	if(!joins) {
		gvkFlush2DBatch(ctx);
		gvk2dbatch.pipeline = pipeline;
		gvk2dbatch.layout = layout;
		gvk2dbatch.setcount = setcount;
		if(setcount > 0) std::copy(sets, sets + setcount, gvk2dbatch.sets);
		gvk2dbatch.pushsize = pushsize;
		gvk2dbatch.pushstages = pushstages;
		gvk2dbatch.masking = masking;
		gvk2dbatch.beginoffset = offset;
		gvk2dbatch.vertexcount = 0;
	}
	gvk2dbatch.vertexcount += count;
	gvk2dbatch.endoffset = offset + sizeof(gvk2DVertex) * count;
	return true;
}

void gvkDrawColored2D(gVKContext& ctx, const glm::vec2* points, int count,
		const glm::vec4& color, const glm::mat4& mvp, int mode, bool additive) {
	if(count <= 0 || points == nullptr) return;

	// Only a triangle list and a line list exist as pipelines, so every other mode
	// is expanded into one of them here. The scratch buffer is reused between calls
	// so an expanded draw still allocates nothing. Triangle strips alternate the
	// first two corners to keep the winding consistent, matching what OpenGL feeds
	// the rasteriser.
	static thread_local std::vector<glm::vec2> expanded;
	const void* vertexdata = points;
	int vertexcount = count;
	if(mode != GVK_DRAW2D_TRIANGLES && mode != GVK_DRAW2D_LINES) {
		expanded.clear();
		switch(mode) {
		case GVK_DRAW2D_TRIANGLESTRIP:
			expanded.reserve(static_cast<size_t>(std::max(count - 2, 0)) * 3);
			for(int i = 2; i < count; i++) {
				if(i % 2 == 0) {
					expanded.push_back(points[i - 2]);
					expanded.push_back(points[i - 1]);
				} else {
					expanded.push_back(points[i - 1]);
					expanded.push_back(points[i - 2]);
				}
				expanded.push_back(points[i]);
			}
			break;
		case GVK_DRAW2D_TRIANGLEFAN:
			expanded.reserve(static_cast<size_t>(std::max(count - 2, 0)) * 3);
			for(int i = 2; i < count; i++) {
				expanded.push_back(points[0]);
				expanded.push_back(points[i - 1]);
				expanded.push_back(points[i]);
			}
			break;
		case GVK_DRAW2D_LINESTRIP:
			expanded.reserve(static_cast<size_t>(std::max(count - 1, 0)) * 2);
			for(int i = 1; i < count; i++) {
				expanded.push_back(points[i - 1]);
				expanded.push_back(points[i]);
			}
			break;
		case GVK_DRAW2D_LINELOOP:
			expanded.reserve(static_cast<size_t>(count) * 2);
			for(int i = 0; i < count; i++) {
				expanded.push_back(points[i]);
				expanded.push_back(points[(i + 1) % count]);
			}
			break;
		default:
			return;
		}
		if(expanded.empty()) return;
		vertexdata = expanded.data();
		vertexcount = static_cast<int>(expanded.size());
	}

	const bool lines = mode == GVK_DRAW2D_LINES || mode == GVK_DRAW2D_LINESTRIP || mode == GVK_DRAW2D_LINELOOP;
	if(!gvkEnsureRenderPass(ctx)) return;
	if(ctx.getCurrentCommandBuffer() == VK_NULL_HANDLE) return;
	VkPipeline pipeline = lines ? ctx.getColor2DLinePipeline()
			: (additive ? ctx.getColor2DAdditivePipeline() : ctx.getColor2DPipeline());
	if(pipeline == VK_NULL_HANDLE) return;

	// The transform the shader used to apply, applied here instead, so the batch
	// can hold shapes that were placed by different matrices. The points are 2D, so
	// the third component is zero and the fourth one.
	static thread_local std::vector<gvk2DVertex> packed;
	const glm::vec2* source = static_cast<const glm::vec2*>(vertexdata);
	packed.clear();
	packed.reserve(static_cast<size_t>(vertexcount));
	for(int i = 0; i < vertexcount; i++) {
		gvk2DVertex vertex;
		vertex.pos = mvp * glm::vec4(source[i], 0.0f, 1.0f);
		vertex.color = color;
		vertex.uv = glm::vec2(0.0f);
		vertex.pad = glm::vec2(0.0f);
		packed.push_back(vertex);
	}

	gvkAppend2D(ctx, pipeline, ctx.getColor2DPipelineLayout(), nullptr, 0,
			ctx.getColor2DPushSize(), ctx.getColor2DPushStages(), 0,
			packed.data(), static_cast<uint32_t>(packed.size()));
}

void gvkDrawTextured2D(gVKContext& ctx, VkDescriptorSet textureSet, VkDescriptorSet maskSet,
		const glm::vec4& tint, const glm::mat4& mvp,
		const glm::vec2& uvOffset, const glm::vec2& uvScale, bool additive) {
	if(!gvkEnsureRenderPass(ctx)) return;
	if(ctx.getCurrentCommandBuffer() == VK_NULL_HANDLE
			|| ctx.getImage2DPipeline() == VK_NULL_HANDLE || textureSet == VK_NULL_HANDLE) return;
	VkPipeline imagepipeline = additive ? ctx.getImage2DAdditivePipeline() : ctx.getImage2DPipeline();

	// Unit quad in [0,1] put through the mvp (projection2d * image model matrix)
	// here rather than in the shader, exactly the same placement the OpenGL image
	// quad gets. The texture coordinates carry the sub-rectangle instead of a shader
	// uniform, so a whole-texture draw and a sub-part draw share one pipeline.
	const glm::vec2 uv0 = uvOffset;
	const glm::vec2 uv1 = uvOffset + uvScale;
	const glm::vec2 corners[6] = {
		{0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f},
		{0.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f},
	};
	const glm::vec2 uvs[6] = {
		{uv0.x, uv0.y}, {uv1.x, uv0.y}, {uv1.x, uv1.y},
		{uv0.x, uv0.y}, {uv1.x, uv1.y}, {uv0.x, uv1.y},
	};
	gvk2DVertex quad[6];
	for(int i = 0; i < 6; i++) {
		quad[i].pos = mvp * glm::vec4(corners[i], 0.0f, 1.0f);
		quad[i].color = tint;
		quad[i].uv = uvs[i];
		quad[i].pad = glm::vec2(0.0f);
	}

	// The fragment shader names the mask sampler whether or not it is read, so set 1
	// always needs a valid set bound. Unmasked draws bind the image's own set there
	// and turn the branch off through the push constant.
	const bool masking = maskSet != VK_NULL_HANDLE;
	VkDescriptorSet sets[2] = {textureSet, masking ? maskSet : textureSet};
	gvkAppend2D(ctx, imagepipeline, ctx.getImage2DPipelineLayout(), sets, 2,
			ctx.getImage2DPushSize(), ctx.getImage2DPushStages(), masking ? 1 : 0, quad, 6);
}

void gvkDrawMesh3D(gVKContext& ctx, VkBuffer vertexBuffer, VkDeviceSize vertexOffset,
		VkBuffer indexBuffer, int count,
		VkIndexType indexType, const gVKMeshPush& push,
		VkDescriptorSet materialSet, VkDescriptorSet shadowSet,
		VkBuffer instanceBuffer, VkDeviceSize instanceOffset, int instanceCount,
		VkPrimitiveTopology topology, bool depthTest, bool depthTestAlways, bool lines,
		const gVKCullState& culling, bool blending, bool cutout) {
	if(count <= 0 || vertexBuffer == VK_NULL_HANDLE) return;
	if(!gvkEnsureRenderPass(ctx)) return;
	// 2D is ordered by draw call, so anything still batched belongs in front of this
	// mesh and has to reach the command buffer before it does.
	gvkFlush2DBatch(ctx);

	VkCommandBuffer cmd = ctx.getCurrentCommandBuffer();
	if(cmd == VK_NULL_HANDLE) return;
	VkPipeline pipeline = lines ? ctx.getMesh3DLinePipeline() : ctx.getMesh3DPipeline(blending, cutout);
	if(pipeline == VK_NULL_HANDLE) return;

	if(ctx.shouldBindPipeline(pipeline)) vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

	// Depth state follows the renderer's, the same way glEnable(GL_DEPTH_TEST) does
	// on the OpenGL side. Writing is tied to testing: the engine has no separate
	// switch for it, and a depth write with no test would let a hidden surface claim
	// the depth buffer. ALWAYS still writes, which matches glDepthFunc(GL_ALWAYS).
	const VkBool32 depthenabled = depthTest ? VK_TRUE : VK_FALSE;
	const VkCompareOp depthcompare = depthTestAlways ? VK_COMPARE_OP_ALWAYS : VK_COMPARE_OP_LESS;
	if(ctx.shouldSetDepthState(depthenabled, depthenabled, depthcompare)) {
		vkCmdSetDepthTestEnable(cmd, depthenabled);
		vkCmdSetDepthWriteEnable(cmd, depthenabled);
		vkCmdSetDepthCompareOp(cmd, depthcompare);
	}
	// The mesh's own draw mode, within the class the bound pipeline was built for.
	if(ctx.shouldSetTopology(topology)) vkCmdSetPrimitiveTopology(cmd, topology);
	// Culling follows the renderer too, and both states have to be set because the
	// 3D pipelines declare them dynamic.
	if(ctx.shouldSetCullState(culling.mode, culling.frontface)) {
		vkCmdSetCullMode(cmd, culling.mode);
		vkCmdSetFrontFace(cmd, culling.frontface);
	}

	// Camera and lights, the same for every mesh in the frame. Bound per draw rather
	// than once per frame because the 2D pipelines are interleaved with these and
	// binding a pipeline with an incompatible layout invalidates the binding.
	// Sets 0..2 in one call: the scene block, the material's three maps and the
	// shadow map. Bound together because they are contiguous and the layout expects
	// all three. Three and not five: Vulkan guarantees only four bound sets, and
	// Adreno and Mali offer exactly four, so a set per texture would leave this
	// pipeline uncreatable on most Android phones.
	VkDescriptorSet sceneset = ctx.getCurrentSceneDescriptorSet();
	if(sceneset != VK_NULL_HANDLE && materialSet != VK_NULL_HANDLE
			&& shadowSet != VK_NULL_HANDLE) {
		VkDescriptorSet sets[] = {sceneset, materialSet, shadowSet};
		if(ctx.shouldBindDescriptorSets(ctx.getMesh3DPipelineLayout(), sets, 3)) {
			vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, ctx.getMesh3DPipelineLayout(),
					0, 3, sets, 0, nullptr);
		}
	}

	// Straight from the mesh's own device local buffer - no staging, no per-frame
	// copy, which is the whole point of uploading a mesh once. Binding 1 carries the
	// per-instance model matrices and is never absent; see gVKRenderEngine's
	// identity instance buffer.
	VkBuffer buffers[] = {vertexBuffer, instanceBuffer};
	VkDeviceSize offsets[] = {vertexOffset, instanceOffset};
	const uint32_t bindingcount = instanceBuffer != VK_NULL_HANDLE ? 2u : 1u;
	if(ctx.shouldBindVertexBuffers(buffers, offsets, bindingcount)) {
		vkCmdBindVertexBuffers(cmd, 0, bindingcount, buffers, offsets);
	}

	const uint32_t pushsize = std::min<uint32_t>(sizeof(push), ctx.getMesh3DPushSize());
	if(pushsize > 0) {
		vkCmdPushConstants(cmd, ctx.getMesh3DPipelineLayout(), ctx.getMesh3DPushStages(), 0, pushsize, &push);
	}

	const uint32_t instances = static_cast<uint32_t>(instanceCount < 1 ? 1 : instanceCount);
	if(indexBuffer != VK_NULL_HANDLE) {
		if(ctx.shouldBindIndexBuffer(indexBuffer, 0, indexType)) {
			vkCmdBindIndexBuffer(cmd, indexBuffer, 0, indexType);
		}
		vkCmdDrawIndexed(cmd, static_cast<uint32_t>(count), instances, 0, 0, 0);
	} else {
		vkCmdDraw(cmd, static_cast<uint32_t>(count), instances, 0, 0);
	}
}

void gvkDrawShadowCaster(gVKContext& ctx, VkBuffer vertexBuffer, VkDeviceSize vertexOffset,
		VkBuffer indexBuffer,
		int count, VkIndexType indexType, const gVKShadowPush& push,
		VkDescriptorSet diffuseSet,
		VkBuffer instanceBuffer, VkDeviceSize instanceOffset, int instanceCount, VkPrimitiveTopology topology) {
	if(count <= 0 || vertexBuffer == VK_NULL_HANDLE) return;
	// No gvkEnsureRenderPass here: the shadow pass is opened by the frame loop
	// before the scene is drawn, and this must not fall back to opening the screen
	// pass instead.
	if(!ctx.isShadowPassActive()) return;

	VkCommandBuffer cmd = ctx.getCurrentCommandBuffer();
	if(cmd == VK_NULL_HANDLE || ctx.getShadowPipeline() == VK_NULL_HANDLE) return;

	// Shadow recording deliberately emits complete state. It is a separate render
	// pass and must not inherit assumptions cached while recording the scene pass.
	if(ctx.shouldBindPipeline(ctx.getShadowPipeline())) {
		vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, ctx.getShadowPipeline());
	}

	// One set, the caster's diffuse map, and only so a cutout material can discard
	// its holes. An opaque mesh binds the white texture here and never samples it.
	//
	// Guarded on the layout actually declaring a set rather than assumed: the
	// reflection that builds this layout only sees the sampler while the shader
	// still reads it, so an edit to shadow3d.frag that stops sampling - live shader
	// reload makes that a runtime possibility, not just a build-time one - leaves a
	// layout with no sets, and binding one into it crashes the driver.
	if(ctx.hasShadowDescriptorSetLayout() && diffuseSet != VK_NULL_HANDLE
			&& ctx.shouldBindDescriptorSets(ctx.getShadowPipelineLayout(), &diffuseSet, 1)) {
		vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, ctx.getShadowPipelineLayout(),
				0, 1, &diffuseSet, 0, nullptr);
	}

	VkBuffer buffers[] = {vertexBuffer, instanceBuffer};
	VkDeviceSize offsets[] = {vertexOffset, instanceOffset};
	const uint32_t bindingcount = instanceBuffer != VK_NULL_HANDLE ? 2u : 1u;
	if(ctx.shouldBindVertexBuffers(buffers, offsets, bindingcount)) {
		vkCmdBindVertexBuffers(cmd, 0, bindingcount, buffers, offsets);
	}

	const uint32_t pushsize = std::min<uint32_t>(sizeof(push), ctx.getShadowPushSize());
	if(pushsize > 0) {
		vkCmdPushConstants(cmd, ctx.getShadowPipelineLayout(), ctx.getShadowPushStages(),
				0, pushsize, &push);
	}

	// This pipeline declares the same dynamic state as the shading one, so depth has
	// to be set even though it never varies here: the map is the pass's only output,
	// and a caster that skipped the test would leave the wrong distance in it. The
	// scene's own enableDepthTest deliberately has no say.
	if(ctx.shouldSetDepthState(VK_TRUE, VK_TRUE, VK_COMPARE_OP_LESS)) {
		vkCmdSetDepthTestEnable(cmd, VK_TRUE);
		vkCmdSetDepthWriteEnable(cmd, VK_TRUE);
		vkCmdSetDepthCompareOp(cmd, VK_COMPARE_OP_LESS);
	}
	if(ctx.shouldSetTopology(topology)) vkCmdSetPrimitiveTopology(cmd, topology);
	// Casters are never culled, whatever the scene asked for: a shadow wants the
	// whole silhouette, and dropping back faces would punch holes in it.
	if(ctx.shouldSetCullState(VK_CULL_MODE_NONE, VK_FRONT_FACE_CLOCKWISE)) {
		vkCmdSetCullMode(cmd, VK_CULL_MODE_NONE);
		vkCmdSetFrontFace(cmd, VK_FRONT_FACE_CLOCKWISE);
	}

	const uint32_t instances = static_cast<uint32_t>(instanceCount < 1 ? 1 : instanceCount);
	if(indexBuffer != VK_NULL_HANDLE) {
		if(ctx.shouldBindIndexBuffer(indexBuffer, 0, indexType)) {
			vkCmdBindIndexBuffer(cmd, indexBuffer, 0, indexType);
		}
		vkCmdDrawIndexed(cmd, static_cast<uint32_t>(count), instances, 0, 0, 0);
	} else {
		vkCmdDraw(cmd, static_cast<uint32_t>(count), instances, 0, 0);
	}
}

void gvkDrawMesh3DPbr(gVKContext& ctx, VkBuffer vertexBuffer, VkDeviceSize vertexOffset,
		VkBuffer indexBuffer, int count,
		VkIndexType indexType, const gVKPbrPush& push, VkDescriptorSet materialSet,
		VkDescriptorSet shadowSet,
		VkBuffer instanceBuffer, VkDeviceSize instanceOffset, int instanceCount,
		VkPrimitiveTopology topology, bool depthTest, bool depthTestAlways,
		const gVKCullState& culling, bool blending) {
	if(count <= 0 || vertexBuffer == VK_NULL_HANDLE) return;
	if(!gvkEnsureRenderPass(ctx)) return;
	// 2D is ordered by draw call, so anything still batched belongs in front of this
	// mesh and has to reach the command buffer before it does.
	gvkFlush2DBatch(ctx);

	VkCommandBuffer cmd = ctx.getCurrentCommandBuffer();
	if(cmd == VK_NULL_HANDLE) return;
	VkPipeline pipeline = ctx.getMesh3DPbrPipeline(blending);
	if(pipeline == VK_NULL_HANDLE) return;

	VkDescriptorSet sceneset = ctx.getCurrentSceneDescriptorSet();
	if(sceneset == VK_NULL_HANDLE || materialSet == VK_NULL_HANDLE ||
			shadowSet == VK_NULL_HANDLE) return;

	if(ctx.shouldBindPipeline(pipeline)) vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

	const VkBool32 depthenabled = depthTest ? VK_TRUE : VK_FALSE;
	const VkCompareOp depthcompare = depthTestAlways ? VK_COMPARE_OP_ALWAYS : VK_COMPARE_OP_LESS;
	if(ctx.shouldSetDepthState(depthenabled, depthenabled, depthcompare)) {
		vkCmdSetDepthTestEnable(cmd, depthenabled);
		vkCmdSetDepthWriteEnable(cmd, depthenabled);
		vkCmdSetDepthCompareOp(cmd, depthcompare);
	}
	if(ctx.shouldSetTopology(topology)) vkCmdSetPrimitiveTopology(cmd, topology);
	if(ctx.shouldSetCullState(culling.mode, culling.frontface)) {
		vkCmdSetCullMode(cmd, culling.mode);
		vkCmdSetFrontFace(cmd, culling.frontface);
	}

	// Three sets: the scene block, the whole material, and the shadow map.
	VkDescriptorSet sets[] = {sceneset, materialSet, shadowSet};
	if(ctx.shouldBindDescriptorSets(ctx.getMesh3DPbrPipelineLayout(), sets, 3)) {
		vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, ctx.getMesh3DPbrPipelineLayout(),
				0, 3, sets, 0, nullptr);
	}

	VkBuffer buffers[] = {vertexBuffer, instanceBuffer};
	VkDeviceSize offsets[] = {vertexOffset, instanceOffset};
	const uint32_t bindingcount = instanceBuffer != VK_NULL_HANDLE ? 2u : 1u;
	if(ctx.shouldBindVertexBuffers(buffers, offsets, bindingcount)) {
		vkCmdBindVertexBuffers(cmd, 0, bindingcount, buffers, offsets);
	}

	const uint32_t pushsize = std::min<uint32_t>(sizeof(push), ctx.getMesh3DPbrPushSize());
	if(pushsize > 0) {
		vkCmdPushConstants(cmd, ctx.getMesh3DPbrPipelineLayout(), ctx.getMesh3DPbrPushStages(),
				0, pushsize, &push);
	}

	const uint32_t instances = static_cast<uint32_t>(instanceCount < 1 ? 1 : instanceCount);
	if(indexBuffer != VK_NULL_HANDLE) {
		if(ctx.shouldBindIndexBuffer(indexBuffer, 0, indexType)) {
			vkCmdBindIndexBuffer(cmd, indexBuffer, 0, indexType);
		}
		vkCmdDrawIndexed(cmd, static_cast<uint32_t>(count), instances, 0, 0, 0);
	} else {
		vkCmdDraw(cmd, static_cast<uint32_t>(count), instances, 0, 0);
	}
}

void gvkDrawTexturedTriangles2D(gVKContext& ctx, VkDescriptorSet textureSet,
		const glm::vec4& tint, const glm::mat4& mvp, const float* xyuv, int vertexCount,
		bool additive) {
	if(xyuv == nullptr || vertexCount <= 0 || !gvkEnsureRenderPass(ctx)) return;
	if(ctx.getCurrentCommandBuffer() == VK_NULL_HANDLE
			|| ctx.getImage2DPipeline() == VK_NULL_HANDLE || textureSet == VK_NULL_HANDLE) return;
	VkPipeline imagepipeline = additive ? ctx.getImage2DAdditivePipeline() : ctx.getImage2DPipeline();

	// The caller hands over interleaved x, y, u, v - gFont's atlas run, mostly. It
	// is repacked rather than copied straight through because the batch's vertex
	// carries the transform and the tint with it; that is what lets a run of text
	// and the images around it share one draw when they share a texture.
	static thread_local std::vector<gvk2DVertex> packed;
	packed.clear();
	packed.reserve(static_cast<size_t>(vertexCount));
	for(int i = 0; i < vertexCount; i++) {
		const float* source = xyuv + static_cast<size_t>(i) * 4;
		gvk2DVertex vertex;
		vertex.pos = mvp * glm::vec4(source[0], source[1], 0.0f, 1.0f);
		vertex.color = tint;
		vertex.uv = glm::vec2(source[2], source[3]);
		vertex.pad = glm::vec2(0.0f);
		packed.push_back(vertex);
	}

	VkDescriptorSet sets[2] = {textureSet, textureSet};
	gvkAppend2D(ctx, imagepipeline, ctx.getImage2DPipelineLayout(), sets, 2,
			ctx.getImage2DPushSize(), ctx.getImage2DPushStages(), 0,
			packed.data(), static_cast<uint32_t>(packed.size()));
}

void gvkDrawSkyboxFace(gVKContext& ctx, VkDescriptorSet faceSet, const float* xyzuv,
		int vertexCount, const glm::mat4& viewProjection, VkCompareOp depthCompare) {
	if(xyzuv == nullptr || vertexCount <= 0 || faceSet == VK_NULL_HANDLE) return;
	if(!gvkEnsureRenderPass(ctx)) return;

	// The sky is drawn behind the scene but still after whatever 2D preceded it in
	// call order, so the batch is closed first - see gvkDrawMesh3D.
	gvkFlush2DBatch(ctx);

	VkCommandBuffer cmd = ctx.getCurrentCommandBuffer();
	if(cmd == VK_NULL_HANDLE || ctx.getSkyboxPipeline() == VK_NULL_HANDLE) return;

	// Five floats per vertex: position then texture coordinate.
	const VkDeviceSize bytes = static_cast<VkDeviceSize>(vertexCount) * 5 * sizeof(float);
	VkDeviceSize offset = ctx.pushDynamicVertices(xyzuv, bytes);
	if(offset == VK_WHOLE_SIZE) {
		gLogw("gVKDraw") << "Dynamic vertex buffer full; dropping a skybox face.";
		return;
	}

	if(ctx.shouldBindPipeline(ctx.getSkyboxPipeline())) {
		vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, ctx.getSkyboxPipeline());
	}
	// Depth is tested but never written: the sky sits behind everything, and letting
	// it claim depth would hide geometry drawn after it.
	if(ctx.shouldSetDepthState(VK_TRUE, VK_FALSE, depthCompare)) {
		vkCmdSetDepthTestEnable(cmd, VK_TRUE);
		vkCmdSetDepthWriteEnable(cmd, VK_FALSE);
		vkCmdSetDepthCompareOp(cmd, depthCompare);
	}
	if(ctx.shouldSetTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)) {
		vkCmdSetPrimitiveTopology(cmd, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
	}

	VkBuffer vbuf = ctx.getCurrentDynamicVertexBuffer();
	if(ctx.shouldBindVertexBuffers(&vbuf, &offset, 1)) {
		vkCmdBindVertexBuffers(cmd, 0, 1, &vbuf, &offset);
	}
	if(ctx.shouldBindDescriptorSets(ctx.getSkyboxPipelineLayout(), &faceSet, 1)) {
		vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, ctx.getSkyboxPipelineLayout(),
				0, 1, &faceSet, 0, nullptr);
	}

	const uint32_t pushsize = std::min<uint32_t>(sizeof(viewProjection), ctx.getSkyboxPushSize());
	if(pushsize > 0) {
		vkCmdPushConstants(cmd, ctx.getSkyboxPipelineLayout(), ctx.getSkyboxPushStages(),
				0, pushsize, &viewProjection);
	}
	vkCmdDraw(cmd, static_cast<uint32_t>(vertexCount), 1, 0, 0);
}

#endif /* GVK_VULKAN */
