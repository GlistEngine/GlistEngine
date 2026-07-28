//
// Created by sadettin on 23.08.2025.
//

#include "gVKRenderEngine.h"

//screenShot Related includes
#include "stb/stb_image_write.h"
#include "gBaseApp.h"
#include "gGrid.h"
#include "gImage.h"
#include "gShader.h"
#include "gTracy.h"

// Vulkan is only wired up on the desktop GLFW platforms. This file is compiled
// everywhere, but gGLFWWindow.cpp is desktop only, so the guard below mirrors
// the CMake condition exactly. <vulkan/vulkan.h> must be included before GLFW
// so that glfwCreateWindowSurface / glfwGetRequiredInstanceExtensions become
// visible.
#if defined(__APPLE__)
#include <TargetConditionals.h>
#endif
#if defined(GLIST_HAS_VULKAN) && !defined(ANDROID) && !defined(EMSCRIPTEN) && !(defined(TARGET_OS_IPHONE) && TARGET_OS_IPHONE)
	#define GVK_DESKTOP_GLFW 1
	#define GLFW_INCLUDE_VULKAN
	#include <vulkan/vulkan.h>
	#include "gGLFWWindow.h"
	#include "gAppManager.h"
	#include <vector>
	#include <set>
	#include <string>
	#include <cstring>
#endif

gVKRenderEngine::~gVKRenderEngine() {
	cleanupVulkan();
	delete originalgrid;
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
	G_CHECK_GL(glClearColor(0.0f, 0.0f, 0.0f, 1.0f));
	G_CHECK_GL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
}

void gVKRenderEngine::clearColor(int r, int g, int b, int a) {
	//    glBindFramebuffer(GL_FRAMEBUFFER, gFbo::defaultfbo);
	G_CHECK_GL(glClearColor((float)r / 255, (float)g / 255, (float)b / 255, (float)a / 255));
	G_CHECK_GL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
}

void gVKRenderEngine::clearColor(gColor color) {
	G_CHECK_GL(glClearColor(color.r, color.g, color.b, color.a));
	G_CHECK_GL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
}

void gVKRenderEngine::enableDepthTest() {
	G_CHECK_GL(enableDepthTest(DEPTHTESTTYPE_LESS));
}

void gVKRenderEngine::enableDepthTest(int depthTestType) {
	G_CHECK_GL(glEnable(GL_DEPTH_TEST));
	G_CHECK_GL(glDepthFunc(depthtesttypeid[depthTestType]));
	isdepthtestenabled = true;
	depthtesttype = depthTestType;
}

void gVKRenderEngine::setDepthTestFunc(int depthTestType) {
	G_CHECK_GL(glDepthFunc(depthtesttypeid[depthTestType]));
	depthtesttype = depthTestType;
}

void gVKRenderEngine::disableDepthTest() {
	G_CHECK_GL(glDisable(GL_DEPTH_TEST));
	isdepthtestenabled = false;
}

bool gVKRenderEngine::isDepthTestEnabled() {
	return isdepthtestenabled;
}

int gVKRenderEngine::getDepthTestType() {
	return depthtesttype;
}

void gVKRenderEngine::enableAlphaBlending() {
	G_CHECK_GL(glEnable(GL_BLEND));
	G_CHECK_GL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
	isalphablendingenabled = true;
}

void gVKRenderEngine::disableAlphaBlending() {
	G_CHECK_GL(glDisable(GL_BLEND));
	isalphablendingenabled = false;
}

bool gVKRenderEngine::isAlphaBlendingEnabled() {
	return isalphablendingenabled;
}

void gVKRenderEngine::enableAlphaTest() {
#if defined(WIN32) || defined(LINUX)
	G_CHECK_GL(glEnable(GL_ALPHA_TEST));
	G_CHECK_GL(glAlphaFunc(GL_GREATER, 0.1));
	isalphatestenabled = true;
#endif
}

