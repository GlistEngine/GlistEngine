//
// Created by sadettin on 23.08.2025.
//

#include "gGLRenderEngine.h"

//screenShot Related includes
#include "stb/stb_image_write.h"
#include "gBaseApp.h"
#include "gGrid.h"
#include "gImage.h"
#include "gShader.h"
#include "gTracy.h"

void gCheckGLErrorAndPrint(const std::string& prefix, const std::string& func, int line) {
	GLenum error = glGetError();
	if (error != GL_NO_ERROR) {
		gLogi("gGLRenderEngine") << prefix << "OpenGL ERROR at " << func << ", line " << line << ", error: " << gToHex(error, 4);
	}
}

void gEnableCulling() {
	G_CHECK_GL(glEnable(GL_CULL_FACE));
}

void gDisableCulling() {
	G_CHECK_GL(glDisable(GL_CULL_FACE));
}

bool gIsCullingEnabled() {
	G_CHECK_GL2(GLboolean res, glIsEnabled(GL_CULL_FACE));
	return res == GL_TRUE;
}

void gCullFace(int cullingFace) {
	G_CHECK_GL(glCullFace(cullingFace));
}

int gGetCullFace() {
	GLint i;
	G_CHECK_GL(glGetIntegerv(GL_CULL_FACE_MODE, &i));
	return i;
}

void gSetCullingDirection(int cullingDirection) {
	G_CHECK_GL(glFrontFace(cullingDirection));
}

int gGetCullingDirection() {
	GLint i;
	G_CHECK_GL(glGetIntegerv(GL_FRONT_FACE, &i));
	return i;
}

gGLRenderEngine::~gGLRenderEngine() {
	delete originalgrid;
}

void gGLRenderEngine::clear() {
	G_CHECK_GL(glClearColor(0.0f, 0.0f, 0.0f, 1.0f));
	G_CHECK_GL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
}

void gGLRenderEngine::clearColor(int r, int g, int b, int a) {
	//    glBindFramebuffer(GL_FRAMEBUFFER, gFbo::defaultfbo);
	G_CHECK_GL(glClearColor((float)r / 255, (float)g / 255, (float)b / 255, (float)a / 255));
	G_CHECK_GL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
}

void gGLRenderEngine::clearColor(gColor color) {
	G_CHECK_GL(glClearColor(color.r, color.g, color.b, color.a));
	G_CHECK_GL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
}

void gGLRenderEngine::enableDepthTest() {
	G_CHECK_GL(enableDepthTest(DEPTHTESTTYPE_LESS));
}

void gGLRenderEngine::enableDepthTest(int depthTestType) {
	G_CHECK_GL(glEnable(GL_DEPTH_TEST));
	G_CHECK_GL(glDepthFunc(depthtesttypeid[depthTestType]));
	isdepthtestenabled = true;
	depthtesttype = depthTestType;
}

void gGLRenderEngine::setDepthTestFunc(int depthTestType) {
	G_CHECK_GL(glDepthFunc(depthtesttypeid[depthTestType]));
	depthtesttype = depthTestType;
}

void gGLRenderEngine::disableDepthTest() {
	G_CHECK_GL(glDisable(GL_DEPTH_TEST));
	isdepthtestenabled = false;
}

bool gGLRenderEngine::isDepthTestEnabled() {
	return isdepthtestenabled;
}

int gGLRenderEngine::getDepthTestType() {
	return depthtesttype;
}

void gGLRenderEngine::enableAlphaBlending() {
	G_CHECK_GL(glEnable(GL_BLEND));
	G_CHECK_GL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
	isalphablendingenabled = true;
}

void gGLRenderEngine::disableAlphaBlending() {
	G_CHECK_GL(glDisable(GL_BLEND));
	isalphablendingenabled = false;
}

bool gGLRenderEngine::isAlphaBlendingEnabled() {
	return isalphablendingenabled;
}

void gGLRenderEngine::enableAlphaTest() {
#if defined(WIN32) || defined(LINUX)
	G_CHECK_GL(glEnable(GL_ALPHA_TEST));
	G_CHECK_GL(glAlphaFunc(GL_GREATER, 0.1));
	isalphatestenabled = true;
#endif
}

void gGLRenderEngine::disableAlphaTest() {
#if defined(WIN32) || defined(LINUX)
	G_CHECK_GL(glDisable(GL_ALPHA_TEST));
	isalphatestenabled = false;
#endif
}

