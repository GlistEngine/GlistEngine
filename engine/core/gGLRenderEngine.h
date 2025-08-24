//
// Created by sadettin on 13.08.2025.
//

#pragma once

#ifndef CORE_GGLENGINERENDERER_H
#define CORE_GGLENGINERENDERER_H
#include "gRenderer.h"
#include "gUbo.h"

class gGLRenderEngine : public gRenderer {
public:
	gGLRenderEngine() = default;
	~gGLRenderEngine();

	void clear();
	void clearColor(int r, int g, int b, int a = 255);
	void clearColor(gColor color);

	void takeScreenshot(gImage& img, int x, int y, int width, int height);
	void takeScreenshot(gImage& img);

	void enableDepthTest();
	void enableDepthTest(int depthTestType);
	void setDepthTestFunc(int depthTestType);
	void disableDepthTest();
	bool isDepthTestEnabled();
	int getDepthTestType();

	void enableAlphaBlending();
	void disableAlphaBlending();
	bool isAlphaBlendingEnabled();
	void enableAlphaTest();
	void disableAlphaTest();
	bool isAlphaTestEnabled();

	/* -------------- gUbo ------------- */
	GLuint genBuffers();
	void deleteBuffer(GLuint& buffer);

	void bindBuffer(GLenum target, GLuint buffer);
	void unbindBuffer(GLenum target);

	void bufSubData(GLuint buffer, int offset, int size, const void* data);
	void setBufferData(GLuint buffer, const void* data, size_t size, int usage);
	void setBufferRange(int index, GLuint buffer, int offset, int size);

	/* -------------- gVbo --------------- */
	GLuint createVAO();
	void deleteVAO(GLuint& vao);

	void bindVAO(GLuint vao);
	void unbindVAO();

	void setVertexBufferData(GLuint vbo, size_t size, const void* data, int usage);
	void setIndexBufferData(GLuint ebo, size_t size, const void* data, int usage);

	void drawArrays(int drawMode, int count);
	void drawElements(int drawMode, int count);

	void enableVertexAttrib(int index);
	void disableVertexAttrib(int index);
	void setVertexAttribPointer(int index, int size, int type, bool normalized, int stride, const void* pointer);

	/* -------------- gFbo --------------- */
	GLuint createFramebuffer();
	void deleteFramebuffer(GLuint& fbo);
	void bindFramebuffer(GLuint fbo);
	void checkFramebufferStatus();

	GLuint createRenderbuffer();
	void deleteRenderbuffer(GLuint& rbo);
	void bindRenderbuffer(GLuint rbo);
	void setRenderbufferStorage(GLenum format, int width, int height);

	void attachTextureToFramebuffer(GLenum attachment, GLenum textarget, GLuint texId, GLuint level = 0);
	void attachRenderbufferToFramebuffer(GLenum attachment, GLuint rbo);

	void setDrawBufferNone();
	void setReadBufferNone();

	void createFullscreenQuad(GLuint& vao, GLuint& vbo);
	void deleteFullscreenQuad(GLuint& vao, GLuint* vbo);

	/* -------------- gShader --------------- */
	// This function loads shaders without preproccesing them. Geometry source can be nullptr.
	GLuint loadProgram(const char* vertexSource, const char* fragmentSource, const char* geometrySource);
	void checkCompileErrors(GLuint shader, const std::string& type);
	void setBool(GLuint id, const std::string& name, bool value);
	void setInt(GLuint id, const std::string& name, int value);
	void setFloat(GLuint id, const std::string& name, float value);
	void setVec2(GLuint id, const std::string& name, const glm::vec2& value);
	void setVec2(GLuint id, const std::string& name, float x, float y);
	void setVec3(GLuint id, const std::string& name, const glm::vec3& value);
	void setVec3(GLuint id, const std::string& name, float x, float y, float z);
	void setVec4(GLuint id, const std::string& name, const glm::vec4& value);
	void setVec4(GLuint id, const std::string& name, float x, float y, float z, float w);
	void setMat2(GLuint id, const std::string& name, const glm::mat2& mat);
	void setMat3(GLuint id, const std::string& name, const glm::mat3& mat);
	void setMat4(GLuint id, const std::string& name, const glm::mat4& mat);

