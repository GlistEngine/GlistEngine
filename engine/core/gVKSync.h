/*
 * gVKSync.h
 *
 * Synchronisation primitives of the Vulkan frame loop. Semaphores order work
 * between GPU operations, fences let the CPU wait for the GPU.
 * Created by: Ozlem Tutuneken.
 */

#pragma once

#ifndef CORE_GVKSYNC_H
#define CORE_GVKSYNC_H

#include "gVKContext.h"

#ifdef GVK_VULKAN

/*
 * Creates the per frame primitives: GVK_MAX_FRAMES_IN_FLIGHT image available
 * semaphores and the same number of fences. The fences must be created already
 * signalled, otherwise the very first vkWaitForFences never returns and the
 * application freezes on its first frame.
 */
bool gvkCreateFrameSyncObjects(gVKContext& ctx);
void gvkDestroyFrameSyncObjects(gVKContext& ctx);

/*
 * Creates one render finished semaphore per swapchain image. These are per image
 * rather than per frame on purpose: presentation waits on the semaphore selected
 * by the acquired image index, and binding them to the frame index instead makes
 * current validation layers report semaphore reuse. Recreated with the swapchain.
 */
bool gvkCreatePresentSemaphores(gVKContext& ctx, uint32_t imagecount);
void gvkDestroyPresentSemaphores(gVKContext& ctx);

#endif /* GVK_VULKAN */

#endif /* CORE_GVKSYNC_H */