bool gGLRenderEngine::isAlphaTestEnabled() {
	return isalphatestenabled;
}

/*
 * Rotates The Pixel Data upside down. Hence rotates flips the image upside down
 */
void flipVertically(unsigned char* pixelData, int width, int height, int numChannels) {
	G_PROFILE_ZONE_SCOPED_N("flipVertically()");
	int rowsize = width * numChannels;
	unsigned char* temprow = new unsigned char[rowsize];

	for(int row = 0; row < height / 2; ++row) {
		// Calculate the corresponding row from the bottom
		int bottomrow = height - row - 1;

		// Swap the rows
		memcpy(temprow, pixelData + row * rowsize, rowsize);
		memcpy(pixelData + row * rowsize, pixelData + bottomrow * rowsize, rowsize);
		memcpy(pixelData + bottomrow * rowsize, temprow, rowsize);
	}

	delete[] temprow;
}

void gGLRenderEngine::takeScreenshot(gImage& img, int x, int y, int width, int height) {
	G_PROFILE_ZONE_SCOPED_N("gGLRenderEngine::takeScreenshot()");
	unsigned char* pixeldata = new unsigned char[width * height * 4];
	G_CHECK_GL(glReadPixels(x, getHeight() - y - height, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixeldata));
	flipVertically(pixeldata, width, height, 4);
	img.setImageData(pixeldata, width, height, 4);
	//std::string imagePath = "output.png";   USE IT TO SAVE THE IMAGE
	// screenShot->saveImage(imagePath);  USE IT TO SAVE THE IMAGE
}

void gGLRenderEngine::takeScreenshot(gImage& img) {
	G_PROFILE_ZONE_SCOPED_N("gGLRenderEngine::takeScreenshot()");
	int height = gBaseApp::getAppManager()->getWindow()->getHeight();
	int width = gBaseApp::getAppManager()->getWindow()->getWidth();
	unsigned char* pixeldata = new unsigned char[width * height * 4];
	G_CHECK_GL(glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixeldata));
	flipVertically(pixeldata, width, height, 4);
	img.setImageData(pixeldata, width, height, 4);
	//std::string imagePath = "output.png";   USE IT TO SAVE THE IMAGE
	// screenShot->saveImage(imagePath);  USE IT TO SAVE THE IMAGE
}


GLuint gGLRenderEngine::genBuffers() {
	GLuint buffer;
	G_CHECK_GL(glGenBuffers(1, &buffer));
	return buffer;
}

void gGLRenderEngine::deleteBuffer(GLuint& buffer) {
	if (buffer != 0) {
		G_CHECK_GL(glDeleteBuffers(1, &buffer));
	}
}

void gGLRenderEngine::bindBuffer(GLenum target, GLuint buffer) {
	G_CHECK_GL(glBindBuffer(target, buffer));
}

void gGLRenderEngine::unbindBuffer(GLenum target) {
	G_CHECK_GL(glBindBuffer(target, 0));
}

void gGLRenderEngine::bufSubData(GLuint buffer, int offset, int size, const void* data) {
	bindBuffer(GL_UNIFORM_BUFFER, buffer);
	G_CHECK_GL(glBufferSubData(GL_UNIFORM_BUFFER, offset, size, data));
	unbindBuffer(GL_UNIFORM_BUFFER);
}

void gGLRenderEngine::setBufferData(GLuint buffer, const void* data, size_t size, int usage) {
	bindBuffer(GL_UNIFORM_BUFFER, buffer);
	G_CHECK_GL(glBufferData(GL_UNIFORM_BUFFER, size, data, usage));
	unbindBuffer(GL_UNIFORM_BUFFER);
}

void gGLRenderEngine::setBufferRange(int index, GLuint buffer, int offset, int size) {
	G_CHECK_GL(glBindBufferRange(GL_UNIFORM_BUFFER, index, buffer, offset, size));
}

// ----- VAO -----l
GLuint gGLRenderEngine::createVAO() {
	GLuint vao;
	G_CHECK_GL(glGenVertexArrays(1, &vao));
	return vao;
}

void gGLRenderEngine::deleteVAO(GLuint& vao) {
	if(vao != 0) {
		G_CHECK_GL(glDeleteVertexArrays(1, &vao));
	}
}

void gGLRenderEngine::bindVAO(GLuint vao) {
	G_CHECK_GL(glBindVertexArray(vao));
}

