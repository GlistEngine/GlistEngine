/*
 * gVKDraw.cpp
 */

#include "gVKDraw.h"

#ifdef GVK_DESKTOP_GLFW

#include "gVKBuffer.h"
#include "gUtils.h"
#include <algorithm>
#include <vector>

// 1 MB of vertices per frame in flight. A 2D frame records only a handful of
// triangles and quads, so this never fills in practice; overflow is dropped with
// a warning rather than growing the buffer mid-frame.
static constexpr VkDeviceSize GVK_DYNAMIC_VERTEX_CAPACITY = 1u << 20;

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
				ctx.mesharenabuffers[i], ctx.mesharenamemories[i])) {
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
				ctx.dynvertexbuffers[i], ctx.dynvertexmemories[i])) {
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

// mvp + colour, matching the push_constant block in color2d.vert / image2d.*. The
// image shaders carry one field more; the colour ones push only the first 80 bytes,
// because the size actually pushed comes from reflecting each shader.
struct gvkPush {
	glm::mat4 mvp;
	glm::vec4 color;
	int masking = 0;
};
struct gvkImageVertex {
	glm::vec2 pos;
	glm::vec2 uv;
};

void gvkDrawColored2D(gVKContext& ctx, const glm::vec2* points, int count,
		const glm::vec4& color, const glm::mat4& mvp, int mode) {
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
	VkCommandBuffer cmd = ctx.getCurrentCommandBuffer();
	if(cmd == VK_NULL_HANDLE) return;
	VkPipeline pipeline = lines ? ctx.getColor2DLinePipeline() : ctx.getColor2DPipeline();
	if(pipeline == VK_NULL_HANDLE) return;

	VkDeviceSize offset = ctx.pushDynamicVertices(vertexdata, sizeof(glm::vec2) * vertexcount);
	if(offset == VK_WHOLE_SIZE) {
		gLogw("gVKDraw") << "Dynamic vertex buffer full; dropping a coloured draw.";
		return;
	}

	if(ctx.shouldBindPipeline(pipeline)) vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
	VkBuffer vbuf = ctx.getCurrentDynamicVertexBuffer();
	vkCmdBindVertexBuffers(cmd, 0, 1, &vbuf, &offset);

	// Size and stages come from reflecting the shader, so a change to its
	// push_constant block needs no matching edit here.
	gvkPush push{mvp, color};
	const uint32_t pushsize = std::min<uint32_t>(sizeof(push), ctx.getColor2DPushSize());
	if(pushsize > 0) {
		vkCmdPushConstants(cmd, ctx.getColor2DPipelineLayout(), ctx.getColor2DPushStages(), 0, pushsize, &push);
	}
	vkCmdDraw(cmd, static_cast<uint32_t>(vertexcount), 1, 0, 0);
}

void gvkDrawTextured2D(gVKContext& ctx, VkDescriptorSet textureSet, VkDescriptorSet maskSet,
		const glm::vec4& tint, const glm::mat4& mvp,
		const glm::vec2& uvOffset, const glm::vec2& uvScale) {
	if(!gvkEnsureRenderPass(ctx)) return;
	VkCommandBuffer cmd = ctx.getCurrentCommandBuffer();
	if(cmd == VK_NULL_HANDLE || ctx.getImage2DPipeline() == VK_NULL_HANDLE || textureSet == VK_NULL_HANDLE) return;

	// Unit quad in [0,1]; the mvp (projection2d * image model matrix) scales and
	// places it, exactly like the OpenGL image quad. The texture coordinates carry
	// the sub-rectangle instead of a shader uniform, so a whole-texture draw and a
	// sub-part draw share one pipeline.
	const glm::vec2 uv0 = uvOffset;
	const glm::vec2 uv1 = uvOffset + uvScale;
	const gvkImageVertex quad[6] = {
		{{0.0f, 0.0f}, {uv0.x, uv0.y}}, {{1.0f, 0.0f}, {uv1.x, uv0.y}}, {{1.0f, 1.0f}, {uv1.x, uv1.y}},
		{{0.0f, 0.0f}, {uv0.x, uv0.y}}, {{1.0f, 1.0f}, {uv1.x, uv1.y}}, {{0.0f, 1.0f}, {uv0.x, uv1.y}},
	};
	VkDeviceSize offset = ctx.pushDynamicVertices(quad, sizeof(quad));
	if(offset == VK_WHOLE_SIZE) {
		gLogw("gVKDraw") << "Dynamic vertex buffer full; dropping a textured draw.";
		return;
	}

	if(ctx.shouldBindPipeline(ctx.getImage2DPipeline())) {
		vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, ctx.getImage2DPipeline());
	}
	VkBuffer vbuf = ctx.getCurrentDynamicVertexBuffer();
	vkCmdBindVertexBuffers(cmd, 0, 1, &vbuf, &offset);
	VkPipelineLayout layout = ctx.getImage2DPipelineLayout();
	// The fragment shader names the mask sampler whether or not it is read, so set 1
	// always needs a valid set bound. Unmasked draws bind the image's own set there
	// and turn the branch off through the push constant.
	const bool masking = maskSet != VK_NULL_HANDLE;
	VkDescriptorSet sets[2] = {textureSet, masking ? maskSet : textureSet};
	if(ctx.shouldBindDescriptorSets(layout, sets, 2)) {
		vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 0, 2, sets, 0, nullptr);
	}

	gvkPush push{mvp, tint, masking ? 1 : 0};
	const uint32_t pushsize = std::min<uint32_t>(sizeof(push), ctx.getImage2DPushSize());
	if(pushsize > 0) vkCmdPushConstants(cmd, layout, ctx.getImage2DPushStages(), 0, pushsize, &push);
	vkCmdDraw(cmd, 6, 1, 0, 0);
}

