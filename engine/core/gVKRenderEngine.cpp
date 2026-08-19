//
// Created by sadettin on 23.08.2025.
//

#include "gVKRenderEngine.h"

//screenShot Related includes
#include "stb/stb_image_write.h"
#include "gBaseApp.h"
#include "gGrid.h"
#include "gImage.h"
// The scene uniform block is filled from these.
#include "gLight.h"
#include "gShader.h"
#include "gTracy.h"

// Vulkan is only wired up on the desktop GLFW platforms and only when the Vulkan
// development files are available. gVKContext.h evaluates that condition, defines
// GVK_VULKAN accordingly and holds the shared state of the backend.
#include "gVKContext.h"

#ifdef GVK_VULKAN
	#include "gAppManager.h"
	#include "gBaseWindow.h"
	#include "gVKSwapchain.h"
	#include "gVKRenderTarget.h"
	#include "gVKCommands.h"
	#include "gVKBuffer.h"
	#include "gVKFrame.h"
	#include "gVKSync.h"
	#include "gVKPipeline.h"
	#include "gVKDraw.h"
	#include "gVKTexture.h"
	#include "gVKMeshBuffer.h"
	#include "gVKUniform.h"
	#include "gVKShadow.h"

	// Defined further down; declared here because drawMesh3D reaches it first. Kept
	// out of the class because it returns a Vulkan handle and gVKRenderEngine.h is
	// deliberately free of Vulkan headers.
	static VkDescriptorSet gvkGetPbrMaterialSet(gVKContext* vkcontext,
			std::unordered_map<GLuint, gVKTexture*>& vktextures, GLuint whitetextureid,
			const gRenderer::gMeshSurface& surface);
	static VkDescriptorSet gvkGetMaterialSet(gVKContext* vkcontext,
			std::unordered_map<GLuint, gVKTexture*>& vktextures, GLuint whitetextureid,
			GLuint diffusemapid, GLuint specularmapid, GLuint normalmapid);
	static GLuint gvkRegisteredTextureId(std::unordered_map<GLuint, gVKTexture*>& vktextures, GLuint id);
	static void gvkReleaseMaterialSetsUsingTexture(gVKContext* vkcontext, GLuint textureid);
	#include <algorithm>
	#include <vector>
	#include <set>
	#include <string>
	#include <cstring>
	// The PBR material cache keys on a std::array and lives in a std::map. Both were
	// reaching this file only through another header on one toolchain, which is the
	// kind of thing that builds on Windows and fails everywhere else.
	#include <array>
	#include <map>
	#include <cstdlib>
#endif

#ifdef GVK_VULKAN
// Unlike OpenGL, clearing is not an immediate operation here: the colour is kept
// and the render pass writes it when the next frame begins.
static void gvkStoreClearColor(gVKContext* ctx, float r, float g, float b, float a) {
	if(ctx == nullptr) return;
	// Through the accessor rather than the member: this helper is a free function,
	// not part of the context's friendship.
	VkClearValue* clearvalue = ctx->getClearValue();
	clearvalue->color.float32[0] = r;
	clearvalue->color.float32[1] = g;
	clearvalue->color.float32[2] = b;
	clearvalue->color.float32[3] = a;
}
#endif

gVKRenderEngine::~gVKRenderEngine() {
	cleanupVulkan();
	delete originalgrid;
	delete rendercolor;
	rendercolor = nullptr;
}

static void flipVertically(unsigned char* pixelData, int width, int height, int numChannels) {
	int rowsize = width * numChannels;
	unsigned char* temprow = new unsigned char[rowsize];

	for (int row = 0; row < height / 2; ++row) {
		int bottomrow = height - row - 1;

		memcpy(temprow, pixelData + row * rowsize, rowsize);
		memcpy(pixelData + row * rowsize, pixelData + bottomrow * rowsize, rowsize);
		memcpy(pixelData + bottomrow * rowsize, temprow, rowsize);
	}

	delete[] temprow;
}

void gVKRenderEngine::clear() {
	flushQueuedDraws();
	// The render pass clears the attachment at the start of every frame, so there
	// is nothing to do at the moment this is called.
}

void gVKRenderEngine::clearColor(int r, int g, int b, int a) {
	flushQueuedDraws();
#ifdef GVK_VULKAN
	gvkStoreClearColor(vkcontext, r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);
#endif
}

void gVKRenderEngine::clearColor(gColor color) {
	flushQueuedDraws();
#ifdef GVK_VULKAN
	gvkStoreClearColor(vkcontext, color.r, color.g, color.b, color.a);
#endif
}

void gVKRenderEngine::setProjectionMatrix(glm::mat4 projectionMatrix) {
	flushQueuedDraws();
	// gCamera builds its matrix with glm::perspective, which targets OpenGL: the
	// clip volume runs from -w to +w in depth. Vulkan's runs from 0 to +w, so a
	// matrix used unchanged would put the near half of the scene behind the viewer
	// and clip it away. This maps z' = (z + w) / 2, turning one range into the other.
	//
	// The Y axis is deliberately not touched. Vulkan's Y also points the opposite way
	// to OpenGL's, but the frame loop already handles that with a negative-height
	// viewport (see gVKFrame.cpp), and flipping it here as well would cancel that out
	// and turn the picture upside down again.
	//
	// Written column-major, the way glm indexes: clip[column][row].
	glm::mat4 clip(1.0f);
	clip[2][2] = 0.5f;
	clip[3][2] = 0.5f;
	gRenderer::setProjectionMatrix(clip * projectionMatrix);
}

// The render state setters below only remember what was asked for. There is no GL
// context under Vulkan, so issuing GL calls here would at best be ignored and at
// worst call through a null GLEW pointer; the state that the 2D path cares about is
// read back through the is...Enabled() accessors instead.
void gVKRenderEngine::enableDepthTest() {
	enableDepthTest(DEPTHTESTTYPE_LESS);
}

void gVKRenderEngine::enableDepthTest(int depthTestType) {
	flushQueuedDraws();
	isdepthtestenabled = true;
	depthtesttype = depthTestType;
}

void gVKRenderEngine::setDepthTestFunc(int depthTestType) {
	flushQueuedDraws();
	depthtesttype = depthTestType;
}

void gVKRenderEngine::disableDepthTest() {
	flushQueuedDraws();
	isdepthtestenabled = false;
}

bool gVKRenderEngine::isDepthTestEnabled() {
	return isdepthtestenabled;
}

int gVKRenderEngine::getDepthTestType() {
	return depthtesttype;
}

void gVKRenderEngine::enableCulling() {
	flushQueuedDraws();
	gRenderer::enableCulling();
}

void gVKRenderEngine::disableCulling() {
	flushQueuedDraws();
	gRenderer::disableCulling();
}

void gVKRenderEngine::setCullFace(int face) {
	flushQueuedDraws();
	gRenderer::setCullFace(face);
}

void gVKRenderEngine::setCullingDirection(int direction) {
	flushQueuedDraws();
	gRenderer::setCullingDirection(direction);
}

void gVKRenderEngine::enableAlphaBlending() {
	flushQueuedDraws();
	// Resets the mode, exactly as the OpenGL path does by reissuing glBlendFunc.
	blendmode = BLENDMODE_ALPHA;
	isalphablendingenabled = true;
}

void gVKRenderEngine::setBlendMode(int blendMode) {
	if(blendmode == blendMode) return;
	// The queued draws were recorded against the pipeline the old mode selects, and
	// the queue is flushed by binding that pipeline once for the batch. Changing the
	// mode has to end the batch, or draws made before the change would be added
	// rather than composited.
	flushQueuedDraws();
	blendmode = blendMode;
}

void gVKRenderEngine::disableAlphaBlending() {
	flushQueuedDraws();
	isalphablendingenabled = false;
}

bool gVKRenderEngine::isAlphaBlendingEnabled() {
	return isalphablendingenabled;
}

void gVKRenderEngine::enableAlphaTest() {
	isalphatestenabled = true;
}

void gVKRenderEngine::disableAlphaTest() {
	isalphatestenabled = false;
}

bool gVKRenderEngine::isAlphaTestEnabled() {
	return isalphatestenabled;
}

void gVKRenderEngine::takeScreenshot(gImage& img, int x, int y, int width, int height) {
	G_PROFILE_ZONE_SCOPED_N("gVKRenderEngine::takeScreenshot()");
	gImage full;
	takeScreenshot(full);
	if(full.getImageData() == nullptr || width <= 0 || height <= 0) return;
	const int sourcewidth = full.getWidth();
	const int sourceheight = full.getHeight();
	x = std::max(0, x); y = std::max(0, y);
	width = std::min(width, sourcewidth - x);
	height = std::min(height, sourceheight - y);
	if(width <= 0 || height <= 0) return;
	auto* pixeldata = new unsigned char[static_cast<size_t>(width) * height * 4];
	for(int row = 0; row < height; row++)
		std::memcpy(pixeldata + static_cast<size_t>(row) * width * 4,
				full.getImageData() + (static_cast<size_t>(y + row) * sourcewidth + x) * 4,
				static_cast<size_t>(width) * 4);
	img.setImageData(pixeldata, width, height, 4);
}

void gVKRenderEngine::takeScreenshot(gImage& img) {
	G_PROFILE_ZONE_SCOPED_N("gVKRenderEngine::takeScreenshot()");
#ifdef GVK_VULKAN
	if(vkcontext == nullptr || vkcontext->device == VK_NULL_HANDLE) return;
	if(vkcontext->screenshotready) {
		auto* pixeldata = new unsigned char[vkcontext->screenshotpixels.size()];
		std::memcpy(pixeldata, vkcontext->screenshotpixels.data(), vkcontext->screenshotpixels.size());
		if(vkcontext->screenshotformat == VK_FORMAT_B8G8R8A8_UNORM
				|| vkcontext->screenshotformat == VK_FORMAT_B8G8R8A8_SRGB) {
			for(size_t i = 0; i < static_cast<size_t>(vkcontext->screenshotwidth) * vkcontext->screenshotheight; i++)
				std::swap(pixeldata[i * 4], pixeldata[i * 4 + 2]);
		}
		img.setImageData(pixeldata, static_cast<int>(vkcontext->screenshotwidth),
				static_cast<int>(vkcontext->screenshotheight), 4);
		vkcontext->screenshotpixels.clear();
		vkcontext->screenshotready = false;
		return;
	}
	if((vkcontext->surfacecapabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT) == 0) {
		gLoge("gVKRenderEngine") << "The swapchain does not support screenshot transfer reads.";
		return;
	}
	vkcontext->screenshotrequested = true;
#endif
}


// The entry points below carry OpenGL's names and OpenGL's shape, because gVbo and
// gTexture are written against them, but none of them issues a GL call: a Vulkan
// window is created with GLFW_NO_API, so there is no current GL context and calling
// into GL would be undefined behaviour - on macOS/MoltenVK it hangs, the same way
// glViewport did during window init.
//
// The buffer and VAO calls are backed by the registries in gVKMeshBuffer.h: a name
// maps to a real VkBuffer and a vertex array remembers which names were bound to it.
// The vertex-attribute calls stay no-ops, because in Vulkan the attribute layout is
// baked into the pipeline rather than set per draw, and gVertex has one fixed layout
// that the 3D pipeline declares once.
GLuint gVKRenderEngine::genBuffers() {
#ifdef GVK_VULKAN
	if(vkcontext == nullptr) return 0;
	// The entry is created empty; the VkBuffer appears on the first upload, because
	// only then is the size and the vertex/index role known.
	GLuint id = nextvkbufferid++;
	vkmeshbuffers[id] = new gVKMeshBuffer();
	return id;
#else
	return 0;
#endif
}

gVKMeshBuffer* gVKRenderEngine::getMeshBuffer(GLuint id) {
	if(id == 0) return nullptr;
	auto it = vkmeshbuffers.find(id);
	return it == vkmeshbuffers.end() ? nullptr : it->second;
}

void gVKRenderEngine::deleteBuffer(GLuint& buffer) {
#ifdef GVK_VULKAN
	if(buffer == 0 || vkcontext == nullptr) return;

	auto it = vkmeshbuffers.find(buffer);
	if(it != vkmeshbuffers.end()) {
		// The GPU may still be reading this buffer for a frame in flight.
		if(vkcontext->getDevice() != nullptr && *vkcontext->getDevice() != VK_NULL_HANDLE) {
			vkDeviceWaitIdle(*vkcontext->getDevice());
		}
		gvkDestroyMeshBuffer(*vkcontext, *it->second);
		delete it->second;
		vkmeshbuffers.erase(it);
	}

	// Any vertex array still pointing at the name has to forget it, otherwise a later
	// draw would look up an id that no longer exists.
	for(auto& entry : vkvertexarrays) {
		if(entry.second.vertexbuffer == buffer) entry.second.vertexbuffer = 0;
		if(entry.second.indexbuffer == buffer) entry.second.indexbuffer = 0;
	}
	if(boundarraybufferid == buffer) boundarraybufferid = 0;
	if(boundelementbufferid == buffer) boundelementbufferid = 0;

	// Matches the OpenGL path, where glDeleteBuffers leaves the caller's name zeroed.
	buffer = 0;
#else
	buffer = 0;
#endif
}

void gVKRenderEngine::bindBuffer(GLenum target, GLuint buffer) {
	// Mirrors the OpenGL bind state so an upload knows which name it is filling and
	// the current vertex array can record the pairing.
	if(target == GL_ELEMENT_ARRAY_BUFFER) {
		boundelementbufferid = buffer;
		if(boundvaoid != 0) vkvertexarrays[boundvaoid].indexbuffer = buffer;
	} else {
		boundarraybufferid = buffer;
	}
}

void gVKRenderEngine::unbindBuffer(GLenum target) {
}

void gVKRenderEngine::bufSubData(GLuint buffer, int offset, int size, const void* data) {
}

void gVKRenderEngine::setBufferData(GLuint buffer, const void* data, size_t size, int usage) {
}

void gVKRenderEngine::setBufferRange(int index, GLuint buffer, int offset, int size) {
}

// ----- VAO -----
GLuint gVKRenderEngine::createVAO() {
	// Vulkan has no vertex array object. The id is still real bookkeeping here: it is
	// the key under which the backend remembers which vertex and index buffer were
	// bound while this array was current, which is what a draw needs. gVbo also
	// treats GL_NONE as "not created yet" and asserts on it, so the id must not be 0.
	GLuint id = nextvkvaoid++;
	vkvertexarrays[id] = gVKVertexArray();
	return id;
}

void gVKRenderEngine::deleteVAO(GLuint& vao) {
	if(vao == 0) return;
	// Only the pairing is dropped. The buffers themselves belong to gVbo, which
	// deletes them through deleteBuffer.
	vkvertexarrays.erase(vao);
	if(boundvaoid == vao) boundvaoid = 0;
	vao = 0;
}

void gVKRenderEngine::bindVAO(GLuint vao) {
	boundvaoid = vao;
}

void gVKRenderEngine::unbindVAO() {
	boundvaoid = 0;
}

void gVKRenderEngine::setVertexBufferData(GLuint vbo, size_t size, const void* data, int usage) {
#ifdef GVK_VULKAN
	if(vkcontext == nullptr) return;

	gVKMeshBuffer* buf = getMeshBuffer(vbo);
	if(buf == nullptr) {
		gLoge("gVKRenderEngine") << "setVertexBufferData called with unknown buffer id " << vbo;
		return;
	}

	// usage is ignored: it is OpenGL's hint about how often the data changes, and the
	// Vulkan side always places mesh data in device local memory. Data that really
	// does change every frame goes through gVKDraw's host visible ring instead.
	if(!gvkUploadMeshBuffer(*vkcontext, *buf, data, static_cast<VkDeviceSize>(size), false)) {
		return;
	}

	// gVbo binds its vertex array, then the buffer, then uploads - so this is where
	// the array learns which buffer holds its vertices. A vertex array only ever has
	// one vertex buffer; a second, different one is the per-instance data that
	// setInstanceData uploads through this same call.
	if(boundvaoid == 0) return;
	gVKVertexArray& array = vkvertexarrays[boundvaoid];
	if(array.vertexbuffer == 0 || array.vertexbuffer == vbo) array.vertexbuffer = vbo;
	else array.instancebuffer = vbo;
#endif
}

void gVKRenderEngine::setIndexBufferData(GLuint ebo, size_t size, const void* data, int usage) {
#ifdef GVK_VULKAN
	if(vkcontext == nullptr) return;

	gVKMeshBuffer* buf = getMeshBuffer(ebo);
	if(buf == nullptr) {
		gLoge("gVKRenderEngine") << "setIndexBufferData called with unknown buffer id " << ebo;
		return;
	}

	if(!gvkUploadMeshBuffer(*vkcontext, *buf, data, static_cast<VkDeviceSize>(size), true)) {
		return;
	}

	// gVbo does not bindBuffer(GL_ELEMENT_ARRAY_BUFFER, ...) before this call the way
	// it does for vertices - the OpenGL path binds inside setIndexBufferData itself -
	// so the pairing is recorded here rather than in bindBuffer.
	if(boundvaoid != 0) vkvertexarrays[boundvaoid].indexbuffer = ebo;
#endif
}

// ----- Draw -----
void gVKRenderEngine::drawArrays(int drawMode, int count) {
}

void gVKRenderEngine::drawElements(int drawMode, int count) {
}

void gVKRenderEngine::drawArraysInstanced(int drawMode, int count, int instanceCount) {
}

void gVKRenderEngine::drawElementsInstanced(int drawMode, int count, int instanceCount) {
}

// ----- vertex attributes -----
void gVKRenderEngine::enableVertexAttrib(int index) {
}

void gVKRenderEngine::disableVertexAttrib(int index) {
}

void gVKRenderEngine::setVertexAttribPointer(int index, int size, int type, bool normalized, int stride,
                                             const void* pointer) {
}

void gVKRenderEngine::setVertexAttribDivisor(int index, int divisor) {
	// Vulkan sets the step rate on the binding when the pipeline is built, not per
	// attribute at draw time, so there is nothing to forward. It is still worth
	// listening to: a non-zero divisor is the caller stating that the array buffer
	// currently bound holds per-instance data, which confirms what
	// setVertexBufferData inferred from the order of the uploads.
	if(divisor <= 0 || boundvaoid == 0 || boundarraybufferid == 0) return;
	gVKVertexArray& array = vkvertexarrays[boundvaoid];
	array.instancebuffer = boundarraybufferid;
	// If this buffer was mistaken for the vertex data, it was the first one this
	// array saw and there is no vertex buffer after all.
	if(array.vertexbuffer == boundarraybufferid) array.vertexbuffer = 0;
}