void gVKRenderEngine::disableAlphaTest() {
#if defined(WIN32) || defined(LINUX)
	G_CHECK_GL(glDisable(GL_ALPHA_TEST));
	isalphatestenabled = false;
#endif
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


GLuint gVKRenderEngine::genBuffers() {
	GLuint buffer;
	G_CHECK_GL(glGenBuffers(1, &buffer));
	return buffer;
}

void gVKRenderEngine::deleteBuffer(GLuint& buffer) {
	if (buffer != 0) {
		G_CHECK_GL(glDeleteBuffers(1, &buffer));
	}
}

void gVKRenderEngine::bindBuffer(GLenum target, GLuint buffer) {
	G_CHECK_GL(glBindBuffer(target, buffer));
}

void gVKRenderEngine::unbindBuffer(GLenum target) {
	G_CHECK_GL(glBindBuffer(target, 0));
}

void gVKRenderEngine::bufSubData(GLuint buffer, int offset, int size, const void* data) {
	bindBuffer(GL_UNIFORM_BUFFER, buffer);
	G_CHECK_GL(glBufferSubData(GL_UNIFORM_BUFFER, offset, size, data));
	unbindBuffer(GL_UNIFORM_BUFFER);
}

void gVKRenderEngine::setBufferData(GLuint buffer, const void* data, size_t size, int usage) {
	bindBuffer(GL_UNIFORM_BUFFER, buffer);
	G_CHECK_GL(glBufferData(GL_UNIFORM_BUFFER, size, data, usage));
	unbindBuffer(GL_UNIFORM_BUFFER);
}

void gVKRenderEngine::setBufferRange(int index, GLuint buffer, int offset, int size) {
	G_CHECK_GL(glBindBufferRange(GL_UNIFORM_BUFFER, index, buffer, offset, size));
}

// ----- VAO -----l
GLuint gVKRenderEngine::createVAO() {
	GLuint vao;
	G_CHECK_GL(glGenVertexArrays(1, &vao));
	return vao;
}

void gVKRenderEngine::deleteVAO(GLuint& vao) {
	if(vao != 0) {
		G_CHECK_GL(glDeleteVertexArrays(1, &vao));
	}
}

void gVKRenderEngine::bindVAO(GLuint vao) {
	G_CHECK_GL(glBindVertexArray(vao));
}

void gVKRenderEngine::unbindVAO() {
	G_CHECK_GL(glBindVertexArray(0));
}

void gVKRenderEngine::setVertexBufferData(GLuint vbo, size_t size, const void* data, int usage) {
	G_CHECK_GL(glBindBuffer(GL_ARRAY_BUFFER, vbo));
	G_CHECK_GL(glBufferData(GL_ARRAY_BUFFER, size, data, usage));
}

void gVKRenderEngine::setIndexBufferData(GLuint ebo, size_t size, const void* data, int usage) {
	G_CHECK_GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo));
	G_CHECK_GL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, data, usage));
}

// ----- Draw -----
void gVKRenderEngine::drawArrays(int drawMode, int count) {
	G_CHECK_GL(glDrawArrays(drawMode, 0, count));
}

void gVKRenderEngine::drawElements(int drawMode, int count) {
	G_CHECK_GL(glDrawElements(drawMode, count, G_INDEX_SIZE, 0));
}

// ----- vertex attributes -----
void gVKRenderEngine::enableVertexAttrib(int index) {
	G_CHECK_GL(glEnableVertexAttribArray(index));
}

void gVKRenderEngine::disableVertexAttrib(int index) {
	G_CHECK_GL(glDisableVertexAttribArray(index));
}

void gVKRenderEngine::setVertexAttribPointer(int index, int size, int type, bool normalized, int stride,
                                             const void* pointer) {
	G_CHECK_GL(glVertexAttribPointer(index, size, type, normalized ? GL_TRUE : GL_FALSE, stride, pointer));
}

void gVKRenderEngine::setViewport(int x, int y, int width, int height) {
	G_CHECK_GL(glViewport(x, y, width, height));
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
}

void gVKRenderEngine::drawVbo(const gVbo& vbo) {
	G_CHECK_GL(glDrawArrays(GL_TRIANGLES, 0, vbo.getVerticesNum()));
}

GLuint gVKRenderEngine::createTextures() {
	GLuint id;
	G_CHECK_GL(glGenTextures(1, &id));
	return id;
}

void gVKRenderEngine::bindTexture(GLuint texId) {
	G_CHECK_GL(glBindTexture(GL_TEXTURE_2D, texId));
}

void gVKRenderEngine::bindTexture(GLuint texId, int textureSlotNo) {
	G_CHECK_GL(glActiveTexture(GL_TEXTURE0 + textureSlotNo));
	G_CHECK_GL(glBindTexture(GL_TEXTURE_2D, texId));
}

void gVKRenderEngine::unbindTexture() {
	G_CHECK_GL(glBindTexture(GL_TEXTURE_2D, 0));
}

void gVKRenderEngine::activateTexture(int textureSlotNo) {
	G_CHECK_GL(glActiveTexture(GL_TEXTURE0 + textureSlotNo));
}

void gVKRenderEngine::resetTexture() {
	G_CHECK_GL(glActiveTexture(GL_TEXTURE0));
}

void gVKRenderEngine::deleteTexture(GLuint& texId) {
	if (texId != 0) {
		G_CHECK_GL(glDeleteTextures(1, &texId));
	}
}

