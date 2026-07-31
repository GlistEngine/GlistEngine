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

#ifdef GVK_DESKTOP_GLFW

#include <glm/glm.hpp>

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
void gvkDrawColored2D(gVKContext& ctx, const glm::vec2* points, int count,
		const glm::vec4& color, const glm::mat4& mvp, int mode = GVK_DRAW2D_TRIANGLES);

// Records a textured unit quad (two triangles) sampled through the given combined
// image sampler descriptor set. tint components are 0..1. uvOffset / uvScale place
// the sampled region inside the texture: the default covers all of it, and a
// sub-part draw narrows it to the requested rectangle. maskSet is the descriptor
// set of an alpha mask whose inverted red channel scales the alpha, or
// VK_NULL_HANDLE to draw unmasked.
void gvkDrawTextured2D(gVKContext& ctx, VkDescriptorSet textureSet, VkDescriptorSet maskSet,
		const glm::vec4& tint, const glm::mat4& mvp,
		const glm::vec2& uvOffset = glm::vec2(0.0f), const glm::vec2& uvScale = glm::vec2(1.0f));

#endif /* GVK_DESKTOP_GLFW */

#endif /* CORE_GVKDRAW_H */