void gvkDrawMesh3D(gVKContext& ctx, VkBuffer vertexBuffer, VkDeviceSize vertexOffset,
		VkBuffer indexBuffer, int count,
		VkIndexType indexType, const gVKMeshPush& push,
		VkDescriptorSet diffuseSet, VkDescriptorSet specularSet, VkDescriptorSet normalSet,
		VkDescriptorSet shadowSet,
		VkBuffer instanceBuffer, VkDeviceSize instanceOffset, int instanceCount,
		VkPrimitiveTopology topology, bool depthTest, bool depthTestAlways, bool lines,
		const gVKCullState& culling, bool blending) {
	if(count <= 0 || vertexBuffer == VK_NULL_HANDLE) return;
	if(!gvkEnsureRenderPass(ctx)) return;

	VkCommandBuffer cmd = ctx.getCurrentCommandBuffer();
	if(cmd == VK_NULL_HANDLE) return;
	VkPipeline pipeline = lines ? ctx.getMesh3DLinePipeline() : ctx.getMesh3DPipeline(blending);
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
	// Sets 0..3 in one call: the scene block, then the three material maps. Bound
	// together because they are contiguous and the layout expects all four.
	VkDescriptorSet sceneset = ctx.getCurrentSceneDescriptorSet();
	if(sceneset != VK_NULL_HANDLE && diffuseSet != VK_NULL_HANDLE &&
			specularSet != VK_NULL_HANDLE && normalSet != VK_NULL_HANDLE &&
			shadowSet != VK_NULL_HANDLE) {
		VkDescriptorSet sets[] = {sceneset, diffuseSet, specularSet, normalSet, shadowSet};
		if(ctx.shouldBindDescriptorSets(ctx.getMesh3DPipelineLayout(), sets, 5)) {
			vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, ctx.getMesh3DPipelineLayout(),
					0, 5, sets, 0, nullptr);
		}
	}

	// Straight from the mesh's own device local buffer - no staging, no per-frame
	// copy, which is the whole point of uploading a mesh once. Binding 1 carries the
	// per-instance model matrices and is never absent; see gVKRenderEngine's
	// identity instance buffer.
	VkBuffer buffers[] = {vertexBuffer, instanceBuffer};
	VkDeviceSize offsets[] = {vertexOffset, instanceOffset};
	vkCmdBindVertexBuffers(cmd, 0, instanceBuffer != VK_NULL_HANDLE ? 2 : 1, buffers, offsets);

	const uint32_t pushsize = std::min<uint32_t>(sizeof(push), ctx.getMesh3DPushSize());
	if(pushsize > 0) {
		vkCmdPushConstants(cmd, ctx.getMesh3DPipelineLayout(), ctx.getMesh3DPushStages(), 0, pushsize, &push);
	}

	const uint32_t instances = static_cast<uint32_t>(instanceCount < 1 ? 1 : instanceCount);
	if(indexBuffer != VK_NULL_HANDLE) {
		vkCmdBindIndexBuffer(cmd, indexBuffer, 0, indexType);
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
	if(diffuseSet == VK_NULL_HANDLE) return;

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
	if(ctx.hasShadowDescriptorSetLayout()) {
		if(ctx.shouldBindDescriptorSets(ctx.getShadowPipelineLayout(), &diffuseSet, 1)) {
			vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, ctx.getShadowPipelineLayout(),
					0, 1, &diffuseSet, 0, nullptr);
		}
	}

	VkBuffer buffers[] = {vertexBuffer, instanceBuffer};
	VkDeviceSize offsets[] = {vertexOffset, instanceOffset};
	vkCmdBindVertexBuffers(cmd, 0, instanceBuffer != VK_NULL_HANDLE ? 2 : 1, buffers, offsets);

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
		vkCmdBindIndexBuffer(cmd, indexBuffer, 0, indexType);
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
	vkCmdBindVertexBuffers(cmd, 0, instanceBuffer != VK_NULL_HANDLE ? 2 : 1, buffers, offsets);

	const uint32_t pushsize = std::min<uint32_t>(sizeof(push), ctx.getMesh3DPbrPushSize());
	if(pushsize > 0) {
		vkCmdPushConstants(cmd, ctx.getMesh3DPbrPipelineLayout(), ctx.getMesh3DPbrPushStages(),
				0, pushsize, &push);
	}

	const uint32_t instances = static_cast<uint32_t>(instanceCount < 1 ? 1 : instanceCount);
	if(indexBuffer != VK_NULL_HANDLE) {
		vkCmdBindIndexBuffer(cmd, indexBuffer, 0, indexType);
		vkCmdDrawIndexed(cmd, static_cast<uint32_t>(count), instances, 0, 0, 0);
	} else {
		vkCmdDraw(cmd, static_cast<uint32_t>(count), instances, 0, 0);
	}
}