void gGLRenderEngine::unbindVAO() {
	G_CHECK_GL(glBindVertexArray(0));
}

void gGLRenderEngine::setVertexBufferData(GLuint vbo, size_t size, const void* data, int usage) {
	G_CHECK_GL(glBindBuffer(GL_ARRAY_BUFFER, vbo));
	G_CHECK_GL(glBufferData(GL_ARRAY_BUFFER, size, data, usage));
}

void gGLRenderEngine::setIndexBufferData(GLuint ebo, size_t size, const void* data, int usage) {
	G_CHECK_GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo));
	G_CHECK_GL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, data, usage));
}

// ----- Draw -----
void gGLRenderEngine::drawArrays(int drawMode, int count) {
	G_CHECK_GL(glDrawArrays(drawMode, 0, count));
}

void gGLRenderEngine::drawElements(int drawMode, int count) {
	G_CHECK_GL(glDrawElements(drawMode, count, GL_UNSIGNED_INT, 0));
}

// ----- vertex attributes -----
void gGLRenderEngine::enableVertexAttrib(int index) {
	G_CHECK_GL(glEnableVertexAttribArray(index));
}

void gGLRenderEngine::disableVertexAttrib(int index) {
	G_CHECK_GL(glDisableVertexAttribArray(index));
}

void gGLRenderEngine::setVertexAttribPointer(int index, int size, int type, bool normalized, int stride,
                                             const void* pointer) {
	G_CHECK_GL(glVertexAttribPointer(index, size, type, normalized ? GL_TRUE : GL_FALSE, stride, pointer));
}

// ----- Framebuffer -----
GLuint gGLRenderEngine::createFramebuffer() {
	GLuint fbo;
	G_CHECK_GL(glGenFramebuffers(1, &fbo));
	return fbo;
}

void gGLRenderEngine::deleteFramebuffer(GLuint& fbo) {
	if(fbo != 0) {
		G_CHECK_GL(glDeleteFramebuffers(1, &fbo));
	}
}

void gGLRenderEngine::bindFramebuffer(GLuint fbo) {
	G_CHECK_GL(glBindFramebuffer(GL_FRAMEBUFFER, fbo));
}

void gGLRenderEngine::checkFramebufferStatus() {
	// check if fbo complete
	G_CHECK_GL2(GLuint status, glCheckFramebufferStatus(GL_FRAMEBUFFER));
	if(status != GL_FRAMEBUFFER_COMPLETE) {
		gLogi("gFbo") << "Framebuffer is not complete! status:" << gToHex(status, 4);
	}
}

// ----- Renderbuffer -----
GLuint gGLRenderEngine::createRenderbuffer() {
	GLuint rbo;
	G_CHECK_GL(glGenRenderbuffers(1, &rbo));
	return rbo;
}

void gGLRenderEngine::deleteRenderbuffer(GLuint& rbo) {
	if(rbo != 0) {
		G_CHECK_GL(glDeleteRenderbuffers(1, &rbo));
	}
}

void gGLRenderEngine::bindRenderbuffer(GLuint rbo) {
	assert(rbo != 0);
	G_CHECK_GL(glBindRenderbuffer(GL_RENDERBUFFER, rbo));
}

void gGLRenderEngine::setRenderbufferStorage(GLenum format, int width, int height) {
	G_CHECK_GL(glRenderbufferStorage(GL_RENDERBUFFER, format, width, height));
}

// ----- Attachments -----
void gGLRenderEngine::attachTextureToFramebuffer(GLenum attachment, GLenum textarget, GLuint texId, GLuint level) {
	G_CHECK_GL(glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, textarget, texId, level));
}

void gGLRenderEngine::attachRenderbufferToFramebuffer(GLenum attachment, GLuint rbo) {
	G_CHECK_GL(glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachment, GL_RENDERBUFFER, rbo));
}

// ----- Draw/Read buffers -----
void gGLRenderEngine::setDrawBufferNone() {
#if defined(GLIST_MOBILE)
	G_CHECK_GL(glDrawBuffers(0, GL_NONE));
#else
	G_CHECK_GL(glDrawBuffer(GL_NONE));
#endif
}

void gGLRenderEngine::setReadBufferNone() {
	G_CHECK_GL(glReadBuffer(GL_NONE));
}

