/*
 * gVKDraw.cpp
 */

#include "gVKDraw.h"

#ifdef GVK_DESKTOP_GLFW

#include "gVKBuffer.h"
#include "gUtils.h"
#include <algorithm>
#include <vector>

// 8 MB primary ring per frame in flight. Draws that exceed the remaining ring
// space receive a frame-owned overflow buffer instead of being dropped.
static constexpr VkDeviceSize GVK_DYNAMIC_VERTEX_CAPACITY = 8u << 20;

bool gvkCreateDrawResources(gVKContext& ctx) {
	if(ctx.device == VK_NULL_HANDLE) return false;

	const int frames = GVK_MAX_FRAMES_IN_FLIGHT;
	ctx.dynvertexcapacity = GVK_DYNAMIC_VERTEX_CAPACITY;
	ctx.dynvertexbuffers.assign(frames, VK_NULL_HANDLE);
	ctx.dynvertexmemories.assign(frames, VK_NULL_HANDLE);
	ctx.dynvertexmapped.assign(frames, nullptr);
	ctx.dynvertexoffsets.assign(frames, 0);
	ctx.dynvertexoverflows.resize(frames);

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
	for(auto& frameoverflows : ctx.dynvertexoverflows) {
		for(auto& overflow : frameoverflows) {
			if(overflow.mapped != nullptr) vkUnmapMemory(ctx.device, overflow.memory);
			if(overflow.buffer != VK_NULL_HANDLE) vkDestroyBuffer(ctx.device, overflow.buffer, nullptr);
			if(overflow.memory != VK_NULL_HANDLE) vkFreeMemory(ctx.device, overflow.memory, nullptr);
		}
	}
	ctx.dynvertexoverflows.clear();
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

bool gvkUploadDynamicVertices(gVKContext& ctx, const void* data, VkDeviceSize size,
		VkBuffer& outBuffer, VkDeviceSize& outOffset) {
	if(data == nullptr || size == 0 || ctx.dynvertexmapped.empty()) return false;
	outOffset = (ctx.dynvertexoffsets[ctx.currentframe] + 15) & ~static_cast<VkDeviceSize>(15);
	if(outOffset + size <= ctx.dynvertexcapacity) {
		std::memcpy(static_cast<char*>(ctx.dynvertexmapped[ctx.currentframe]) + outOffset, data, size);
		ctx.dynvertexoffsets[ctx.currentframe] = outOffset + size;
		outBuffer = ctx.dynvertexbuffers[ctx.currentframe];
		return true;
	}

	gVKContext::DynamicVertexOverflow overflow;
	if(!gvkCreateBuffer(ctx, size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			overflow.buffer, overflow.memory)) return false;
	if(vkMapMemory(ctx.device, overflow.memory, 0, size, 0, &overflow.mapped) != VK_SUCCESS) {
		vkDestroyBuffer(ctx.device, overflow.buffer, nullptr);
		vkFreeMemory(ctx.device, overflow.memory, nullptr);
		return false;
	}
	std::memcpy(overflow.mapped, data, size);
	outBuffer = overflow.buffer;
	outOffset = 0;
	ctx.dynvertexoverflows[ctx.currentframe].push_back(overflow);
	return true;
}

// mvp + colour, matching the push_constant block in color2d.vert / image2d.*. The
// image shaders carry one field more; the colour ones push only the first 80 bytes,
// because the size actually pushed comes from reflecting each shader.
struct gvkPush {
	glm::mat4 mvp;
	glm::vec4 color;
	int masking = 0;
};
struct gvkPush3D {
	glm::mat4 mvp;
	glm::vec4 ambientProduct;
	glm::vec4 diffuseProduct;
	glm::vec4 lightDirectionTextured;
	glm::vec4 cameraPositionPbr;
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
	if(!gvkEnsureRendering(ctx)) return;
	VkCommandBuffer cmd = ctx.getCurrentCommandBuffer();
	if(cmd == VK_NULL_HANDLE) return;
	VkPipeline pipeline = lines ? ctx.getColor2DLinePipeline() : ctx.getColor2DPipeline();
	if(pipeline == VK_NULL_HANDLE) return;

	VkBuffer vbuf = VK_NULL_HANDLE;
	VkDeviceSize offset = 0;
	if(!gvkUploadDynamicVertices(ctx, vertexdata, sizeof(glm::vec2) * vertexcount, vbuf, offset)) return;

	vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
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

void gvkDrawMesh3D(gVKContext& ctx, const gRenderer::MeshVertex3D* vertices, int count,
		VkDescriptorSet textureSet, const glm::vec4& ambientProduct,
		const glm::vec4& diffuseProduct, const glm::vec3& lightDirection, bool textured,
		bool pbr, const glm::vec3& cameraPosition, const glm::mat4& mvp) {
	if(count <= 0 || vertices == nullptr) return;
	if(!gvkEnsureRendering(ctx)) return;
	VkCommandBuffer cmd = ctx.getCurrentCommandBuffer();
	if(cmd == VK_NULL_HANDLE || ctx.getColor3DPipeline() == VK_NULL_HANDLE) return;

	VkBuffer vbuf = VK_NULL_HANDLE;
	VkDeviceSize offset = 0;
	if(!gvkUploadDynamicVertices(ctx, vertices, sizeof(gRenderer::MeshVertex3D) * count, vbuf, offset)) return;
	vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, ctx.getColor3DPipeline());
	vkCmdBindVertexBuffers(cmd, 0, 1, &vbuf, &offset);
	if(textureSet != VK_NULL_HANDLE) {
		vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, ctx.getColor3DPipelineLayout(),
				0, 1, &textureSet, 0, nullptr);
	}
	gvkPush3D push{mvp, ambientProduct, diffuseProduct,
			glm::vec4(lightDirection, textured ? 1.0f : 0.0f),
			glm::vec4(cameraPosition, pbr ? 1.0f : 0.0f)};
	const uint32_t pushsize = std::min<uint32_t>(sizeof(push), ctx.getColor3DPushSize());
	if(pushsize > 0) vkCmdPushConstants(cmd, ctx.getColor3DPipelineLayout(),
			ctx.getColor3DPushStages(), 0, pushsize, &push);
	vkCmdDraw(cmd, static_cast<uint32_t>(count), 1, 0, 0);
}

