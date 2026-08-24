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

#include "gVKReflect.h"
#include <vector>

// Newest modification time across the shader sources the 2D pipelines are built
// from, or 0 when runtime shader compilation is unavailable. Comparing successive
// readings is how the renderer notices an edit worth reloading for.
long long gvkShaderSourcesTimestamp();
void gvkDestroyPipelineCache(gVKContext& ctx);

/*
 * A pipeline built from a shader the application supplied rather than one of the
 * engine's own. Everything in it is reflected out of the given SPIR-V, exactly as
 * the built-in pipelines are, so a gShader loaded at run time gets the same
 * treatment as mesh3d: the shader declares its interface and the layout follows.
 *
 * The vertex input is the one exception. It is taken from the engine's own mesh
 * layout rather than from reflection, because the buffer the draw path binds is
 * a gVertex array whatever the shader happens to declare. Vulkan allows a
 * pipeline to describe attributes the shader never reads, so a shader consuming
 * only position and texcoords still works against the full layout - what it must
 * not do is declare an input the layout has no attribute for, which
 * gvkBuildUserPipeline rejects.
 */
struct gVKUserPipeline {
	gVKPipelineVariants variants[GVK_PIPELINE_SAMPLE_VARIANTS];
	VkPipelineLayout layout = VK_NULL_HANDLE;
	std::vector<VkDescriptorSetLayout> setlayouts;
	uint32_t pushsize = 0;
	VkShaderStageFlags pushstages = 0;
	// What the shader declared, kept so values can be addressed by name.
	gVKReflectedLayout reflected;
	// True when the pipeline describes the engine's gVertex layout, so it can be
	// drawn with mesh buffers. False means the layout came from reflection and the
	// shader is fed by the fullscreen quad path instead.
	bool usesmeshlayout = false;
	// Bytes per vertex of whichever layout was chosen.
	uint32_t vertexstride = 0;
};

// Builds one. Returns false with everything released on any failure, so a caller
// can report the shader as unloaded and carry on.
bool gvkBuildUserPipeline(gVKContext& ctx, const std::vector<uint32_t>& vertSpirv,
		const std::vector<uint32_t>& fragSpirv, const char* debugName, gVKUserPipeline& out);

// Releases one. Safe on an all-null instance, and leaves it that way.
void gvkDestroyUserPipeline(gVKContext& ctx, gVKUserPipeline& pipeline);

#endif /* GVK_VULKAN */

#endif /* CORE_GVKPIPELINE_H */
