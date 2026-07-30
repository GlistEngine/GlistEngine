/*
 * gVKDraw.cpp
 */

#include "gVKDraw.h"

#ifdef GVK_DESKTOP_GLFW

#include "gVKBuffer.h"
#include "gUtils.h"

// 1 MB of vertices per frame in flight. A 2D frame records only a handful of
// triangles and quads, so this never fills in practice; overflow is dropped with
// a warning rather than growing the buffer mid-frame.
static constexpr VkDeviceSize GVK_DYNAMIC_VERTEX_CAPACITY = 1u << 20;

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
	gLogi("gVKDraw") << "Dynamic vertex buffers ready.";
	return true;
}

void gvkDestroyDrawResources(gVKContext& ctx) {
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

// mvp + colour, matching the push_constant block in color2d.vert / image2d.*.
namespace {
struct gvkPush {
	glm::mat4 mvp;
	glm::vec4 color;
};
struct gvkImageVertex {
	glm::vec2 pos;
	glm::vec2 uv;
};
}

void gvkDrawColored2D(gVKContext& ctx, const glm::vec2* points, int count,
		const glm::vec4& color, const glm::mat4& mvp) {
	if(count <= 0 || points == nullptr) return;
	if(!gvkEnsureRenderPass(ctx)) return;
	VkCommandBuffer cmd = ctx.getCurrentCommandBuffer();
	if(cmd == VK_NULL_HANDLE || ctx.getColor2DPipeline() == VK_NULL_HANDLE) return;

	VkDeviceSize offset = ctx.pushDynamicVertices(points, sizeof(glm::vec2) * count);
	if(offset == VK_WHOLE_SIZE) {
		gLogw("gVKDraw") << "Dynamic vertex buffer full; dropping a coloured draw.";
		return;
	}

	vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, ctx.getColor2DPipeline());
	VkBuffer vbuf = ctx.getCurrentDynamicVertexBuffer();
	vkCmdBindVertexBuffers(cmd, 0, 1, &vbuf, &offset);

	gvkPush push{mvp, color};
	vkCmdPushConstants(cmd, ctx.getColor2DPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(push), &push);
	vkCmdDraw(cmd, static_cast<uint32_t>(count), 1, 0, 0);
}

void gvkDrawTextured2D(gVKContext& ctx, VkDescriptorSet textureSet,
		const glm::vec4& tint, const glm::mat4& mvp) {
	if(!gvkEnsureRenderPass(ctx)) return;
	VkCommandBuffer cmd = ctx.getCurrentCommandBuffer();
	if(cmd == VK_NULL_HANDLE || ctx.getImage2DPipeline() == VK_NULL_HANDLE || textureSet == VK_NULL_HANDLE) return;

	// Unit quad in [0,1]; the mvp (projection2d * image model matrix) scales and
	// places it, exactly like the OpenGL image quad.
	const gvkImageVertex quad[6] = {
		{{0.0f, 0.0f}, {0.0f, 0.0f}}, {{1.0f, 0.0f}, {1.0f, 0.0f}}, {{1.0f, 1.0f}, {1.0f, 1.0f}},
		{{0.0f, 0.0f}, {0.0f, 0.0f}}, {{1.0f, 1.0f}, {1.0f, 1.0f}}, {{0.0f, 1.0f}, {0.0f, 1.0f}},
	};
	VkDeviceSize offset = ctx.pushDynamicVertices(quad, sizeof(quad));
	if(offset == VK_WHOLE_SIZE) {
		gLogw("gVKDraw") << "Dynamic vertex buffer full; dropping a textured draw.";
		return;
	}

	vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, ctx.getImage2DPipeline());
	VkBuffer vbuf = ctx.getCurrentDynamicVertexBuffer();
	vkCmdBindVertexBuffers(cmd, 0, 1, &vbuf, &offset);
	VkPipelineLayout layout = ctx.getImage2DPipelineLayout();
	vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 0, 1, &textureSet, 0, nullptr);

	gvkPush push{mvp, tint};
	vkCmdPushConstants(cmd, layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(push), &push);
	vkCmdDraw(cmd, 6, 1, 0, 0);
}

#endif /* GVK_DESKTOP_GLFW */