void gVKRenderEngine::setViewport(int x, int y, int width, int height) {
	// The frame's viewport is dynamic state set when the render pass opens; this
	// only records what was asked for.
	viewportx = x;
	viewporty = y;
	viewportwidth = width;
	viewportheight = height;
}

#ifdef GVK_VULKAN
// One offscreen render target. The attachments are gTextures the application owns,
// so only the pass and the framebuffer wrapping them belong here. Both are built on
// the first bind rather than at creation, because gFbo attaches its textures after
// asking for the framebuffer.
struct gVKFramebuffer {
	gVKTexture* color = nullptr;
	gVKTexture* depth = nullptr;
	VkRenderPass renderpass = VK_NULL_HANDLE;
	VkFramebuffer framebuffer = VK_NULL_HANDLE;
	uint32_t width = 0;
	uint32_t height = 0;
	// True while its render pass is the one being recorded into.
	bool active = false;
};

// Builds the pass and the framebuffer for a target, once. Returns false when the
// target has nothing usable attached.
bool gVKRenderEngine::ensureFramebufferPass(gVKFramebuffer* target) {
	if(target == nullptr || vkcontext == nullptr || vkcontext->device == VK_NULL_HANDLE) return false;
	if(target->renderpass != VK_NULL_HANDLE && target->framebuffer != VK_NULL_HANDLE) return true;
	if(target->width == 0 || target->height == 0) return false;

	VkAttachmentDescription attachments[2]{};
	VkAttachmentReference colorref{};
	VkAttachmentReference depthref{};
	VkImageView views[2]{};
	uint32_t count = 0;

	if(target->color != nullptr) {
		colorref.attachment = count;
		colorref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		VkAttachmentDescription& a = attachments[count];
		a.format = target->color->format;
		a.samples = VK_SAMPLE_COUNT_1_BIT;
		a.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		a.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		a.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		a.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		a.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		// Left ready to sample, so nothing else has to transition it afterwards -
		// the same trick the shadow map's pass uses.
		a.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		views[count] = target->color->view;
		count++;
	}
	if(target->depth != nullptr) {
		depthref.attachment = count;
		depthref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		VkAttachmentDescription& a = attachments[count];
		a.format = target->depth->format;
		a.samples = VK_SAMPLE_COUNT_1_BIT;
		a.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		a.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		a.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		a.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		a.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		a.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		views[count] = target->depth->view;
		count++;
	}
	if(count == 0) return false;

	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = target->color != nullptr ? 1 : 0;
	subpass.pColorAttachments = target->color != nullptr ? &colorref : nullptr;
	subpass.pDepthStencilAttachment = target->depth != nullptr ? &depthref : nullptr;

	// One dependency, matching the screen pass exactly. Render pass compatibility is
	// what lets the pipelines built for the screen be recorded into an FBO, and the
	// validation layer compares this too - two dependencies here made every draw
	// into an FBO an incompatibility error.
	VkSubpassDependency dependency{};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
			| VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
			| VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
			| VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

	VkRenderPassCreateInfo passinfo{};
	passinfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	passinfo.attachmentCount = count;
	passinfo.pAttachments = attachments;
	passinfo.subpassCount = 1;
	passinfo.pSubpasses = &subpass;
	passinfo.dependencyCount = 1;
	passinfo.pDependencies = &dependency;
	if(vkCreateRenderPass(vkcontext->device, &passinfo, nullptr, &target->renderpass) != VK_SUCCESS) {
		gLoge("gVKRenderEngine") << "vkCreateRenderPass failed for a framebuffer.";
		target->renderpass = VK_NULL_HANDLE;
		return false;
	}

	VkFramebufferCreateInfo fbinfo{};
	fbinfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	fbinfo.renderPass = target->renderpass;
	fbinfo.attachmentCount = count;
	fbinfo.pAttachments = views;
	fbinfo.width = target->width;
	fbinfo.height = target->height;
	fbinfo.layers = 1;
	if(vkCreateFramebuffer(vkcontext->device, &fbinfo, nullptr, &target->framebuffer) != VK_SUCCESS) {
		gLoge("gVKRenderEngine") << "vkCreateFramebuffer failed for a framebuffer.";
		vkDestroyRenderPass(vkcontext->device, target->renderpass, nullptr);
		target->renderpass = VK_NULL_HANDLE;
		target->framebuffer = VK_NULL_HANDLE;
		return false;
	}
	return true;
}

void gVKRenderEngine::endOffscreenPass() {
	if(vkcontext == nullptr || !vkcontext->frameactive) return;
	for(auto& entry : vkframebuffers) {
		if(entry.second == nullptr || !entry.second->active) continue;
		// Anything the canvas batched into this target has to be recorded before the
		// pass it belongs to closes.
		gvkFlush2DBatch(*vkcontext);
		vkCmdEndRenderPass(vkcontext->commandbuffers[vkcontext->currentframe]);
		entry.second->active = false;
		vkcontext->renderpassactive = false;
		// The pass's finalLayout already left both attachments sampleable.
		return;
	}
}

void gVKRenderEngine::destroyAllFramebuffers() {
	for(auto& entry : vkframebuffers) {
		if(entry.second == nullptr) continue;
		if(vkcontext != nullptr && vkcontext->device != VK_NULL_HANDLE) {
			if(entry.second->framebuffer != VK_NULL_HANDLE) {
				vkDestroyFramebuffer(vkcontext->device, entry.second->framebuffer, nullptr);
			}
			if(entry.second->renderpass != VK_NULL_HANDLE) {
				vkDestroyRenderPass(vkcontext->device, entry.second->renderpass, nullptr);
			}
		}
		delete entry.second;
	}
	vkframebuffers.clear();
	boundframebuffer = gFbo::defaultfbo;
}
#endif

// ----- Framebuffer -----
GLuint gVKRenderEngine::createFramebuffer() {
#ifdef GVK_VULKAN
	const GLuint id = nextvkframebufferid++;
	vkframebuffers[id] = new gVKFramebuffer();
	return id;
#else
	return 0;
#endif
}

void gVKRenderEngine::deleteFramebuffer(GLuint& fbo) {
#ifdef GVK_VULKAN
	if(fbo == 0) return;
	if(boundframebuffer == fbo) bindFramebuffer(gFbo::defaultfbo);
	auto it = vkframebuffers.find(fbo);
	if(it == vkframebuffers.end()) return;
	// The attachments themselves belong to the gTextures that own them; only the
	// pass and the framebuffer built around them are freed here.
	if(vkcontext != nullptr && vkcontext->device != VK_NULL_HANDLE) {
		vkDeviceWaitIdle(vkcontext->device);
		if(it->second->framebuffer != VK_NULL_HANDLE) {
			vkDestroyFramebuffer(vkcontext->device, it->second->framebuffer, nullptr);
		}
		if(it->second->renderpass != VK_NULL_HANDLE) {
			vkDestroyRenderPass(vkcontext->device, it->second->renderpass, nullptr);
		}
	}
	delete it->second;
	vkframebuffers.erase(it);
	fbo = 0;
#endif
}

void gVKRenderEngine::bindFramebuffer(GLuint fbo) {
#ifdef GVK_VULKAN
	flushQueuedDraws();
	if(vkcontext == nullptr) { boundframebuffer = fbo; return; }

	// Whatever pass is open has to close before another can open. Outside a frame
	// there is nothing to close and nothing to begin, so the bind is only recorded
	// and acted on when the frame that uses it starts drawing.
	endOffscreenPass();
	boundframebuffer = fbo;
	if(!vkcontext->frameactive || fbo == gFbo::defaultfbo) return;

	auto it = vkframebuffers.find(fbo);
	if(it == vkframebuffers.end() || it->second == nullptr) return;
	gVKFramebuffer* target = it->second;
	if(target->color == nullptr && target->depth == nullptr) return;
	if(!ensureFramebufferPass(target)) return;

	// The screen pass may already be open - a canvas can bind an FBO half way
	// through drawing - and two passes cannot be recorded at once.
	if(vkcontext->renderpassactive) {
		gvkFlush2DBatch(*vkcontext);
		vkCmdEndRenderPass(vkcontext->commandbuffers[vkcontext->currentframe]);
		vkcontext->renderpassactive = false;
	}

	VkCommandBuffer cmd = vkcontext->commandbuffers[vkcontext->currentframe];
	VkClearValue clears[2];
	uint32_t clearcount = 0;
	if(target->color != nullptr) clears[clearcount++] = vkcontext->clearvalue;
	if(target->depth != nullptr) clears[clearcount++].depthStencil = {1.0f, 0};

	VkRenderPassBeginInfo info{};
	info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	info.renderPass = target->renderpass;
	info.framebuffer = target->framebuffer;
	info.renderArea.offset = {0, 0};
	info.renderArea.extent = {target->width, target->height};
	info.clearValueCount = clearcount;
	info.pClearValues = clears;
	vkCmdBeginRenderPass(cmd, &info, VK_SUBPASS_CONTENTS_INLINE);
	vkcontext->resetRecordedDrawState();

	// Negative height as on the screen pass: an FBO is drawn into with the same
	// top-left origin projections the engine builds everywhere else.
	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = static_cast<float>(target->height);
	viewport.width = static_cast<float>(target->width);
	viewport.height = -static_cast<float>(target->height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(cmd, 0, 1, &viewport);
	VkRect2D scissor{};
	scissor.offset = {0, 0};
	scissor.extent = {target->width, target->height};
	vkCmdSetScissor(cmd, 0, 1, &scissor);

	vkcontext->renderpassactive = true;
	// Offscreen targets are never multisampled - their attachments are textures the
	// application samples afterwards - so from here until this pass closes the draws
	// have to use the single-sample pipeline build. See the design note in
	// gVKContext.h for why the two builds exist at all.
	vkcontext->useOffscreenPipelines();
	target->active = true;
	if(target->color != nullptr) target->color->layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	if(target->depth != nullptr) target->depth->layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
#endif
}

void gVKRenderEngine::checkFramebufferStatus() {
#ifdef GVK_VULKAN
	auto it = vkframebuffers.find(boundframebuffer);
	if(it == vkframebuffers.end() || it->second == nullptr
			|| (it->second->color == nullptr && it->second->depth == nullptr)) {
		gLogi("gFbo") << "Framebuffer is not complete! No attachment was bound to it.";
	}
#endif
}

// ----- Renderbuffer -----
// Vulkan has no renderbuffer object. gFbo is steered onto a sampleable depth
// texture instead (see gFbo::allocate), so these only have to hand out ids that
// are unique and non-zero, and do nothing with them.
GLuint gVKRenderEngine::createRenderbuffer() {
	return nextvkrenderbufferid++;
}

void gVKRenderEngine::deleteRenderbuffer(GLuint& rbo) {
	rbo = 0;
}

void gVKRenderEngine::bindRenderbuffer(GLuint rbo) {
}

void gVKRenderEngine::setRenderbufferStorage(GLenum format, int width, int height) {
}

// ----- Attachments -----
void gVKRenderEngine::attachTextureToFramebuffer(GLenum attachment, GLenum textarget, GLuint texId, GLuint level) {
#ifdef GVK_VULKAN
	auto fb = vkframebuffers.find(boundframebuffer);
	auto tex = vktextures.find(texId);
	if(fb == vkframebuffers.end() || fb->second == nullptr
			|| tex == vktextures.end() || tex->second == nullptr) return;

	if(attachment == GL_DEPTH_ATTACHMENT || attachment == GL_DEPTH_STENCIL_ATTACHMENT) {
		fb->second->depth = tex->second;
	} else {
		fb->second->color = tex->second;
	}
	fb->second->width = static_cast<uint32_t>(tex->second->width);
	fb->second->height = static_cast<uint32_t>(tex->second->height);
	// The pass is built lazily on first bind, so an attachment added after one
	// already exists invalidates it rather than being ignored.
	if(fb->second->renderpass != VK_NULL_HANDLE && vkcontext != nullptr
			&& vkcontext->device != VK_NULL_HANDLE) {
		vkDeviceWaitIdle(vkcontext->device);
		vkDestroyFramebuffer(vkcontext->device, fb->second->framebuffer, nullptr);
		vkDestroyRenderPass(vkcontext->device, fb->second->renderpass, nullptr);
		fb->second->framebuffer = VK_NULL_HANDLE;
		fb->second->renderpass = VK_NULL_HANDLE;
	}
#endif
}

void gVKRenderEngine::attachRenderbufferToFramebuffer(GLenum attachment, GLuint rbo) {
	// Nothing to attach; see createRenderbuffer.
}

// ----- Draw/Read buffers -----
void gVKRenderEngine::setDrawBufferNone() {
#if defined(GLIST_OPENGLES)
	G_CHECK_GL(glDrawBuffers(0, GL_NONE));
#else
	G_CHECK_GL(glDrawBuffer(GL_NONE));
#endif
}

void gVKRenderEngine::setReadBufferNone() {
	G_CHECK_GL(glReadBuffer(GL_NONE));
}

// ----- Fullscreen Quad -----
void gVKRenderEngine::createFullscreenQuad(GLuint& vao, GLuint& vbo) {
	static const float QUAD_VERTICES[] = {
		// positions   // texCoords
		-1.0f, 1.0f, 0.0f, 1.0f,
		-1.0f, -1.0f, 0.0f, 0.0f,
		1.0f, -1.0f, 1.0f, 0.0f,

		-1.0f, 1.0f, 0.0f, 1.0f,
		1.0f, -1.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 1.0f, 1.0f
	};

	G_CHECK_GL(glGenVertexArrays(1, &vao));
	G_CHECK_GL(glGenBuffers(1, &vbo));

	G_CHECK_GL(glBindVertexArray(vao));
	G_CHECK_GL(glBindBuffer(GL_ARRAY_BUFFER, vbo));
	G_CHECK_GL(glBufferData(GL_ARRAY_BUFFER, sizeof(QUAD_VERTICES), &QUAD_VERTICES, GL_STATIC_DRAW));

	G_CHECK_GL(glEnableVertexAttribArray(0));
	G_CHECK_GL(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0));

	G_CHECK_GL(glEnableVertexAttribArray(1));
	G_CHECK_GL(glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float))));
}

void gVKRenderEngine::deleteFullscreenQuad(GLuint& vao, GLuint* vbo) {
	if(vao != GL_NONE) {
		G_CHECK_GL(glDeleteVertexArrays(1, &vao));
	}
	if(vbo != GL_NONE) {
		G_CHECK_GL(glDeleteBuffers(1, vbo));
	}
}

// The gShader family below is inherited from gRenderer and every one of these
// used to reach straight for OpenGL - glCreateShader, glUseProgram, glUniform* -
// inside a backend that has no GL context at all. Nothing crashed only because the
// Vulkan draw paths never call them: gMesh::draw() returns before drawExtraShaders,
// and the engine's own shaders are SPIR-V compiled ahead of time. An application
// that builds a gShader of its own walked straight into it.
//
// They refuse instead. A user supplied GLSL program is not something this backend
// can honour: Vulkan wants SPIR-V, and the GLSL such a program is written in uses
// OpenGL's model - bare "uniform mat4 model", samplers with no set or binding -
// which is not valid Vulkan GLSL and could not simply be recompiled. Saying so once
// is worth more than a silent no-op or an undefined call.
static void gvkReportNoUserShaders(const char* what) {
	static bool reported = false;
	if(reported) return;
	reported = true;
	gLogw("gVKRenderEngine") << what << " is not supported on the Vulkan backend. "
			<< "Shader programs built through gShader are OpenGL programs; this backend "
			<< "draws with SPIR-V pipelines built from the engine's own shaders. A "
			<< "material's extra shaders are skipped, and gMesh draws once with the "
			<< "standard pipeline.";
}

GLuint gVKRenderEngine::loadProgram(const char* vertexSource, const char* fragmentSource, const char* geometrySource) {
	gvkReportNoUserShaders("gShader::load");
	(void)vertexSource; (void)fragmentSource; (void)geometrySource;
	return 0;
	unsigned int vertex = GL_NONE;
	unsigned int fragment = GL_NONE;
#if defined(WIN32) || defined(LINUX)
	unsigned int geometry = GL_NONE;
#endif
	// vertex shader
	G_CHECK_GL2(vertex, glCreateShader(GL_VERTEX_SHADER));
	G_CHECK_GL(glShaderSource(vertex, 1, &vertexSource, nullptr));
	G_CHECK_GL(glCompileShader(vertex));
	checkCompileErrors(vertex, "VERTEX");

	// fragment Shader
	G_CHECK_GL2(fragment, glCreateShader(GL_FRAGMENT_SHADER));
	G_CHECK_GL(glShaderSource(fragment, 1, &fragmentSource, nullptr));
	G_CHECK_GL(glCompileShader(fragment));
	checkCompileErrors(fragment, "FRAGMENT");

	GLuint id;
	// shader Program
	G_CHECK_GL2(id, glCreateProgram());
	G_CHECK_GL(glAttachShader(id, vertex));
	G_CHECK_GL(glAttachShader(id, fragment));

#if defined(WIN32) || defined(LINUX)
	if (geometrySource != nullptr) {
		G_CHECK_GL2(geometry, glCreateShader(GL_GEOMETRY_SHADER));
		if (geometry != 0) {
			G_CHECK_GL(glShaderSource(geometry, 1, &geometrySource, nullptr));
			G_CHECK_GL(glCompileShader(geometry));
			checkCompileErrors(geometry, "GEOMETRY");
			G_CHECK_GL(glAttachShader(id, geometry));
		} else {
			gLoge("Geometry Shader") << "Failed to create geometry shader: not supported by this context";
		}
	}
#endif
	G_CHECK_GL(glLinkProgram(id));
	checkCompileErrors(id, "PROGRAM");
	// delete the shaders as they're linked into our program now and no longer necessery
	G_CHECK_GL(glDeleteShader(vertex));
	G_CHECK_GL(glDeleteShader(fragment));
#if defined(WIN32) || defined(LINUX)
	if(geometrySource != nullptr)
		G_CHECK_GL(glDeleteShader(geometry));
#endif
	return id;
}

