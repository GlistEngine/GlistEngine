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
// GVK_DESKTOP_GLFW accordingly and holds the shared state of the backend.
#include "gVKContext.h"

#ifdef GVK_DESKTOP_GLFW
	// GLFW_INCLUDE_VULKAN has to be defined before GLFW is pulled in, otherwise
	// glfwCreateWindowSurface and glfwGetRequiredInstanceExtensions stay hidden.
	#define GLFW_INCLUDE_VULKAN
	#include "gGLFWWindow.h"
	#include "gAppManager.h"
	#include "gVKSwapchain.h"
	#include "gVKRenderTarget.h"
	#include "gVKCommands.h"
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
	#include <vector>
	#include <set>
	#include <string>
	#include <cstring>
	#include <cstdlib>
#endif

#ifdef GVK_DESKTOP_GLFW
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
	// The render pass clears the attachment at the start of every frame, so there
	// is nothing to do at the moment this is called.
}

void gVKRenderEngine::clearColor(int r, int g, int b, int a) {
#ifdef GVK_DESKTOP_GLFW
	gvkStoreClearColor(vkcontext, r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);
#endif
}

void gVKRenderEngine::clearColor(gColor color) {
#ifdef GVK_DESKTOP_GLFW
	gvkStoreClearColor(vkcontext, color.r, color.g, color.b, color.a);
#endif
}

void gVKRenderEngine::setProjectionMatrix(glm::mat4 projectionMatrix) {
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
	isdepthtestenabled = true;
	depthtesttype = depthTestType;
}

void gVKRenderEngine::setDepthTestFunc(int depthTestType) {
	depthtesttype = depthTestType;
}

void gVKRenderEngine::disableDepthTest() {
	isdepthtestenabled = false;
}

bool gVKRenderEngine::isDepthTestEnabled() {
	return isdepthtestenabled;
}

int gVKRenderEngine::getDepthTestType() {
	return depthtesttype;
}

void gVKRenderEngine::enableAlphaBlending() {
	isalphablendingenabled = true;
}

void gVKRenderEngine::disableAlphaBlending() {
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
	unsigned char* pixeldata = new unsigned char[width * height * 4];
	G_CHECK_GL(glReadPixels(x, getHeight() - y - height, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixeldata));
	flipVertically(pixeldata, width, height, 4);
	img.setImageData(pixeldata, width, height, 4);
	//std::string imagePath = "output.png";   USE IT TO SAVE THE IMAGE
	// screenShot->saveImage(imagePath);  USE IT TO SAVE THE IMAGE
}

void gVKRenderEngine::takeScreenshot(gImage& img) {
	G_PROFILE_ZONE_SCOPED_N("gVKRenderEngine::takeScreenshot()");
	int height = gBaseApp::getAppManager()->getWindow()->getHeight();
	int width = gBaseApp::getAppManager()->getWindow()->getWidth();
	unsigned char* pixeldata = new unsigned char[width * height * 4];
	G_CHECK_GL(glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixeldata));
	flipVertically(pixeldata, width, height, 4);
	img.setImageData(pixeldata, width, height, 4);
	//std::string imagePath = "output.png";   USE IT TO SAVE THE IMAGE
	// screenShot->saveImage(imagePath);  USE IT TO SAVE THE IMAGE
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
	if(vkcontext == nullptr) return 0;
	// The entry is created empty; the VkBuffer appears on the first upload, because
	// only then is the size and the vertex/index role known.
	GLuint id = nextvkbufferid++;
	vkmeshbuffers[id] = new gVKMeshBuffer();
	return id;
}

gVKMeshBuffer* gVKRenderEngine::getMeshBuffer(GLuint id) {
	if(id == 0) return nullptr;
	auto it = vkmeshbuffers.find(id);
	return it == vkmeshbuffers.end() ? nullptr : it->second;
}

void gVKRenderEngine::deleteBuffer(GLuint& buffer) {
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
}

