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
// with GVK_VULKAN so those translation units stay empty when Vulkan is not
// available, and the engine still builds.
#if defined(GLIST_HAS_VULKAN) && !defined(EMSCRIPTEN)
#define GVK_VULKAN 1
#endif

#ifdef GVK_VULKAN

#include <vulkan/vulkan.h>
#include <algorithm>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>
#include <map>
#include <array>

class gBaseWindow;
struct gVKContext;

// How many frames the CPU may prepare while the GPU is still busy with earlier ones.
inline constexpr int GVK_MAX_FRAMES_IN_FLIGHT = 2;
inline constexpr uint32_t GVK_SCENE_UNIFORM_SLOTS = 16;

// Declared here as well so the struct can befriend them.
bool gvkCreateSwapchain(gVKContext& ctx, gBaseWindow* window);
void gvkDestroySwapchain(gVKContext& ctx);
bool gvkRecreateSwapchain(gVKContext& ctx, gBaseWindow* window);
VkFormat gvkFindDepthFormat(gVKContext& ctx);
bool gvkCreateDepthResources(gVKContext& ctx);
void gvkDestroyDepthResources(gVKContext& ctx);
bool gvkCreateMsaaColorResources(gVKContext& ctx);
void gvkDestroyMsaaColorResources(gVKContext& ctx);
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
bool gvkBeginFrame(gVKContext& ctx, gBaseWindow* window);
bool gvkEndFrame(gVKContext& ctx, gBaseWindow* window);

// 2D draw path. Declared here so the struct can befriend them; defined in
// gVKFrame.cpp (render pass) / gVKPipeline.cpp (pipelines) / gVKDraw.cpp (ring).
bool gvkEnsureRenderPass(gVKContext& ctx);
bool gvkCreateGraphicsPipelines(gVKContext& ctx);
bool gvkReloadGraphicsPipelines(gVKContext& ctx);
void gvkDestroyGraphicsPipelines(gVKContext& ctx);
bool gvkCreateDrawResources(gVKContext& ctx);
bool gvkEnsureMeshArena(gVKContext& ctx, VkDeviceSize capacity);
void gvkDestroyMeshArena(gVKContext& ctx);
void gvkDestroyDrawResources(gVKContext& ctx);
bool gvkCreateShadowResources(gVKContext& ctx, uint32_t width, uint32_t height);
bool gvkCreateShadowPipeline(gVKContext& ctx);
void gvkDestroyShadowPipeline(gVKContext& ctx);
void gvkDestroyShadowResources(gVKContext& ctx);
bool gvkBeginShadowPass(gVKContext& ctx);
void gvkEndShadowPass(gVKContext& ctx);
bool gvkCreateUniformResources(gVKContext& ctx);
void gvkDestroyUniformResources(gVKContext& ctx);

// Multisampling and the render-pass compatibility problem it creates.
//
// A graphics pipeline is baked against a render pass, and the sample count of
// every attachment is part of render-pass *compatibility*: a pipeline built for a
// 4x pass cannot be recorded into a 1x one, nor the other way round. This backend
// has three kinds of pass - the screen pass, one pass per offscreen render target
// (gFbo), and the shadow pass - and the same 2D/3D pipelines are recorded into the
// first two. Switching the screen pass to 4x therefore breaks every draw into an
// FBO unless something is done about it.
//
// The two honest ways out are to put every pass on the same sample count, or to
// key the pipelines by sample count and build the variants that are actually used.
// This backend does the second, for three reasons:
//
//  - The shadow pass must stay 1x. Multisampling a depth-only map that is compared
//    against rather than displayed buys nothing and costs bandwidth and memory on
//    exactly the mobile parts this backend targets. So "one count everywhere" is
//    already impossible; the only question is where the seam sits.
//  - An FBO's attachments are gTextures the application samples afterwards. Making
//    those passes multisampled means a transient MSAA image *plus* the resolve
//    target for every render target the app owns, and a depth-only FBO - which
//    gFbo produces for depth prepasses - has no resolve path at all before VK 1.2's
//    separate depth/stencil resolve. A post-processing chain wants the raw 1x
//    buffer regardless.
//  - The cost of the second design is one extra set of VkPipeline objects, and only
//    while MSAA is on. They come out of the same VkPipelineCache and reuse the same
//    modules and layouts, so the second build is a fraction of the first.
//
// So: index 0 is the single-sample build, used by every offscreen pass and by the
// screen pass while MSAA is off; index 1 is the build for the screen pass's sample
// count and only exists while that is above one. getActivePipelineVariant() is what
// the pipeline getters index with, and whichever pass opens sets it.
inline constexpr uint32_t GVK_PIPELINE_SAMPLE_VARIANTS = 2;