void gVKRenderEngine::checkCompileErrors(GLuint shader, const std::string& type) {
	// Nothing is ever compiled here, so there is nothing to check.
	(void)shader; (void)type;
	return;
	GLint success;
	GLchar infoLog[1024];
	if(type != "PROGRAM") {
		G_CHECK_GL(glGetShaderiv(shader, GL_COMPILE_STATUS, &success));
		if(!success) {
			G_CHECK_GL(glGetShaderInfoLog(shader, 1024, nullptr, infoLog));
			gLoge("gShader") << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog <<
					"\n -- --------------------------------------------------- -- " << std::endl;
		}
	} else {
		G_CHECK_GL(glGetProgramiv(shader, GL_LINK_STATUS, &success));
		if(!success) {
			G_CHECK_GL(glGetProgramInfoLog(shader, 1024, nullptr, infoLog));
			gLoge("gShader") << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog <<
					"\n -- --------------------------------------------------- -- " << std::endl;
		}
	}
#ifdef DEBUG
	assert(success);
#endif
}

void gVKRenderEngine::setBool(GLuint uniformloc, bool value) {
	return;  // no GL context here; see gvkReportNoUserShaders
	G_CHECK_GL(glUniform1i(uniformloc, (int)value));
}

void gVKRenderEngine::setInt(GLuint uniformloc, int value) {
	return;  // no GL context here; see gvkReportNoUserShaders
	G_CHECK_GL(glUniform1i(uniformloc, value));
}

void gVKRenderEngine::setUnsignedInt(GLuint uniformloc, unsigned int value) {
	return;  // no GL context here; see gvkReportNoUserShaders
	G_CHECK_GL(glUniform1ui(uniformloc, value));
}

void gVKRenderEngine::setFloat(GLuint uniformloc, float value) {
	return;  // no GL context here; see gvkReportNoUserShaders
	G_CHECK_GL(glUniform1f(uniformloc, value));
}

void gVKRenderEngine::setVec2(GLuint uniformloc, const glm::vec2& value) {
	return;  // no GL context here; see gvkReportNoUserShaders
	G_CHECK_GL(glUniform2fv(uniformloc, 1, &value[0]));
}

void gVKRenderEngine::setVec2(GLuint uniformloc, float x, float y) {
	return;  // no GL context here; see gvkReportNoUserShaders
	G_CHECK_GL(glUniform2f(uniformloc, x, y));
}

void gVKRenderEngine::setVec3(GLuint uniformloc, const glm::vec3& value) {
	return;  // no GL context here; see gvkReportNoUserShaders
	G_CHECK_GL(glUniform3fv(uniformloc, 1, &value[0]));
}

void gVKRenderEngine::setVec3(GLuint uniformloc, float x, float y, float z) {
	return;  // no GL context here; see gvkReportNoUserShaders
	G_CHECK_GL(glUniform3f(uniformloc, x, y, z));
}

void gVKRenderEngine::setVec4(GLuint uniformloc, const glm::vec4& value) {
	return;  // no GL context here; see gvkReportNoUserShaders
	G_CHECK_GL(glUniform4fv(uniformloc, 1, &value[0]));
}

void gVKRenderEngine::setVec4(GLuint uniformloc, float x, float y, float z, float w) {
	return;  // no GL context here; see gvkReportNoUserShaders
	G_CHECK_GL(glUniform4f(uniformloc, x, y, z, w));
}

void gVKRenderEngine::setMat2(GLuint uniformloc, const glm::mat2& mat) {
	return;  // no GL context here; see gvkReportNoUserShaders
	G_CHECK_GL(glUniformMatrix2fv(uniformloc, 1, GL_FALSE, &mat[0][0]));
}

void gVKRenderEngine::setMat3(GLuint uniformloc, const glm::mat3& mat) {
	return;  // no GL context here; see gvkReportNoUserShaders
	G_CHECK_GL(glUniformMatrix3fv(uniformloc, 1, GL_FALSE, &mat[0][0]));
}

void gVKRenderEngine::setMat4(GLuint uniformloc, const glm::mat4& mat) {
	return;  // no GL context here; see gvkReportNoUserShaders
	G_CHECK_GL(glUniformMatrix4fv(uniformloc, 1, GL_FALSE, &mat[0][0]));
}

GLuint gVKRenderEngine::getUniformLocation(GLuint id, const std::string& name) {
	// The same value glGetUniformLocation returns for a name that is not there, so
	// a caller testing the result behaves as it would on an OpenGL program without
	// that uniform.
	gvkReportNoUserShaders("gShader uniform lookup");
	(void)id; (void)name;
	return static_cast<GLuint>(-1);
	G_CHECK_GL2(GLuint location, glGetUniformLocation(id, name.c_str()));
	return location;
}

void gVKRenderEngine::useShader(GLuint id) const {
	// A gShader id means nothing here; the pipeline a draw uses is chosen by the
	// draw path itself.
	(void)id;
	return;
	if (currentprogram == id) return;
	currentprogram = id;
	G_CHECK_GL(glUseProgram(id));
}

void gVKRenderEngine::resetShader(GLuint id, bool loaded) const {
	(void)id; (void)loaded;
	return;
	if(loaded) {
		if (currentprogram == id) currentprogram = 0;
		G_CHECK_GL(glDeleteShader(id));
	}
}

void gVKRenderEngine::clearScreen(bool color, bool depth) {
	#ifdef GVK_VULKAN
	flushQueuedDraws();
	// Screen, shadow and offscreen passes all clear through their attachment
	// loadOp when they begin. Mid-pass clears from the legacy OpenGL traversal must
	// not be replayed here: several scene helpers issue them while sharing a Vulkan
	// pass, which would erase previously recorded draws and produce flicker.
	(void) color;
	(void) depth;
	#else
	(void) color;
	(void) depth;
	#endif
}

void gVKRenderEngine::bindQuadVAO() {
	G_CHECK_GL(glBindVertexArray(fullscreenquadvao));
}

void gVKRenderEngine::drawFullscreenQuad() {
	flushQueuedDraws();
	G_CHECK_GL(glDrawArrays(GL_TRIANGLES, 0, 6));
}

void gVKRenderEngine::bindDefaultFramebuffer() {
	// gFbo::unbind() comes through here rather than bindFramebuffer, so this is what
	// closes an offscreen pass and lets the screen pass be reopened by the next draw.
	// It used to call glBindFramebuffer, which raised an error on every unbind.
	bindFramebuffer(gFbo::defaultfbo);
}

void gVKRenderEngine::drawVbo(const gVbo& vbo) {
    gMeshSurface surface;

    surface.ambient = glm::vec4(1.0f);
    surface.diffuse = glm::vec4(1.0f);
    surface.specular = glm::vec4(1.0f);
    surface.shininess = 0.5f;

    drawVbo(vbo, glm::mat4(1.0f),surface);
}

void gVKRenderEngine::drawVbo(const gVbo& vbo, const glm::mat4& model, const gMeshSurface& surface) {
    drawMesh3D(vbo.getVAOid(), vbo.getVerticesNum(), vbo.getIndicesNum(), model, surface, GL_TRIANGLES, 1);
}

// These emulate just enough of the OpenGL texture object / bind state that
// gTexture and gImage rely on. createTextures() mints an id; bindTexture() records
// which id later texImage2D() uploads into; texImage2D() turns the pixels into a
// real Vulkan texture (gVKTexture) kept in the registry. The wrap/filter/swizzle
// setters have no OpenGL work to do here - the sampler is created with sensible
// defaults in gvkCreateTextureRGBA8 - so they are no-ops.
GLuint gVKRenderEngine::createTextures() {
#ifdef GVK_VULKAN
	GLuint id = nextvktextureid++;
	boundtextureid = id;
	return id;
#else
	return 0;
#endif
}

void gVKRenderEngine::bindTexture(GLuint texId) {
	boundtextureid = texId;
}

void gVKRenderEngine::bindTexture(GLuint texId, int textureSlotNo) {
	boundtextureid = texId;
}

void gVKRenderEngine::unbindTexture() {
	boundtextureid = 0;
}

void gVKRenderEngine::activateTexture(int textureSlotNo) {
}

void gVKRenderEngine::resetTexture() {
}

void gVKRenderEngine::deleteTexture(GLuint& texId) {
#ifdef GVK_VULKAN
	if(texId != 0 && vkcontext != nullptr) {
		auto it = vktextures.find(texId);
		if(it != vktextures.end()) {
			// A runtime delete may target a texture a previous frame still samples,
			// so drain the device before tearing it down.
			if(*vkcontext->getDevice() != VK_NULL_HANDLE) vkDeviceWaitIdle(*vkcontext->getDevice());
			gvkReleaseMaterialSetsUsingTexture(vkcontext, texId);
			gvkDestroyTexture(*vkcontext, it->second);
			vktextures.erase(it);
		}
	}
#endif
	texId = 0;
}

void gVKRenderEngine::texImage2D(GLenum target, GLint internalFormat, int width, int height, GLint format, GLint type, void* data, GLint level) {
#ifdef GVK_VULKAN
	if(vkcontext == nullptr || boundtextureid == 0) return;
	if(width <= 0 || height <= 0) return;

	// An allocation-only call is an FBO attachment: gFbo makes its colour and depth
	// textures by asking for storage with no pixels in it. Those become renderable
	// images rather than sampled-only ones, and are registered under the same id so
	// the rest of the engine can bind them as ordinary textures afterwards.
	if(data == nullptr) {
		// Only the two constants the engine itself uses for a depth target, which is
		// what gFbo and gTexture ask for. The sized 16 and 32F spellings were also
		// tested for, but nothing here produces them and they are not guaranteed to
		// exist in every platform's GL headers.
		const bool depth = internalFormat == GL_DEPTH_COMPONENT || format == GL_DEPTH_COMPONENT
				|| internalFormat == GL_DEPTH_COMPONENT24;
		auto existing = vktextures.find(boundtextureid);
		if(existing != vktextures.end()) {
			vkDeviceWaitIdle(vkcontext->device);
			gvkReleaseMaterialSetsUsingTexture(vkcontext, boundtextureid);
			gvkDestroyTexture(*vkcontext, existing->second);
			vktextures.erase(existing);
		}
		gVKTexture* target = gvkCreateAttachmentTexture(*vkcontext, width, height, depth);
		if(target != nullptr) vktextures[boundtextureid] = target;
		return;
	}

	// Only 8-bit colour uploads become a sampled texture; float / HDR ones are
	// outside what the image path handles.
	if(type != GL_UNSIGNED_BYTE) return;

	int components = 4;
	if(format == GL_RED) components = 1;
	else if(format == GL_RG) components = 2;
	else if(format == GL_RGB) components = 3;
	else if(format == GL_RGBA) components = 4;

	// The Vulkan image is always R8G8B8A8_UNORM, so narrower formats are expanded.
	const unsigned char* src = static_cast<const unsigned char*>(data);
	const size_t pixels = static_cast<size_t>(width) * static_cast<size_t>(height);
	std::vector<unsigned char> rgba(pixels * 4);
	// Whether this image can trip mesh3d.frag's cutout test, which discards at an
	// alpha of 0.5. Found here because the loop below already touches every texel;
	// a separate pass would double the cost of an upload to learn the same thing.
	// The threshold is the shader's, expressed in bytes.
	bool hascutout = false;
	for(size_t i = 0; i < pixels; i++) {
		const unsigned char* p = src + i * components;
		unsigned char r = 0, g = 0, b = 0, a = 255;
		// The expansion mirrors what OpenGL samples, so an image looks the same on
		// both backends: a one channel texture reads as (r, 0, 0, 1) because the GL
		// path applies no swizzle to it, while a two channel one is swizzled to
		// (r, r, r, g) there and is expanded the same way here.
		if(components == 1) { r = p[0]; g = 0; b = 0; }
		else if(components == 2) { r = g = b = p[0]; a = p[1]; }
		else if(components == 3) { r = p[0]; g = p[1]; b = p[2]; }
		else { r = p[0]; g = p[1]; b = p[2]; a = p[3]; }
		rgba[i * 4 + 0] = r;
		rgba[i * 4 + 1] = g;
		rgba[i * 4 + 2] = b;
		rgba[i * 4 + 3] = a;
		if(a < 128) hascutout = true;
	}

	// The load path uploads twice (once around generateMipMap), so replace any
	// texture already registered for this id. The first upload is never sampled in
	// a submitted frame, so tearing it down here needs no device wait.
	auto it = vktextures.find(boundtextureid);
	if(it != vktextures.end()) {
		// Runtime glyph-atlas growth can replace a texture that an in-flight frame
		// still samples. Drain the device before destroying that image and its
		// descriptor resources. Atlas uploads are cache misses, not per-frame work.
		if(it->second->sampled && *vkcontext->getDevice() != VK_NULL_HANDLE) {
			vkDeviceWaitIdle(*vkcontext->getDevice());
		}
		gvkReleaseMaterialSetsUsingTexture(vkcontext, boundtextureid);
		gvkDestroyTexture(*vkcontext, it->second);
		vktextures.erase(it);
	}
	gVKTexture* tex = gvkCreateTextureRGBA8(*vkcontext, rgba.data(), width, height);
	if(tex != nullptr) {
		tex->hascutout = hascutout;
		vktextures[boundtextureid] = tex;
	}
#endif
}

void gVKRenderEngine::setTextureMaxLevel(GLenum target, int maxLevel) {
    /* no-op */
}

#ifdef GVK_VULKAN
// gTexture speaks in GL enums. Nearest is the only distinction the 2D path needs -
// there is a single mip level, so the mipmap variants collapse onto their base
// filter - and the clamping wrap modes all map onto clamp to edge, which is what
// the GL path resolves them to as well.
// OpenGL folds two decisions into the minification filter: how a texel is sampled
// and whether the mip chain is used at all. Vulkan keeps them apart, so both are
// read out of the same constant here.
static bool gvkMipmapsFromGL(GLint filter) {
	return filter == GL_NEAREST_MIPMAP_NEAREST || filter == GL_NEAREST_MIPMAP_LINEAR
			|| filter == GL_LINEAR_MIPMAP_NEAREST || filter == GL_LINEAR_MIPMAP_LINEAR;
}

static VkSamplerMipmapMode gvkMipmapModeFromGL(GLint filter) {
	return (filter == GL_NEAREST_MIPMAP_NEAREST || filter == GL_LINEAR_MIPMAP_NEAREST)
			? VK_SAMPLER_MIPMAP_MODE_NEAREST : VK_SAMPLER_MIPMAP_MODE_LINEAR;
}

static VkFilter gvkFilterFromGL(GLint filter) {
	return (filter == GL_NEAREST || filter == GL_NEAREST_MIPMAP_NEAREST || filter == GL_NEAREST_MIPMAP_LINEAR)
			? VK_FILTER_NEAREST : VK_FILTER_LINEAR;
}

static VkSamplerAddressMode gvkAddressFromGL(GLint wrap) {
	if(wrap == GL_MIRRORED_REPEAT) return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
	if(wrap == GL_REPEAT) return VK_SAMPLER_ADDRESS_MODE_REPEAT;
	return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
}
#endif

// The bound texture's sampler is rebuilt from whichever half of the state the call
// carries; the other half is whatever the texture already has. A call that arrives
// before the pixels do is ignored, because the upload path sets both again right
// afterwards.
void gVKRenderEngine::setWrapping(GLenum target, GLint wrapS, GLint wrapT) {
#ifdef GVK_VULKAN
	gVKTexture* tex = getBoundVKTexture();
	if(tex == nullptr) return;
	gvkSetTextureSampler(*vkcontext, tex, tex->minfilter, tex->magfilter,
			gvkAddressFromGL(wrapS), gvkAddressFromGL(wrapT));
#endif
}

void gVKRenderEngine::setWrapping(GLenum target, GLint wrapS, GLint wrapT, GLint wrapR) {
	setWrapping(target, wrapS, wrapT);
}

void gVKRenderEngine::setFiltering(GLenum target, GLint minFilter, GLint magFilter) {
#ifdef GVK_VULKAN
	gVKTexture* tex = getBoundVKTexture();
	if(tex == nullptr) return;
	gvkSetTextureSampler(*vkcontext, tex, gvkFilterFromGL(minFilter), gvkFilterFromGL(magFilter),
			tex->addressu, tex->addressv, gvkMipmapsFromGL(minFilter), gvkMipmapModeFromGL(minFilter));
#endif
}

void gVKRenderEngine::setWrappingAndFiltering(GLenum target, GLint wrapS, GLint wrapT, GLint minFilter,
                                              GLint magFilter) {
#ifdef GVK_VULKAN
	gVKTexture* tex = getBoundVKTexture();
	if(tex == nullptr) return;
	gvkSetTextureSampler(*vkcontext, tex, gvkFilterFromGL(minFilter), gvkFilterFromGL(magFilter),
			gvkAddressFromGL(wrapS), gvkAddressFromGL(wrapT),
			gvkMipmapsFromGL(minFilter), gvkMipmapModeFromGL(minFilter));
#endif
}

void gVKRenderEngine::setWrappingAndFiltering(GLenum target, GLint wrapS, GLint wrapT, GLint wrapR, GLint minFilter,
                                              GLint magFilter) {
	setWrappingAndFiltering(target, wrapS, wrapT, minFilter, magFilter);
}

void gVKRenderEngine::setSwizzleMask(GLint swizzleMask[4]) {
}

void gVKRenderEngine::readTexturePixels(unsigned char* inPixels, GLuint textureId, int width, int height,
                                        GLenum format) {
	GLuint fbo;
	G_CHECK_GL(glGenFramebuffers(1, &fbo));
	G_CHECK_GL(glBindFramebuffer(GL_FRAMEBUFFER, fbo));
	G_CHECK_GL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureId, 0));

	G_CHECK_GL(glReadPixels(0, 0, width, height, format, GL_UNSIGNED_BYTE, inPixels));

	G_CHECK_GL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
	G_CHECK_GL(glDeleteFramebuffers(1, &fbo));
}

