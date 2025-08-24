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

void gCheckGLErrorAndPrint(const std::string& prefix, const std::string& func, int line) {
	GLenum error = glGetError();
	if (error != GL_NO_ERROR) {
		gLogi("gGLRenderEngine") << prefix << "OpenGL ERROR at " << func << ", line " << line << ", error: " << gToHex(error, 4);
	}
}
void gEnableCulling() {
	glEnable(GL_CULL_FACE);
}

void gDisableCulling() {
	glDisable(GL_CULL_FACE);
}

bool gIsCullingEnabled() {
	return glIsEnabled(GL_CULL_FACE);
}

void gCullFace(int cullingFace) {
	glCullFace(cullingFace);
}

int gGetCullFace() {
	GLint i;
	glGetIntegerv(GL_CULL_FACE_MODE, &i);
	return i;
}

void gSetCullingDirection(int cullingDirection) {
	glFrontFace(cullingDirection);
}

int gGetCullingDirection() {
	GLint i;
	glGetIntegerv(GL_FRONT_FACE, &i);
	return i;
}

gGLRenderEngine::~gGLRenderEngine() {
	delete colorshader;
	delete textureshader;
	delete imageshader;
	delete fontshader;
	delete skyboxshader;
	delete shadowmapshader;
	delete pbrshader;
	delete equirectangularshader;
	delete irradianceshader;
	delete prefiltershader;
	delete brdfshader;
	delete fboshader;
	delete rendercolor;
	delete lightsubo;
	delete gridshader;
	if(!isdevelopergrid) delete originalgrid;
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
	enableDepthTest(DEPTHTESTTYPE_LESS);
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
	glEnable(GL_ALPHA_TEST);
	glAlphaFunc(GL_GREATER, 0.1);
	isalphatestenabled = true;
#endif
}

void gGLRenderEngine::disableAlphaTest() {
#if defined(WIN32) || defined(LINUX)
	glDisable(GL_ALPHA_TEST);
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

gImage gGLRenderEngine::takeScreenshot(int x, int y, int width, int height) {
	unsigned char* pixeldata = new unsigned char[width * height * 4];
	glReadPixels(x, getHeight() - y - height, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixeldata);
	flipVertically(pixeldata, width, height, 4);
	gImage screenshot;
	screenshot.setImageData(pixeldata, width, height, 4);
	//std::string imagePath = "output.png";   USE IT TO SAVE THE IMAGE
	// screenShot->saveImage(imagePath);  USE IT TO SAVE THE IMAGE
	return screenshot;
}

gImage gGLRenderEngine::takeScreenshot() {
	int height = gBaseApp::getAppManager()->getWindow()->getHeight();
	int width = gBaseApp::getAppManager()->getWindow()->getWidth();
	unsigned char* pixeldata = new unsigned char[width * height * 4];
	glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixeldata);
	flipVertically(pixeldata, width, height, 4);
	gImage screenshot;
	screenshot.setImageData(pixeldata, width, height, 4);
	//std::string imagePath = "output.png";   USE IT TO SAVE THE IMAGE
	// screenShot->saveImage(imagePath);  USE IT TO SAVE THE IMAGE
	return screenshot;
}


GLuint gGLRenderEngine::genBuffers() {
	GLuint buffer;
	glGenBuffers(1, &buffer);
	return buffer;
}

void gGLRenderEngine::deleteBuffer(GLuint* buffer) {
	if(buffer != 0) {
		glDeleteBuffers(1, buffer);
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
	glGenVertexArrays(1, &vao);
	return vao;
}

void gGLRenderEngine::deleteVAO(GLuint* vao) {
	if(vao != 0) {
		glDeleteVertexArrays(1, vao);
	}
}

void gGLRenderEngine::bindVAO(GLuint vao) {
	assert(vao != 0);
	G_CHECK_GL(glBindVertexArray(vao));
}

void gGLRenderEngine::unbindVAO() {
	G_CHECK_GL(glBindVertexArray(0));
}

void gGLRenderEngine::setVertexBufferData(GLuint vbo, size_t size, const void* data, int usage) {
	assert(vbo != 0);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, size, data, usage);
}

void gGLRenderEngine::setIndexBufferData(GLuint ebo, size_t size, const void* data, int usage) {
	assert(ebo != 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, data, usage);
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
	glEnableVertexAttribArray(index);
}

