/*
 * gVKContext.h
 *
 * Shared state of the Vulkan render backend. gVKRenderEngine.h only forward
 * declares gVKContext, so including that header still never pulls in
 * <vulkan/vulkan.h>; the layout lives here instead of inside a single .cpp so
 * every gVK* module of the backend can work against the same contract.
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
#include <string>
#include <vector>

struct GLFWwindow;
struct gVKContext;

// How many frames the CPU may prepare while the GPU is still busy with earlier ones.
inline constexpr int GVK_MAX_FRAMES_IN_FLIGHT = 2;

// Declared here as well so the struct can befriend them.
bool gvkCreateSwapchain(gVKContext& ctx, GLFWwindow* window);
void gvkDestroySwapchain(gVKContext& ctx);
bool gvkRecreateSwapchain(gVKContext& ctx, GLFWwindow* window);
bool gvkCreateRenderPass(gVKContext& ctx);
void gvkDestroyRenderPass(gVKContext& ctx);
bool gvkCreateFramebuffers(gVKContext& ctx);
void gvkDestroyFramebuffers(gVKContext& ctx);
bool gvkCreateCommandResources(gVKContext& ctx);
void gvkDestroyCommandResources(gVKContext& ctx);
bool gvkCreateFrameSyncObjects(gVKContext& ctx);
void gvkDestroyFrameSyncObjects(gVKContext& ctx);
bool gvkCreatePresentSemaphores(gVKContext& ctx, uint32_t imagecount);
void gvkDestroyPresentSemaphores(gVKContext& ctx);
bool gvkBeginFrame(gVKContext& ctx, GLFWwindow* window);
bool gvkEndFrame(gVKContext& ctx, GLFWwindow* window);

// Validation layers cost performance, so the default follows the same DEBUG
// condition the OpenGL debug output already uses in this engine. A developer can
// still override it per context through setValidationEnabled().
#if defined(DEBUG) || defined(ENGINE_OPENGL_CHECKS)
inline constexpr bool gvkdefaultvalidation = true;
#else
inline constexpr bool gvkdefaultvalidation = false;
#endif

// Every Vulkan handle lives here so the header stays Vulkan free, right next to
// the developer facing settings that shape initialisation. Handles start as
// VK_NULL_HANDLE, which is what makes the "destroy only if non null" teardown
// correct even when initialisation fails half way through.
//
// The public surface is accessor based: settings go in through setters, and both
// settings and handles come back out through pointer returning getters. Handing
// back the address of a handle is deliberate - that is exactly the shape most
// Vulkan entry points want for their out parameters, so the same getter both
// reads a handle and receives it when the next phase (swapchain, pipeline, ...)
// is wired up. gVKRenderEngine drives creation, so it is a friend and reaches
// the raw members directly; every other consumer goes through the accessors.
struct gVKContext {
	friend class gVKRenderEngine;
	// The backend's own modules create and tear down the members below, so they
	// reach them directly just like gVKRenderEngine does. Everything outside the
	// backend goes through the accessors.
	friend bool gvkCreateSwapchain(gVKContext&, GLFWwindow*);
	friend void gvkDestroySwapchain(gVKContext&);
	friend bool gvkRecreateSwapchain(gVKContext&, GLFWwindow*);
	friend bool gvkCreateRenderPass(gVKContext&);
	friend void gvkDestroyRenderPass(gVKContext&);
	friend bool gvkCreateFramebuffers(gVKContext&);
	friend void gvkDestroyFramebuffers(gVKContext&);
	friend bool gvkCreateCommandResources(gVKContext&);
	friend void gvkDestroyCommandResources(gVKContext&);
	friend bool gvkCreateFrameSyncObjects(gVKContext&);
	friend void gvkDestroyFrameSyncObjects(gVKContext&);
	friend bool gvkCreatePresentSemaphores(gVKContext&, uint32_t);
	friend void gvkDestroyPresentSemaphores(gVKContext&);
	friend bool gvkBeginFrame(gVKContext&, GLFWwindow*);
	friend bool gvkEndFrame(gVKContext&, GLFWwindow*);

	/* ---------------- configurable settings ---------------- */
	// Set these before the backend initialises to influence instance and device
	// creation. Reading them afterwards simply reports what was used.

	// Identity handed to VkApplicationInfo. Informational to drivers and tools,
	// but handy for profiling and crash triage.
	void setAppName(const std::string& name) { appname = name; }
	void setEngineName(const std::string& name) { enginename = name; }
	void setAppVersion(uint32_t version) { appversion = version; }
	void setEngineVersion(uint32_t version) { engineversion = version; }

	// The minimum Vulkan version the engine requires (the floor), e.g.
	// VK_API_VERSION_1_3. init does NOT target this value: it targets the highest
	// version the loader offers, so newer Vulkan (1.4 today, 1.5+ later) is picked
	// up automatically with no code change. This floor only decides when to give
	// up - if the loader cannot even reach it, init fails instead of limping on.
	void setMinApiVersion(uint32_t version) { minapiversion = version; }

	// Validation layers are a debugging aid; on by default only in debug builds.
	void setValidationEnabled(bool enabled) { enablevalidation = enabled; }

	// Extra names appended on top of the mandatory GLFW / portability ones the
	// engine always requests. The pointed to strings must outlive init, so string
	// literals (or otherwise long lived storage) are the natural fit.
	void addInstanceExtension(const char* name) { extrainstanceextensions.push_back(name); }
	void addDeviceExtension(const char* name) { extradeviceextensions.push_back(name); }
	void addLayer(const char* name) { extralayers.push_back(name); }

	// Pointer returning getters for the settings, so a caller can both inspect
	// and, when a Vulkan struct wants an address, forward it without copying.
	std::string* getAppName() { return &appname; }
	std::string* getEngineName() { return &enginename; }
	uint32_t* getAppVersion() { return &appversion; }
	uint32_t* getEngineVersion() { return &engineversion; }
	uint32_t* getMinApiVersion() { return &minapiversion; }
	bool* getValidationEnabled() { return &enablevalidation; }
	std::vector<const char*>* getInstanceExtensions() { return &extrainstanceextensions; }
	std::vector<const char*>* getDeviceExtensions() { return &extradeviceextensions; }
	std::vector<const char*>* getLayers() { return &extralayers; }

	/* ---------------- created Vulkan handles ---------------- */
	// Filled during init. Each getter returns the address of the handle, matching
	// the out parameter shape of the Vulkan calls that will consume them.

	VkInstance* getInstance() { return &instance; }
	VkDebugUtilsMessengerEXT* getDebugMessenger() { return &debugmessenger; }
	VkSurfaceKHR* getSurface() { return &surface; }
	VkPhysicalDevice* getPhysicalDevice() { return &physicaldevice; }

	// The full set of GPUs the instance enumerated, and how many. init keeps only
	// the first device that can both render and present (getPhysicalDevice());
	// these expose the whole list so code can inspect or pick a different one.
	uint32_t* getDeviceCount() { return &devicecount; }
	std::vector<VkPhysicalDevice>* getPhysicalDevices() { return &physicaldevices; }

	// Properties and features for every enumerated GPU (parallel to
	// getPhysicalDevices()), including the ones init did not pick, so code can
	// compare and choose a different device without querying each handle itself.
	std::vector<VkPhysicalDeviceProperties>* getAllDeviceProperties() { return &physicaldeviceproperties; }
	std::vector<VkPhysicalDeviceFeatures>* getAllDeviceFeatures() { return &physicaldevicefeatures; }

	VkDevice* getDevice() { return &device; }
	VkQueue* getGraphicsQueue() { return &graphicsqueue; }
	VkQueue* getPresentQueue() { return &presentqueue; }
	uint32_t* getGraphicsFamily() { return &graphicsfamily; }
	uint32_t* getPresentFamily() { return &presentfamily; }

	// Queue families of the selected physical device as the driver reported them:
	// queue counts and capability flags (graphics/compute/transfer/...). init reads
	// these to choose the graphics and present indices; kept for later multi-queue
	// work (e.g. a dedicated transfer or compute queue).
	std::vector<VkQueueFamilyProperties>* getQueueFamilyProperties() { return &queuefamilyproperties; }

	// Per queue family of the selected device: whether that family can present to
	// the surface (parallel to getQueueFamilyProperties()). init keeps only the
	// first presentable family index; this exposes every family's support.
	std::vector<VkBool32>* getQueueFamilyPresentSupport() { return &queuefamilypresentsupport; }

	// The instance extensions, layers and device extensions actually enabled at
	// creation: the mandatory GLFW / portability / swapchain / validation names
	// merged with the developer's additions. getInstanceExtensions() / getLayers()
	// / getDeviceExtensions() above return only the developer's extra requests;
	// these return the full effective set that was handed to Vulkan.
	std::vector<const char*>* getEnabledInstanceExtensions() { return &enabledinstanceextensions; }
	std::vector<const char*>* getEnabledLayers() { return &enabledlayers; }
	std::vector<const char*>* getEnabledDeviceExtensions() { return &enableddeviceextensions; }

	// Everything the instance / GPU actually supports (not just what we enabled),
	// enumerated once at init so a developer can check for a capability without
	// re-querying: is extension X available on this GPU, is layer Y installed.
	std::vector<VkExtensionProperties>* getAvailableInstanceExtensions() { return &availableinstanceextensions; }
	std::vector<VkLayerProperties>* getAvailableLayers() { return &availablelayers; }
	std::vector<VkExtensionProperties>* getAvailableDeviceExtensions() { return &availabledeviceextensions; }

	// The three core physical-device capability blocks, queried once during init.
	// Properties: limits and identity. Features: optional capabilities the GPU
	// supports (samplerAnisotropy, geometryShader, ...). Memory: heaps and memory
	// types, needed to pick where every buffer and image gets allocated.
	VkPhysicalDeviceProperties* getDeviceProperties() { return &deviceproperties; }

	// Convenience for the selected GPU's own Vulkan version (same as
	// getDeviceProperties()->apiVersion). Check against this before using
	// version-specific core features; getInstanceApiVersion() is the loader side.
	uint32_t getDeviceApiVersion() const { return deviceproperties.apiVersion; }

	VkPhysicalDeviceFeatures* getDeviceFeatures() { return &devicefeatures; }
	VkPhysicalDeviceMemoryProperties* getDeviceMemoryProperties() { return &devicememoryproperties; }

	// The surface's capabilities and the formats / present modes it supports on the
	// selected device - what the swapchain is built from: extent and image-count
	// bounds, colour formats, and vsync / present modes.
	VkSurfaceCapabilitiesKHR* getSurfaceCapabilities() { return &surfacecapabilities; }
	std::vector<VkSurfaceFormatKHR>* getSurfaceFormats() { return &surfaceformats; }
	std::vector<VkPresentModeKHR>* getSurfacePresentModes() { return &surfacepresentmodes; }

	// The instance-level Vulkan version the loader actually supports (from
	// vkEnumerateInstanceVersion). This is exactly what init targets - it is the
	// highest version available; getMinApiVersion() is only the floor init checks
	// it against.
	uint32_t* getInstanceApiVersion() { return &instanceapiversion; }

	// Whether validation is actually running, which is not the same as whether it
	// was requested: setValidationEnabled(true) still yields false here when the
	// layer or debug-utils extension is missing at runtime. getValidationEnabled()
	// reports the request; this reports the outcome.
	bool isValidationActive() const { return validationactive; }

	// True once a logical device exists, i.e. init reached the point where the
	// context is actually usable for swapchains, pipelines and queues.
	bool isInitialized() const { return device != VK_NULL_HANDLE; }

	/* ---------------- presentation and frame path ---------------- */
	// Built after the device, in this order: swapchain, render pass, framebuffers,
	// command pool and buffers, synchronisation. Same pointer returning shape as
	// the handles above, for the same reason.

	VkSwapchainKHR* getSwapchain() { return &swapchain; }
	std::vector<VkImage>* getSwapchainImages() { return &swapchainimages; }
	std::vector<VkImageView>* getSwapchainImageViews() { return &swapchainimageviews; }
	VkFormat* getSwapchainFormat() { return &swapchainformat; }
	VkExtent2D* getSwapchainExtent() { return &swapchainextent; }

	VkRenderPass* getRenderPass() { return &renderpass; }
	std::vector<VkFramebuffer>* getFramebuffers() { return &framebuffers; }

	VkCommandPool* getCommandPool() { return &commandpool; }
	std::vector<VkCommandBuffer>* getCommandBuffers() { return &commandbuffers; }

	// Image available semaphores and fences are per frame in flight; the render
	// finished ones are per swapchain image, because presentation waits on the
	// semaphore chosen by the acquired image index.
	std::vector<VkSemaphore>* getImageAvailableSemaphores() { return &imageavailablesemaphores; }
	std::vector<VkFence>* getInFlightFences() { return &inflightfences; }
	std::vector<VkSemaphore>* getRenderFinishedSemaphores() { return &renderfinishedsemaphores; }

	// The colour the render pass writes over the whole attachment when a frame
	// begins; gVKRenderEngine::clearColor() feeds it.
	VkClearValue* getClearValue() { return &clearvalue; }

	// The window the surface was created from. The frame loop needs it to notice
	// resizes and rebuild the swapchain.
	GLFWwindow* getWindow() { return window; }

	// True once the frame path exists too, so a frame can actually be recorded.
	// isInitialized() only promises a logical device.
	bool isFramePathReady() const {
		return swapchain != VK_NULL_HANDLE && renderpass != VK_NULL_HANDLE;
	}