	void useShader(GLuint id) const;
	void resetShader(GLuint id, bool loaded) const;
	GLint getUniformLocation(GLuint id, const std::string& name);

	void attachUbo(GLuint id, const gUbo<gSceneLights>* ubo, const std::string& uboName) {
		unsigned int blockIndex;
		G_CHECK_GL2(blockIndex, glGetUniformBlockIndex(id, uboName.c_str()));
		G_CHECK_GL(glUniformBlockBinding(id, blockIndex, ubo->getBindingPoint()));
		/*if (blockIndex != GL_INVALID_INDEX) {
			GLint blockSize;
			G_CHECK_GL(glGetActiveUniformBlockiv(id, blockIndex, GL_UNIFORM_BLOCK_DATA_SIZE, &blockSize));

			GLint blockAlignment;
			G_CHECK_GL(glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &blockAlignment));

			std::cout << "Uniform block alignment: " << blockAlignment << " bytes" << std::endl;

			// Now 'blockSize' contains the size of the uniform block in bytes
			std::cout << "Uniform block size: " << blockSize << " bytes" << std::endl;
			if (ubo->getSize() != blockSize) {
				std::cout << "Error: Uniform block size mismatch. actual size: " << ubo->getSize() << ", block size: " << blockSize << std::endl;
			}
		} else {
			std::cerr << "Error: Unable to find uniform block index." << std::endl;
		}*/
	}

	/* ------------ gPostProcessManager ------------- */
	void clearScreen(bool color = true, bool depth = true);
	void bindQuadVAO();
	void drawFullscreenQuad();
	void bindDefaultFramebuffer();

	/* -------------- gGrid --------------- */
	void drawVbo(const gVbo& vbo);

	/* ---------------- gTexture ---------------- */
	GLuint createTextures();
	void bindTexture(GLuint texId);
	void bindTexture(GLuint texId, int textureSlotNo);
	void unbindTexture();
	void activateTexture(int textureSlotNo = 0);
	void resetTexture();
	void deleteTexture(GLuint& texId);

	void texImage2D(GLenum target, GLint internalFormat, int width, int height, GLint format, GLint type, void* data);
	void setWrapping(GLenum target, GLint wrapS, GLint wrapT);
	void setWrapping(GLenum target, GLint wrapS, GLint wrapT, GLint wrapR);

	void setFiltering(GLenum target, GLint minFilter, GLint magFilter);
	void setWrappingAndFiltering(GLenum target, GLint wrapS, GLint wrapT, GLint minFilter, GLint magFilter);
	void setWrappingAndFiltering(GLenum target, GLint wrapS, GLint wrapT, GLint wrapR, GLint minFilter, GLint magFilter);
	void setSwizzleMask(GLint swizzleMask[4]);

	void readTexturePixels(unsigned char* inPixels, GLuint textureId, int width, int height, GLenum format);

	void generateMipMap();

	/* ---------------- gSkybox ---------------- */
	void bindSkyTexture(GLuint texId);
	void bindSkyTexture(GLuint texId, int textureSlot);
	void unbindSkyTexture();
	void unbindSkyTexture(int textureSlotNo);
	void generateSkyMipMap();
	void enableDepthTestEqual();
	void createQuad(GLuint& inQuadVAO, GLuint& inQuadVBO);
	void enableCubeMapSeemless();
	void checkEnableCubeMap4Android();

	/* ---------------- gRenderObject ---------------- */
	void pushMatrix();
	void popMatrix();
private:
	void updatePackUnpackAlignment(int i);
};

#endif