void gVKRenderEngine::readTexturePixelsHDR(float* inPixels, GLuint textureId, int width, int height,
                                           GLenum format) {
	GLuint fbo;
	G_CHECK_GL(glGenFramebuffers(1, &fbo));
	G_CHECK_GL(glBindFramebuffer(GL_FRAMEBUFFER, fbo));
	G_CHECK_GL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureId, 0));

	G_CHECK_GL(glReadPixels(0, 0, width, height, format, GL_FLOAT, inPixels));

	G_CHECK_GL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
	G_CHECK_GL(glDeleteFramebuffers(1, &fbo));
}

void gVKRenderEngine::generateMipMap() {
	// The 2D image textures are single-level; nothing to generate here.
}

void gVKRenderEngine::bindSkyTexture(GLuint texId) {
	G_CHECK_GL(glBindTexture(GL_TEXTURE_CUBE_MAP, texId));
}

void gVKRenderEngine::bindSkyTexture(GLuint texId, int textureSlot) {
	G_CHECK_GL(glActiveTexture(textureSlot));
	G_CHECK_GL(glBindTexture(GL_TEXTURE_CUBE_MAP, texId));
}

void gVKRenderEngine::unbindSkyTexture() {
	G_CHECK_GL(glBindTexture(GL_TEXTURE_CUBE_MAP, 0));
}

void gVKRenderEngine::unbindSkyTexture(int textureSlotNo) {
	G_CHECK_GL(glActiveTexture(GL_TEXTURE0 + textureSlotNo));
	G_CHECK_GL(glBindTexture(GL_TEXTURE_CUBE_MAP, 0));
}

void gVKRenderEngine::generateSkyMipMap() {
	G_CHECK_GL(glGenerateMipmap(GL_TEXTURE_CUBE_MAP));
}

void gVKRenderEngine::enableDepthTestEqual() {
	// Was a raw glDepthFunc call, which has no meaning with no OpenGL context. The
	// only caller is gSkybox, and the depth compare it wants is applied where it
	// belongs on this backend: drawSkyboxFace sets LESS_OR_EQUAL on the command
	// buffer, because the sky pipeline declares that state dynamic.
}

void gVKRenderEngine::createQuad(GLuint& inQuadVAO, GLuint& inQuadVBO) {
	float quadVertices[] = {
		// positions        // texture Coords
		-1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
		-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
		1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
		1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
	};
	// setup plane VAO
	G_CHECK_GL(glGenVertexArrays(1, &inQuadVAO));
	G_CHECK_GL(glGenBuffers(1, &inQuadVBO));
	G_CHECK_GL(glBindVertexArray(inQuadVAO));
	G_CHECK_GL(glBindBuffer(GL_ARRAY_BUFFER, inQuadVBO));
	G_CHECK_GL(glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW));
	G_CHECK_GL(glEnableVertexAttribArray(0));
	G_CHECK_GL(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0));
	G_CHECK_GL(glEnableVertexAttribArray(1));
	G_CHECK_GL(glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float))));
}

void gVKRenderEngine::enableCubeMap() {
#if defined(GLIST_OPENGLES)
	G_CHECK_GL(glEnable(GL_TEXTURE_CUBE_MAP)); // OpenGL ES does not support GL_TEXTURE_CUBE_MAP_SEAMLESS
#else
	G_CHECK_GL(glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS));
#endif
}

void gVKRenderEngine::pushMatrix() {
#ifndef GLIST_OPENGLES
	G_CHECK_GL(glPushMatrix());
#endif
}

void gVKRenderEngine::popMatrix() {
#ifndef GLIST_OPENGLES
	G_CHECK_GL(glPopMatrix());
#endif
}

#ifdef GVK_VULKAN

// gvkdefaultvalidation and the gVKContext layout now live in gVKContext.h, so
// every module of the backend can share them.

static const char* const GVK_VALIDATION_LAYER = "VK_LAYER_KHRONOS_validation";

// Routes validation output into the engine log instead of stderr, so Vulkan
// problems show up next to every other engine message.
static VKAPI_ATTR VkBool32 VKAPI_CALL gvkDebugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT severity,
		VkDebugUtilsMessageTypeFlagsEXT type,
		const VkDebugUtilsMessengerCallbackDataEXT* data,
		void* userdata) {
	if(severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
		gLoge("gVKRenderEngine") << "Validation: " << data->pMessage;
	} else if(severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
		gLogw("gVKRenderEngine") << "Validation: " << data->pMessage;
	}
	return VK_FALSE;
}

// Hands the loader a search path, but never overrides a value the developer
// exported themselves, so pointing the engine at a different driver or layer
// build stays possible without touching the code.
static void gvkSetEnvIfUnset(const char* name, const char* value) {
	if(getenv(name) != nullptr) return;
#if defined(_WIN32)
	_putenv_s(name, value);
#else
	setenv(name, value, 0);
#endif
}

static bool gvkHasLayer(const char* name) {
	uint32_t count = 0;
	if(vkEnumerateInstanceLayerProperties(&count, nullptr) != VK_SUCCESS || count == 0) return false;
	std::vector<VkLayerProperties> items(count);
	vkEnumerateInstanceLayerProperties(&count, items.data());
	for(const auto& item : items) {
		if(strcmp(item.layerName, name) == 0) return true;
	}
	return false;
}

static bool gvkHasInstanceExtension(const char* name) {
	uint32_t count = 0;
	if(vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr) != VK_SUCCESS || count == 0) return false;
	std::vector<VkExtensionProperties> items(count);
	vkEnumerateInstanceExtensionProperties(nullptr, &count, items.data());
	for(const auto& item : items) {
		if(strcmp(item.extensionName, name) == 0) return true;
	}
	return false;
}

#endif


gVKContext* gVKRenderEngine::getContext() {
#ifdef GVK_VULKAN
	// Create it on first access so callers never dereference null. init() adopts
	// whatever exists here, and setContext() can still replace it afterwards.
	if(vkcontext == nullptr) vkcontext = new gVKContext();
#endif
	return vkcontext;
}


void gVKRenderEngine::setContext(gVKContext* context) {
	if(vkcontext == context) return;
	cleanupVulkan();
	vkcontext = context;
}