void gVKRenderEngine::texImage2D(GLenum target, GLint internalFormat, int width, int height, GLint format,
                                 GLint type, void* data) {
	G_CHECK_GL(glTexImage2D(target, 0, internalFormat, width, height, 0, format, type, data));
}

void gVKRenderEngine::setWrapping(GLenum target, GLint wrapS, GLint wrapT) {
	G_CHECK_GL(glTexParameteri(target, GL_TEXTURE_WRAP_S, wrapS));
	G_CHECK_GL(glTexParameteri(target, GL_TEXTURE_WRAP_T, wrapT));
}

void gVKRenderEngine::setWrapping(GLenum target, GLint wrapS, GLint wrapT, GLint wrapR) {
	G_CHECK_GL(glTexParameteri(target, GL_TEXTURE_WRAP_S, wrapS));
	G_CHECK_GL(glTexParameteri(target, GL_TEXTURE_WRAP_T, wrapT));
	G_CHECK_GL(glTexParameteri(target, GL_TEXTURE_WRAP_R, wrapR));
}

void gVKRenderEngine::setFiltering(GLenum target, GLint minFilter, GLint magFilter) {
	G_CHECK_GL(glTexParameteri(target, GL_TEXTURE_MIN_FILTER, minFilter));
	G_CHECK_GL(glTexParameteri(target, GL_TEXTURE_MAG_FILTER, magFilter));
}

void gVKRenderEngine::setWrappingAndFiltering(GLenum target, GLint wrapS, GLint wrapT, GLint minFilter,
                                              GLint magFilter) {
	G_CHECK_GL(glTexParameteri(target, GL_TEXTURE_WRAP_S, wrapS));
	G_CHECK_GL(glTexParameteri(target, GL_TEXTURE_WRAP_T, wrapT));
	G_CHECK_GL(glTexParameteri(target, GL_TEXTURE_MIN_FILTER, minFilter));
	G_CHECK_GL(glTexParameteri(target, GL_TEXTURE_MAG_FILTER, magFilter));
}

void gVKRenderEngine::setWrappingAndFiltering(GLenum target, GLint wrapS, GLint wrapT, GLint wrapR, GLint minFilter,
                                              GLint magFilter) {
	G_CHECK_GL(glTexParameteri(target, GL_TEXTURE_WRAP_S, wrapS));
	G_CHECK_GL(glTexParameteri(target, GL_TEXTURE_WRAP_T, wrapT));
	G_CHECK_GL(glTexParameteri(target, GL_TEXTURE_WRAP_R, wrapR));
	G_CHECK_GL(glTexParameteri(target, GL_TEXTURE_MIN_FILTER, minFilter));
	G_CHECK_GL(glTexParameteri(target, GL_TEXTURE_MAG_FILTER, magFilter));
}

void gVKRenderEngine::setSwizzleMask(GLint swizzleMask[4]) {
#if defined(GLIST_OPENGLES)
	G_CHECK_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_R, swizzleMask[0]));
	G_CHECK_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_G, swizzleMask[1]));
	G_CHECK_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B, swizzleMask[2]));
	G_CHECK_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_A, swizzleMask[3]));
#else
	G_CHECK_GL(glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask));
#endif
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
	G_CHECK_GL(glGenerateMipmap(GL_TEXTURE_2D));
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

// Validation layers cost performance, so the default follows the same DEBUG
// condition the OpenGL debug output already uses in this engine. A developer can
// still override it per context through setValidationEnabled().
#if defined(DEBUG) || defined(ENGINE_OPENGL_CHECKS)
static constexpr bool gvkdefaultvalidation = true;
#else
static constexpr bool gvkdefaultvalidation = false;
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

	/* ---------------- configurable settings ---------------- */
	// Set these before the backend initialises to influence instance and device
	// creation. Reading them afterwards simply reports what was used.

	// Identity handed to VkApplicationInfo. Informational to drivers and tools,
	// but handy for profiling and crash triage.
	void setAppName(const std::string& name) { appname = name; }
	void setEngineName(const std::string& name) { enginename = name; }
	void setAppVersion(uint32_t version) { appversion = version; }
	void setEngineVersion(uint32_t version) { engineversion = version; }

	// The Vulkan API level the instance targets, e.g. VK_API_VERSION_1_2.
	void setApiVersion(uint32_t version) { apiversion = version; }

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
	uint32_t* getApiVersion() { return &apiversion; }
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
	VkPhysicalDeviceFeatures* getDeviceFeatures() { return &devicefeatures; }
	VkPhysicalDeviceMemoryProperties* getDeviceMemoryProperties() { return &devicememoryproperties; }

	// The surface's capabilities and the formats / present modes it supports on the
	// selected device - what the swapchain is built from: extent and image-count
	// bounds, colour formats, and vsync / present modes.
	VkSurfaceCapabilitiesKHR* getSurfaceCapabilities() { return &surfacecapabilities; }
	std::vector<VkSurfaceFormatKHR>* getSurfaceFormats() { return &surfaceformats; }
	std::vector<VkPresentModeKHR>* getSurfacePresentModes() { return &surfacepresentmodes; }

	// Whether validation is actually running, which is not the same as whether it
	// was requested: setValidationEnabled(true) still yields false here when the
	// layer or debug-utils extension is missing at runtime. getValidationEnabled()
	// reports the request; this reports the outcome.
	bool isValidationActive() const { return validationactive; }

	// True once a logical device exists, i.e. init reached the point where the
	// context is actually usable for swapchains, pipelines and queues.
	bool isInitialized() const { return device != VK_NULL_HANDLE; }

