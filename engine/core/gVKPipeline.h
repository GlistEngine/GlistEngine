/*
 * gVKPipeline.h
 *
 * Builds the graphics pipelines the Vulkan 2D draw path uses: one for coloured
 * geometry (triangle, rectangle) and one for textured quads (gImage). Also owns
 * the image descriptor-set layout and the descriptor pool textures draw from.
 *
 * The entry points gvkCreateGraphicsPipelines / gvkDestroyGraphicsPipelines are
 * declared in gVKContext.h (so the context can befriend them); this header just
 * documents the module and pulls in the context definition.
 */

#pragma once

#ifndef CORE_GVKPIPELINE_H
#define CORE_GVKPIPELINE_H

#include "gVKContext.h"

#endif /* CORE_GVKPIPELINE_H */