void gGLRenderEngine::disableVertexAttrib(int index) {
	glDisableVertexAttribArray(index);
}

void gGLRenderEngine::setVertexAttribPointer(int index, int size, int type, bool normalized, int stride,
                                             const void* pointer) {
	glVertexAttribPointer(index, size, type, normalized ? GL_TRUE : GL_FALSE, stride, pointer);
}

// ----- Framebuffer -----
GLuint gGLRenderEngine::createFramebuffer() {
	GLuint fbo;
	glGenFramebuffers(1, &fbo);
	return fbo;
}

void gGLRenderEngine::deleteFramebuffer(GLuint* fbo) {
	if(fbo != 0) {
		glDeleteFramebuffers(1, fbo);
	}
}

void gGLRenderEngine::bindFramebuffer(GLuint fbo) {
	G_CHECK_GL(glBindFramebuffer(GL_FRAMEBUFFER, fbo));
}

void gGLRenderEngine::checkFramebufferStatus() {
	// check if fbo complete
	GLuint status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
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

void gGLRenderEngine::deleteRenderbuffer(GLuint* rbo) {
	if(rbo != 0) {
		glDeleteRenderbuffers(1, rbo);
	}
}

void gGLRenderEngine::bindRenderbuffer(GLuint rbo) {
	assert(rbo != 0);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
}

void gGLRenderEngine::setRenderbufferStorage(GLenum format, int width, int height) {
	glRenderbufferStorage(GL_RENDERBUFFER, format, width, height);
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
	float quadVertices[] = {
		// positions   // texCoords
		-1.0f, 1.0f, 0.0f, 1.0f,
		-1.0f, -1.0f, 0.0f, 0.0f,
		1.0f, -1.0f, 1.0f, 0.0f,

		-1.0f, 1.0f, 0.0f, 1.0f,
		1.0f, -1.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 1.0f, 1.0f
	};

	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);

	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
}

void gGLRenderEngine::deleteFullscreenQuad(GLuint* vao, GLuint* vbo) {
	if(vao != 0)
		glDeleteVertexArrays(1, vao);
	if(vbo != 0)
		glDeleteBuffers(1, vbo);
}

GLuint gGLRenderEngine::loadProgram(const char* vertexSource, const char* fragmentSource, const char* geometrySource) {
	unsigned int vertex = GL_NONE;
	unsigned int fragment = GL_NONE;
#if defined(WIN32) || defined(LINUX)
	unsigned int geometry = GL_NONE;
#endif
	// vertex shader
	G_CHECK_GL2(vertex, glCreateShader(GL_VERTEX_SHADER));
	glShaderSource(vertex, 1, &vertexSource, nullptr);
	glCompileShader(vertex);
	checkCompileErrors(vertex, "VERTEX");

	// fragment Shader
	G_CHECK_GL2(fragment, glCreateShader(GL_FRAGMENT_SHADER));
	glShaderSource(fragment, 1, &fragmentSource, nullptr);
	glCompileShader(fragment);
	checkCompileErrors(fragment, "FRAGMENT");

	// if geometry shader is given, compile geometry shader
#if defined(WIN32) || defined(LINUX)
	if(geometrySource != nullptr) {
		geometry = glCreateShader(GL_GEOMETRY_SHADER);
		glShaderSource(geometry, 1, &geometrySource, nullptr);
		glCompileShader(geometry);
		checkCompileErrors(geometry, "GEOMETRY");
	}
#endif

	// shader Program
	GLuint id = glCreateProgram();
	glAttachShader(id, vertex);
	glAttachShader(id, fragment);
#if defined(WIN32) || defined(LINUX)
	if(geometrySource != nullptr)
		glAttachShader(id, geometry);
#endif
	glLinkProgram(id);
	checkCompileErrors(id, "PROGRAM");
	// delete the shaders as they're linked into our program now and no longer necessery
	glDeleteShader(vertex);
	glDeleteShader(fragment);
#if defined(WIN32) || defined(LINUX)
	if(geometrySource != nullptr)
		glDeleteShader(geometry);
#endif

	return id;
}