bool gVKRenderEngine::initVulkan() {
#ifndef GVK_VULKAN
	gLoge("gVKRenderEngine") << "Vulkan backend is not supported on this platform.";
	return false;
#else
	// Honour a context a developer injected through setContext() so its settings
	// survive; only allocate a default one when none was provided.
	if(vkcontext == nullptr) vkcontext = new gVKContext();
	gVKContext* ctx = vkcontext;

	// Re-initialising an already-live context would overwrite - and leak - the
	// instance and device it already holds, so bail out if init already ran.
	if(ctx->instance != VK_NULL_HANDLE) {
		gLogw("gVKRenderEngine") << "Vulkan is already initialised; skipping re-initialisation.";
		return true;
	}

	// The driver manifest on macOS and the validation layer manifest on both
	// desktop platforms live outside the directories the loader searches by
	// itself, so their locations arrive as build definitions. Whether they are
	// defined at all is decided per platform in engine/CMakeLists.txt.
#ifdef GLIST_VK_ICD_FILE
	gvkSetEnvIfUnset("VK_ICD_FILENAMES", GLIST_VK_ICD_FILE);
#endif
#ifdef GLIST_VK_LAYER_PATH
	gvkSetEnvIfUnset("VK_LAYER_PATH", GLIST_VK_LAYER_PATH);
#endif

	gBaseWindow* platformwindow = appmanager != nullptr ? appmanager->getWindow() : nullptr;
	if(platformwindow == nullptr || !platformwindow->supportsVulkan()) {
		gLoge("gVKRenderEngine") << "Vulkan init: the platform window cannot create a Vulkan surface.";
		cleanupVulkan();
		return false;
	}
	// The frame loop reads this to react to resizes.
	ctx->window = platformwindow;
	ctx->vsyncenabled = platformwindow->isVsyncEnabled();

	// Presentation pacing has to be known before the swapchain is built, because
	// that is where Vulkan expresses it. Taken from the window so an app that
	// asked for vsync either way gets what it asked for on this backend too.
	ctx->setVsyncEnabled(platformwindow->isVsyncEnabled());

	// Record what the instance level offers (every extension and layer present) so
	// support can be queried later without re-enumerating. Done after the Apple
	// env block above, since that is what points the loader at the layers.
	uint32_t availinstextcount = 0;
	if(vkEnumerateInstanceExtensionProperties(nullptr, &availinstextcount, nullptr) == VK_SUCCESS && availinstextcount > 0) {
		ctx->availableinstanceextensions.resize(availinstextcount);
		vkEnumerateInstanceExtensionProperties(nullptr, &availinstextcount, ctx->availableinstanceextensions.data());
	}
	uint32_t availlayercount = 0;
	if(vkEnumerateInstanceLayerProperties(&availlayercount, nullptr) == VK_SUCCESS && availlayercount > 0) {
		ctx->availablelayers.resize(availlayercount);
		vkEnumerateInstanceLayerProperties(&availlayercount, ctx->availablelayers.data());
	}

	/* ---------------- instance ---------------- */
	// Aliased onto the context so the effective list lives on as engine state
	// instead of dying with this local when init returns.
	std::vector<const char*>& extensions = ctx->enabledinstanceextensions;
	extensions.clear();
	platformwindow->getVulkanInstanceExtensions(extensions);
	if(extensions.empty()) {
		gLoge("gVKRenderEngine") << "Vulkan init: the platform reported no surface extensions.";
		cleanupVulkan();
		return false;
	}

	// OpenGL's default framebuffer is a linear, pass-through target. When the
	// presentation system exposes the matching Vulkan colour space, enable it so
	// the compositor does not apply an extra sRGB colour-profile conversion.
	if(gvkHasInstanceExtension(VK_EXT_SWAPCHAIN_COLOR_SPACE_EXTENSION_NAME)) {
		extensions.push_back(VK_EXT_SWAPCHAIN_COLOR_SPACE_EXTENSION_NAME);
	}

#if defined(__APPLE__)
	// Loader-based MoltenVK exposes portability enumeration, while directly
	// linked iOS MoltenVK already enumerates itself and may omit that extension.
	// Request each helper only when this runtime advertises it.
	const bool useportabilityenumeration =
			gvkHasInstanceExtension(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
	if(useportabilityenumeration) {
		extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
	}
	if(gvkHasInstanceExtension(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
		extensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
	}
#endif

	// Only ask for the layer if it is actually installed; requesting a missing
	// layer makes vkCreateInstance fail outright.
	std::vector<const char*>& layers = ctx->enabledlayers;
	bool usevalidation = false;
	if(ctx->enablevalidation) {
		if(gvkHasLayer(GVK_VALIDATION_LAYER) && gvkHasInstanceExtension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME)) {
			layers.push_back(GVK_VALIDATION_LAYER);
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
			usevalidation = true;
		} else {
			gLogi("gVKRenderEngine") << "Validation layer not found, continuing without it. "
					"Point VK_LAYER_PATH at the folder holding VkLayer_khronos_validation.json to enable it.";
		}
	}

	// Record the outcome (requested validation may have been dropped above) so it
	// is queryable through isValidationActive() rather than only reaching the log.
	ctx->validationactive = usevalidation;

	// Developer supplied names come last, so they can extend but never displace
	// the extensions and layers the engine needs to function.
	extensions.insert(extensions.end(), ctx->extrainstanceextensions.begin(), ctx->extrainstanceextensions.end());
	layers.insert(layers.end(), ctx->extralayers.begin(), ctx->extralayers.end());

	// Target the highest API version the loader offers so newer Vulkan (1.4 today,
	// 1.5+ later) is used automatically with no code change. vkEnumerateInstanceVersion
	// (Vulkan 1.1+) reports that ceiling; requesting exactly it never makes
	// vkCreateInstance over-ask. minapiversion is the hard floor - if the runtime
	// cannot even reach it, there is nothing usable to fall back to, so fail here
	// rather than limp on. Packed version numbers compare monotonically, so plain
	// relational operators are correct.
	ctx->instanceapiversion = VK_API_VERSION_1_0;
	vkEnumerateInstanceVersion(&ctx->instanceapiversion);
	if(ctx->instanceapiversion < ctx->minapiversion) {
		gLoge("gVKRenderEngine") << "Vulkan " << VK_API_VERSION_MAJOR(ctx->minapiversion) << "."
				<< VK_API_VERSION_MINOR(ctx->minapiversion) << " is required, but the loader only supports "
				<< VK_API_VERSION_MAJOR(ctx->instanceapiversion) << "."
				<< VK_API_VERSION_MINOR(ctx->instanceapiversion) << ". Update the GPU driver / Vulkan runtime.";
		return false;
	}
	uint32_t targetapiversion = ctx->instanceapiversion;

	VkApplicationInfo appinfo{};
	appinfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appinfo.pApplicationName = ctx->appname.c_str();
	appinfo.applicationVersion = ctx->appversion;
	appinfo.pEngineName = ctx->enginename.c_str();
	appinfo.engineVersion = ctx->engineversion;
	appinfo.apiVersion = targetapiversion;

	VkInstanceCreateInfo instanceinfo{};
	instanceinfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceinfo.pApplicationInfo = &appinfo;
#if defined(__APPLE__)
	if(useportabilityenumeration) {
		instanceinfo.flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
	}
#endif
	instanceinfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	instanceinfo.ppEnabledExtensionNames = extensions.data();
	instanceinfo.enabledLayerCount = static_cast<uint32_t>(layers.size());
	instanceinfo.ppEnabledLayerNames = layers.empty() ? nullptr : layers.data();

	VkResult result = vkCreateInstance(&instanceinfo, nullptr, &ctx->instance);
	if(result == VK_ERROR_LAYER_NOT_PRESENT && usevalidation) {
		// The layer enumerated a moment ago but could not be loaded, which means its
		// manifest names a library the loader cannot find - a packaging problem, not
		// a missing layer. engine/CMakeLists.txt rewrites the manifest for the known
		// case; if the run still lands here, say exactly what happened and carry on
		// without validation rather than refusing to start over a development aid.
		gLogw("gVKRenderEngine") << "The validation layer is installed but could not be loaded "
				"(VK_ERROR_LAYER_NOT_PRESENT). Its manifest most likely names the layer library "
				"by bare name and the loader cannot find it. Continuing without validation; to "
				"get it back, point VK_LAYER_PATH at a manifest whose library_path is absolute, "
				"or put the layer library on the library search path.";
		layers.clear();
		usevalidation = false;
		ctx->validationactive = false;
		instanceinfo.enabledLayerCount = 0;
		instanceinfo.ppEnabledLayerNames = nullptr;
		result = vkCreateInstance(&instanceinfo, nullptr, &ctx->instance);
	}
	if(result != VK_SUCCESS) {
		gLoge("gVKRenderEngine") << "vkCreateInstance failed! VkResult: " << result;
		cleanupVulkan();
		return false;
	}

	/* ---------------- debug messenger ---------------- */
	if(usevalidation) {
		VkDebugUtilsMessengerCreateInfoEXT debuginfo{};
		debuginfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		debuginfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
				VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		debuginfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
				VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
				VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		debuginfo.pfnUserCallback = gvkDebugCallback;
		// Extension entry points are not exported by the loader, so fetch them.
		auto createmessenger = (PFN_vkCreateDebugUtilsMessengerEXT)
				vkGetInstanceProcAddr(ctx->instance, "vkCreateDebugUtilsMessengerEXT");
		if(createmessenger != nullptr) {
			createmessenger(ctx->instance, &debuginfo, nullptr, &ctx->debugmessenger);
		}
	}

	/* ---------------- surface ---------------- */
	if(!platformwindow->createVulkanSurface(&ctx->instance, &ctx->surface)) {
		gLoge("gVKRenderEngine") << "The platform failed to create a Vulkan presentation surface.";
		cleanupVulkan();
		return false;
	}

	/* ---------------- physical device ---------------- */
	vkEnumeratePhysicalDevices(ctx->instance, &ctx->devicecount, nullptr);
	if(ctx->devicecount == 0) {
		gLoge("gVKRenderEngine") << "No Vulkan capable GPU was found.";
		cleanupVulkan();
		return false;
	}
	ctx->physicaldevices.resize(ctx->devicecount);
	vkEnumeratePhysicalDevices(ctx->instance, &ctx->devicecount, ctx->physicaldevices.data());

	// Properties and features for every GPU, so a developer can compare devices -
	// including the ones init does not pick - through getAllDeviceProperties() /
	// getAllDeviceFeatures().
	ctx->physicaldeviceproperties.resize(ctx->devicecount);
	ctx->physicaldevicefeatures.resize(ctx->devicecount);
	for(uint32_t i = 0; i < ctx->devicecount; i++) {
		vkGetPhysicalDeviceProperties(ctx->physicaldevices[i], &ctx->physicaldeviceproperties[i]);
		vkGetPhysicalDeviceFeatures(ctx->physicaldevices[i], &ctx->physicaldevicefeatures[i]);
	}

	// A GPU is usable only if it can both render and present to our surface, and
	// in Vulkan those are separate queue family capabilities queried per family.
	// It must also meet the version floor: the device's own apiVersion - not the
	// loader's - is what caps the core features actually available on it, so a
	// device below minapiversion cannot deliver what the engine requires.
	bool rejectedforversion = false;
	for(const auto& dev : ctx->physicaldevices) {
		uint32_t familycount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(dev, &familycount, nullptr);
		if(familycount == 0) continue;
		std::vector<VkQueueFamilyProperties> families(familycount);
		vkGetPhysicalDeviceQueueFamilyProperties(dev, &familycount, families.data());
		std::vector<VkBool32> presentsupportlist(familycount, VK_FALSE);

		bool foundgraphics = false, foundpresent = false;
		uint32_t graphics = 0, present = 0;
		for(uint32_t i = 0; i < familycount; i++) {
			if(!foundgraphics && (families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)) {
				graphics = i;
				foundgraphics = true;
			}
			vkGetPhysicalDeviceSurfaceSupportKHR(dev, i, ctx->surface, &presentsupportlist[i]);
			if(!foundpresent && presentsupportlist[i] == VK_TRUE) {
				present = i;
				foundpresent = true;
			}
		}
		if(foundgraphics && foundpresent) {
			VkPhysicalDeviceProperties devprops{};
			vkGetPhysicalDeviceProperties(dev, &devprops);
			if(devprops.apiVersion < ctx->minapiversion) {
				// Otherwise fine, but too old - keep looking for a newer GPU.
				rejectedforversion = true;
				continue;
			}
			ctx->physicaldevice = dev;
			ctx->graphicsfamily = graphics;
			ctx->presentfamily = present;
			// Keep the chosen device's families (queue counts + capability flags)
			// and which of them can present, so later phases need no re-query.
			ctx->queuefamilyproperties = families;
			ctx->queuefamilypresentsupport = presentsupportlist;
			break;
		}
	}
	if(ctx->physicaldevice == VK_NULL_HANDLE) {
		if(rejectedforversion) {
			gLoge("gVKRenderEngine") << "No GPU supports the required Vulkan "
					<< VK_API_VERSION_MAJOR(ctx->minapiversion) << "."
					<< VK_API_VERSION_MINOR(ctx->minapiversion)
					<< ". Update the GPU driver.";
		} else {
			gLoge("gVKRenderEngine") << "No GPU can both render and present to this surface.";
		}
		cleanupVulkan();
		return false;
	}

	/* ---------------- logical device ---------------- */
	// A set, because graphics and present are usually the same family and the
	// same family must not be requested twice.
	std::set<uint32_t> uniquefamilies = {ctx->graphicsfamily, ctx->presentfamily};
	std::vector<VkDeviceQueueCreateInfo> queueinfos;
	float queuepriority = 1.0f;
	for(uint32_t family : uniquefamilies) {
		VkDeviceQueueCreateInfo queueinfo{};
		queueinfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueinfo.queueFamilyIndex = family;
		queueinfo.queueCount = 1;
		queueinfo.pQueuePriorities = &queuepriority;
		queueinfos.push_back(queueinfo);
	}

	// Swapchain is requested now because presentation is the next phase and
	// asking here proves the device supports it.
	std::vector<const char*>& deviceextensions = ctx->enableddeviceextensions;
	deviceextensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
	// VK_KHR_portability_subset is mandatory on portability drivers such as
	// MoltenVK and absent on conformant ones, so it is queried, never assumed.
	uint32_t devextcount = 0;
	vkEnumerateDeviceExtensionProperties(ctx->physicaldevice, nullptr, &devextcount, nullptr);
	if(devextcount > 0) {
		ctx->availabledeviceextensions.resize(devextcount);
		vkEnumerateDeviceExtensionProperties(ctx->physicaldevice, nullptr, &devextcount, ctx->availabledeviceextensions.data());
		for(const auto& ext : ctx->availabledeviceextensions) {
			if(strcmp(ext.extensionName, "VK_KHR_portability_subset") == 0) {
				deviceextensions.push_back("VK_KHR_portability_subset");
				break;
			}
		}
	}
	// Same rule as the instance side: developer requests extend the mandatory set.
	deviceextensions.insert(deviceextensions.end(), ctx->extradeviceextensions.begin(), ctx->extradeviceextensions.end());

	// Anisotropic filtering is the one optional feature this backend asks for, and it
	// has to be enabled here or the samplers may not use it. It is what keeps a ground
	// or road texture sharp where the surface runs away from the camera: the pixel's
	// footprint there is long in one direction and narrow in the other, an isotropic
	// sampler has to pick a single mip level wide enough to cover the long axis, and
	// the result is the blur that makes distant ground look like mud. Supported by
	// every GPU this engine runs on, but optional in the specification, so it is
	// queried rather than assumed - a driver that says no simply gets samplers
	// without it (see gvkAcquireSampler).
	VkPhysicalDeviceFeatures supportedfeatures{};
	vkGetPhysicalDeviceFeatures(ctx->physicaldevice, &supportedfeatures);
	VkPhysicalDeviceFeatures enabledfeatures{};
	enabledfeatures.samplerAnisotropy = supportedfeatures.samplerAnisotropy;
	VkDeviceCreateInfo deviceinfo{};
	deviceinfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceinfo.queueCreateInfoCount = static_cast<uint32_t>(queueinfos.size());
	deviceinfo.pQueueCreateInfos = queueinfos.data();
	deviceinfo.pEnabledFeatures = &enabledfeatures;
	deviceinfo.enabledExtensionCount = static_cast<uint32_t>(deviceextensions.size());
	deviceinfo.ppEnabledExtensionNames = deviceextensions.data();
	// Device-level layers were deprecated in Vulkan 1.0: the layers enabled on the
	// instance already cover the device, and passing them again here is a spec
	// violation the validation layer reports. The spec requires these to be zero.
	deviceinfo.enabledLayerCount = 0;
	deviceinfo.ppEnabledLayerNames = nullptr;

	result = vkCreateDevice(ctx->physicaldevice, &deviceinfo, nullptr, &ctx->device);
	if(result != VK_SUCCESS) {
		gLoge("gVKRenderEngine") << "vkCreateDevice failed! VkResult: " << result;
		cleanupVulkan();
		return false;
	}
	vkGetDeviceQueue(ctx->device, ctx->graphicsfamily, 0, &ctx->graphicsqueue);
	vkGetDeviceQueue(ctx->device, ctx->presentfamily, 0, &ctx->presentqueue);

	// Cached on the context so later phases can read them without re-querying:
	// limits/identity, the GPU's supported features, and its memory layout.
	vkGetPhysicalDeviceProperties(ctx->physicaldevice, &ctx->deviceproperties);
	vkGetPhysicalDeviceFeatures(ctx->physicaldevice, &ctx->devicefeatures);
	vkGetPhysicalDeviceMemoryProperties(ctx->physicaldevice, &ctx->devicememoryproperties);

	// Surface capabilities, formats and present modes for the selected device -
	// the inputs the swapchain phase consumes. The surface and device both exist
	// by now, so cache them here rather than re-querying later.
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(ctx->physicaldevice, ctx->surface, &ctx->surfacecapabilities);
	uint32_t surfaceformatcount = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(ctx->physicaldevice, ctx->surface, &surfaceformatcount, nullptr);
	if(surfaceformatcount > 0) {
		ctx->surfaceformats.resize(surfaceformatcount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(ctx->physicaldevice, ctx->surface, &surfaceformatcount, ctx->surfaceformats.data());
	}
	uint32_t presentmodecount = 0;
	vkGetPhysicalDeviceSurfacePresentModesKHR(ctx->physicaldevice, ctx->surface, &presentmodecount, nullptr);
	if(presentmodecount > 0) {
		ctx->surfacepresentmodes.resize(presentmodecount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(ctx->physicaldevice, ctx->surface, &presentmodecount, ctx->surfacepresentmodes.data());
	}

	VkPhysicalDeviceProperties& props = ctx->deviceproperties;
	gLogi("gVKRenderEngine") << "Vulkan Instance successfully created! API Version: "
			<< VK_API_VERSION_MAJOR(props.apiVersion) << "."
			<< VK_API_VERSION_MINOR(props.apiVersion) << "."
			<< VK_API_VERSION_PATCH(props.apiVersion);
	// Three distinct numbers - separated so a version mismatch is unambiguous:
	// the floor the engine requires, the highest the loader offers (which is what
	// the instance actually targets), and the highest the selected device supports.
	gLogi("gVKRenderEngine") << "API versions -> min required: "
			<< VK_API_VERSION_MAJOR(ctx->minapiversion) << "." << VK_API_VERSION_MINOR(ctx->minapiversion)
			<< " | loader max (targeted): "
			<< VK_API_VERSION_MAJOR(ctx->instanceapiversion) << "." << VK_API_VERSION_MINOR(ctx->instanceapiversion)
			<< " | device max: "
			<< VK_API_VERSION_MAJOR(props.apiVersion) << "." << VK_API_VERSION_MINOR(props.apiVersion);
	gLogi("gVKRenderEngine") << "Vulkan device: " << props.deviceName
			<< " | graphics family: " << ctx->graphicsfamily
			<< " | present family: " << ctx->presentfamily
			<< " | validation: " << (usevalidation ? "on" : "off");
	// Every texture and every static mesh buffer holds one vkAllocateMemory, and a
	// mobile driver caps how many of those can exist at once - commonly 4096. The
	// limit is logged next to the shutdown counts so the two can be read together
	// when deciding whether a level is anywhere near it.
	// The two limits the emulator and MoltenVK both report generously and real
	// mobile hardware does not. maxBoundDescriptorSets is 4 on Adreno and Mali, so a
	// pipeline layout asking for five sets is simply not creatable there; the mesh
	// pipelines are built to three for that reason. maxMemoryAllocationCount is
	// commonly 4096, and every texture and mesh buffer is one allocation today.
	gLogi("gVKRenderEngine") << "Device limits: maxBoundDescriptorSets="
			<< props.limits.maxBoundDescriptorSets
			<< " maxMemoryAllocationCount=" << props.limits.maxMemoryAllocationCount
			<< " maxPushConstantsSize=" << props.limits.maxPushConstantsSize;

	/* ---------------- presentation resources ---------------- */
	// An application can ask for MSAA before the backend exists - in its constructor,
	// or anywhere ahead of the first frame - so a request made that early is carried
	// here rather than triggering a rebuild of objects that have not been built yet.
	if(pendingsamplecount > 0) {
		ctx->setSampleCount(pendingsamplecount);
		pendingsamplecount = 0;
	}
	// The remaining modules of the frame path (render pass, framebuffers, command
	// buffers and synchronisation) are hooked in here as they are implemented.
	if(!gvkCreateSwapchain(*ctx, platformwindow)) {
		gLoge("gVKRenderEngine") << "Vulkan init: the swapchain could not be created.";
		cleanupVulkan();
		return false;
	}
	if(!gvkCreateRenderPass(*ctx)) {
		gLoge("gVKRenderEngine") << "Vulkan init: the render pass could not be created.";
		cleanupVulkan();
		return false;
	}
	// Before the framebuffers, which pair every swapchain view with this one. Both
	// depend on the sample count the render pass just resolved, which is why they
	// come after it.
	if(!gvkCreateDepthResources(*ctx)) {
		gLoge("gVKRenderEngine") << "Vulkan init: the depth buffer could not be created.";
		cleanupVulkan();
		return false;
	}
	// Does nothing while MSAA is off; with it on, this is the transient attachment
	// the pass renders into and resolves out of.
	if(!gvkCreateMsaaColorResources(*ctx)) {
		gLoge("gVKRenderEngine") << "Vulkan init: the MSAA colour buffer could not be created.";
		cleanupVulkan();
		return false;
	}
	if(!gvkCreateFramebuffers(*ctx)) {
		gLoge("gVKRenderEngine") << "Vulkan init: the framebuffers could not be created.";
		cleanupVulkan();
		return false;
	}
	if(!gvkCreateCommandResources(*ctx)) {
		gLoge("gVKRenderEngine") << "Vulkan init: the command resources could not be created.";
		cleanupVulkan();
		return false;
	}
	if(!gvkCreateFrameSyncObjects(*ctx)) {
		gLoge("gVKRenderEngine") << "Vulkan init: the frame synchronisation objects could not be created.";
		cleanupVulkan();
		return false;
	}
	if(!gvkCreatePresentSemaphores(*ctx, static_cast<uint32_t>(ctx->swapchainimages.size()))) {
		gLoge("gVKRenderEngine") << "Vulkan init: the present semaphores could not be created.";
		cleanupVulkan();
		return false;
	}
	// The 2D draw path: graphics pipelines (colour + image) and the per-frame
	// vertex ring the triangle / rectangle / image helpers record into.
	if(!gvkCreateGraphicsPipelines(*ctx)) {
		gLoge("gVKRenderEngine") << "Vulkan init: the 2D graphics pipelines could not be created.";
		cleanupVulkan();
		return false;
	}
	if(!gvkCreateDrawResources(*ctx)) {
		gLoge("gVKRenderEngine") << "Vulkan init: the draw resources could not be created.";
		cleanupVulkan();
		return false;
	}
	// After the pipelines and the pool, both of which it allocates from.
	if(!gvkCreateUniformResources(*ctx)) {
		gLoge("gVKRenderEngine") << "Vulkan init: the scene uniform buffers could not be created.";
		cleanupVulkan();
		return false;
	}
	return true;
#endif
}


void gVKRenderEngine::cleanupVulkan() {
#ifdef GVK_VULKAN
	if(vkcontext == nullptr) return;
	gVKContext* ctx = vkcontext;
	// Destroying objects the GPU is still using is a validation error, so the
	// device is drained first.
	if(ctx->device != VK_NULL_HANDLE) vkDeviceWaitIdle(ctx->device);
	// Strict reverse creation order: Vulkan requires children to be destroyed
	// before their parent, and the surface must die before its instance. The 2D
	// draw path was built last, so it is torn down first (pipelines reference the
	// render pass, which is still alive at this point). Textures free their
	// descriptor sets, so they go before the descriptor pool inside
	// gvkDestroyGraphicsPipelines.
	destroyAllTextures();
	destroyAllMeshBuffers();
	// Shadow resources are created on demand after the main pipelines. Their
	// pipeline references the shadow render pass and their descriptor set belongs
	// to the main descriptor pool, so the whole shadow unit must die before that
	// pool and before the device.
	gvkDestroyShadowResources(*ctx);
	gvkDestroyUniformResources(*ctx);
	gvkDestroyDrawResources(*ctx);
	gvkDestroyGraphicsPipelines(*ctx);
	gvkDestroyPipelineCache(*ctx);
	gvkDestroyPresentSemaphores(*ctx);
	gvkDestroyFrameSyncObjects(*ctx);
	// The upload batches hold command buffers from the pool below and fences of
	// their own, so they go before the pool does.
	gvkDestroyUploadContext(*ctx);
	gvkDestroyCommandResources(*ctx);
	gvkDestroyFramebuffers(*ctx);
	gvkDestroyMsaaColorResources(*ctx);
	gvkDestroyDepthResources(*ctx);
	gvkDestroyRenderPass(*ctx);
	gvkDestroySwapchain(*ctx);
	if(ctx->device != VK_NULL_HANDLE) vkDestroyDevice(ctx->device, nullptr);
	if(ctx->surface != VK_NULL_HANDLE) vkDestroySurfaceKHR(ctx->instance, ctx->surface, nullptr);
	if(ctx->debugmessenger != VK_NULL_HANDLE) {
		auto destroymessenger = (PFN_vkDestroyDebugUtilsMessengerEXT)
				vkGetInstanceProcAddr(ctx->instance, "vkDestroyDebugUtilsMessengerEXT");
		if(destroymessenger != nullptr) destroymessenger(ctx->instance, ctx->debugmessenger, nullptr);
	}
	if(ctx->instance != VK_NULL_HANDLE) vkDestroyInstance(ctx->instance, nullptr);
	delete vkcontext;
#endif
	vkcontext = nullptr;
}


bool gVKRenderEngine::beginFrame() {
#ifdef GVK_VULKAN
	if(vkcontext == nullptr) return false;
	// Between frames is the only safe point to swap pipelines out: no command
	// buffer is recording and the previous frame can be drained.
	applyPendingSampleCount();
	checkShaderReload();
	// The new frame writes into a different uniform buffer, so whatever the previous
	// one gathered does not carry over.
	sceneuniformswritten = false;
	vkcontext->resetSceneUniformSlots();
	return gvkBeginFrame(*vkcontext, vkcontext->window);
#else
	return false;
#endif
}

// The present mode is fixed when the swapchain is created, so changing vsync means
// building a new swapchain. That cannot happen here: this is reached from wherever
// the app changed the setting, which may be mid-frame with a command buffer already
// recording. The request is flagged and gvkBeginFrame acts on it at the next frame
// boundary, where no work is in flight.
void gVKRenderEngine::setVsync(bool enabled) {
#ifdef GVK_VULKAN
	if(vkcontext == nullptr || vkcontext->vsyncenabled == enabled) return;
	vkcontext->vsyncenabled = enabled;
	if(vkcontext->swapchain != VK_NULL_HANDLE) {
		vkcontext->swapchainrecreaterequested = true;
	}
#else
	(void)enabled;
#endif
}

// Same deferral as setVsync, and for a stronger reason: the sample count is baked
// into the render pass, its attachments and every pipeline, so changing it destroys
// and rebuilds objects a recording command buffer may be pointing at. The request is
// parked and beginFrame acts on it where no work is in flight.
void gVKRenderEngine::setMultiSampling(int samples) {
#ifdef GVK_VULKAN
	const int requested = samples < 1 ? 1 : samples;
	if(vkcontext == nullptr) {
		// Before the backend exists there is nothing to rebuild; initVulkan picks
		// this up and builds everything at the right count the first time.
		pendingsamplecount = requested;
		return;
	}
	if(vkcontext->getRequestedSampleCount() == requested) return;
	if(!vkcontext->isFramePathReady()) {
		vkcontext->setSampleCount(requested);
		return;
	}
	pendingsamplecount = requested;
#else
	(void)samples;
#endif
}

// What was achieved, not what was asked for: a request is capped at what the device
// supports for colour and depth attachments alike.
int gVKRenderEngine::getMultiSampling() const {
#ifdef GVK_VULKAN
	return vkcontext == nullptr ? 1 : vkcontext->getActiveSampleCount();
#else
	return 1;
#endif
}

void gVKRenderEngine::applyPendingSampleCount() {
#ifdef GVK_VULKAN
	if(pendingsamplecount == 0 || vkcontext == nullptr) return;
	const int requested = pendingsamplecount;
	pendingsamplecount = 0;
	if(!vkcontext->isFramePathReady()) {
		vkcontext->setSampleCount(requested);
		return;
	}

	gVKContext* ctx = vkcontext;
	// Everything below is referenced by work the GPU may not have finished, and the
	// descriptor pool about to be destroyed backs sets that live elsewhere.
	vkDeviceWaitIdle(ctx->device);

	// The shadow map's descriptor set and its pipeline come out of the pool and cache
	// that gvkDestroyGraphicsPipelines takes with it, so a map that is already
	// allocated is torn down here and rebuilt at the same size afterwards. Its pass
	// stays 1x throughout; only the pool underneath it changes.
	const bool hadshadowmap = ctx->hasShadowMap();
	const VkExtent2D shadowextent = ctx->shadowextent;
	if(hadshadowmap) gvkDestroyShadowResources(*ctx);
	// Scene descriptor sets belong to the same pool; release them before it goes,
	// exactly as the shader reload path does.
	gvkDestroyUniformResources(*ctx);
	gvkDestroyGraphicsPipelines(*ctx);
	gvkDestroyFramebuffers(*ctx);
	gvkDestroyMsaaColorResources(*ctx);
	gvkDestroyDepthResources(*ctx);
	gvkDestroyRenderPass(*ctx);

	ctx->setSampleCount(requested);

	// Same order as init: the render pass resolves the sample count, the attachments
	// are allocated at it, the framebuffers bind them together and the pipelines are
	// built against the pass.
	if(!gvkCreateRenderPass(*ctx) || !gvkCreateDepthResources(*ctx)
			|| !gvkCreateMsaaColorResources(*ctx) || !gvkCreateFramebuffers(*ctx)
			|| !gvkCreateGraphicsPipelines(*ctx) || !gvkCreateUniformResources(*ctx)) {
		gLoge("gVKRenderEngine") << "Could not rebuild the frame path for " << requested
				<< "x MSAA; rendering will be broken until this is resolved.";
		return;
	}
	// The rebuild destroyed the descriptor pool, so every texture still loaded needs
	// pointing at the new one - the same fix-up the shader reload does.
	for(auto& entry : vktextures) {
		if(entry.second != nullptr) gvkWriteTextureDescriptorSet(*ctx, entry.second);
	}
	if(hadshadowmap) gvkCreateShadowResources(*ctx, shadowextent.width, shadowextent.height);

	gLogi("gVKRenderEngine") << "MSAA set to " << ctx->getActiveSampleCount() << "x"
			<< (ctx->getActiveSampleCount() != requested ? " (capped from the requested "
					+ std::to_string(requested) + "x)" : "");
#endif
}

void gVKRenderEngine::checkShaderReload() {
#ifdef GVK_VULKAN
	if(vkcontext == nullptr) return;
	// Stating the sources every frame would be wasteful; a few times a second
	// still feels immediate when a shader is saved.
	if(--shaderpollcountdown > 0) return;
	shaderpollcountdown = 20;

	const long long newest = gvkShaderSourcesTimestamp();
	if(newest == 0 || newest == shadersourcetimestamp) return;
	const bool firstreading = shadersourcetimestamp == 0;
	shadersourcetimestamp = newest;
	// The first reading only establishes the baseline; it is not an edit.
	if(firstreading) return;

	// Scene descriptor sets belong to the descriptor pool rebuilt by a reload.
	// Release their buffers first, then recreate both buffers and sets against the
	// new reflected layout; retaining the old handles would bind freed sets.
	gvkDestroyUniformResources(*vkcontext);
	if(!gvkReloadGraphicsPipelines(*vkcontext)) return;
	if(!gvkCreateUniformResources(*vkcontext)) {
		gLoge("gVKRenderEngine") << "Could not recreate scene uniforms after shader reload.";
		return;
	}
	// The reload destroys the descriptor pool, and with it every set allocated
	// from it, so the textures that are still loaded need pointing at the new one.
	for(auto& entry : vktextures) {
		if(entry.second != nullptr) gvkWriteTextureDescriptorSet(*vkcontext, entry.second);
	}
#endif
}

void gVKRenderEngine::endFrame() {
#ifdef GVK_VULKAN
	if(vkcontext == nullptr) return;
	flushQueuedDraws();
	// An application that forgot to unbind its FBO would otherwise leave that pass
	// open, and the frame loop would close it believing it was the screen one -
	// leaving the swapchain image never rendered and never transitioned.
	endOffscreenPass();
	boundframebuffer = gFbo::defaultfbo;
	gvkEndFrame(*vkcontext, vkcontext->window);
#endif
}

// FNV-1a over the fields a merge depends on. Written out field by field rather than
// hashing the structs as bytes: both carry padding, and padding holds whatever was
// on the stack, so two identical materials would digest differently often enough to
// lose most of the merges this exists to find.
static void gvkHashValue(uint64_t& hash, const void* data, size_t size) {
	const unsigned char* bytes = static_cast<const unsigned char*>(data);
	for(size_t i = 0; i < size; i++) {
		hash ^= bytes[i];
		hash *= 1099511628211ull;
	}
}

template<typename T>
static void gvkHashField(uint64_t& hash, const T& value) {
	gvkHashValue(hash, &value, sizeof(T));
}

// Whether reordering this draw among its neighbours can be seen. A mesh that depth
// tests normally is decided by the depth buffer rather than by when it was
// submitted, so two of them may swap places freely. One drawn with testing off, or
// with an ALWAYS compare, is decided purely by sequence - whatever is drawn last
// wins - so it must stay exactly where the application put it.
static bool gvkQueuedDrawIsOrderIndependent(bool depthtest, int depthtesttype) {
	return depthtest && depthtesttype != gRenderer::DEPTHTESTTYPE_ALWAYS;
}

// Every input canMergeQueuedDraws compares, in one number. Two draws that can merge
// always produce the same key; two that cannot may collide, which costs a merge that
// would not have happened anyway once the exact comparison runs.
uint64_t gVKRenderEngine::gvkQueuedDrawKey(const QueuedMeshDraw& draw) {
	uint64_t hash = 14695981039346656037ull;
	gvkHashField(hash, draw.vertexarrayid);
	gvkHashField(hash, draw.vertexcount);
	gvkHashField(hash, draw.indexcount);
	gvkHashField(hash, draw.drawmode);
	gvkHashField(hash, draw.tint);
	gvkHashField(hash, draw.depthtest);
	gvkHashField(hash, draw.depthtesttype);
	gvkHashField(hash, draw.culling);
	gvkHashField(hash, draw.cullface);
	gvkHashField(hash, draw.cullingdirection);
	gvkHashField(hash, draw.surface.ambient);
	gvkHashField(hash, draw.surface.diffuse);
	gvkHashField(hash, draw.surface.specular);
	gvkHashField(hash, draw.surface.shininess);
	gvkHashField(hash, draw.surface.diffusemapid);
	gvkHashField(hash, draw.surface.specularmapid);
	gvkHashField(hash, draw.surface.normalmapid);
	gvkHashField(hash, draw.surface.ispbr);
	gvkHashField(hash, draw.surface.albedomapid);
	gvkHashField(hash, draw.surface.pbrnormalmapid);
	gvkHashField(hash, draw.surface.metallicmapid);
	gvkHashField(hash, draw.surface.roughnessmapid);
	gvkHashField(hash, draw.surface.aomapid);
	return hash;
}

void gVKRenderEngine::flushQueuedDraws() {
	if(queuedmeshdraws.empty() || flushingqueueddraws) return;

	// Adjacent draws that agree on everything merge into one instanced draw below.
	// What used to stop that from firing was interleaving: a scene submits a crate,
	// a barrel, another crate, and the two crates never meet even though they are
	// the same mesh with the same material. Sorting by the merge key brings them
	// together, and the run of identical draws then collapses into a single
	// vkCmdDraw with an instance count.
	//
	// Only where the reordering cannot be seen. Anything order dependent (see above)
	// is left in place and acts as a barrier: the run before it is sorted, it stays,
	// and the next run starts after it. The sort is stable, so draws sharing a key
	// keep their submission order relative to each other.
	size_t segmentbegin = 0;
	for(size_t i = 0; i <= queuedmeshdraws.size(); i++) {
		const bool barrier = i == queuedmeshdraws.size()
				|| !gvkQueuedDrawIsOrderIndependent(queuedmeshdraws[i].depthtest, queuedmeshdraws[i].depthtesttype);
		if(!barrier) continue;
		if(i - segmentbegin > 1) {
			std::stable_sort(queuedmeshdraws.begin() + segmentbegin, queuedmeshdraws.begin() + i,
					[](const QueuedMeshDraw& a, const QueuedMeshDraw& b) { return a.mergekey < b.mergekey; });
		}
		segmentbegin = i + 1;
	}

	for(size_t first = 0; first < queuedmeshdraws.size();) {
		size_t count = 1;
		while(first + count < queuedmeshdraws.size()
				&& canMergeQueuedDraws(queuedmeshdraws[first], queuedmeshdraws[first + count])) {
			++count;
		}
		recordQueuedDrawGroup(first, count);
		first += count;
	}
	queuedmeshdraws.clear();
}

bool gVKRenderEngine::canMergeQueuedDraws(const QueuedMeshDraw& first, const QueuedMeshDraw& next) const {
	auto samevec4 = [](const glm::vec4& a, const glm::vec4& b) {
		return a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w;
	};
	const gMeshSurface& a = first.surface;
	const gMeshSurface& b = next.surface;
	return first.vertexarrayid == next.vertexarrayid
			&& first.instancecount == 1 && next.instancecount == 1
			&& first.vertexcount == next.vertexcount && first.indexcount == next.indexcount
			&& first.drawmode == next.drawmode && samevec4(first.tint, next.tint)
			&& first.depthtest == next.depthtest && first.depthtesttype == next.depthtesttype
			&& first.culling == next.culling && first.cullface == next.cullface
			&& first.cullingdirection == next.cullingdirection
			&& samevec4(a.ambient, b.ambient) && samevec4(a.diffuse, b.diffuse)
			&& samevec4(a.specular, b.specular) && a.shininess == b.shininess
			&& a.diffusemapid == b.diffusemapid && a.specularmapid == b.specularmapid
			&& a.normalmapid == b.normalmapid && a.ispbr == b.ispbr
			&& a.albedomapid == b.albedomapid && a.pbrnormalmapid == b.pbrnormalmapid
			&& a.metallicmapid == b.metallicmapid && a.roughnessmapid == b.roughnessmapid
			&& a.aomapid == b.aomapid;
}

void gVKRenderEngine::recordQueuedDrawGroup(size_t first, size_t count) {
	if(first >= queuedmeshdraws.size() || count == 0) return;
	const QueuedMeshDraw& draw = queuedmeshdraws[first];

	const bool saveddepthtest = isdepthtestenabled;
	const int saveddepthtesttype = depthtesttype;
	const bool savedculling = iscullingenabled;
	const int savedcullface = cullface;
	const int savedcullingdirection = cullingdirection;
	const gColor savedcolor = rendercolor != nullptr ? *rendercolor : gColor();
	isdepthtestenabled = draw.depthtest;
	depthtesttype = draw.depthtesttype;
	iscullingenabled = draw.culling;
	cullface = draw.cullface;
	cullingdirection = draw.cullingdirection;
	if(rendercolor != nullptr) rendercolor->set(draw.tint.x, draw.tint.y, draw.tint.z, draw.tint.w);

	flushingqueueddraws = true;
#ifdef GVK_VULKAN
	// The normal mesh path already understands instancing.  Supplying a short-lived
	// view of the current frame's mapped mesh arena lets it use that exact path for
	// a gathered run, including PBR and shadow casters, without a second material
	// implementation.  Command buffers retain only the VkBuffer and offset, so the
	// view can disappear as soon as recording finishes.
	if(count > 1 && vkcontext != nullptr) {
		// Reuse the scratch allocation across groups and frames. Crowds commonly
		// produce several merged runs per pass; allocating a temporary vector for
		// every run showed up directly in mobile command-record time.
		static thread_local std::vector<glm::mat4> models;
		models.clear();
		models.reserve(count);
		for(size_t i = 0; i < count; ++i) models.push_back(queuedmeshdraws[first + i].model);
		const VkDeviceSize offset = vkcontext->pushMeshData(models.data(), models.size() * sizeof(glm::mat4));
		const VkBuffer buffer = vkcontext->getCurrentMeshArena();
		if(offset != VK_WHOLE_SIZE && buffer != VK_NULL_HANDLE) {
			gVKMeshBuffer instances;
			instances.isdynamic = true;
			instances.arenabuffer = buffer;
			instances.arenaoffset = offset;
			instances.arenageneration = vkcontext->getMeshGeneration();
			const GLuint temporaryid = nextvkbufferid++;
			auto arrayentry = vkvertexarrays.find(draw.vertexarrayid);
			if(arrayentry != vkvertexarrays.end()) {
				const GLuint savedinstancebuffer = arrayentry->second.instancebuffer;
				vkmeshbuffers[temporaryid] = &instances;
				arrayentry->second.instancebuffer = temporaryid;
				drawMesh3D(draw.vertexarrayid, draw.vertexcount, draw.indexcount, glm::mat4(1.0f),
						draw.surface, draw.drawmode, static_cast<int>(count));
				arrayentry->second.instancebuffer = savedinstancebuffer;
				vkmeshbuffers.erase(temporaryid);
			} else {
				for(size_t i = 0; i < count; ++i) {
					const QueuedMeshDraw& item = queuedmeshdraws[first + i];
					drawMesh3D(item.vertexarrayid, item.vertexcount, item.indexcount, item.model,
							item.surface, item.drawmode, item.instancecount);
				}
			}
		} else {
			for(size_t i = 0; i < count; ++i) {
				const QueuedMeshDraw& item = queuedmeshdraws[first + i];
				drawMesh3D(item.vertexarrayid, item.vertexcount, item.indexcount, item.model,
						item.surface, item.drawmode, item.instancecount);
			}
		}
	} else
#endif
	{
		for(size_t i = 0; i < count; ++i) {
			const QueuedMeshDraw& item = queuedmeshdraws[first + i];
			drawMesh3D(item.vertexarrayid, item.vertexcount, item.indexcount, item.model,
					item.surface, item.drawmode, item.instancecount);
		}
	}
	flushingqueueddraws = false;

	isdepthtestenabled = saveddepthtest;
	depthtesttype = saveddepthtesttype;
	iscullingenabled = savedculling;
	cullface = savedcullface;
	cullingdirection = savedcullingdirection;
	if(rendercolor != nullptr) *rendercolor = savedcolor;
}


void gVKRenderEngine::init() {
	// gRenderer::init() is deliberately not called: it compiles shaders and
	// builds GL objects, and there is no GL context under Vulkan. originalgrid
	// is assigned only inside that function and is never initialised in a
	// constructor, so it is nulled here to keep the destructor's delete safe.
    originalgrid = nullptr;
	// gRenderer::init() also allocates rendercolor, but it is skipped under Vulkan;
	// the 2D draw helpers read the current colour, so create a white default here.
    rendercolor = new gColor();
    rendercolor->set(255, 255, 255, 255);
	// Retained by clear(), so this is a one-time allocation for the normal mobile
	// scene rather than vector growth during the first shadow and colour passes.
	queuedmeshdraws.reserve(2048);
	// The primitive meshes are created by gRenderer::init() as well. They hold no GL
	// objects until they are drawn, and the 2D ones now record through the backend's
	// draw path, so drawLine / drawCircle / drawRectangle and friends need them here
	// just as much as the OpenGL path does.
    if(!initVulkan()) {
        gLoge("gVKRenderEngine")
            << "Vulkan initialization failed; the Vulkan backend is not usable.";
        return;
    }

    createPrimitiveMeshes();
}

void gVKRenderEngine::cleanup() {
	// The GL resources gRenderer::cleanup() would release were never created,
	// because init() skips gRenderer::init(). The primitive meshes and rendercolor
	// are what init() did allocate, so they are released here.
	destroyPrimitiveMeshes();
	cleanupVulkan();
	delete rendercolor;
	rendercolor = nullptr;
}

void gVKRenderEngine::drawColored2D(const glm::vec2* points, int count, const glm::vec4& color, const glm::mat4& mvp,
		int drawMode) {
#ifdef GVK_VULKAN
	flushQueuedDraws();
	if(vkcontext == nullptr) return;
	// The mesh path speaks in GL primitive constants; translate them into the modes
	// the Vulkan draw path expands. GL_POINTS has no pipeline of its own and no 2D
	// primitive asks for it, so such a mesh is skipped rather than drawn as
	// something else.
	int mode;
	switch(drawMode) {
	case GL_TRIANGLES: mode = GVK_DRAW2D_TRIANGLES; break;
	case GL_TRIANGLE_STRIP: mode = GVK_DRAW2D_TRIANGLESTRIP; break;
	case GL_TRIANGLE_FAN: mode = GVK_DRAW2D_TRIANGLEFAN; break;
	case GL_LINES: mode = GVK_DRAW2D_LINES; break;
	case GL_LINE_STRIP: mode = GVK_DRAW2D_LINESTRIP; break;
	case GL_LINE_LOOP: mode = GVK_DRAW2D_LINELOOP; break;
	default: return;
	}
	// The colour pipeline always blends, while OpenGL only blends when the app asked
	// for it - with blending off it writes the colour straight into the framebuffer.
	// Forcing alpha to 1 reproduces exactly that, since src * 1 + dst * 0 is the
	// source colour, and it avoids a second pipeline just for the disabled state.
	glm::vec4 drawcolor = color;
	if(islightingenabled) {
		bool hasenabledlight = false;
		for(gLight* light : scenelights) {
			if(light != nullptr && light->isEnabled()) {
				hasenabledlight = true;
				break;
			}
		}
		// OpenGL's mesh shader falls back to global ambient when lighting is on
		// but every scene light is disabled. Coloured 2D primitives use that same
		// mesh path there, so preserve the observable API behaviour in Vulkan.
		if(!hasenabledlight) {
			const glm::vec4 ambient = globalambientcolor.asVec4();
			drawcolor.r *= ambient.r;
			drawcolor.g *= ambient.g;
			drawcolor.b *= ambient.b;
		}
	}
	if(!isalphablendingenabled) drawcolor.a = 1.0f;
	gvkDrawColored2D(*vkcontext, points, count, drawcolor, mvp, mode,
			blendmode == BLENDMODE_ADDITIVE);
#endif
}

#ifdef GVK_VULKAN
void gVKRenderEngine::updateSceneUniforms() {
	// Rebuilt every frame rather than tracked for changes: it is one memcpy into
	// already mapped memory, and the OpenGL path's change tracking exists to avoid
	// glBufferSubData calls that have no equivalent here.
	gVKSceneUniforms uniforms{};
	uniforms.projection = projectionmatrix;
	uniforms.view = viewmatrix;
	uniforms.viewpos = glm::vec4(cameraposition, 1.0f);
	uniforms.globalambientcolor = globalambientcolor.asVec4();
	// Still filled in, but no shader reads it any more: renderColor reaches a mesh
	// through the push constant instead, because it has to be able to change between
	// two draws of the same pass and a uniform block cannot express that. The field
	// stays because both 3D shaders still declare it in this block, and the C++ side
	// has to match that layout byte for byte. Dropping it means editing the two
	// shaders as well; worth doing, not worth doing in the same change as the fix.
	uniforms.rendercolor = rendercolor != nullptr ? rendercolor->asVec4() : glm::vec4(1.0f);

	// The w component doubles as the "is a shadow map bound" flag the shader tests,
	// which keeps it out of the integer block below and its padding rules.
	const bool shadowsready = shadowmapenabled && vkcontext->hasShadowMap();
	uniforms.lightmatrix = shadowlightmatrix;
	uniforms.shadowlightpos = glm::vec4(shadowlightposition, shadowsready ? 1.0f : 0.0f);
	uniforms.softshadows = shadowsoft ? 1 : 0;

	// The same bits gRenderer::updateScene() builds for the OpenGL scene block, from
	// the same state, so a scene reads identically on both backends. SSAO's bit is
	// carried for completeness; nothing in this backend acts on it yet.
	uniforms.flags = 0;
	if(isssaoenabled) uniforms.flags |= ENABLE_SSAO;
	if(isfogenabled) uniforms.flags |= ENABLE_FOG;
	if(isgammacorrectionenabled) uniforms.flags |= ENABLE_GAMMA;
	if(ishdrenabled) uniforms.flags |= ENABLE_HDR;
	if(issoftshadowsenabled) uniforms.flags |= ENABLE_SOFT_SHADOWS;

	// Written whether or not fog is on: the shader only reads it behind the flag,
	// and writing it unconditionally keeps this free of a branch that would other-
	// wise leave stale values behind the one time fog is switched back on.
	const glm::vec3 fogrgb = fogcolor.asVec3();
	uniforms.fogcolor = glm::vec4(fogrgb, fogmode == FOGMODE_EXP ? 1.0f : 0.0f);
	uniforms.fogparams = glm::vec4(fogdensity, foggradient, foglinearstart, foglinearend);

	uniforms.lightnum = std::min((int) scenelights.size(), GVK_MAX_LIGHTS);
	uniforms.enabledlights = 0;
	for(int i = 0; i < uniforms.lightnum; i++) {
		gLight* light = scenelights[i];
		// The bitmask is what the shader tests, so a light that is off simply never
		// gets its bit set - matching gRenderer::updateLights on the OpenGL side,
		// where lighting being disabled globally switches every light off at once.
		if(islightingenabled && light->isEnabled()) uniforms.enabledlights |= (1 << i);

		gVKLightData& data = uniforms.lights[i];
		data.type = light->getType();
		data.position = light->getPosition();
		data.direction = light->getDirection();
		data.ambient = light->getAmbientColor()->asVec4();
		data.diffuse = light->getDiffuseColor()->asVec4();
		data.specular = light->getSpecularColor()->asVec4();
		data.constant = light->getAttenuationConstant();
		data.linear = light->getAttenuationLinear();
		data.quadratic = light->getAttenuationQuadratic();
		data.cutoff = light->getSpotCutOffAngle();
		data.outercutoff = light->getSpotOuterCutOffAngle();
	}

	if(!gvkWriteSceneUniforms(*vkcontext, uniforms)) return;
}
#endif

void gVKRenderEngine::drawMesh3D(GLuint vertexArrayId, int vertexCount, int indexCount,
		const glm::mat4& model, const gMeshSurface& surface, int drawMode, int instanceCount) {
#ifdef GVK_VULKAN
	if(vkcontext == nullptr || vertexArrayId == 0 || instanceCount <= 0) return;
	const bool shadowqueued = vkcontext->isShadowPassActive();
	// Snapshot camera and lighting when the first screen mesh arrives, not later
	// when a state change happens to flush the queue. Otherwise light/shadow changes
	// made by overlays (such as a muzzle flash) can recolour every mesh that was
	// queued before them.
	if(!shadowqueued && !sceneuniformswritten) {
		updateSceneUniforms();
		sceneuniformswritten = true;
	}
	if(!flushingqueueddraws && (shadowqueued || (instanceCount == 1 && !isalphablendingenabled
			&& boundframebuffer == gFbo::defaultfbo))) {
		QueuedMeshDraw queued;
		queued.vertexarrayid = vertexArrayId;
		queued.vertexcount = vertexCount;
		queued.indexcount = indexCount;
		queued.drawmode = drawMode;
		queued.instancecount = instanceCount;
		queued.model = model;
		queued.surface = surface;
		queued.tint = rendercolor != nullptr ? rendercolor->asVec4() : glm::vec4(1.0f);
		queued.depthtest = isdepthtestenabled;
		queued.depthtesttype = depthtesttype;
		queued.culling = iscullingenabled;
		queued.cullface = cullface;
		queued.cullingdirection = cullingdirection;
		queued.mergekey = gvkQueuedDrawKey(queued);
		queuedmeshdraws.push_back(queued);
		return;
	}

	// The vertex array is what gVbo bound while it uploaded, so it knows which two
	// buffers this mesh lives in.
	auto arrayentry = vkvertexarrays.find(vertexArrayId);
	if(arrayentry == vkvertexarrays.end()) return;

	// Resolved rather than read straight off the struct: a mesh that rewrites its
	// vertices every frame keeps one buffer per frame in flight, and this is what
	// picks the one belonging to the frame being recorded. A mesh uploaded once
	// hands back its single buffer and nothing else happens.
	gVKMeshBuffer* vertices = getMeshBuffer(arrayentry->second.vertexbuffer);
	if(vertices == nullptr) return;
	VkDeviceSize vertexoffset = 0;
	const VkBuffer vertexbuffer = gvkResolveMeshBuffer(*vkcontext, *vertices, vertexoffset);
	if(vertexbuffer == VK_NULL_HANDLE) return;
	gVKMeshBuffer* indices = getMeshBuffer(arrayentry->second.indexbuffer);
	// Indices are never rewritten per frame, so their offset is always zero.
	VkDeviceSize indexoffset = 0;
	const VkBuffer indexbuffer = indices != nullptr
			? gvkResolveMeshBuffer(*vkcontext, *indices, indexoffset) : VK_NULL_HANDLE;
	const bool indexed = indexbuffer != VK_NULL_HANDLE && indexCount > 0;

	// The mesh's draw mode picks both the pipeline (triangle or line) and the
	// topology set on the command buffer. Strips are native to Vulkan, so they are
	// drawn as they are rather than expanded - important here because a 3D mesh is
	// drawn straight from device memory, and expanding it would mean uploading a
	// second copy of it. gSphere is a triangle strip, so this is not a corner case.
	//
	// GL_TRIANGLE_FAN and GL_LINE_LOOP never reach this point: gMesh rewrites their
	// indices into a triangle list and a line strip before calling, because Vulkan
	// has no line loop at all and MoltenVK no triangle fan. Reaching the default
	// below therefore means a mode nothing in the engine produces.
	bool lines;
	VkPrimitiveTopology topology;
	switch(drawMode) {
	case GL_TRIANGLES:      lines = false; topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST; break;
	case GL_TRIANGLE_STRIP: lines = false; topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP; break;
	case GL_LINES:          lines = true;  topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST; break;
	case GL_LINE_STRIP:     lines = true;  topology = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP; break;
	default: return;
	}

	// The scene block is per frame, but a frame is only known to have 3D in it once
	// a mesh actually arrives, so the first such draw is what fills it in.
	// The shadow pass draws the same meshes with a different pipeline and nothing
	// but their position: no material, no lights, no descriptor sets. Handled before
	// any of that is gathered, so the depth pass costs a fraction of a shading one.
	// This has to come before the PBR branch below, not after: a PBR mesh returns
	// there, so leaving this second meant such a mesh never reached the depth pass
	// and cast no shadow at all. Nothing failed loudly either - the PBR draw simply
	// found the shadow pass open and dropped itself in gvkEnsureRenderPass.
	if(vkcontext->isShadowPassActive()) {
		// Line geometry is skipped rather than drawn into the map. There is only a
		// triangle shadow pipeline, and Vulkan will not let a line topology be set on
		// it - the topologies have to stay in the same class. Building a second
		// pipeline for it would buy nothing either: a wireframe or a grid casting a
		// one-pixel-wide shadow is not what any of this is for.
		if(lines) return;

		VkBuffer shadowinstances = VK_NULL_HANDLE;
		VkDeviceSize shadowinstancesoffset = 0;
		if(instanceCount > 1) {
			gVKMeshBuffer* instances = getMeshBuffer(arrayentry->second.instancebuffer);
			if(instances == nullptr) return;
			shadowinstances = gvkResolveMeshBuffer(*vkcontext, *instances, shadowinstancesoffset);
			if(shadowinstances == VK_NULL_HANDLE) return;
		} else {
			if(!ensureIdentityInstanceBuffer()) return;
			shadowinstances = identityinstancebuffer->buffer;
			shadowinstancesoffset = 0;
		}
		gVKShadowPush shadowpush{};
		// The light's matrix is folded into the model matrix on the CPU; see
		// shadow3d.vert for why the three are not sent separately.
		shadowpush.lightmodel = shadowlightmatrix * model;

		const VkIndexType shadowindextype = sizeof(gIndex) == 2 ? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32;
		gvkDrawShadowCaster(*vkcontext, vertexbuffer, vertexoffset,
				indexed ? indexbuffer : VK_NULL_HANDLE,
				indexed ? indexCount : vertexCount, shadowindextype, shadowpush, VK_NULL_HANDLE,
				shadowinstances, shadowinstancesoffset, instanceCount, topology);
		return;
	}

	// The renderer's culling state, translated once for whichever of the two 3D
	// pipelines records this mesh. Both are built with the cull mode and the front
	// face dynamic, so every draw has to set them. See gVKCullState for why the
	// front face comes out as the opposite of the OpenGL one.
	gVKCullState cullstate;
	if(iscullingenabled) {
		if(cullface == GL_FRONT) cullstate.mode = VK_CULL_MODE_FRONT_BIT;
		else if(cullface == GL_FRONT_AND_BACK) cullstate.mode = VK_CULL_MODE_FRONT_AND_BACK;
		else cullstate.mode = VK_CULL_MODE_BACK_BIT;
	}
	cullstate.frontface = cullingdirection == GL_CW
			? VK_FRONT_FACE_COUNTER_CLOCKWISE : VK_FRONT_FACE_CLOCKWISE;

	// The material colours are filled in for the PBR branch below, which takes them
	// as they are. The non-PBR path overwrites all three further down, where
	// renderColor gets folded into them.
	gVKMeshPush push{};
	push.model = model;
	push.ambient = surface.ambient;
	push.diffuse = surface.diffuse;
	push.specular = surface.specular;
	// PBR takes a separate pipeline and a separate shader, so it branches out before
	// the non-PBR material work below.
	if(surface.ispbr) {
		if(!ensureWhiteTexture()) return;
		VkDescriptorSet materialset = gvkGetPbrMaterialSet(vkcontext, vktextures, whitetextureid, surface);
		if(materialset == VK_NULL_HANDLE) return;

		// Set 2, and it has to point at a real descriptor whether or not a shadow map
		// exists. The white texture reads as "nothing was ever closer to the light",
		// so it casts no shadow even before the shader's flag says not to look.
		VkDescriptorSet pbrshadowset = VK_NULL_HANDLE;
		if(vkcontext->hasShadowMap()) pbrshadowset = vkcontext->getShadowDescriptorSet();
		if(pbrshadowset == VK_NULL_HANDLE) pbrshadowset = vktextures[whitetextureid]->descriptorset;

		gVKPbrPush pbrpush{};
		pbrpush.model = model;
		pbrpush.maps0 = glm::ivec4(surface.albedomapid != 0 ? 1 : 0, surface.pbrnormalmapid != 0 ? 1 : 0,
				surface.metallicmapid != 0 ? 1 : 0, surface.roughnessmapid != 0 ? 1 : 0);
		pbrpush.maps1 = glm::ivec4(surface.aomapid != 0 ? 1 : 0, 0, 0, 0);

		VkBuffer pbrinstances = VK_NULL_HANDLE;
		VkDeviceSize pbrinstancesoffset = 0;
		if(instanceCount > 1) {
			gVKMeshBuffer* instances = getMeshBuffer(arrayentry->second.instancebuffer);
			if(instances == nullptr) return;
			pbrinstances = gvkResolveMeshBuffer(*vkcontext, *instances, pbrinstancesoffset);
			if(pbrinstances == VK_NULL_HANDLE) return;
		} else {
			if(!ensureIdentityInstanceBuffer()) return;
			pbrinstances = identityinstancebuffer->buffer;
			pbrinstancesoffset = 0;
		}

		const VkIndexType pbrindextype = sizeof(gIndex) == 2 ? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32;
		gvkDrawMesh3DPbr(*vkcontext, vertexbuffer, vertexoffset, indexed ? indexbuffer : VK_NULL_HANDLE,
				indexed ? indexCount : vertexCount, pbrindextype, pbrpush, materialset,
				pbrshadowset,
				pbrinstances, pbrinstancesoffset, instanceCount, topology,
				isdepthtestenabled, depthtesttype == DEPTHTESTTYPE_ALWAYS, cullstate,
				isalphablendingenabled);
		return;
	}

	// A map only counts once its texture is actually registered here; a material can
	// name one that never made it through texImage2D (an unsupported format, say),
	// and sampling white is closer to the OpenGL result than sampling nothing.
	const GLuint diffusemapid = gvkRegisteredTextureId(vktextures, surface.diffusemapid);
	const GLuint specularmapid = gvkRegisteredTextureId(vktextures, surface.specularmapid);
	const GLuint normalmapid = gvkRegisteredTextureId(vktextures, surface.normalmapid);

	push.misc = glm::vec4(surface.shininess,
			diffusemapid != 0 ? 1.0f : 0.0f,
			specularmapid != 0 ? 1.0f : 0.0f,
			normalmapid != 0 ? 1.0f : 0.0f);

	// renderColor is folded into the material here rather than read from the scene
	// block, and that is what makes it per draw. The OpenGL shader multiplies its
	// finished result by it, and gRenderer::setColor republishes it the moment it
	// changes, so a canvas that recolours between two meshes - fading one enemy out
	// while the next stays opaque - gets a different colour on each. A uniform block
	// cannot express that: it is read when the commands execute, not when they are
	// recorded, so every draw in the frame would see whichever value was written
	// last.
	//
	// Folding it in is exact rather than an approximation: every term the shader
	// sums is linear in these three colours, so scaling all three scales the result
	// by the same factor. Where a colour comes from a texture instead, the matching
	// push slot is unused by that branch, so it carries the colour on its own and
	// the shader multiplies the sampled value by it.
	const glm::vec4 tint = rendercolor != nullptr ? rendercolor->asVec4() : glm::vec4(1.0f);
	push.ambient = diffusemapid != 0 ? tint : surface.ambient * tint;
	push.diffuse = diffusemapid != 0 ? tint : surface.diffuse * tint;
	push.specular = specularmapid != 0 ? tint : surface.specular * tint;

	// Every binding must point at a real descriptor even when the flags say not to
	// sample it, so the unused ones fall back to the 1x1 white texture.
	if(!ensureWhiteTexture()) return;
	VkDescriptorSet whiteset = vktextures[whitetextureid]->descriptorset;

	// The three maps travel as one set (set 1), the way the PBR path's five do, so
	// this pipeline binds three sets rather than five and stays inside the four
	// Vulkan guarantees - which is exactly four on Adreno and Mali. Cached by the
	// combination of ids, because materials share maps far more often than each one
	// is unique.
	VkDescriptorSet materialset = gvkGetMaterialSet(vkcontext, vktextures, whitetextureid,
			diffusemapid, specularmapid, normalmapid);
	if(materialset == VK_NULL_HANDLE) return;

	// Shadow map, bound as set 2. Falls back to the white texture when there is
	// none, which reads as "nothing was ever closer to the light" and so casts no
	// shadow - the shader's flag says not to look at it anyway.
	VkDescriptorSet shadowset = VK_NULL_HANDLE;
	if(vkcontext->hasShadowMap()) shadowset = vkcontext->getShadowDescriptorSet();
	if(shadowset == VK_NULL_HANDLE) shadowset = whiteset;
	// No alpha fixup here, unlike the 2D path: the 3D pipeline does not blend at all
	// (see gVKPipeline.cpp), so what the lighting sum leaves in the alpha channel
	// never reaches the framebuffer as transparency.

	// The instance buffer is always bound, because the shader always reads a model
	// matrix from it. An instanced draw uses the one gVbo uploaded; anything else
	// gets the shared identity, which multiplies out to no change at all.
	VkBuffer instancebuffer = VK_NULL_HANDLE;
	VkDeviceSize instancebufferoffset = 0;
	if(instanceCount > 1) {
		gVKMeshBuffer* instances = getMeshBuffer(arrayentry->second.instancebuffer);
		if(instances == nullptr) return;
		instancebuffer = gvkResolveMeshBuffer(*vkcontext, *instances, instancebufferoffset);
		if(instancebuffer == VK_NULL_HANDLE) return;
	} else {
		if(!ensureIdentityInstanceBuffer()) return;
		instancebuffer = identityinstancebuffer->buffer;
		instancebufferoffset = 0;
	}

	// gIndex is what gVbo uploaded, so the index type follows it: 16 bit on Android,
	// 32 bit everywhere else.
	const VkIndexType indextype = sizeof(gIndex) == 2 ? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32;

	// Whether the shader's cutout test can fire at all for this draw. Only a sampled
	// diffuse map reaches that branch, and the alpha of its texels was recorded when
	// the pixels were uploaded, so this is a fact about the image rather than a guess:
	// a material with no diffuse map, or one whose map is fully opaque - most of them -
	// goes through the pipeline that keeps early depth rejection.
	bool cutout = false;
	if(diffusemapid != 0) {
		auto diffuseit = vktextures.find(diffusemapid);
		cutout = diffuseit != vktextures.end() && diffuseit->second != nullptr
				&& diffuseit->second->hascutout;
	}

	gvkDrawMesh3D(*vkcontext, vertexbuffer, vertexoffset, indexed ? indexbuffer : VK_NULL_HANDLE,
			indexed ? indexCount : vertexCount, indextype, push, materialset, shadowset,
			instancebuffer, instancebufferoffset, instanceCount,
			topology, isdepthtestenabled, depthtesttype == DEPTHTESTTYPE_ALWAYS, lines, cullstate,
			isalphablendingenabled, cutout);
#endif
}

void gVKRenderEngine::drawTexturedRect2D(GLuint textureId, GLuint maskTextureId, const glm::vec4& tint,
		const glm::mat4& mvp, const glm::vec2& uvOffset, const glm::vec2& uvScale) {
#ifdef GVK_VULKAN
	flushQueuedDraws();
	if(vkcontext == nullptr) return;
	auto it = vktextures.find(textureId);
	if(it == vktextures.end() || it->second == nullptr) return;
	it->second->sampled = true;
	VkDescriptorSet maskset = VK_NULL_HANDLE;
	if(maskTextureId != 0) {
		auto maskit = vktextures.find(maskTextureId);
		if(maskit != vktextures.end() && maskit->second != nullptr) {
			maskit->second->sampled = true;
			maskset = maskit->second->descriptorset;
		}
	}
	gvkDrawTextured2D(*vkcontext, it->second->descriptorset, maskset, tint, mvp, uvOffset, uvScale,
			blendmode == BLENDMODE_ADDITIVE);
#endif
}

void gVKRenderEngine::drawTexturedTriangles2D(GLuint textureId, const glm::vec4& tint,
		const glm::mat4& mvp, const float* xyuv, int vertexCount) {
#ifdef GVK_VULKAN
	flushQueuedDraws();
	if(vkcontext == nullptr) return;
	auto it = vktextures.find(textureId);
	if(it == vktextures.end() || it->second == nullptr) return;
	it->second->sampled = true;
	gvkDrawTexturedTriangles2D(*vkcontext, it->second->descriptorset, tint, mvp, xyuv, vertexCount,
			blendmode == BLENDMODE_ADDITIVE);
#endif
}

gVKTexture* gVKRenderEngine::getBoundVKTexture() {
#ifdef GVK_VULKAN
	if(vkcontext == nullptr || boundtextureid == 0) return nullptr;
	auto it = vktextures.find(boundtextureid);
	return it == vktextures.end() ? nullptr : it->second;
#else
	return nullptr;
#endif
}

void gVKRenderEngine::destroyAllTextures() {
#ifdef GVK_VULKAN
	if(vkcontext == nullptr) return;
	// The framebuffers first: they hold render passes built around these textures'
	// views, and a pass outliving its attachment is what turns a clean shutdown into
	// a validation error.
	destroyAllFramebuffers();
	// Each of these owns one device memory allocation, so the count reads directly
	// against the limit logged when the device was selected.
	gLogi("gVKRenderEngine") << "Textures released at shutdown: " << vktextures.size();
	for(auto& entry : vktextures) gvkDestroyTexture(*vkcontext, entry.second);
	vktextures.clear();
#endif
}

bool gVKRenderEngine::ensureWhiteTexture() {
#ifdef GVK_VULKAN
	if(whitetextureid != 0) return true;
	if(vkcontext == nullptr) return false;

	// Built through the ordinary texture entry points so it gets an image, a sampler
	// and a descriptor set the same way every other texture does.
	const GLuint previousbound = boundtextureid;
	GLuint id = createTextures();
	const unsigned char white[4] = {255, 255, 255, 255};
	texImage2D(GL_TEXTURE_2D, GL_RGBA, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE,
			const_cast<unsigned char*>(white));
	boundtextureid = previousbound;

	auto it = vktextures.find(id);
	if(it == vktextures.end() || it->second == nullptr) {
		gLoge("gVKRenderEngine") << "Could not create the 1x1 white fallback texture.";
		return false;
	}
	whitetextureid = id;
	return true;
#else
	return false;
#endif
}

#ifdef GVK_VULKAN
// Kept out of the class because its return type is a Vulkan handle and
// gVKRenderEngine.h is deliberately free of Vulkan headers.
// The texture id a material named, or 0 when nothing was ever registered under it.
// A material can name a texture that never made it through texImage2D - an
// unsupported format, say - and sampling white is closer to the OpenGL result than
// sampling nothing. Collapsing the two cases to 0 here means the material cache
// key, the shader's "has this map" flag and the descriptor write all agree.
static GLuint gvkRegisteredTextureId(std::unordered_map<GLuint, gVKTexture*>& vktextures, GLuint id) {
	if(id == 0) return 0;
	auto it = vktextures.find(id);
	if(it == vktextures.end() || it->second == nullptr) return 0;
	return id;
}

// Allocates one descriptor set holding `count` combined image samplers, in binding
// order, with the 1x1 white texture standing in wherever the material named no map.
// Every binding is written whether or not it is sampled: the shader names all of
// them, so an unwritten one would be invalid, and the flags in the push constant
// are what decide which are actually read.
static VkDescriptorSet gvkWriteMaterialSet(gVKContext* vkcontext,
		std::unordered_map<GLuint, gVKTexture*>& vktextures, GLuint whitetextureid,
		VkDescriptorSetLayout layout, const uint32_t* ids, uint32_t count, const char* what) {
	VkDevice device = *vkcontext->getDevice();
	VkDescriptorSetAllocateInfo allocinfo{};
	allocinfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocinfo.descriptorPool = vkcontext->getDescriptorPool();
	allocinfo.descriptorSetCount = 1;
	allocinfo.pSetLayouts = &layout;
	VkDescriptorSet set = VK_NULL_HANDLE;
	if(vkAllocateDescriptorSets(device, &allocinfo, &set) != VK_SUCCESS) {
		gLoge("gVKRenderEngine") << "Could not allocate a " << what << " material descriptor set.";
		return VK_NULL_HANDLE;
	}

	gVKTexture* white = vktextures[whitetextureid];
	VkDescriptorImageInfo images[5]{};
	VkWriteDescriptorSet writes[5]{};
	if(count > 5) count = 5;
	for(uint32_t i = 0; i < count; i++) {
		gVKTexture* tex = nullptr;
		auto it = vktextures.find(ids[i]);
		if(ids[i] != 0 && it != vktextures.end()) tex = it->second;
		if(tex == nullptr) tex = white;

		images[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		images[i].imageView = tex->view;
		images[i].sampler = tex->sampler;

		writes[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writes[i].dstSet = set;
		writes[i].dstBinding = i;
		writes[i].dstArrayElement = 0;
		writes[i].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		writes[i].descriptorCount = 1;
		writes[i].pImageInfo = &images[i];
	}
	vkUpdateDescriptorSets(device, count, writes, 0, nullptr);
	return set;
}

// A cached material set holds the image view and sampler of every texture it was
// written from, so it outlives them unless the cache is told. Both paths that end a
// texture come through here: a plain delete, and the replace that a growing glyph
// atlas or a resized render target performs - that one keeps the id and swaps the
// image underneath it, which would leave the cached set pointing at a destroyed
// view. Freeing the sets also returns them to the descriptor pool, which is what
// keeps a long session of loading and unloading from draining it.
static void gvkReleaseMaterialSetsUsingTexture(gVKContext* vkcontext, GLuint textureid) {
	if(vkcontext == nullptr || textureid == 0) return;
	VkDevice device = *vkcontext->getDevice();
	VkDescriptorPool pool = vkcontext->getDescriptorPool();
	if(device == VK_NULL_HANDLE || pool == VK_NULL_HANDLE) return;

	const auto uses = [textureid](const auto& key) {
		return std::find(key.begin(), key.end(), static_cast<uint32_t>(textureid)) != key.end();
	};
	auto& pbrsets = *vkcontext->getPbrMaterialSets();
	for(auto it = pbrsets.begin(); it != pbrsets.end(); ) {
		if(!uses(it->first)) { ++it; continue; }
		if(it->second != VK_NULL_HANDLE) vkFreeDescriptorSets(device, pool, 1, &it->second);
		it = pbrsets.erase(it);
	}
	auto& sets = *vkcontext->getMaterialSets();
	for(auto it = sets.begin(); it != sets.end(); ) {
		if(!uses(it->first)) { ++it; continue; }
		if(it->second != VK_NULL_HANDLE) vkFreeDescriptorSets(device, pool, 1, &it->second);
		it = sets.erase(it);
	}
}

static VkDescriptorSet gvkGetPbrMaterialSet(gVKContext* vkcontext,
		std::unordered_map<GLuint, gVKTexture*>& vktextures, GLuint whitetextureid,
		const gRenderer::gMeshSurface& surface) {
	if(vkcontext == nullptr || whitetextureid == 0) return VK_NULL_HANDLE;
	VkDescriptorSetLayout layout = vkcontext->getMesh3DPbrMaterialSetLayout();
	if(layout == VK_NULL_HANDLE || vkcontext->getDescriptorPool() == VK_NULL_HANDLE) return VK_NULL_HANDLE;

	const std::array<uint32_t, 5> key = {surface.albedomapid, surface.pbrnormalmapid,
			surface.metallicmapid, surface.roughnessmapid, surface.aomapid};
	std::map<std::array<uint32_t, 5>, VkDescriptorSet>& cache = *vkcontext->getPbrMaterialSets();
	auto cached = cache.find(key);
	if(cached != cache.end()) return cached->second;

	VkDescriptorSet set = gvkWriteMaterialSet(vkcontext, vktextures, whitetextureid, layout,
			key.data(), 5, "PBR");
	if(set == VK_NULL_HANDLE) return VK_NULL_HANDLE;
	cache[key] = set;
	return set;
}

// The classic path's diffuse/specular/normal trio, cached the same way. The ids
// have already been through gvkRegisteredTextureId, so 0 means "no map" and the
// white texture fills that binding.
static VkDescriptorSet gvkGetMaterialSet(gVKContext* vkcontext,
		std::unordered_map<GLuint, gVKTexture*>& vktextures, GLuint whitetextureid,
		GLuint diffusemapid, GLuint specularmapid, GLuint normalmapid) {
	if(vkcontext == nullptr || whitetextureid == 0) return VK_NULL_HANDLE;
	VkDescriptorSetLayout layout = vkcontext->getMesh3DMaterialSetLayout();
	if(layout == VK_NULL_HANDLE || vkcontext->getDescriptorPool() == VK_NULL_HANDLE) return VK_NULL_HANDLE;

	const std::array<uint32_t, 3> key = {diffusemapid, specularmapid, normalmapid};
	std::map<std::array<uint32_t, 3>, VkDescriptorSet>& cache = *vkcontext->getMaterialSets();
	auto cached = cache.find(key);
	if(cached != cache.end()) return cached->second;

	VkDescriptorSet set = gvkWriteMaterialSet(vkcontext, vktextures, whitetextureid, layout,
			key.data(), 3, "mesh");
	if(set == VK_NULL_HANDLE) return VK_NULL_HANDLE;
	cache[key] = set;
	return set;
}
#endif

bool gVKRenderEngine::drawSkyboxFace(GLuint textureId, const float* xyzuv, int vertexCount,
		const glm::mat4& viewProjection) {
#ifdef GVK_VULKAN
	flushQueuedDraws();
	if(vkcontext == nullptr || textureId == 0 || xyzuv == nullptr || vertexCount <= 0) return false;
	// The sky is not a caster. Drawing it into the depth map would put a wall at the
	// far edge of the light's frustum and shadow the whole scene.
	if(vkcontext->isShadowPassActive()) return true;

	auto it = vktextures.find(textureId);
	if(it == vktextures.end() || it->second == nullptr) return false;

	// LESS_OR_EQUAL rather than LESS: the sky is drawn at the far plane and has to
	// survive a depth value equal to what is already there, which is the same reason
	// the OpenGL path switches to GL_LEQUAL for it.
	gvkDrawSkyboxFace(*vkcontext, it->second->descriptorset, xyzuv, vertexCount,
			viewProjection, VK_COMPARE_OP_LESS_OR_EQUAL);
	return true;
#else
	return false;
#endif
}

bool gVKRenderEngine::allocateShadowMap(int width, int height) {
#ifdef GVK_VULKAN
	if(vkcontext == nullptr || width <= 0 || height <= 0) return false;
	// The map outlives individual frames, so anything still reading the old one has
	// to be finished before it is replaced.
	vkDeviceWaitIdle(*vkcontext->getDevice());
	return gvkCreateShadowResources(*vkcontext,
			static_cast<uint32_t>(width), static_cast<uint32_t>(height));
#else
	return false;
#endif
}

void gVKRenderEngine::releaseShadowMap() {
#ifdef GVK_VULKAN
	if(vkcontext == nullptr) return;
	vkDeviceWaitIdle(*vkcontext->getDevice());
	gvkDestroyShadowResources(*vkcontext);
	shadowmapenabled = false;
#endif
}

bool gVKRenderEngine::beginShadowPass() {
#ifdef GVK_VULKAN
	if(vkcontext == nullptr) return false;
	// Deliberately does not gather the scene uniforms. This runs before the canvas
	// has drawn anything, so gCamera::begin() has not set the view and projection
	// yet - writing the block here would freeze last frame's camera into it, and
	// because the block is written once per frame the shading pass would then
	// inherit it and place the whole scene wrongly. The shadow pipeline reads no
	// descriptors at all, so it has nothing to gain from an early write; the first
	// mesh of the shading pass fills the block, by which time the camera is set.
	return gvkBeginShadowPass(*vkcontext);
#else
	return false;
#endif
}

void gVKRenderEngine::updateLights() {
	flushQueuedDraws();
	gRenderer::updateLights();
#ifdef GVK_VULKAN
	// A light was enabled, disabled or recoloured. The scene block this backend
	// hands the shaders holds that state, and it is gathered once and then left
	// alone for the rest of the pass, so it has to be dropped here or the change
	// only lands on the frame after next.
	//
	// This is what a canvas doing the ordinary thing runs into: enable the light,
	// draw the scene, disable it again. Without this the first draw of the pass
	// captures the state from before the enable - the light off - and every mesh in
	// that pass is shaded by the global ambient fallback instead, which comes out
	// far brighter than the light would have made it.
	sceneuniformswritten = false;
#endif
}

void gVKRenderEngine::endShadowPass() {
#ifdef GVK_VULKAN
	flushQueuedDraws();
	if(vkcontext == nullptr) return;
	gvkEndShadowPass(*vkcontext);
	// The scene block is gathered again for the pass that follows, rather than once
	// for the whole frame. It carries renderColor, which the OpenGL shader applies
	// to every mesh and which gRenderer::setColor republishes on the spot - so on
	// that backend the screen pass sees whatever colour was set during the shadow
	// pass, including one left behind by a canvas that draws text. Writing this
	// once per frame meant Vulkan kept the value from before that, and the two
	// backends shaded the same scene through different colours.
	//
	// Safe to rewrite here even though the shadow pass has already recorded its
	// draws: a uniform buffer is read when the commands execute, not when they are
	// recorded, but the depth-only pipeline binds no scene set at all - its only
	// descriptor is the caster's diffuse map - so nothing recorded so far reads it.
	sceneuniformswritten = false;
#endif
}

bool gVKRenderEngine::isShadowPassActive() const {
#ifdef GVK_VULKAN
	return vkcontext != nullptr && vkcontext->isShadowPassActive();
#else
	return false;
#endif
}

void gVKRenderEngine::setShadowMapState(bool enabled, const glm::mat4& lightMatrix,
		const glm::vec3& lightPosition, bool softShadows) {
	flushQueuedDraws();
	shadowmapenabled = enabled;
	// The same -1..1 to 0..1 depth correction setProjectionMatrix applies to the
	// camera. gShadowMap builds the light's projection with glm::ortho, which targets
	// OpenGL's clip volume, and it never passes through setProjectionMatrix - so
	// without this the depth pass writes values outside what Vulkan keeps and the
	// shading pass compares against the wrong range. The result is a shadow map that
	// is technically produced and visibly does nothing.
	glm::mat4 clip(1.0f);
	clip[2][2] = 0.5f;
	clip[3][2] = 0.5f;
	shadowlightmatrix = clip * lightMatrix;
	shadowlightposition = lightPosition;
	shadowsoft = softShadows;
}

bool gVKRenderEngine::ensureIdentityInstanceBuffer() {
#ifdef GVK_VULKAN
	if(identityinstancebuffer != nullptr && identityinstancebuffer->buffer != VK_NULL_HANDLE) return true;
	if(vkcontext == nullptr) return false;

	if(identityinstancebuffer == nullptr) identityinstancebuffer = new gVKMeshBuffer();
	const glm::mat4 identity(1.0f);
	if(!gvkUploadMeshBuffer(*vkcontext, *identityinstancebuffer, &identity, sizeof(identity), false)) {
		gLoge("gVKRenderEngine") << "Could not create the identity instance buffer.";
		return false;
	}
	return true;
#else
	return false;
#endif
}

void gVKRenderEngine::destroyAllMeshBuffers() {
#ifdef GVK_VULKAN
	if(vkcontext == nullptr) return;
	// Called from cleanupVulkan after vkDeviceWaitIdle, so nothing is still reading
	// these; the per-buffer wait in deleteBuffer would be redundant here.
	size_t live = 0;
	VkDeviceSize bytes = 0;
	for(auto& entry : vkmeshbuffers) {
		if(entry.second->buffer != VK_NULL_HANDLE) {
			live++;
			bytes += entry.second->size;
		}
		gvkDestroyMeshBuffer(*vkcontext, *entry.second);
		delete entry.second;
	}
	// Whatever is still here at shutdown was never released by its gVbo. That is not
	// an error - the engine tears objects down in its own order - but the count is
	// what tells you the buffer path ran at all, and how much it was holding.
	gLogi("gVKRenderEngine") << "Mesh buffers released at shutdown: " << live
			<< " (" << bytes << " bytes), across " << vkvertexarrays.size() << " vertex arrays";
	// The device local buffers that promotions replaced, held until now because the
	// frame being recorded at the time may still have referenced them.
	gvkDestroyRetiredMeshBuffers(*vkcontext);
	if(identityinstancebuffer != nullptr) {
		gvkDestroyMeshBuffer(*vkcontext, *identityinstancebuffer);
		delete identityinstancebuffer;
		identityinstancebuffer = nullptr;
	}
	vkmeshbuffers.clear();
	vkvertexarrays.clear();
	boundvaoid = 0;
	boundarraybufferid = 0;
	boundelementbufferid = 0;
#endif
}

void gVKRenderEngine::updatePackUnpackAlignment(int i) {
	G_CHECK_GL(glPixelStorei(GL_UNPACK_ALIGNMENT, i));
	G_CHECK_GL(glPixelStorei(GL_PACK_ALIGNMENT, i));
}