// The forms one pipeline family takes at a single sample count. Which of them get
// built is decided per family by gvkPipelineOptions; the ones that are not stay
// null and the getters fall back to the plain pipeline. gVKContext holds one array
// of these per family, indexed by sample-count variant.
struct gVKPipelineVariants {
	VkPipeline pipeline = VK_NULL_HANDLE;
	// The same pipeline with a line topology, for stroking unfilled shapes and for
	// meshes drawn as wireframe.
	VkPipeline linepipeline = VK_NULL_HANDLE;
	// The same pipeline with the opposite blend setting; blending cannot be dynamic
	// state, so following the renderer's setting means choosing a pipeline.
	VkPipeline blendvariantpipeline = VK_NULL_HANDLE;
	// The same pipeline adding instead of compositing, for BLENDMODE_ADDITIVE.
	VkPipeline additivepipeline = VK_NULL_HANDLE;
	// The same pipeline with the cutout discard specialised out; see
	// gvkPipelineOptions::nocutoutvariant in gVKPipeline.cpp.
	VkPipeline nocutoutpipeline = VK_NULL_HANDLE;
};

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
	friend bool gvkCreateSwapchain(gVKContext&, gBaseWindow*);
	friend void gvkDestroySwapchain(gVKContext&);
	friend bool gvkRecreateSwapchain(gVKContext&, gBaseWindow*);
	friend VkFormat gvkFindDepthFormat(gVKContext&);
	friend bool gvkCreateDepthResources(gVKContext&);
	friend void gvkDestroyDepthResources(gVKContext&);
	friend bool gvkCreateMsaaColorResources(gVKContext&);
	friend void gvkDestroyMsaaColorResources(gVKContext&);
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
	friend bool gvkBeginFrame(gVKContext&, gBaseWindow*);
	friend bool gvkEndFrame(gVKContext&, gBaseWindow*);
	friend bool gvkEnsureRenderPass(gVKContext&);
	friend bool gvkCreateGraphicsPipelines(gVKContext&);
	friend bool gvkReloadGraphicsPipelines(gVKContext&);
	friend void gvkDestroyGraphicsPipelines(gVKContext&);
	friend void gvkDestroyPipelineCache(gVKContext&);
	friend bool gvkCreateDrawResources(gVKContext&);
	friend bool gvkEnsureMeshArena(gVKContext&, VkDeviceSize);
	friend void gvkDestroyMeshArena(gVKContext&);
	friend void gvkDestroyDrawResources(gVKContext&);
	friend bool gvkCreateShadowResources(gVKContext&, uint32_t, uint32_t);
	friend bool gvkCreateShadowPipeline(gVKContext&);
	friend void gvkDestroyShadowPipeline(gVKContext&);
	friend void gvkDestroyShadowResources(gVKContext&);
	friend bool gvkBeginShadowPass(gVKContext&);
	friend void gvkEndShadowPass(gVKContext&);
	friend bool gvkCreateUniformResources(gVKContext&);
	friend void gvkDestroyUniformResources(gVKContext&);
	friend struct gVKSceneUniforms;
	friend bool gvkWriteSceneUniforms(gVKContext&, const struct gVKSceneUniforms&);

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
	VkRenderPass getShadowRenderPass() const { return shadowrenderpass; }
	VkDescriptorSet getShadowDescriptorSet() const { return shadowdescriptorset; }
	bool hasShadowMap() const { return shadowframebuffer != VK_NULL_HANDLE; }
	VkPipeline getShadowPipeline() const { return shadowpipeline; }
	VkPipelineLayout getShadowPipelineLayout() const { return shadowpipelinelayout; }
	// False when the shadow shader does not sample its cutout map, in which case the
	// reflected layout has no descriptor set to bind into.
	bool hasShadowDescriptorSetLayout() const { return !shadowsetlayouts.empty(); }
	uint32_t getShadowPushSize() const { return shadowpushsize; }
	VkShaderStageFlags getShadowPushStages() const { return shadowpushstages; }
	bool isShadowPassActive() const { return shadowpassactive; }
	std::vector<VkFramebuffer>* getFramebuffers() { return &framebuffers; }

	// Depth attachments are private to each frame in flight.
	VkFormat getDepthFormat() const { return depthformat; }
	VkImageView getDepthImageView() const {
		return depthimageviews.empty() ? VK_NULL_HANDLE : depthimageviews[0];
	}

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
	gBaseWindow* getWindow() { return window; }

	// True once the frame path exists too, so a frame can actually be recorded.
	// isInitialized() only promises a logical device.
	bool isFramePathReady() const {
		return swapchain != VK_NULL_HANDLE && renderpass != VK_NULL_HANDLE;
	}

	/* ---------------- 2D draw path ---------------- */
	// Pipelines and descriptor infrastructure the Vulkan triangle / rectangle /
	// image helpers use, plus the per-frame host-visible vertex ring. Built by
	// gvkCreateGraphicsPipelines and gvkCreateDrawResources, consumed by gVKDraw.

	// Every getter here indexes by getActivePipelineVariant(), which the open pass
	// set: the screen pass hands out the pipelines built for its sample count, an
	// offscreen pass the single-sampled ones. See GVK_PIPELINE_SAMPLE_VARIANTS.
	VkPipeline getColor2DPipeline() { return color2d[activepipelinevariant].pipeline; }
	// Same pipeline with a line topology, for stroking unfilled shapes.
	VkPipeline getColor2DLinePipeline() { return color2d[activepipelinevariant].linepipeline; }
	// Additive copies of the 2D pipelines, for gRenderer::BLENDMODE_ADDITIVE. Blend
	// factors are baked into a pipeline, so the mode is chosen by binding one or the
	// other; both fall back to the compositing pipeline if the copy failed to build.
	VkPipeline getColor2DAdditivePipeline() {
		const gVKPipelineVariants& v = color2d[activepipelinevariant];
		return v.additivepipeline != VK_NULL_HANDLE ? v.additivepipeline : v.pipeline;
	}
	VkPipelineLayout getColor2DPipelineLayout() { return color2dpipelinelayout; }
	VkPipeline getImage2DPipeline() { return image2d[activepipelinevariant].pipeline; }
	VkPipeline getImage2DAdditivePipeline() {
		const gVKPipelineVariants& v = image2d[activepipelinevariant];
		return v.additivepipeline != VK_NULL_HANDLE ? v.additivepipeline : v.pipeline;
	}
	VkPipelineLayout getImage2DPipelineLayout() { return image2dpipelinelayout; }
	// The 3D mesh pipeline, and the same pipeline with a line topology for meshes
	// drawn as wireframe.
	VkPipeline getMesh3DPipeline() { return mesh3d[activepipelinevariant].pipeline; }
	VkPipeline getMesh3DLinePipeline() { return mesh3d[activepipelinevariant].linepipeline; }
	VkPipeline getSkyboxPipeline() { return skybox[activepipelinevariant].pipeline; }
	VkPipelineLayout getSkyboxPipelineLayout() { return skyboxpipelinelayout; }
	uint32_t getSkyboxPushSize() const { return skyboxpushsize; }
	VkShaderStageFlags getSkyboxPushStages() const { return skyboxpushstages; }
	// Blending cannot be a dynamic state, so each 3D path keeps a second pipeline
	// with the opposite setting and the draw picks by the renderer's current one.
	// The line variant has no blended twin: a wireframe pass is a debugging aid.
	VkPipeline getMesh3DPipeline(bool blending) {
		const gVKPipelineVariants& v = mesh3d[activepipelinevariant];
		return blending && v.blendvariantpipeline != VK_NULL_HANDLE ? v.blendvariantpipeline : v.pipeline;
	}
	// cutout false means the caller has established that this draw's diffuse map holds
	// no texel transparent enough for the shader's cutout test, so it can go through
	// the copy with the discard compiled out and keep early depth rejection. Only the
	// opaque form has that copy: a blended draw is sorted and layered rather than
	// depth rejected, so it gains nothing, and the wireframe one is a debugging aid.
	VkPipeline getMesh3DPipeline(bool blending, bool cutout) {
		const gVKPipelineVariants& v = mesh3d[activepipelinevariant];
		if(!blending && !cutout && v.nocutoutpipeline != VK_NULL_HANDLE) return v.nocutoutpipeline;
		return getMesh3DPipeline(blending);
	}
	VkPipeline getMesh3DPbrPipeline(bool blending) {
		const gVKPipelineVariants& v = mesh3dpbr[activepipelinevariant];
		return blending && v.blendvariantpipeline != VK_NULL_HANDLE ? v.blendvariantpipeline : v.pipeline;
	}
	VkPipelineLayout getMesh3DPipelineLayout() { return mesh3dpipelinelayout; }
	uint32_t getMesh3DPushSize() const { return mesh3dpushsize; }
	VkShaderStageFlags getMesh3DPushStages() const { return mesh3dpushstages; }

	VkPipeline getMesh3DPbrPipeline() { return mesh3dpbr[activepipelinevariant].pipeline; }
	VkPipelineLayout getMesh3DPbrPipelineLayout() { return mesh3dpbrpipelinelayout; }
	uint32_t getMesh3DPbrPushSize() const { return mesh3dpbrpushsize; }
	VkShaderStageFlags getMesh3DPbrPushStages() const { return mesh3dpbrpushstages; }
	// Layout of the PBR material set (set 1), which gVKRenderEngine allocates one of
	// per distinct combination of maps.
	std::map<std::array<uint32_t, 5>, VkDescriptorSet>* getPbrMaterialSets() { return &pbrmaterialsets; }
	VkDescriptorSetLayout getMesh3DPbrMaterialSetLayout() {
		return mesh3dpbrsetlayouts.size() > 1 ? mesh3dpbrsetlayouts[1] : VK_NULL_HANDLE;
	}
	// The same arrangement for the classic path's diffuse/specular/normal trio.
	std::map<std::array<uint32_t, 3>, VkDescriptorSet>* getMaterialSets() { return &materialsets; }
	VkDescriptorSetLayout getMesh3DMaterialSetLayout() {
		return mesh3dsetlayouts.size() > 1 ? mesh3dsetlayouts[1] : VK_NULL_HANDLE;
	}

	// The scene descriptor set of the frame being recorded: camera matrices and
	// lights. VK_NULL_HANDLE when the 3D path has no uniform block.
	VkDescriptorSet getCurrentSceneDescriptorSet() const {
		return sceneuniformsets[currentframe][currentsceneuniformslot];
	}
	void resetSceneUniformSlots() { currentsceneuniformslot = 0; sceneuniformslotcount = 0; }
	// Which of the frames in flight is being recorded. Anything the CPU rewrites
	// while the GPU may still be reading the previous frame needs one copy per
	// index and has to write the one this returns: the frame loop waits on that
	// index's fence before recording, which is what makes the copy free again.
	uint32_t getCurrentFrame() const { return currentframe; }

	// Whether presentation waits for the display. Vulkan expresses this as the
	// swapchain's present mode rather than a call like glfwSwapInterval, so a
	// change only takes effect when the swapchain is rebuilt - which is what
	// gVKRenderEngine::setVsync asks for.
	void setVsyncEnabled(bool enabled) { vsyncenabled = enabled; }
	bool isVsyncEnabled() const { return vsyncenabled; }

	/* ---------------- multisampling ---------------- */
	// What the application asked for, and what it got. The two are not the same
	// thing: a request is capped at the intersection of the device's
	// framebufferColorSampleCounts and framebufferDepthSampleCounts, so asking for
	// 8x on a part that offers 4x colour and 4x depth yields 4x, and asking on a
	// part that offers neither yields 1x - MSAA simply off. Like vsync, the sample
	// count is baked into objects (the render pass, its attachments and every
	// pipeline) rather than set per draw, so a change only lands when those are
	// rebuilt; gVKRenderEngine::setMultiSampling does that at a frame boundary.
	void setSampleCount(int samples) {
		requestedsamples = samples < 1 ? 1 : static_cast<uint32_t>(samples);
	}
	int getRequestedSampleCount() const { return static_cast<int>(requestedsamples); }
	int getActiveSampleCount() const { return static_cast<int>(samplecount); }
	VkSampleCountFlagBits getSampleCountFlag() const { return samplecount; }
	bool isMultiSampled() const { return samplecount != VK_SAMPLE_COUNT_1_BIT; }

	// Sample shading runs the fragment shader per sample instead of per pixel, which
	// is the only thing that antialiases *inside* a triangle - specular sparkle on a
	// normal-mapped surface, alpha-tested foliage edges. It is off by default and
	// should stay off on mobile: coverage MSAA costs the resolve and nothing else,
	// while sample shading multiplies the fragment shader's own cost by
	// minSampleShading x rasterizationSamples, which on a tiler with a heavy PBR
	// shader is the difference between free and unshippable. Requires the device's
	// sampleRateShading feature; the request is ignored when it is missing.
	void setSampleShadingEnabled(bool enabled) { sampleshadingenabled = enabled; }
	bool isSampleShadingEnabled() const { return sampleshadingenabled; }
	void setMinSampleShading(float fraction) { minsampleshading = fraction; }
	float getMinSampleShading() const { return minsampleshading; }

	// Which sample-count build of the pipelines the pass currently being recorded
	// needs; every pipeline getter above indexes with it. Whichever pass opens sets
	// it, so a draw can never reach a pipeline that pass would reject.
	uint32_t getActivePipelineVariant() const { return activepipelinevariant; }
	void useScreenPipelines() { activepipelinevariant = isMultiSampled() ? 1 : 0; }
	void useOffscreenPipelines() { activepipelinevariant = 0; }

	VkDeviceSize getMinUniformBufferOffsetAlignment() const {
		return deviceproperties.limits.minUniformBufferOffsetAlignment;
	}
	// The layout of the image pipeline's descriptor set 0, which is where a
	// texture's combined image sampler goes.
	VkDescriptorSetLayout getImageDescriptorSetLayout() {
		return image2dsetlayouts.empty() ? VK_NULL_HANDLE : image2dsetlayouts[0];
	}
	VkDescriptorPool getDescriptorPool() { return descriptorpool; }
	std::map<uint64_t, std::pair<VkSampler, uint32_t>>& getSamplerCache() { return samplercache; }
	bool isRenderPassActive() const { return renderpassactive; }

	// Push constant block each 2D pipeline declares, as reported by reflecting its
	// SPIR-V. gVKDraw pushes exactly this much to exactly these stages, so editing
	// the push_constant block in a shader needs no matching change in the draw path.
	uint32_t getColor2DPushSize() const { return color2dpushsize; }
	VkShaderStageFlags getColor2DPushStages() const { return color2dpushstages; }
	uint32_t getImage2DPushSize() const { return image2dpushsize; }
	VkShaderStageFlags getImage2DPushStages() const { return image2dpushstages; }

	// The command buffer of the frame currently being recorded, or VK_NULL_HANDLE
	// when no frame is active.
	VkCommandBuffer getCurrentCommandBuffer() {
		return frameactive ? commandbuffers[currentframe] : VK_NULL_HANDLE;
	}
	// Whether a frame is being recorded. Uploads that arrive outside one - a canvas
	// building its meshes in setup() - cannot use anything the frame loop rewinds.
	bool isFrameActive() const { return frameactive; }

	// Per-frame vertex ring. resetDynamicVertices() rewinds the current frame's
	// buffer at frame start; pushDynamicVertices() appends vertex bytes (16-byte
	// aligned) and returns the byte offset to bind from, or VK_WHOLE_SIZE when the
	// buffer is full.
	void resetDynamicVertices() {
		if(!dynvertexoffsets.empty()) dynvertexoffsets[currentframe] = 0;
	}
	VkBuffer getCurrentDynamicVertexBuffer() {
		return dynvertexbuffers.empty() ? VK_NULL_HANDLE : dynvertexbuffers[currentframe];
	}
	// alignment is the caller's vertex stride wherever the caller means to reach its
	// slice through firstVertex rather than by binding at the offset: the index is
	// offset/stride, so the offset has to be a whole number of vertices from the
	// start of the buffer. Paths that bind at their own offset can leave it at the
	// default. Getting this wrong shifts a draw by a fraction of a vertex and
	// scrambles it, so it is a parameter rather than an assumption - the ring is
	// shared, and one path's stride is not another's.
	VkDeviceSize pushDynamicVertices(const void* data, VkDeviceSize size, VkDeviceSize alignment = 16) {
		if(dynvertexmapped.empty()) return VK_WHOLE_SIZE;
		if(alignment == 0) alignment = 1;
		VkDeviceSize offset = ((dynvertexoffsets[currentframe] + alignment - 1) / alignment) * alignment;
		if(offset + size > dynvertexcapacity) return VK_WHOLE_SIZE;
		std::memcpy(static_cast<char*>(dynvertexmapped[currentframe]) + offset, data, size);
		dynvertexoffsets[currentframe] = offset + size;
		return offset;
	}

	// The same idea again, for 3D meshes whose vertices the CPU rewrites: a per
	// frame arena that every upload takes its own slice of.
	//
	// One slice per upload rather than one buffer per mesh, because a mesh can be
	// uploaded to several times in a single frame and drawn in between. A game
	// posing one soldier model once per enemy does exactly that, and a single
	// buffer per mesh gives every one of them the last pose written - the draws are
	// recorded now and executed later, so they all read whatever the buffer holds
	// by the end of the frame. Slices keep each draw pointing at the data it was
	// recorded with.
	//
	// pushMeshData returns the byte offset to bind from, or VK_WHOLE_SIZE when the
	// arena is full - the caller falls back to its own buffer for that frame, and
	// the arena grows to the high-water mark at the next frame start.
	void resetMeshArena() {
		if(mesharenaoffsets.empty()) return;
		if(mesharenaoffsets[currentframe] > mesharenahighwater) {
			mesharenahighwater = mesharenaoffsets[currentframe];
		}
		mesharenaoffsets[currentframe] = 0;
		meshgeneration++;
	}
	VkBuffer getCurrentMeshArena() {
		return mesharenabuffers.empty() ? VK_NULL_HANDLE : mesharenabuffers[currentframe];
	}
	VkDeviceSize pushMeshData(const void* data, VkDeviceSize size) {
		if(mesharenamapped.empty() || mesharenamapped[currentframe] == nullptr) return VK_WHOLE_SIZE;
		VkDeviceSize offset = (mesharenaoffsets[currentframe] + 63) & ~static_cast<VkDeviceSize>(63);
		if(offset + size > mesharenacapacity) {
			// Remember what was actually asked for, so the grow at the next reset is
			// big enough rather than one slice short of it.
			mesharenahighwater = std::max(mesharenahighwater, offset + size);
			return VK_WHOLE_SIZE;
		}
		std::memcpy(static_cast<char*>(mesharenamapped[currentframe]) + offset, data, size);
		mesharenaoffsets[currentframe] = offset + size;
		return offset;
	}
	// Which frame the arena is on. A slice handed out in an earlier frame points
	// into memory that has since been rewound, so a mesh drawn without a fresh
	// upload compares this before trusting the slice it kept.
	uint64_t getMeshGeneration() const { return meshgeneration; }

	// Command-buffer-local state cache. Vulkan state persists between draw calls,
	// so recording an unchanged bind or dynamic state again only adds CPU/driver
	// work. The frame and every render-pass begin reset this cache.
	void resetRecordedDrawState() {
		recordedpipeline = VK_NULL_HANDLE;
		recordeddescriptorlayout = VK_NULL_HANDLE;
		recordeddescriptorcount = 0;
		recordeddepthvalid = false;
		recordedtopologyvalid = false;
		recordedcullvalid = false;
		recordedvertexcount = 0;
		recordedindexbuffer = VK_NULL_HANDLE;
	}
	bool shouldBindPipeline(VkPipeline pipeline) {
		if(recordedpipeline == pipeline) return false;
		recordedpipeline = pipeline;
		recordeddescriptorlayout = VK_NULL_HANDLE;
		recordeddescriptorcount = 0;
		return true;
	}
	bool shouldBindDescriptorSets(VkPipelineLayout layout, const VkDescriptorSet* sets, uint32_t count) {
		if(recordeddescriptorlayout == layout && recordeddescriptorcount == count
				&& std::equal(sets, sets + count, recordeddescriptors.begin())) return false;
		recordeddescriptorlayout = layout;
		recordeddescriptorcount = count;
		std::copy(sets, sets + count, recordeddescriptors.begin());
		return true;
	}
	bool shouldSetDepthState(VkBool32 test, VkBool32 write, VkCompareOp compare) {
		if(recordeddepthvalid && recordeddepthtest == test && recordeddepthwrite == write
				&& recordeddepthcompare == compare) return false;
		recordeddepthvalid = true;
		recordeddepthtest = test;
		recordeddepthwrite = write;
		recordeddepthcompare = compare;
		return true;
	}
	bool shouldSetTopology(VkPrimitiveTopology topology) {
		if(recordedtopologyvalid && recordedtopology == topology) return false;
		recordedtopologyvalid = true;
		recordedtopology = topology;
		return true;
	}
	bool shouldSetCullState(VkCullModeFlags mode, VkFrontFace frontface) {
		if(recordedcullvalid && recordedcullmode == mode && recordedfrontface == frontface) return false;
		recordedcullvalid = true;
		recordedcullmode = mode;
		recordedfrontface = frontface;
		return true;
	}
	// Vertex and index bindings are command buffer state in their own right: binding a
	// pipeline does not disturb them, so a run of draws over one mesh - a crowd posed
	// from the same model, or the same buffer drawn once per material - can rebind the
	// same handles hundreds of times per frame for nothing. The contents behind a
	// handle may change between draws; that does not matter here, because a draw reads
	// the buffer when the GPU executes it, not when the bind was recorded.
	//
	// Every vkCmdBindVertexBuffers / vkCmdBindIndexBuffer in the backend goes through
	// these. One that did not would leave the cache describing a binding that is no
	// longer current, and the next draw would skip a bind it actually needed.
	bool shouldBindVertexBuffers(const VkBuffer* buffers, const VkDeviceSize* offsets, uint32_t count) {
		if(count > gvkmaxvertexbindings) return true;
		if(recordedvertexcount == count
				&& std::equal(buffers, buffers + count, recordedvertexbuffers.begin())
				&& std::equal(offsets, offsets + count, recordedvertexoffsets.begin())) {
			return false;
		}
		recordedvertexcount = count;
		std::copy(buffers, buffers + count, recordedvertexbuffers.begin());
		std::copy(offsets, offsets + count, recordedvertexoffsets.begin());
		return true;
	}
	bool shouldBindIndexBuffer(VkBuffer buffer, VkDeviceSize offset, VkIndexType type) {
		if(recordedindexbuffer == buffer && recordedindexoffset == offset
				&& recordedindextype == type) return false;
		recordedindexbuffer = buffer;
		recordedindexoffset = offset;
		recordedindextype = type;
		return true;
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

	gBaseWindow* window = nullptr;
	bool vsyncenabled = false;
	bool swapchainrecreaterequested = false;

	VkSwapchainKHR swapchain = VK_NULL_HANDLE;
	std::vector<VkImage> swapchainimages;
	std::vector<VkImageView> swapchainimageviews;
	VkFormat swapchainformat = VK_FORMAT_UNDEFINED;
	VkExtent2D swapchainextent = {0, 0};

	// One depth attachment per frame in flight. This avoids overlapping writes
	// without allocating one for every (often more numerous) swapchain image.
	std::vector<VkImage> depthimages;
	std::vector<VkDeviceMemory> depthimagememories;
	std::vector<VkImageView> depthimageviews;
	VkFormat depthformat = VK_FORMAT_UNDEFINED;

	// Multisampling. requestedsamples is the application's ask, samplecount is what
	// the device agreed to and what everything below is built with; see the setters.
	uint32_t requestedsamples = 1;
	VkSampleCountFlagBits samplecount = VK_SAMPLE_COUNT_1_BIT;
	bool sampleshadingenabled = false;
	float minsampleshading = 0.25f;

	// The multisampled colour attachment the screen pass renders into while MSAA is
	// on, one per frame in flight for the same reason depth is: private slots avoid
	// overlapping writes without one per swapchain image. It never leaves tile
	// memory - the pass clears it, resolves it into the acquired swapchain image and
	// discards the samples - so it is created TRANSIENT and, where the device offers
	// it, backed by LAZILY_ALLOCATED memory that a tiler never has to page in.
	std::vector<VkImage> msaacolorimages;
	std::vector<VkDeviceMemory> msaacolormemories;
	std::vector<VkImageView> msaacolorviews;

	// Shadow map: a depth-only target drawn from the light's point of view and
	// sampled while shading. See gVKShadow.h.
	VkImage shadowimage = VK_NULL_HANDLE;
	VkDeviceMemory shadowmemory = VK_NULL_HANDLE;
	VkImageView shadowview = VK_NULL_HANDLE;
	VkSampler shadowsampler = VK_NULL_HANDLE;
	VkRenderPass shadowrenderpass = VK_NULL_HANDLE;
	VkFramebuffer shadowframebuffer = VK_NULL_HANDLE;
	VkDescriptorSet shadowdescriptorset = VK_NULL_HANDLE;
	// Depth-only pipeline that fills the map. Built with the shadow render pass,
	// so it cannot exist before that pass does.
	VkPipeline shadowpipeline = VK_NULL_HANDLE;
	VkPipelineLayout shadowpipelinelayout = VK_NULL_HANDLE;
	// One set layout, for the cutout diffuse map. Held here rather than left in the
	// build's scratch struct because it has to outlive the build and be destroyed
	// with the pipeline; before the cutout sampler existed this list was empty.
	std::vector<VkDescriptorSetLayout> shadowsetlayouts;
	uint32_t shadowpushsize = 0;
	VkShaderStageFlags shadowpushstages = 0;
	VkFormat shadowformat = VK_FORMAT_UNDEFINED;
	VkExtent2D shadowextent = {0, 0};
	// True between gvkBeginShadowPass and gvkEndShadowPass, so the draw path can
	// tell which of the two passes it is recording into.
	bool shadowpassactive = false;

	VkRenderPass renderpass = VK_NULL_HANDLE;
	// A 1x colour+depth pass that is never begun. It exists only while the screen
	// pass is multisampled, as the compatibility template the single-sample
	// pipelines are built against - a pipeline needs a render pass at build time,
	// and the passes those pipelines really run in (the per-FBO ones) are created
	// later, on demand, one per render target. VK_NULL_HANDLE when MSAA is off,
	// where the screen pass is itself the 1x template.
	VkRenderPass singlesamplerenderpass = VK_NULL_HANDLE;
	// One per (frame-in-flight, swapchain image) pair.
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
	// Match gBaseCanvas::clearBackground(), the default used by the OpenGL path.
	VkClearValue clearvalue = {{{0.0f, 0.0f, 30.0f / 255.0f, 0.0f}}};

	// 2D draw path. Built after the frame path; VK_NULL_HANDLE / empty until then.
	// renderpassactive tracks the lazily-begun render pass within a frame: geometry
	// must be recorded inside vkCmdBeginRenderPass..EndRenderPass, but the clear
	// colour is only final once the canvas has drawn, so the pass is opened on the
	// first draw (or in endFrame) rather than in beginFrame.
	bool renderpassactive = false;
	bool screenshotrequested = false;
	bool screenshotready = false;
	std::vector<unsigned char> screenshotpixels;
	uint32_t screenshotwidth = 0;
	uint32_t screenshotheight = 0;
	VkFormat screenshotformat = VK_FORMAT_UNDEFINED;
	// One entry per sample count in use; index 0 is always the single-sample build.
	// Layouts and reflected sizes are shared by both, because the variants differ
	// only in their multisample state and the pass they were built against.
	VkPipelineLayout color2dpipelinelayout = VK_NULL_HANDLE;
	gVKPipelineVariants color2d[GVK_PIPELINE_SAMPLE_VARIANTS];
	VkPipelineLayout image2dpipelinelayout = VK_NULL_HANDLE;
	gVKPipelineVariants image2d[GVK_PIPELINE_SAMPLE_VARIANTS];
	// Which of the two the getters hand out; see useScreenPipelines().
	uint32_t activepipelinevariant = 0;
	// 3D mesh path. Same shape as the 2D pipelines above, but with depth test and
	// depth write on and a vertex layout matching gVertex.
	VkPipelineLayout mesh3dpipelinelayout = VK_NULL_HANDLE;
	gVKPipelineVariants mesh3d[GVK_PIPELINE_SAMPLE_VARIANTS];
	std::vector<VkDescriptorSetLayout> mesh3dsetlayouts;
	uint32_t mesh3dpushsize = 0;
	VkShaderStageFlags mesh3dpushstages = 0;

	// PBR variant. Its five maps share one descriptor set rather than taking one
	// each: Vulkan only guarantees four bound sets, and scene plus five textures
	// would need six.
	VkPipelineLayout mesh3dpbrpipelinelayout = VK_NULL_HANDLE;
	// The skybox: six textured quads around the camera, with a pipeline of its own
	// because nothing it draws is lit and it needs none of the mesh push constants.
	VkPipelineLayout skyboxpipelinelayout = VK_NULL_HANDLE;
	gVKPipelineVariants skybox[GVK_PIPELINE_SAMPLE_VARIANTS];
	std::vector<VkDescriptorSetLayout> skyboxsetlayouts;
	uint32_t skyboxpushsize = 0;
	VkShaderStageFlags skyboxpushstages = 0;
	gVKPipelineVariants mesh3dpbr[GVK_PIPELINE_SAMPLE_VARIANTS];
	std::vector<VkDescriptorSetLayout> mesh3dpbrsetlayouts;
	uint32_t mesh3dpbrpushsize = 0;
	VkShaderStageFlags mesh3dpbrpushstages = 0;
	// One descriptor set per distinct combination of the five PBR maps, keyed by
	// their texture ids. Materials commonly share maps, and a descriptor pool is
	// finite, so caching by combination rather than per mesh keeps allocations down.
	// Freed with the pool in gvkDestroyGraphicsPipelines, so this only needs clearing.
	std::map<std::array<uint32_t, 5>, VkDescriptorSet> pbrmaterialsets;
	// And one per distinct diffuse/specular/normal trio for the classic path.
	std::map<std::array<uint32_t, 3>, VkDescriptorSet> materialsets;

	// Camera and lights for the 3D path, one set per frame in flight so the CPU can
	// write the next frame while the GPU still reads the previous one. Permanently
	// mapped; see gVKUniform.h.
	VkBuffer sceneuniformbuffers[GVK_MAX_FRAMES_IN_FLIGHT] = {};
	VkDeviceMemory sceneuniformmemories[GVK_MAX_FRAMES_IN_FLIGHT] = {};
	void* sceneuniformmapped[GVK_MAX_FRAMES_IN_FLIGHT] = {};
	VkDescriptorSet sceneuniformsets[GVK_MAX_FRAMES_IN_FLIGHT][GVK_SCENE_UNIFORM_SLOTS] = {};
	uint32_t currentsceneuniformslot = 0;
	uint32_t sceneuniformslotcount = 0;
	// Descriptor set layouts of each pipeline, in set order, and the push constant
	// block each declares. All of it is reflected out of the compiled SPIR-V rather
	// than written out here, so the shaders stay the single source of truth.
	std::vector<VkDescriptorSetLayout> color2dsetlayouts;
	std::vector<VkDescriptorSetLayout> image2dsetlayouts;
	uint32_t color2dpushsize = 0;
	VkShaderStageFlags color2dpushstages = 0;
	uint32_t image2dpushsize = 0;
	VkShaderStageFlags image2dpushstages = 0;
	VkDescriptorPool descriptorpool = VK_NULL_HANDLE;
	VkPipelineCache pipelinecache = VK_NULL_HANDLE;
	// Identical filter/wrap combinations share one immutable sampler object.
	std::map<uint64_t, std::pair<VkSampler, uint32_t>> samplercache;
	// One host-visible, persistently mapped vertex buffer per frame in flight,
	// filled linearly each frame and rewound at the start of the next.
	std::vector<VkBuffer> dynvertexbuffers;
	std::vector<VkDeviceMemory> dynvertexmemories;
	std::vector<void*> dynvertexmapped;
	std::vector<VkDeviceSize> dynvertexoffsets;
	VkDeviceSize dynvertexcapacity = 0;

	// Per-frame arena for mesh vertices the CPU rewrites; see pushMeshData.
	std::vector<VkBuffer> mesharenabuffers;
	std::vector<VkDeviceMemory> mesharenamemories;
	std::vector<void*> mesharenamapped;
	std::vector<VkDeviceSize> mesharenaoffsets;
	VkDeviceSize mesharenacapacity = 0;
	VkDeviceSize mesharenahighwater = 0;
	uint64_t meshgeneration = 0;

	VkPipeline recordedpipeline = VK_NULL_HANDLE;
	VkPipelineLayout recordeddescriptorlayout = VK_NULL_HANDLE;
	std::array<VkDescriptorSet, 5> recordeddescriptors{};
	uint32_t recordeddescriptorcount = 0;
	bool recordeddepthvalid = false;
	VkBool32 recordeddepthtest = VK_FALSE;
	VkBool32 recordeddepthwrite = VK_FALSE;
	VkCompareOp recordeddepthcompare = VK_COMPARE_OP_ALWAYS;
	bool recordedtopologyvalid = false;
	VkPrimitiveTopology recordedtopology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	bool recordedcullvalid = false;
	VkCullModeFlags recordedcullmode = VK_CULL_MODE_NONE;
	VkFrontFace recordedfrontface = VK_FRONT_FACE_CLOCKWISE;
	// Two: a mesh binds its vertices plus the per-instance model matrices, and the
	// 2D paths bind one. A larger bind is simply never cached.
	static const uint32_t gvkmaxvertexbindings = 2;
	std::array<VkBuffer, gvkmaxvertexbindings> recordedvertexbuffers{};
	std::array<VkDeviceSize, gvkmaxvertexbindings> recordedvertexoffsets{};
	uint32_t recordedvertexcount = 0;
	VkBuffer recordedindexbuffer = VK_NULL_HANDLE;
	VkDeviceSize recordedindexoffset = 0;
	VkIndexType recordedindextype = VK_INDEX_TYPE_UINT32;
};

#endif /* GVK_VULKAN */

#endif /* CORE_GVKCONTEXT_H */