private:
	std::string appname = "GlistApp";
	std::string enginename = "GlistEngine";
	uint32_t appversion = VK_MAKE_API_VERSION(0, 1, 0, 0);
	uint32_t engineversion = VK_MAKE_API_VERSION(0, 1, 0, 0);
	uint32_t apiversion = VK_API_VERSION_1_2;
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
	bool validationactive = false;
};

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


bool gVKRenderEngine::initVulkan() {
#ifndef GVK_DESKTOP_GLFW
	gLoge("gVKRenderEngine") << "Vulkan backend is not supported on this platform.";
	return false;
#else
	// Honour a context a developer injected through setContext() so its settings
	// survive; only allocate a default one when none was provided.
	if(vkcontext == nullptr) vkcontext = new gVKContext();
	gVKContext* ctx = vkcontext;

#if defined(__APPLE__)
	// MoltenVK (the macOS driver) and the Homebrew validation layers are not on
	// the loader's default search path. The trailing 0 means "do not overwrite",
	// so an explicitly exported value always wins.
#ifdef GLIST_VK_ICD_FILE
	setenv("VK_ICD_FILENAMES", GLIST_VK_ICD_FILE, 0);
#endif
#ifdef GLIST_VK_LAYER_PATH
	setenv("VK_LAYER_PATH", GLIST_VK_LAYER_PATH, 0);
#endif
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

	VkApplicationInfo appinfo{};
	appinfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appinfo.pApplicationName = ctx->appname.c_str();
	appinfo.applicationVersion = ctx->appversion;
	appinfo.pEngineName = ctx->enginename.c_str();
	appinfo.engineVersion = ctx->engineversion;
	appinfo.apiVersion = ctx->apiversion;

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
		gLoge("gVKRenderEngine") << "No GPU can both render and present to this surface.";
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
	deviceinfo.enabledLayerCount = static_cast<uint32_t>(layers.size());
	deviceinfo.ppEnabledLayerNames = layers.empty() ? nullptr : layers.data();

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
	gLogi("gVKRenderEngine") << "Vulkan device: " << props.deviceName
			<< " | graphics family: " << ctx->graphicsfamily
			<< " | present family: " << ctx->presentfamily
			<< " | validation: " << (usevalidation ? "on" : "off");
	return true;
#endif
}


void gVKRenderEngine::cleanupVulkan() {
#ifdef GVK_DESKTOP_GLFW
	if(vkcontext == nullptr) return;
	gVKContext* ctx = vkcontext;
	// Strict reverse creation order: Vulkan requires children to be destroyed
	// before their parent, and the surface must die before its instance.
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


void gVKRenderEngine::init() {
	// gRenderer::init() is deliberately not called: it compiles shaders and
	// builds GL objects, and there is no GL context under Vulkan. originalgrid
	// is assigned only inside that function and is never initialised in a
	// constructor, so it is nulled here to keep the destructor's delete safe.
	originalgrid = nullptr;
	if(!initVulkan()) {
		gLoge("gVKRenderEngine") << "Vulkan initialization failed; the Vulkan backend is not usable.";
	}
}


void gVKRenderEngine::cleanup() {
	// The GL resources gRenderer::cleanup() would release were never created,
	// because init() skips gRenderer::init().
	cleanupVulkan();
}

void gVKRenderEngine::updatePackUnpackAlignment(int i) {
	G_CHECK_GL(glPixelStorei(GL_UNPACK_ALIGNMENT, i));
	G_CHECK_GL(glPixelStorei(GL_PACK_ALIGNMENT, i));
}