void gVKRenderEngine::setIndexBufferData(GLuint ebo, size_t size, const void* data, int usage) {
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

// ----- Framebuffer -----
GLuint gVKRenderEngine::createFramebuffer() {
	GLuint fbo;
	G_CHECK_GL(glGenFramebuffers(1, &fbo));
	return fbo;
}

void gVKRenderEngine::deleteFramebuffer(GLuint& fbo) {
	if(fbo != 0) {
		G_CHECK_GL(glDeleteFramebuffers(1, &fbo));
	}
}

void gVKRenderEngine::bindFramebuffer(GLuint fbo) {
	G_CHECK_GL(glBindFramebuffer(GL_FRAMEBUFFER, fbo));
	boundframebuffer = fbo;
}

void gVKRenderEngine::checkFramebufferStatus() {
	// check if fbo complete
	G_CHECK_GL2(GLuint status, glCheckFramebufferStatus(GL_FRAMEBUFFER));
	if(status != GL_FRAMEBUFFER_COMPLETE) {
		gLogi("gFbo") << "Framebuffer is not complete! status:" << gToHex(status, 4);
	}
}

// ----- Renderbuffer -----
GLuint gVKRenderEngine::createRenderbuffer() {
	GLuint rbo;
	G_CHECK_GL(glGenRenderbuffers(1, &rbo));
	return rbo;
}

void gVKRenderEngine::deleteRenderbuffer(GLuint& rbo) {
	if(rbo != 0) {
		G_CHECK_GL(glDeleteRenderbuffers(1, &rbo));
	}
}

void gVKRenderEngine::bindRenderbuffer(GLuint rbo) {
	assert(rbo != 0);
	G_CHECK_GL(glBindRenderbuffer(GL_RENDERBUFFER, rbo));
}

void gVKRenderEngine::setRenderbufferStorage(GLenum format, int width, int height) {
	G_CHECK_GL(glRenderbufferStorage(GL_RENDERBUFFER, format, width, height));
}

// ----- Attachments -----
void gVKRenderEngine::attachTextureToFramebuffer(GLenum attachment, GLenum textarget, GLuint texId, GLuint level) {
	G_CHECK_GL(glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, textarget, texId, level));
}

void gVKRenderEngine::attachRenderbufferToFramebuffer(GLenum attachment, GLuint rbo) {
	G_CHECK_GL(glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachment, GL_RENDERBUFFER, rbo));
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

GLuint gVKRenderEngine::loadProgram(const char* vertexSource, const char* fragmentSource, const char* geometrySource) {
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
	G_CHECK_GL(glUniform1i(uniformloc, (int)value));
}

void gVKRenderEngine::setInt(GLuint uniformloc, int value) {
	G_CHECK_GL(glUniform1i(uniformloc, value));
}

void gVKRenderEngine::setUnsignedInt(GLuint uniformloc, unsigned int value) {
	G_CHECK_GL(glUniform1ui(uniformloc, value));
}

void gVKRenderEngine::setFloat(GLuint uniformloc, float value) {
	G_CHECK_GL(glUniform1f(uniformloc, value));
}

void gVKRenderEngine::setVec2(GLuint uniformloc, const glm::vec2& value) {
	G_CHECK_GL(glUniform2fv(uniformloc, 1, &value[0]));
}

void gVKRenderEngine::setVec2(GLuint uniformloc, float x, float y) {
	G_CHECK_GL(glUniform2f(uniformloc, x, y));
}

void gVKRenderEngine::setVec3(GLuint uniformloc, const glm::vec3& value) {
	G_CHECK_GL(glUniform3fv(uniformloc, 1, &value[0]));
}

void gVKRenderEngine::setVec3(GLuint uniformloc, float x, float y, float z) {
	G_CHECK_GL(glUniform3f(uniformloc, x, y, z));
}

void gVKRenderEngine::setVec4(GLuint uniformloc, const glm::vec4& value) {
	G_CHECK_GL(glUniform4fv(uniformloc, 1, &value[0]));
}

void gVKRenderEngine::setVec4(GLuint uniformloc, float x, float y, float z, float w) {
	G_CHECK_GL(glUniform4f(uniformloc, x, y, z, w));
}

void gVKRenderEngine::setMat2(GLuint uniformloc, const glm::mat2& mat) {
	G_CHECK_GL(glUniformMatrix2fv(uniformloc, 1, GL_FALSE, &mat[0][0]));
}

void gVKRenderEngine::setMat3(GLuint uniformloc, const glm::mat3& mat) {
	G_CHECK_GL(glUniformMatrix3fv(uniformloc, 1, GL_FALSE, &mat[0][0]));
}

void gVKRenderEngine::setMat4(GLuint uniformloc, const glm::mat4& mat) {
	G_CHECK_GL(glUniformMatrix4fv(uniformloc, 1, GL_FALSE, &mat[0][0]));
}