// ----- Fullscreen Quad -----
void gGLRenderEngine::createFullscreenQuad(GLuint& vao, GLuint& vbo) {
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

void gGLRenderEngine::deleteFullscreenQuad(GLuint& vao, GLuint* vbo) {
	if(vao != GL_NONE) {
		G_CHECK_GL(glDeleteVertexArrays(1, &vao));
	}
	if(vbo != GL_NONE) {
		G_CHECK_GL(glDeleteBuffers(1, vbo));
	}
}

GLuint gGLRenderEngine::loadProgram(const char* vertexSource, const char* fragmentSource, const char* geometrySource) {
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

void gGLRenderEngine::checkCompileErrors(GLuint shader, const std::string& type) {
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

void gGLRenderEngine::setBool(GLuint uniformloc, bool value) {
	G_CHECK_GL(glUniform1i(uniformloc, (int)value));
}

void gGLRenderEngine::setInt(GLuint uniformloc, int value) {
	G_CHECK_GL(glUniform1i(uniformloc, value));
}

void gGLRenderEngine::setUnsignedInt(GLuint uniformloc, unsigned int value) {
	G_CHECK_GL(glUniform1ui(uniformloc, value));
}

void gGLRenderEngine::setFloat(GLuint uniformloc, float value) {
	G_CHECK_GL(glUniform1f(uniformloc, value));
}

void gGLRenderEngine::setVec2(GLuint uniformloc, const glm::vec2& value) {
	G_CHECK_GL(glUniform2fv(uniformloc, 1, &value[0]));
}

void gGLRenderEngine::setVec2(GLuint uniformloc, float x, float y) {
	G_CHECK_GL(glUniform2f(uniformloc, x, y));
}

void gGLRenderEngine::setVec3(GLuint uniformloc, const glm::vec3& value) {
	G_CHECK_GL(glUniform3fv(uniformloc, 1, &value[0]));
}

void gGLRenderEngine::setVec3(GLuint uniformloc, float x, float y, float z) {
	G_CHECK_GL(glUniform3f(uniformloc, x, y, z));
}

void gGLRenderEngine::setVec4(GLuint uniformloc, const glm::vec4& value) {
	G_CHECK_GL(glUniform4fv(uniformloc, 1, &value[0]));
}

void gGLRenderEngine::setVec4(GLuint uniformloc, float x, float y, float z, float w) {
	G_CHECK_GL(glUniform4f(uniformloc, x, y, z, w));
}

void gGLRenderEngine::setMat2(GLuint uniformloc, const glm::mat2& mat) {
	G_CHECK_GL(glUniformMatrix2fv(uniformloc, 1, GL_FALSE, &mat[0][0]));
}

void gGLRenderEngine::setMat3(GLuint uniformloc, const glm::mat3& mat) {
	G_CHECK_GL(glUniformMatrix3fv(uniformloc, 1, GL_FALSE, &mat[0][0]));
}

void gGLRenderEngine::setMat4(GLuint uniformloc, const glm::mat4& mat) {
	G_CHECK_GL(glUniformMatrix4fv(uniformloc, 1, GL_FALSE, &mat[0][0]));
}

GLuint gGLRenderEngine::getUniformLocation(GLuint id, const std::string& name) {
	G_CHECK_GL2(GLuint location, glGetUniformLocation(id, name.c_str()));
	return location;
}

void gGLRenderEngine::useShader(GLuint id) const {
	G_CHECK_GL(glUseProgram(id));
}

void gGLRenderEngine::resetShader(GLuint id, bool loaded) const {
	if(loaded) {
		G_CHECK_GL(glDeleteShader(id));
	}
}

void gGLRenderEngine::clearScreen(bool color, bool depth) {
	GLbitfield mask = 0;
	if(color) mask |= GL_COLOR_BUFFER_BIT;
	if(depth) mask |= GL_DEPTH_BUFFER_BIT;
	glClear(mask);
}

void gGLRenderEngine::bindQuadVAO() {
	G_CHECK_GL(glBindVertexArray(gFbo::getQuadVao()));
}

void gGLRenderEngine::drawFullscreenQuad() {
	G_CHECK_GL(glDrawArrays(GL_TRIANGLES, 0, 6));
}

void gGLRenderEngine::bindDefaultFramebuffer() {
	G_CHECK_GL(glBindFramebuffer(GL_FRAMEBUFFER, gFbo::defaultfbo));
}

void gGLRenderEngine::drawVbo(const gVbo& vbo) {
	G_CHECK_GL(glDrawArrays(GL_TRIANGLES, 0, vbo.getVerticesNum()));
}

GLuint gGLRenderEngine::createTextures() {
	GLuint id;
	G_CHECK_GL(glGenTextures(1, &id));
	return id;
}

void gGLRenderEngine::bindTexture(GLuint texId) {
	G_CHECK_GL(glBindTexture(GL_TEXTURE_2D, texId));
}

void gGLRenderEngine::bindTexture(GLuint texId, int textureSlotNo) {
	G_CHECK_GL(glActiveTexture(GL_TEXTURE0 + textureSlotNo));
	G_CHECK_GL(glBindTexture(GL_TEXTURE_2D, texId));
}

void gGLRenderEngine::unbindTexture() {
	G_CHECK_GL(glBindTexture(GL_TEXTURE_2D, 0));
}

void gGLRenderEngine::activateTexture(int textureSlotNo) {
	G_CHECK_GL(glActiveTexture(GL_TEXTURE0 + textureSlotNo));
}

void gGLRenderEngine::resetTexture() {
	G_CHECK_GL(glActiveTexture(GL_TEXTURE0));
}

void gGLRenderEngine::deleteTexture(GLuint& texId) {
	if (texId != 0) {
		G_CHECK_GL(glDeleteTextures(1, &texId));
	}
}

void gGLRenderEngine::texImage2D(GLenum target, GLint internalFormat, int width, int height, GLint format,
                                 GLint type, void* data) {
	G_CHECK_GL(glTexImage2D(target, 0, internalFormat, width, height, 0, format, type, data));
}

void gGLRenderEngine::setWrapping(GLenum target, GLint wrapS, GLint wrapT) {
	G_CHECK_GL(glTexParameteri(target, GL_TEXTURE_WRAP_S, wrapS));
	G_CHECK_GL(glTexParameteri(target, GL_TEXTURE_WRAP_T, wrapT));
}

void gGLRenderEngine::setWrapping(GLenum target, GLint wrapS, GLint wrapT, GLint wrapR) {
	G_CHECK_GL(glTexParameteri(target, GL_TEXTURE_WRAP_S, wrapS));
	G_CHECK_GL(glTexParameteri(target, GL_TEXTURE_WRAP_T, wrapT));
	G_CHECK_GL(glTexParameteri(target, GL_TEXTURE_WRAP_R, wrapR));
}

void gGLRenderEngine::setFiltering(GLenum target, GLint minFilter, GLint magFilter) {
	G_CHECK_GL(glTexParameteri(target, GL_TEXTURE_MIN_FILTER, minFilter));
	G_CHECK_GL(glTexParameteri(target, GL_TEXTURE_MAG_FILTER, magFilter));
}

void gGLRenderEngine::setWrappingAndFiltering(GLenum target, GLint wrapS, GLint wrapT, GLint minFilter,
                                              GLint magFilter) {
	G_CHECK_GL(glTexParameteri(target, GL_TEXTURE_WRAP_S, wrapS));
	G_CHECK_GL(glTexParameteri(target, GL_TEXTURE_WRAP_T, wrapT));
	G_CHECK_GL(glTexParameteri(target, GL_TEXTURE_MIN_FILTER, minFilter));
	G_CHECK_GL(glTexParameteri(target, GL_TEXTURE_MAG_FILTER, magFilter));
}

void gGLRenderEngine::setWrappingAndFiltering(GLenum target, GLint wrapS, GLint wrapT, GLint wrapR, GLint minFilter,
                                              GLint magFilter) {
	G_CHECK_GL(glTexParameteri(target, GL_TEXTURE_WRAP_S, wrapS));
	G_CHECK_GL(glTexParameteri(target, GL_TEXTURE_WRAP_T, wrapT));
	G_CHECK_GL(glTexParameteri(target, GL_TEXTURE_WRAP_R, wrapR));
	G_CHECK_GL(glTexParameteri(target, GL_TEXTURE_MIN_FILTER, minFilter));
	G_CHECK_GL(glTexParameteri(target, GL_TEXTURE_MAG_FILTER, magFilter));
}

void gGLRenderEngine::setSwizzleMask(GLint swizzleMask[4]) {
#if defined(GLIST_MOBILE)
	G_CHECK_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_R, swizzleMask[0]));
	G_CHECK_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_G, swizzleMask[1]));
	G_CHECK_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B, swizzleMask[2]));
	G_CHECK_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_A, swizzleMask[3]));