private:
	std::string appname = "GlistApp";
	std::string enginename = "GlistEngine";
	uint32_t appversion = VK_MAKE_API_VERSION(0, 1, 0, 0);
	uint32_t engineversion = VK_MAKE_API_VERSION(0, 1, 0, 0);
	uint32_t minapiversion = VK_API_VERSION_1_3;
	bool enablevalidation = gvkdefaultvalidation;
	std::vector<const char*> extrainstanceextensions;
	std::vector<const char*> extradeviceextensions;
	std::vector<const char*> extralayers;

	VkInstance instance = VK_NULL_HANDLE;
	VkDebugUtilsMessengerEXT debugmessenger = VK_NULL_HANDLE;
	VkSurfaceKHR surface = VK_NULL_HANDLE;
	VkPhysicalDevice physicaldevice = VK_NULL_HANDLE;
	VkDevice device = VK_NULL_HANDLE;
	VkQueue graphicsqueue = VK_NULL_HANDLE;
	VkQueue presentqueue = VK_NULL_HANDLE;
	uint32_t graphicsfamily = 0;
	uint32_t presentfamily = 0;
	uint32_t devicecount = 0;
	std::vector<VkPhysicalDevice> physicaldevices;
	std::vector<VkPhysicalDeviceProperties> physicaldeviceproperties;
	std::vector<VkPhysicalDeviceFeatures> physicaldevicefeatures;
	std::vector<VkQueueFamilyProperties> queuefamilyproperties;
	std::vector<VkBool32> queuefamilypresentsupport;
	std::vector<const char*> enabledinstanceextensions;
	std::vector<const char*> enabledlayers;
	std::vector<const char*> enableddeviceextensions;
	std::vector<VkExtensionProperties> availableinstanceextensions;
	std::vector<VkLayerProperties> availablelayers;
	std::vector<VkExtensionProperties> availabledeviceextensions;
	VkPhysicalDeviceProperties deviceproperties{};
	VkPhysicalDeviceFeatures devicefeatures{};
	VkPhysicalDeviceMemoryProperties devicememoryproperties{};
	VkSurfaceCapabilitiesKHR surfacecapabilities{};
	std::vector<VkSurfaceFormatKHR> surfaceformats;
	std::vector<VkPresentModeKHR> surfacepresentmodes;
	uint32_t instanceapiversion = 0;
	bool validationactive = false;

	GLFWwindow* window = nullptr;

	VkSwapchainKHR swapchain = VK_NULL_HANDLE;
	std::vector<VkImage> swapchainimages;
	std::vector<VkImageView> swapchainimageviews;
	VkFormat swapchainformat = VK_FORMAT_UNDEFINED;
	VkExtent2D swapchainextent = {0, 0};

	VkRenderPass renderpass = VK_NULL_HANDLE;
	// One per swapchain image view.
	std::vector<VkFramebuffer> framebuffers;

	VkCommandPool commandpool = VK_NULL_HANDLE;
	// GVK_MAX_FRAMES_IN_FLIGHT entries, indexed by currentframe.
	std::vector<VkCommandBuffer> commandbuffers;

	// GVK_MAX_FRAMES_IN_FLIGHT entries, indexed by currentframe.
	std::vector<VkSemaphore> imageavailablesemaphores;
	std::vector<VkFence> inflightfences;
	// One per swapchain image, indexed by currentimageindex.
	std::vector<VkSemaphore> renderfinishedsemaphores;

	uint32_t currentframe = 0;
	uint32_t currentimageindex = 0;
	bool frameactive = false;
	// Cornflower blue until clearColor() says otherwise.
	VkClearValue clearvalue = {{{0.39f, 0.58f, 0.93f, 1.0f}}};
};

#endif /* GVK_DESKTOP_GLFW */

#endif /* CORE_GVKCONTEXT_H */