GLuint gVKRenderEngine::getUniformLocation(GLuint id, const std::string& name) {
	G_CHECK_GL2(GLuint location, glGetUniformLocation(id, name.c_str()));
	return location;
}

void gVKRenderEngine::useShader(GLuint id) const {
	if (currentprogram == id) return;
	currentprogram = id;
	G_CHECK_GL(glUseProgram(id));
}

void gVKRenderEngine::resetShader(GLuint id, bool loaded) const {
	if(loaded) {
		if (currentprogram == id) currentprogram = 0;
		G_CHECK_GL(glDeleteShader(id));
	}
}

void gVKRenderEngine::clearScreen(bool color, bool depth) {
	GLbitfield mask = 0;
	if(color) mask |= GL_COLOR_BUFFER_BIT;
	if(depth) mask |= GL_DEPTH_BUFFER_BIT;
	glClear(mask);
}

void gVKRenderEngine::bindQuadVAO() {
	G_CHECK_GL(glBindVertexArray(fullscreenquadvao));
}

void gVKRenderEngine::drawFullscreenQuad() {
	G_CHECK_GL(glDrawArrays(GL_TRIANGLES, 0, 6));
}

void gVKRenderEngine::bindDefaultFramebuffer() {
	G_CHECK_GL(glBindFramebuffer(GL_FRAMEBUFFER, gFbo::defaultfbo));
	boundframebuffer = gFbo::defaultfbo;
}

void gVKRenderEngine::drawVbo(const gVbo& vbo) {
	G_CHECK_GL(glDrawArrays(GL_TRIANGLES, 0, vbo.getVerticesNum()));
}

// These emulate just enough of the OpenGL texture object / bind state that
// gTexture and gImage rely on. createTextures() mints an id; bindTexture() records
// which id later texImage2D() uploads into; texImage2D() turns the pixels into a
// real Vulkan texture (gVKTexture) kept in the registry. The wrap/filter/swizzle
// setters have no OpenGL work to do here - the sampler is created with sensible
// defaults in gvkCreateTextureRGBA8 - so they are no-ops.
GLuint gVKRenderEngine::createTextures() {
#ifdef GVK_DESKTOP_GLFW
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
#ifdef GVK_DESKTOP_GLFW
	if(texId != 0 && vkcontext != nullptr) {
		auto it = vktextures.find(texId);
		if(it != vktextures.end()) {
			// A runtime delete may target a texture a previous frame still samples,
			// so drain the device before tearing it down.
			if(*vkcontext->getDevice() != VK_NULL_HANDLE) vkDeviceWaitIdle(*vkcontext->getDevice());
			gvkDestroyTexture(*vkcontext, it->second);
			vktextures.erase(it);
		}
	}
#endif
	texId = 0;
}

void gVKRenderEngine::texImage2D(GLenum target, GLint internalFormat, int width, int height, GLint format, GLint type, void* data, GLint level) {
#ifdef GVK_DESKTOP_GLFW
	if(vkcontext == nullptr || boundtextureid == 0) return;
	// Only 8-bit colour uploads that carry pixels become a texture. Allocation-only
	// calls (data == nullptr, e.g. FBO colour targets) and float / HDR uploads are
	// outside what the 2D image path handles.
	if(data == nullptr || type != GL_UNSIGNED_BYTE || width <= 0 || height <= 0) return;

	int components = 4;
	if(format == GL_RED) components = 1;
	else if(format == GL_RG) components = 2;
	else if(format == GL_RGB) components = 3;
	else if(format == GL_RGBA) components = 4;

	// The Vulkan image is always R8G8B8A8_UNORM, so narrower formats are expanded.
	const unsigned char* src = static_cast<const unsigned char*>(data);
	const size_t pixels = static_cast<size_t>(width) * static_cast<size_t>(height);
	std::vector<unsigned char> rgba(pixels * 4);
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
	}

	// The load path uploads twice (once around generateMipMap), so replace any
	// texture already registered for this id. The first upload is never sampled in
	// a submitted frame, so tearing it down here needs no device wait.
	auto it = vktextures.find(boundtextureid);
	if(it != vktextures.end()) {
		gvkDestroyTexture(*vkcontext, it->second);
		vktextures.erase(it);
	}
	gVKTexture* tex = gvkCreateTextureRGBA8(*vkcontext, rgba.data(), width, height);
	if(tex != nullptr) vktextures[boundtextureid] = tex;
#endif
}

void gVKRenderEngine::setTextureMaxLevel(GLenum target, int maxLevel) {
    /* no-op */
}

