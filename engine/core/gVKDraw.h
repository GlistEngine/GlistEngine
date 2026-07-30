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

// Records a filled coloured triangle list. points holds `count` 2D positions
// (three per triangle); colour components are 0..1; mvp is the full 2D transform.
void gvkDrawColored2D(gVKContext& ctx, const glm::vec2* points, int count,
		const glm::vec4& color, const glm::mat4& mvp);

// Records a textured unit quad (two triangles) sampled through the given combined
// image sampler descriptor set. tint components are 0..1.
void gvkDrawTextured2D(gVKContext& ctx, VkDescriptorSet textureSet,
		const glm::vec4& tint, const glm::mat4& mvp);

#endif /* GVK_DESKTOP_GLFW */

#endif /* CORE_GVKDRAW_H */
