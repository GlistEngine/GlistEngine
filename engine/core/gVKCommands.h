/*
 * gVKCommands.h
 *
 * Command pool and command buffers of the Vulkan backend. Vulkan never executes a
 * command directly: commands are recorded into a command buffer and submitted to a
 * queue as a batch.
 * Created by: Efe Arda Palali.
 */

#pragma once

#ifndef CORE_GVKCOMMANDS_H
#define CORE_GVKCOMMANDS_H

#include "gVKContext.h"

#ifdef GVK_VULKAN

/*
 * Creates the command pool on the graphics queue family and allocates
 * GVK_MAX_FRAMES_IN_FLIGHT primary command buffers from it. The pool is created
 * with the reset flag because every frame re-records its buffer.
 */
bool gvkCreateCommandResources(gVKContext& ctx);

/*
 * Destroys the pool, which frees the command buffers with it. Safe to call twice.
 */
void gvkDestroyCommandResources(gVKContext& ctx);

#endif /* GVK_VULKAN */

#endif /* CORE_GVKCOMMANDS_H */