#ifdef GVK_DESKTOP_GLFW
// gTexture speaks in GL enums. Nearest is the only distinction the 2D path needs -
// there is a single mip level, so the mipmap variants collapse onto their base
// filter - and the clamping wrap modes all map onto clamp to edge, which is what
// the GL path resolves them to as well.
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
#ifdef GVK_DESKTOP_GLFW
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
#ifdef GVK_DESKTOP_GLFW
	gVKTexture* tex = getBoundVKTexture();
	if(tex == nullptr) return;
	gvkSetTextureSampler(*vkcontext, tex, gvkFilterFromGL(minFilter), gvkFilterFromGL(magFilter),
			tex->addressu, tex->addressv);
#endif
}

void gVKRenderEngine::setWrappingAndFiltering(GLenum target, GLint wrapS, GLint wrapT, GLint minFilter,
                                              GLint magFilter) {
#ifdef GVK_DESKTOP_GLFW
	gVKTexture* tex = getBoundVKTexture();
	if(tex == nullptr) return;
	gvkSetTextureSampler(*vkcontext, tex, gvkFilterFromGL(minFilter), gvkFilterFromGL(magFilter),
			gvkAddressFromGL(wrapS), gvkAddressFromGL(wrapT));
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
	G_CHECK_GL(glDepthFunc(GL_LEQUAL));
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

#ifdef GVK_DESKTOP_GLFW

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
#ifdef GVK_DESKTOP_GLFW
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
#ifndef GVK_DESKTOP_GLFW
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

	gGLFWWindow* glfwwindow = dynamic_cast<gGLFWWindow*>(appmanager != nullptr ? appmanager->getWindow() : nullptr);
	if(glfwwindow == nullptr) {
		gLoge("gVKRenderEngine") << "Vulkan init: no GLFW window available for surface creation.";
		cleanupVulkan();
		return false;
	}
	GLFWwindow* handle = glfwwindow->getGLFWWindow();
	if(handle == nullptr) {
		gLoge("gVKRenderEngine") << "Vulkan init: the GLFW window handle is null.";
		cleanupVulkan();
		return false;
	}
	// The frame loop reads this to react to resizes.
	ctx->window = handle;

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
	// Asking GLFW for the surface extensions keeps this portable instead of
	// hardcoding VK_KHR_win32_surface / VK_EXT_metal_surface per platform.
	uint32_t glfwextcount = 0;
	const char** glfwexts = glfwGetRequiredInstanceExtensions(&glfwextcount);
	if(glfwexts == nullptr) {
		gLoge("gVKRenderEngine") << "Vulkan init: GLFW could not report the required instance extensions.";
		cleanupVulkan();
		return false;
	}
	// Aliased onto the context so the effective list lives on as engine state
	// instead of dying with this local when init returns.
	std::vector<const char*>& extensions = ctx->enabledinstanceextensions;
	extensions.assign(glfwexts, glfwexts + glfwextcount);

	// OpenGL's default framebuffer is a linear, pass-through target. When the
	// presentation system exposes the matching Vulkan colour space, enable it so
	// the compositor does not apply an extra sRGB colour-profile conversion.
	if(gvkHasInstanceExtension(VK_EXT_SWAPCHAIN_COLOR_SPACE_EXTENSION_NAME)) {
		extensions.push_back(VK_EXT_SWAPCHAIN_COLOR_SPACE_EXTENSION_NAME);
	}

#if defined(__APPLE__)
	// MoltenVK is a portability driver: without these the loader does not even
	// enumerate it and vkCreateInstance fails with VK_ERROR_INCOMPATIBLE_DRIVER.
	// They are Apple only - requesting them on a conformant driver would fail.
	extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
	extensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
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
	instanceinfo.flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
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
	// GLFW creates it, so the engine never touches Win32/Cocoa/Metal directly.
	result = glfwCreateWindowSurface(ctx->instance, handle, nullptr, &ctx->surface);
	if(result != VK_SUCCESS) {
		gLoge("gVKRenderEngine") << "glfwCreateWindowSurface failed! VkResult: " << result;
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

	// Empty: no optional features are switched on yet. Kept separate from the
	// context's devicefeatures, which records what the GPU actually supports.
	VkPhysicalDeviceFeatures enabledfeatures{};
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

	/* ---------------- presentation resources ---------------- */
	// The remaining modules of the frame path (render pass, framebuffers, command
	// buffers and synchronisation) are hooked in here as they are implemented.
	if(!gvkCreateSwapchain(*ctx, handle)) {
		gLoge("gVKRenderEngine") << "Vulkan init: the swapchain could not be created.";
		cleanupVulkan();
		return false;
	}
	if(!gvkCreateRenderPass(*ctx)) {
		gLoge("gVKRenderEngine") << "Vulkan init: the render pass could not be created.";
		cleanupVulkan();
		return false;
	}
	// Before the framebuffers, which pair every swapchain view with this one.
	if(!gvkCreateDepthResources(*ctx)) {
		gLoge("gVKRenderEngine") << "Vulkan init: the depth buffer could not be created.";
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
#ifdef GVK_DESKTOP_GLFW
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
	gvkDestroyUniformResources(*ctx);
	gvkDestroyDrawResources(*ctx);
	gvkDestroyGraphicsPipelines(*ctx);
	gvkDestroyPresentSemaphores(*ctx);
	gvkDestroyFrameSyncObjects(*ctx);
	gvkDestroyCommandResources(*ctx);
	gvkDestroyFramebuffers(*ctx);
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
#ifdef GVK_DESKTOP_GLFW
	if(vkcontext == nullptr) return false;
	// Between frames is the only safe point to swap pipelines out: no command
	// buffer is recording and the previous frame can be drained.
	checkShaderReload();
	// The new frame writes into a different uniform buffer, so whatever the previous
	// one gathered does not carry over.
	sceneuniformswritten = false;
	return gvkBeginFrame(*vkcontext, vkcontext->window);
#else
	return false;
#endif
}

void gVKRenderEngine::checkShaderReload() {
#ifdef GVK_DESKTOP_GLFW
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

	if(!gvkReloadGraphicsPipelines(*vkcontext)) return;
	// The reload destroys the descriptor pool, and with it every set allocated
	// from it, so the textures that are still loaded need pointing at the new one.
	for(auto& entry : vktextures) {
		if(entry.second != nullptr) gvkWriteTextureDescriptorSet(*vkcontext, entry.second);
	}
#endif
}

void gVKRenderEngine::endFrame() {
#ifdef GVK_DESKTOP_GLFW
	if(vkcontext == nullptr) return;
	gvkEndFrame(*vkcontext, vkcontext->window);
#endif
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
	// The primitive meshes are created by gRenderer::init() as well. They hold no GL
	// objects until they are drawn, and the 2D ones now record through the backend's
	// draw path, so drawLine / drawCircle / drawRectangle and friends need them here
	// just as much as the OpenGL path does.
	createPrimitiveMeshes();
	if(!initVulkan()) {
		gLoge("gVKRenderEngine") << "Vulkan initialization failed; the Vulkan backend is not usable.";
	}
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
#ifdef GVK_DESKTOP_GLFW
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
	if(!isalphablendingenabled) drawcolor.a = 1.0f;
	gvkDrawColored2D(*vkcontext, points, count, drawcolor, mvp, mode);
#endif
}

#ifdef GVK_DESKTOP_GLFW
void gVKRenderEngine::updateSceneUniforms() {
	// Rebuilt every frame rather than tracked for changes: it is one memcpy into
	// already mapped memory, and the OpenGL path's change tracking exists to avoid
	// glBufferSubData calls that have no equivalent here.
	gVKSceneUniforms uniforms{};
	uniforms.projection = projectionmatrix;
	uniforms.view = viewmatrix;
	uniforms.viewpos = glm::vec4(cameraposition, 1.0f);
	uniforms.globalambientcolor = globalambientcolor.asVec4();

	// The w component doubles as the "is a shadow map bound" flag the shader tests,
	// which keeps it out of the integer block below and its padding rules.
	const bool shadowsready = shadowmapenabled && vkcontext->hasShadowMap();
	uniforms.lightmatrix = shadowlightmatrix;
	uniforms.shadowlightpos = glm::vec4(shadowlightposition, shadowsready ? 1.0f : 0.0f);
	uniforms.softshadows = shadowsoft ? 1 : 0;

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

	gvkWriteSceneUniforms(*vkcontext, uniforms);
}
#endif

void gVKRenderEngine::drawMesh3D(GLuint vertexArrayId, int vertexCount, int indexCount,
		const glm::mat4& model, const gMeshSurface& surface, int drawMode, int instanceCount) {
#ifdef GVK_DESKTOP_GLFW
	if(vkcontext == nullptr || vertexArrayId == 0 || instanceCount <= 0) return;

	// The vertex array is what gVbo bound while it uploaded, so it knows which two
	// buffers this mesh lives in.
	auto arrayentry = vkvertexarrays.find(vertexArrayId);
	if(arrayentry == vkvertexarrays.end()) return;

	gVKMeshBuffer* vertices = getMeshBuffer(arrayentry->second.vertexbuffer);
	if(vertices == nullptr || vertices->buffer == VK_NULL_HANDLE) return;
	gVKMeshBuffer* indices = getMeshBuffer(arrayentry->second.indexbuffer);
	const bool indexed = indices != nullptr && indices->buffer != VK_NULL_HANDLE && indexCount > 0;

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
	if(!sceneuniformswritten) {
		updateSceneUniforms();
		sceneuniformswritten = true;
	}

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
		if(instanceCount > 1) {
			gVKMeshBuffer* instances = getMeshBuffer(arrayentry->second.instancebuffer);
			if(instances == nullptr || instances->buffer == VK_NULL_HANDLE) return;
			shadowinstances = instances->buffer;
		} else {
			if(!ensureIdentityInstanceBuffer()) return;
			shadowinstances = identityinstancebuffer->buffer;
		}
		// Cutout casters: a mesh whose diffuse map punches holes in it has to cast a
		// shadow with the same holes, so the map is looked up here and the fragment
		// stage discards against it. Only the diffuse map and only a non-PBR mesh,
		// because that is exactly where mesh3d.frag discards in the shading pass -
		// mesh3dpbr.frag has no cutout, and a PBR mesh casting holes it does not
		// render would be the same disagreement the other way round.
		VkDescriptorSet cutoutset = VK_NULL_HANDLE;
		if(!surface.ispbr && surface.diffusemapid != 0) {
			auto it = vktextures.find(surface.diffusemapid);
			if(it != vktextures.end() && it->second != nullptr) cutoutset = it->second->descriptorset;
		}
		gVKShadowPush shadowpush{};
		// The light's matrix is folded into the model matrix on the CPU; see
		// shadow3d.vert for why the three are not sent separately.
		shadowpush.lightmodel = shadowlightmatrix * model;
		shadowpush.misc = glm::vec4(cutoutset != VK_NULL_HANDLE ? 1.0f : 0.0f, 0.0f, 0.0f, 0.0f);
		// The binding has to be filled either way, so an opaque caster gets the white
		// texture and the flag above tells the shader not to sample it.
		if(cutoutset == VK_NULL_HANDLE) {
			if(!ensureWhiteTexture()) return;
			cutoutset = vktextures[whitetextureid]->descriptorset;
		}

		const VkIndexType shadowindextype = sizeof(gIndex) == 2 ? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32;
		gvkDrawShadowCaster(*vkcontext, vertices->buffer,
				indexed ? indices->buffer : VK_NULL_HANDLE,
				indexed ? indexCount : vertexCount, shadowindextype, shadowpush, cutoutset,
				shadowinstances, instanceCount, topology);
		return;
	}

	// renderColor is deliberately not applied to a 3D mesh, even though
	// color_frag.glsl ends with "result * renderColor". Measured against the OpenGL
	// backend, a mesh comes out as its material colour alone: setColor moves the 2D
	// drawing colour and never reaches a mesh there. Multiplying it in here would
	// tint every mesh by whatever colour the canvas last drew text in, which is
	// exactly the mismatch this was found through.
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
		if(instanceCount > 1) {
			gVKMeshBuffer* instances = getMeshBuffer(arrayentry->second.instancebuffer);
			if(instances == nullptr || instances->buffer == VK_NULL_HANDLE) return;
			pbrinstances = instances->buffer;
		} else {
			if(!ensureIdentityInstanceBuffer()) return;
			pbrinstances = identityinstancebuffer->buffer;
		}

		const VkIndexType pbrindextype = sizeof(gIndex) == 2 ? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32;
		gvkDrawMesh3DPbr(*vkcontext, vertices->buffer, indexed ? indices->buffer : VK_NULL_HANDLE,
				indexed ? indexCount : vertexCount, pbrindextype, pbrpush, materialset,
				pbrshadowset,
				pbrinstances, instanceCount, topology,
				isdepthtestenabled, depthtesttype == DEPTHTESTTYPE_ALWAYS);
		return;
	}

	// A map only counts once its texture is actually registered here; a material can
	// name one that never made it through texImage2D (an unsupported format, say),
	// and sampling white is closer to the OpenGL result than sampling nothing.
	VkDescriptorSet diffuseset = VK_NULL_HANDLE;
	VkDescriptorSet specularset = VK_NULL_HANDLE;
	// Shadow map, bound as set 4. Falls back to the white texture when there is
	// none, which reads as "nothing was ever closer to the light" and so casts no
	// shadow - the shader's flag says not to look at it anyway.
	VkDescriptorSet shadowset = VK_NULL_HANDLE;
	if(vkcontext->hasShadowMap()) shadowset = vkcontext->getShadowDescriptorSet();
	if(surface.diffusemapid != 0) {
		auto it = vktextures.find(surface.diffusemapid);
		if(it != vktextures.end() && it->second != nullptr) diffuseset = it->second->descriptorset;
	}
	if(surface.specularmapid != 0) {
		auto it = vktextures.find(surface.specularmapid);
		if(it != vktextures.end() && it->second != nullptr) specularset = it->second->descriptorset;
	}
	VkDescriptorSet normalset = VK_NULL_HANDLE;
	if(surface.normalmapid != 0) {
		auto it = vktextures.find(surface.normalmapid);
		if(it != vktextures.end() && it->second != nullptr) normalset = it->second->descriptorset;
	}

	push.misc = glm::vec4(surface.shininess,
			diffuseset != VK_NULL_HANDLE ? 1.0f : 0.0f,
			specularset != VK_NULL_HANDLE ? 1.0f : 0.0f,
			normalset != VK_NULL_HANDLE ? 1.0f : 0.0f);

	// Both bindings must point at a real descriptor even when the flags say not to
	// sample them, so the unused ones fall back to the 1x1 white texture.
	if(diffuseset == VK_NULL_HANDLE || specularset == VK_NULL_HANDLE ||
			normalset == VK_NULL_HANDLE || shadowset == VK_NULL_HANDLE) {
		if(!ensureWhiteTexture()) return;
		VkDescriptorSet whiteset = vktextures[whitetextureid]->descriptorset;
		if(diffuseset == VK_NULL_HANDLE) diffuseset = whiteset;
		if(specularset == VK_NULL_HANDLE) specularset = whiteset;
		if(normalset == VK_NULL_HANDLE) normalset = whiteset;
		if(shadowset == VK_NULL_HANDLE) shadowset = whiteset;
	}
	// No alpha fixup here, unlike the 2D path: the 3D pipeline does not blend at all
	// (see gVKPipeline.cpp), so what the lighting sum leaves in the alpha channel
	// never reaches the framebuffer as transparency.

	// The instance buffer is always bound, because the shader always reads a model
	// matrix from it. An instanced draw uses the one gVbo uploaded; anything else
	// gets the shared identity, which multiplies out to no change at all.
	VkBuffer instancebuffer = VK_NULL_HANDLE;
	if(instanceCount > 1) {
		gVKMeshBuffer* instances = getMeshBuffer(arrayentry->second.instancebuffer);
		if(instances == nullptr || instances->buffer == VK_NULL_HANDLE) return;
		instancebuffer = instances->buffer;
	} else {
		if(!ensureIdentityInstanceBuffer()) return;
		instancebuffer = identityinstancebuffer->buffer;
	}

	// gIndex is what gVbo uploaded, so the index type follows it: 16 bit on Android,
	// 32 bit everywhere else.
	const VkIndexType indextype = sizeof(gIndex) == 2 ? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32;

	gvkDrawMesh3D(*vkcontext, vertices->buffer, indexed ? indices->buffer : VK_NULL_HANDLE,
			indexed ? indexCount : vertexCount, indextype, push, diffuseset, specularset, normalset,
			shadowset,
			instancebuffer, instanceCount,
			topology, isdepthtestenabled, depthtesttype == DEPTHTESTTYPE_ALWAYS, lines);
#endif
}

