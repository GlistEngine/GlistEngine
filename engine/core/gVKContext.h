/*
 * gVKContext.h
 *
 * Shared state of the Vulkan render backend. Every gVK* module reads from and
 * writes to this single structure, so no module has to include another module's
 * header. The comments mark which module owns which block.
 */

#pragma once

#ifndef CORE_GVKCONTEXT_H
#define CORE_GVKCONTEXT_H

// TARGET_OS_IPHONE only exists once TargetConditionals.h has been included, so it
// has to be pulled in before the guard below is evaluated.
#if defined(__APPLE__)
#include <TargetConditionals.h>
#endif

// Vulkan is an optional dependency of this engine: engine/CMakeLists.txt defines
// GLIST_HAS_VULKAN only when the development files are actually present, which is
// not the case on every machine or CI runner. Every gVK* module guards its body
// with GVK_DESKTOP_GLFW so those translation units stay empty when Vulkan is not
// available, and the engine still builds.
#if defined(GLIST_HAS_VULKAN) && !defined(ANDROID) && !defined(EMSCRIPTEN) && !(defined(TARGET_OS_IPHONE) && TARGET_OS_IPHONE)
#define GVK_DESKTOP_GLFW 1
#endif

#ifdef GVK_DESKTOP_GLFW

#include <vulkan/vulkan.h>
#include <vector>

struct GLFWwindow;

// How many frames the CPU may prepare while the GPU is still busy with earlier ones.
inline constexpr int GVK_MAX_FRAMES_IN_FLIGHT = 2;

// Every Vulkan handle of the backend lives here. All members start out empty,
// which is what makes the "destroy only if non null" teardown correct even when
// initialisation fails half way through.
struct gVKContext {
	/* ---------------- created by the init phase, do not modify ---------------- */
	VkInstance instance = VK_NULL_HANDLE;
	VkDebugUtilsMessengerEXT debugmessenger = VK_NULL_HANDLE;
	VkSurfaceKHR surface = VK_NULL_HANDLE;
	VkPhysicalDevice physicaldevice = VK_NULL_HANDLE;
	VkDevice device = VK_NULL_HANDLE;
	VkQueue graphicsqueue = VK_NULL_HANDLE;
	VkQueue presentqueue = VK_NULL_HANDLE;
	uint32_t graphicsfamily = 0;
	uint32_t presentfamily = 0;

	// The window the surface was created from. The frame loop needs it to notice
	// resizes and to rebuild the swapchain.
	GLFWwindow* window = nullptr;

	/* ---------------- gVKSwapchain ---------------- */
	VkSwapchainKHR swapchain = VK_NULL_HANDLE;
	std::vector<VkImage> swapchainimages;
	std::vector<VkImageView> swapchainimageviews;
	VkFormat swapchainformat = VK_FORMAT_UNDEFINED;
	VkExtent2D swapchainextent = {0, 0};

	/* ---------------- gVKRenderTarget ---------------- */
	VkRenderPass renderpass = VK_NULL_HANDLE;
	// One framebuffer per swapchain image view.
	std::vector<VkFramebuffer> framebuffers;

	/* ---------------- gVKCommands ---------------- */
	VkCommandPool commandpool = VK_NULL_HANDLE;
	// GVK_MAX_FRAMES_IN_FLIGHT entries, indexed by currentframe.
	std::vector<VkCommandBuffer> commandbuffers;

	/* ---------------- gVKSync ---------------- */
	// GVK_MAX_FRAMES_IN_FLIGHT entries, indexed by currentframe.
	std::vector<VkSemaphore> imageavailablesemaphores;
	std::vector<VkFence> inflightfences;
	// One per swapchain image, indexed by currentimageindex.
	std::vector<VkSemaphore> renderfinishedsemaphores;

	/* ---------------- gVKFrame ---------------- */
	uint32_t currentframe = 0;
	uint32_t currentimageindex = 0;
	bool frameactive = false;
	// Cornflower blue. gVKRenderEngine::clearColor writes into this value.
	VkClearValue clearvalue = {{{0.39f, 0.58f, 0.93f, 1.0f}}};
};

#endif /* GVK_DESKTOP_GLFW */

#endif /* CORE_GVKCONTEXT_H */