void gGLRenderEngine::checkCompileErrors(GLuint shader, const std::string& type) {
	GLint success;
	GLchar infoLog[1024];
	if(type != "PROGRAM") {
		G_CHECK_GL(glGetShaderiv(shader, GL_COMPILE_STATUS, &success));
		if(!success) {
			glGetShaderInfoLog(shader, 1024, nullptr, infoLog);
			gLoge("gShader") << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog <<
					"\n -- --------------------------------------------------- -- " << std::endl;
		}
	} else {
		glGetProgramiv(shader, GL_LINK_STATUS, &success);
		if(!success) {
			glGetProgramInfoLog(shader, 1024, nullptr, infoLog);
			gLoge("gShader") << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog <<
					"\n -- --------------------------------------------------- -- " << std::endl;
		}
	}
#ifdef DEBUG
	assert(success);
#endif
}

void gGLRenderEngine::setBool(GLuint id, const std::string& name, bool value) {
	G_CHECK_GL(glUniform1i(getUniformLocation(id, name), (int)value));
}

void gGLRenderEngine::setInt(GLuint id, const std::string& name, int value) {
	G_CHECK_GL(glUniform1i(getUniformLocation(id, name), value));
}

void gGLRenderEngine::setFloat(GLuint id, const std::string& name, float value) {
	G_CHECK_GL(glUniform1f(getUniformLocation(id, name), value));
}

void gGLRenderEngine::setVec2(GLuint id, const std::string& name, const glm::vec2& value) {
	G_CHECK_GL(glUniform2fv(getUniformLocation(id, name), 1, &value[0]));
}

void gGLRenderEngine::setVec2(GLuint id, const std::string& name, float x, float y) {
	G_CHECK_GL(glUniform2f(getUniformLocation(id, name), x, y));
}

void gGLRenderEngine::setVec3(GLuint id, const std::string& name, const glm::vec3& value) {
	G_CHECK_GL(glUniform3fv(getUniformLocation(id, name), 1, &value[0]));
}

void gGLRenderEngine::setVec3(GLuint id, const std::string& name, float x, float y, float z) {
	G_CHECK_GL(glUniform3f(getUniformLocation(id, name), x, y, z));
}

void gGLRenderEngine::setVec4(GLuint id, const std::string& name, const glm::vec4& value) {
	G_CHECK_GL(glUniform4fv(getUniformLocation(id, name), 1, &value[0]));
}

void gGLRenderEngine::setVec4(GLuint id, const std::string& name, float x, float y, float z, float w) {
	G_CHECK_GL(glUniform4f(getUniformLocation(id, name), x, y, z, w));
}

void gGLRenderEngine::setMat2(GLuint id, const std::string& name, const glm::mat2& mat) {
	G_CHECK_GL(glUniformMatrix2fv(getUniformLocation(id, name), 1, GL_FALSE, &mat[0][0]));
}

void gGLRenderEngine::setMat3(GLuint id, const std::string& name, const glm::mat3& mat) {
	G_CHECK_GL(glUniformMatrix3fv(getUniformLocation(id, name), 1, GL_FALSE, &mat[0][0]));
}

void gGLRenderEngine::setMat4(GLuint id, const std::string& name, const glm::mat4& mat) {
	G_CHECK_GL(glUniformMatrix4fv(getUniformLocation(id, name), 1, GL_FALSE, &mat[0][0]));
}

void gGLRenderEngine::useShader(GLuint id) const {
	G_CHECK_GL(glUseProgram(id));
}

void gGLRenderEngine::resetShader(GLuint id, bool loaded) const {
	if(loaded) {
		glDeleteShader(id);
	}
}

GLint gGLRenderEngine::getUniformLocation(GLuint id, const std::string& name) {
	return glGetUniformLocation(id, name.c_str());
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
	glDrawArrays(GL_TRIANGLES, 0, 6);
}

void gGLRenderEngine::bindDefaultFramebuffer() {
	glBindFramebuffer(GL_FRAMEBUFFER, gFbo::defaultfbo);
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

void gGLRenderEngine::deleteTexture(GLuint* texId) {
	G_CHECK_GL(glDeleteTextures(1, texId));
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
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureId, 0);

	glReadPixels(0, 0, width, height, format, GL_UNSIGNED_BYTE, inPixels);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDeleteFramebuffers(1, &fbo);
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
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
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
	glGenVertexArrays(1, &inQuadVAO);
	glGenBuffers(1, &inQuadVBO);
	glBindVertexArray(inQuadVAO);
	glBindBuffer(GL_ARRAY_BUFFER, inQuadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
}