void gVKRenderEngine::drawTexturedRect2D(GLuint textureId, GLuint maskTextureId, const glm::vec4& tint,
		const glm::mat4& mvp, const glm::vec2& uvOffset, const glm::vec2& uvScale) {
#ifdef GVK_DESKTOP_GLFW
	if(vkcontext == nullptr) return;
	auto it = vktextures.find(textureId);
	if(it == vktextures.end() || it->second == nullptr) return;
	VkDescriptorSet maskset = VK_NULL_HANDLE;
	if(maskTextureId != 0) {
		auto maskit = vktextures.find(maskTextureId);
		if(maskit != vktextures.end() && maskit->second != nullptr) maskset = maskit->second->descriptorset;
	}
	gvkDrawTextured2D(*vkcontext, it->second->descriptorset, maskset, tint, mvp, uvOffset, uvScale);
#endif
}

void gVKRenderEngine::drawTexturedTriangles2D(GLuint textureId, const glm::vec4& tint,
		const glm::mat4& mvp, const float* xyuv, int vertexCount) {
#ifdef GVK_DESKTOP_GLFW
	if(vkcontext == nullptr) return;
	auto it = vktextures.find(textureId);
	if(it == vktextures.end() || it->second == nullptr) return;
	gvkDrawTexturedTriangles2D(*vkcontext, it->second->descriptorset, tint, mvp, xyuv, vertexCount);
#endif
}

