/*
 * gVKDraw.h
 *
 * The Vulkan 2D draw path: a per-frame host-visible vertex ring plus the record
 * helpers that the engine's 2D functions dispatch to under Vulkan. Every helper
 * opens the render pass lazily (gvkEnsureRenderPass), so the clear colour set by
 * the canvas earlier in the same draw() is what covers the screen.
 *
 * gvkCreateDrawResources / gvkDestroyDrawResources are declared in gVKContext.h
 * (befriended there); the record helpers below need glm, so they live here.
 */

#pragma once

#ifndef CORE_GVKDRAW_H
#define CORE_GVKDRAW_H

#include "gVKContext.h"
// For gVKMeshPush, the 3D path's push constant block.
#include "gVKUniform.h"

#ifdef GVK_VULKAN

#include <glm/glm.hpp>

// The renderer's culling state, handed to a 3D draw the same way the depth state is.
// Already in Vulkan terms: gVKRenderEngine translates the engine's GL-flavoured
// culling API, so this header needs no GL constants.
//
// Inside the guard, like everything else here: a build without the Vulkan backend
// never sees a Vulkan header, so a type named outside it does not exist at all.
//
// The front face is deliberately the opposite of the OpenGL one. The frame loop
// draws through a negative-height viewport, which mirrors the y axis and reverses
// the winding a triangle appears to have on screen, so geometry OpenGL calls
// counter-clockwise reaches this rasteriser as clockwise.
struct gVKCullState {
	VkCullModeFlags mode = VK_CULL_MODE_NONE;
	VkFrontFace frontface = VK_FRONT_FACE_CLOCKWISE;
};

// How the points handed to gvkDrawColored2D are connected. These mirror the mesh
// draw modes the 2D primitives use (gMesh::DRAWMODE_*), without this header having
// to know the GL constants behind them. The fan / strip / loop forms are expanded
// on the CPU, because the two colour pipelines are built as a triangle list and a
// line list and Vulkan has no line loop topology at all.
enum gvkDraw2DMode {
	GVK_DRAW2D_TRIANGLES,
	GVK_DRAW2D_TRIANGLESTRIP,
	GVK_DRAW2D_TRIANGLEFAN,
	GVK_DRAW2D_LINES,
	GVK_DRAW2D_LINESTRIP,
	GVK_DRAW2D_LINELOOP
};

// Records coloured 2D geometry. Colour components are 0..1; mvp is the full 2D
// transform. points holds `count` positions connected as `mode` describes.
// additive selects the pipeline that adds to the target instead of compositing
// over it, for gRenderer::BLENDMODE_ADDITIVE. Blend factors are baked into a
// Vulkan pipeline, so this is a choice of pipeline rather than a state to set.
void gvkDrawColored2D(gVKContext& ctx, const glm::vec2* points, int count,
		const glm::vec4& color, const glm::mat4& mvp, int mode = GVK_DRAW2D_TRIANGLES,
		bool additive = false);

// Records a textured unit quad (two triangles) sampled through the given combined
// image sampler descriptor set. tint components are 0..1. uvOffset / uvScale place
// the sampled region inside the texture: the default covers all of it, and a
// sub-part draw narrows it to the requested rectangle. maskSet is the descriptor
// set of an alpha mask whose inverted red channel scales the alpha, or
// VK_NULL_HANDLE to draw unmasked.
void gvkDrawTextured2D(gVKContext& ctx, VkDescriptorSet textureSet, VkDescriptorSet maskSet,
		const glm::vec4& tint, const glm::mat4& mvp,
		const glm::vec2& uvOffset = glm::vec2(0.0f), const glm::vec2& uvScale = glm::vec2(1.0f),
		bool additive = false);

// Records an expanded textured triangle list. xyuv has four floats per vertex:
// position then texture coordinate.
void gvkDrawTexturedTriangles2D(gVKContext& ctx, VkDescriptorSet textureSet,
		const glm::vec4& tint, const glm::mat4& mvp, const float* xyuv, int vertexCount,
		bool additive = false);

// The 2D calls above do not record a draw of their own. Each appends its vertices
// to an open batch, and the batch becomes a single vkCmdDraw here - a frame of a
// couple of hundred sprites and glyphs collapses into a handful of draws whenever
// consecutive ones share a texture and a blend mode.
//
// Order is preserved by ending the batch rather than reordering around it, so
// anything that records into the same pass and must appear after the 2D issued
// before it has to call this first. The 3D paths in this file already do; a new
// one that records directly into the command buffer has to as well, and so does
// any code that ends a render pass while the frame is still going.
void gvkFlush2DBatch(gVKContext& ctx);

// Abandons the open batch without recording it, for the frame boundary: the range
// it names is about to be rewound underneath it.
void gvkReset2DBatch();

