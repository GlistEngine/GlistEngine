/*
 * gVKRenderTarget.h
 *
 * Attachment formats and depth images used by Vulkan Dynamic Rendering.
 * Created by: Anil Emre Akkoc.
 */

#pragma once

#ifndef CORE_GVKRENDERTARGET_H
#define CORE_GVKRENDERTARGET_H

#include "gVKContext.h"

#ifdef GVK_DESKTOP_GLFW

/*
 * Selects formats consumed by VkPipelineRenderingCreateInfo.
 */
bool gvkSelectRenderingFormats(gVKContext& ctx);
void gvkResetRenderingFormats(gVKContext& ctx);

/*
 * Creates one depth target per swapchain image. No VkRenderPass or VkFramebuffer
 * objects are needed; Dynamic Rendering consumes the image views directly.
 */
bool gvkCreateDepthTargets(gVKContext& ctx);
void gvkDestroyDepthTargets(gVKContext& ctx);

#endif /* GVK_DESKTOP_GLFW */

#endif /* CORE_GVKRENDERTARGET_H */