#else
	G_CHECK_GL(glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask));
#endif
}

void gGLRenderEngine::readTexturePixels(unsigned char* inPixels, GLuint textureId, int width, int height,
                                        GLenum format) {
	GLuint fbo;
	G_CHECK_GL(glGenFramebuffers(1, &fbo));
	G_CHECK_GL(glBindFramebuffer(GL_FRAMEBUFFER, fbo));
	G_CHECK_GL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureId, 0));

	G_CHECK_GL(glReadPixels(0, 0, width, height, format, GL_UNSIGNED_BYTE, inPixels));

	G_CHECK_GL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
	G_CHECK_GL(glDeleteFramebuffers(1, &fbo));
}

void gGLRenderEngine::generateMipMap() {
	G_CHECK_GL(glGenerateMipmap(GL_TEXTURE_2D));
}

void gGLRenderEngine::bindSkyTexture(GLuint texId) {
	G_CHECK_GL(glBindTexture(GL_TEXTURE_CUBE_MAP, texId));
}

void gGLRenderEngine::bindSkyTexture(GLuint texId, int textureSlot) {
	G_CHECK_GL(glActiveTexture(textureSlot));
	G_CHECK_GL(glBindTexture(GL_TEXTURE_CUBE_MAP, texId));
}

void gGLRenderEngine::unbindSkyTexture() {
	G_CHECK_GL(glBindTexture(GL_TEXTURE_CUBE_MAP, 0));
}