// vertexOffset is where this mesh's vertices start inside the bound buffer. Zero
// for a mesh uploaded once; a mesh whose vertices the CPU rewrites is given a
// slice of the frame's arena instead, so that each of its draws reads the data
// it was recorded with rather than whatever the last upload of the frame left.
// Records one 3D mesh out of buffers that already live on the device, unlike the
// 2D calls above, which copy their geometry into a per-frame ring first. A mesh
// uploads once through gVKMeshBuffer and is then drawn straight from there.
//
// indexBuffer may be VK_NULL_HANDLE, in which case count is a vertex count and the
// draw is non-indexed; otherwise count is an index count and indexType says how
// wide those indices are - the caller passes it because gIndex is 16 bit on Android
// and 32 bit elsewhere, and this file has no reason to depend on that header.
//
// push carries the model matrix and the material; the camera and the lights come
// from the frame's scene descriptor set, which this binds.
//
// depthTest / depthTestAlways mirror the renderer's current depth state and are set
// on the command buffer rather than baked into the pipeline, because the engine lets
// an app toggle depth testing between draws. lines selects the wireframe pipeline.
// materialSet holds the diffuse, specular and normal maps as bindings 0, 1 and 2 of
// set 1, and shadowSet is set 2. Neither may be VK_NULL_HANDLE: the shader declares
// every sampler, so each binding needs a valid descriptor even for a mesh that uses
// no maps - the caller fills the unused ones with a 1x1 white texture.
// topology has to belong to the class the chosen pipeline was built for: a triangle
// one when lines is false, a line one when it is true.
// instanceBuffer holds one model matrix per instance and is bound as binding 1. It
// is required even for a single draw, because the shader always reads from it - the
// caller passes a one-element identity buffer when the draw is not instanced.
// cutout says whether this draw's diffuse map can produce a texel the shader's
// alpha test would discard. False routes it through the pipeline with the discard
// compiled out, which is what lets a tile based GPU reject its fragments on depth
// before shading them; it must only be false where the caller knows that for
// certain, because a cutout material drawn through it would render its holes
// opaque. See GVK_CUTOUT in mesh3d.frag.
void gvkDrawMesh3D(gVKContext& ctx, VkBuffer vertexBuffer, VkDeviceSize vertexOffset,
		VkBuffer indexBuffer, int count,
		VkIndexType indexType, const gVKMeshPush& push,
		VkDescriptorSet materialSet, VkDescriptorSet shadowSet,
		VkBuffer instanceBuffer, VkDeviceSize instanceOffset, int instanceCount,
		VkPrimitiveTopology topology, bool depthTest, bool depthTestAlways, bool lines,
		const gVKCullState& culling, bool blending, bool cutout);

// Records one mesh into the shadow map through the depth-only pipeline. diffuseSet
// is the caster's diffuse map, bound as set 0 so a cutout material can discard the
// holes it punches; it must be a real descriptor even for an opaque mesh, where the
// 1x1 white texture stands in and push.misc.x says not to sample it. Only valid
// while the shadow render pass is open.
void gvkDrawShadowCaster(gVKContext& ctx, VkBuffer vertexBuffer, VkDeviceSize vertexOffset,
		VkBuffer indexBuffer,
		int count, VkIndexType indexType, const gVKShadowPush& push,
		VkDescriptorSet diffuseSet,
		VkBuffer instanceBuffer, VkDeviceSize instanceOffset, int instanceCount, VkPrimitiveTopology topology);

// Records one 3D mesh through the PBR pipeline. materialSet holds all five maps in
// a single descriptor set (bindings 0..4), unlike the non-PBR path where each map
// is its own set: Vulkan only guarantees four bound sets, and scene plus five
// textures would need six. shadowSet is the depth map from the light's point of
// view and goes in as set 2, per frame rather than per material. There is no line
// variant - a wireframe PBR draw has no meaning that the OpenGL path offers either.
void gvkDrawMesh3DPbr(gVKContext& ctx, VkBuffer vertexBuffer, VkDeviceSize vertexOffset,
		VkBuffer indexBuffer, int count,
		VkIndexType indexType, const gVKPbrPush& push, VkDescriptorSet materialSet,
		VkDescriptorSet shadowSet,
		VkBuffer instanceBuffer, VkDeviceSize instanceOffset, int instanceCount,
		VkPrimitiveTopology topology, bool depthTest, bool depthTestAlways,
		const gVKCullState& culling, bool blending);

// Records one face of the skybox: a quad list in world space, sampled from a plain
// 2D texture rather than a cube map. xyzuv holds five floats per vertex, three of
// position and two of texture coordinate, and goes through the per-frame dynamic
// ring because the quads are rebuilt around the camera every frame.
//
// depthCompare is the renderer's current depth function, so gSkybox can ask for the
// same EQUAL test the OpenGL path uses to keep the sky behind everything else.
void gvkDrawSkyboxFace(gVKContext& ctx, VkDescriptorSet faceSet, const float* xyzuv,
		int vertexCount, const glm::mat4& viewProjection, VkCompareOp depthCompare);

#endif /* GVK_VULKAN */

#endif /* CORE_GVKDRAW_H */