void gvkDrawTexturedTriangles2D(gVKContext& ctx, VkDescriptorSet textureSet,
		const glm::vec4& tint, const glm::mat4& mvp, const float* xyuv, int vertexCount) {
	if(xyuv == nullptr || vertexCount <= 0 || !gvkEnsureRenderPass(ctx)) return;
	VkCommandBuffer cmd = ctx.getCurrentCommandBuffer();
	if(cmd == VK_NULL_HANDLE || ctx.getImage2DPipeline() == VK_NULL_HANDLE || textureSet == VK_NULL_HANDLE) return;

	const VkDeviceSize vertexbytes = static_cast<VkDeviceSize>(vertexCount) * sizeof(gvkImageVertex);
	VkDeviceSize offset = ctx.pushDynamicVertices(xyuv, vertexbytes);
	if(offset == VK_WHOLE_SIZE) {
		gLogw("gVKDraw") << "Dynamic vertex buffer full; dropping a textured triangle batch.";
		return;
	}

	if(ctx.shouldBindPipeline(ctx.getImage2DPipeline())) {
		vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, ctx.getImage2DPipeline());
	}
	VkBuffer vbuf = ctx.getCurrentDynamicVertexBuffer();
	vkCmdBindVertexBuffers(cmd, 0, 1, &vbuf, &offset);

	VkPipelineLayout layout = ctx.getImage2DPipelineLayout();
	VkDescriptorSet sets[2] = {textureSet, textureSet};
	if(ctx.shouldBindDescriptorSets(layout, sets, 2)) {
		vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 0, 2, sets, 0, nullptr);
	}

	gvkPush push{mvp, tint, 0};
	const uint32_t pushsize = std::min<uint32_t>(sizeof(push), ctx.getImage2DPushSize());
	if(pushsize > 0) vkCmdPushConstants(cmd, layout, ctx.getImage2DPushStages(), 0, pushsize, &push);
	vkCmdDraw(cmd, static_cast<uint32_t>(vertexCount), 1, 0, 0);
}

void gvkDrawSkyboxFace(gVKContext& ctx, VkDescriptorSet faceSet, const float* xyzuv,
		int vertexCount, const glm::mat4& viewProjection, VkCompareOp depthCompare) {
	if(xyzuv == nullptr || vertexCount <= 0 || faceSet == VK_NULL_HANDLE) return;
	if(!gvkEnsureRenderPass(ctx)) return;

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
	vkCmdBindVertexBuffers(cmd, 0, 1, &vbuf, &offset);
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

#endif /* GVK_DESKTOP_GLFW */