void gGLRenderEngine::unbindSkyTexture(int textureSlotNo) {
	G_CHECK_GL(glActiveTexture(GL_TEXTURE0 + textureSlotNo));
	G_CHECK_GL(glBindTexture(GL_TEXTURE_CUBE_MAP, 0));
}

void gGLRenderEngine::generateSkyMipMap() {
	G_CHECK_GL(glGenerateMipmap(GL_TEXTURE_CUBE_MAP));
}

void gGLRenderEngine::enableDepthTestEqual() {
	G_CHECK_GL(glDepthFunc(GL_LEQUAL));
}

void gGLRenderEngine::createQuad(GLuint& inQuadVAO, GLuint& inQuadVBO) {
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

void gGLRenderEngine::enableCubeMapSeemless() {
#if defined(GLIST_MOBILE)
	G_CHECK_GL(glEnable(GL_TEXTURE_CUBE_MAP); // OpenGL ES does not support GL_TEXTURE_CUBE_MAP_SEAMLES)S
#else
	G_CHECK_GL(glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS));
#endif
}

void gGLRenderEngine::checkEnableCubeMap4Android() {
#if defined(ANDROID)
	G_CHECK_GL(glEnable(GL_TEXTURE_CUBE_MAP)); // OpenGL ES does not support GL_TEXTURE_CUBE_MAP_SEAMLESS
#endif
}

void gGLRenderEngine::pushMatrix() {
	G_CHECK_GL(glPushMatrix());
}

void gGLRenderEngine::popMatrix() {
	G_CHECK_GL(glPopMatrix());
}

void GLAPIENTRY openglErrorCallback(GLenum source, GLenum type, GLuint id,
								   GLenum severity, GLsizei length,
								   const GLchar* message, const void* userParam) {
	if (severity == GL_DEBUG_SEVERITY_NOTIFICATION) {
		return;
	}

	gLoge("gGLRenderEngine") << "Received OpenGL Debug Message: "<<
			"src: " << source << "\n"
			"type: " << type << "\n"
			"id: " << id << "\n"
			"severity: " << severity << "\n"
			"message: " << message;
	// We flush here because it might not show on the console immediately, having this function being spammed will slow down the game but for purpose of debugging, it is required.
	std::cerr << std::flush;
}

void gGLRenderEngine::init() {
#if defined(DEBUG) || defined(ENGINE_OPENGL_CHECKS)
	// On newer versions of OpenGL, debug callbacks are available; we enable them only for debug builds because it might have a performance impact.
	// You can place a debug point and go back to the original source of the message from the stack trace, because it is sync.
	if (GLEW_VERSION_4_3 || GLEW_ARB_debug_output) {
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(openglErrorCallback, nullptr);
	}
#endif
	gRenderer::init();
}

void gGLRenderEngine::updatePackUnpackAlignment(int i) {
	G_CHECK_GL(glPixelStorei(GL_UNPACK_ALIGNMENT, i));
	G_CHECK_GL(glPixelStorei(GL_PACK_ALIGNMENT, i));
}