void gvkDrawTextured2D(gVKContext& ctx, VkDescriptorSet textureSet, VkDescriptorSet maskSet,
		const glm::vec4& tint, const glm::mat4& mvp,
		const glm::vec2& uvOffset, const glm::vec2& uvScale) {
	if(!gvkEnsureRendering(ctx)) return;
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
	VkBuffer vbuf = VK_NULL_HANDLE;
	VkDeviceSize offset = 0;
	if(!gvkUploadDynamicVertices(ctx, quad, sizeof(quad), vbuf, offset)) return;

	vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, ctx.getImage2DPipeline());
	vkCmdBindVertexBuffers(cmd, 0, 1, &vbuf, &offset);
	VkPipelineLayout layout = ctx.getImage2DPipelineLayout();
	// The fragment shader names the mask sampler whether or not it is read, so set 1
	// always needs a valid set bound. Unmasked draws bind the image's own set there
	// and turn the branch off through the push constant.
	const bool masking = maskSet != VK_NULL_HANDLE;
	VkDescriptorSet sets[2] = {textureSet, masking ? maskSet : textureSet};
	vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 0, 2, sets, 0, nullptr);

	gvkPush push{mvp, tint, masking ? 1 : 0};
	const uint32_t pushsize = std::min<uint32_t>(sizeof(push), ctx.getImage2DPushSize());
	if(pushsize > 0) vkCmdPushConstants(cmd, layout, ctx.getImage2DPushStages(), 0, pushsize, &push);
	vkCmdDraw(cmd, 6, 1, 0, 0);
}

#endif /* GVK_DESKTOP_GLFW */