gVKTexture* gVKRenderEngine::getBoundVKTexture() {
#ifdef GVK_DESKTOP_GLFW
	if(vkcontext == nullptr || boundtextureid == 0) return nullptr;
	auto it = vktextures.find(boundtextureid);
	return it == vktextures.end() ? nullptr : it->second;
#else
	return nullptr;
#endif
}

void gVKRenderEngine::destroyAllTextures() {
#ifdef GVK_DESKTOP_GLFW
	if(vkcontext == nullptr) return;
	for(auto& entry : vktextures) gvkDestroyTexture(*vkcontext, entry.second);
	vktextures.clear();
#endif
}

bool gVKRenderEngine::ensureWhiteTexture() {
#ifdef GVK_DESKTOP_GLFW
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

#ifdef GVK_DESKTOP_GLFW
// Kept out of the class because its return type is a Vulkan handle and
// gVKRenderEngine.h is deliberately free of Vulkan headers.
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

	VkDevice device = *vkcontext->getDevice();
	VkDescriptorSetAllocateInfo allocinfo{};
	allocinfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocinfo.descriptorPool = vkcontext->getDescriptorPool();
	allocinfo.descriptorSetCount = 1;
	allocinfo.pSetLayouts = &layout;
	VkDescriptorSet set = VK_NULL_HANDLE;
	if(vkAllocateDescriptorSets(device, &allocinfo, &set) != VK_SUCCESS) {
		gLoge("gVKRenderEngine") << "Could not allocate a PBR material descriptor set.";
		return VK_NULL_HANDLE;
	}

	// Every binding is written, whether or not the material supplies that map: the
	// shader names all five samplers, so an unwritten binding would be invalid. The
	// flags in the push constant are what decide which ones are actually read.
	gVKTexture* white = vktextures[whitetextureid];
	VkDescriptorImageInfo images[5]{};
	VkWriteDescriptorSet writes[5]{};
	for(int i = 0; i < 5; i++) {
		gVKTexture* tex = nullptr;
		auto it = vktextures.find(key[i]);
		if(key[i] != 0 && it != vktextures.end()) tex = it->second;
		if(tex == nullptr) tex = white;

		images[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		images[i].imageView = tex->view;
		images[i].sampler = tex->sampler;

		writes[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writes[i].dstSet = set;
		writes[i].dstBinding = static_cast<uint32_t>(i);
		writes[i].dstArrayElement = 0;
		writes[i].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		writes[i].descriptorCount = 1;
		writes[i].pImageInfo = &images[i];
	}
	vkUpdateDescriptorSets(device, 5, writes, 0, nullptr);

	cache[key] = set;
	return set;
}
#endif

bool gVKRenderEngine::allocateShadowMap(int width, int height) {
#ifdef GVK_DESKTOP_GLFW
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
#ifdef GVK_DESKTOP_GLFW
	if(vkcontext == nullptr) return;
	vkDeviceWaitIdle(*vkcontext->getDevice());
	gvkDestroyShadowResources(*vkcontext);
	shadowmapenabled = false;
#endif
}

bool gVKRenderEngine::beginShadowPass() {
#ifdef GVK_DESKTOP_GLFW
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

void gVKRenderEngine::endShadowPass() {
#ifdef GVK_DESKTOP_GLFW
	if(vkcontext == nullptr) return;
	gvkEndShadowPass(*vkcontext);
#endif
}

void gVKRenderEngine::setShadowMapState(bool enabled, const glm::mat4& lightMatrix,
		const glm::vec3& lightPosition, bool softShadows) {
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
#ifdef GVK_DESKTOP_GLFW
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
#ifdef GVK_DESKTOP_GLFW
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
