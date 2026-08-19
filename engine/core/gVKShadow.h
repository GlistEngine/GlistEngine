/*
 * gVKShadow.h
 *
 * The shadow map of the Vulkan 3D path: a depth-only render target that the scene
 * is drawn into from the light's point of view, then sampled while shading.
 *
 * This is a second render pass with its own image, its own framebuffer and its own
 * pipeline, and it runs before the one that draws to the screen. Which of the two
 * a frame is currently recording is decided by gObject::renderpassno, the counter
 * gAppManager already increments when gShadowMap asks for two passes - so the scene
 * is drawn twice by the engine and this file only has to know which pass it is in.
 *
 * The depth image is both a depth attachment and a sampled texture, which is what
 * lets the second pass read what the first one wrote. It carries its own descriptor
 * set, shaped like a texture's, so the mesh shader binds it the same way it binds a
 * material map.
 *
 * Created by: Veysel Burak Eroglu.
 */

#pragma once

#ifndef CORE_GVKSHADOW_H
#define CORE_GVKSHADOW_H

#include "gVKContext.h"

#ifdef GVK_VULKAN

/*
 * Creates the depth image, its view and sampler, the render pass and framebuffer
 * that write into it, and the descriptor set that reads it back. Sized in texels;
 * gShadowMap decides the size, defaulting to what the OpenGL path uses.
 *
 * Safe to call again with a different size: the previous resources are released
 * first. Returns false and leaves nothing allocated on failure, which is what makes
 * a scene fall back to being drawn unshadowed rather than not drawn at all.
 */
bool gvkCreateShadowResources(gVKContext& ctx, uint32_t width, uint32_t height);
void gvkDestroyShadowResources(gVKContext& ctx);

/*
 * Opens the shadow render pass on the frame's command buffer and sets a viewport
 * covering the whole shadow map. Unlike the main pass this one is begun eagerly:
 * there is no clear colour to wait for, only a depth clear.
 *
 * Returns false when there is no shadow map, no frame in flight, or a render pass
 * is already open - in each case the caller simply draws nothing.
 */
bool gvkBeginShadowPass(gVKContext& ctx);
void gvkEndShadowPass(gVKContext& ctx);

#endif /* GVK_VULKAN */

#endif /* CORE_GVKSHADOW_H */
