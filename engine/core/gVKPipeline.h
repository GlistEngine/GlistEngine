/*
 * gVKPipeline.h
 *
 * Builds the graphics pipelines the Vulkan 2D draw path uses: one for coloured
 * geometry (triangle, rectangle) and one for textured quads (gImage). Also owns
 * the descriptor-set layouts and the descriptor pool textures draw from.
 *
 * Nothing about those layouts is written out here: the vertex attributes, the
 * push constant range and the descriptor bindings are all reflected out of the
 * compiled SPIR-V by gVKReflect, so the shader sources under
 * graphics/shaders/vk are the only place the interface is declared. Editing a
 * shader is enough - the pipeline follows.
 *
 * The entry points gvkCreateGraphicsPipelines / gvkReloadGraphicsPipelines /
 * gvkDestroyGraphicsPipelines are declared in gVKContext.h (so the context can
 * befriend them); this header adds only what needs no access to the context.
 */

#pragma once

#ifndef CORE_GVKPIPELINE_H
#define CORE_GVKPIPELINE_H

#include "gVKContext.h"

#ifdef GVK_VULKAN

// Newest modification time across the shader sources the 2D pipelines are built
// from, or 0 when runtime shader compilation is unavailable. Comparing successive
// readings is how the renderer notices an edit worth reloading for.
long long gvkShaderSourcesTimestamp();
void gvkDestroyPipelineCache(gVKContext& ctx);

#endif /* GVK_VULKAN */

#endif /* CORE_GVKPIPELINE_H */
