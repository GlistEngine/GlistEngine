//
// Created by sadettin on 13.08.2025.
//

#pragma once

#ifndef CORE_GVKRENDERENGINE_H
#define CORE_GVKRENDERENGINE_H
#include "gRenderer.h"
#include "gUbo.h"
#include <unordered_map>

// All Vulkan objects live behind these opaque types, so <vulkan/vulkan.h> is not
// pushed into every translation unit that includes this header and the class
// still compiles on platforms that have no Vulkan headers.
struct gVKContext;
struct gVKTexture;

class gVKRenderEngine : public gRenderer {
public:
	gVKRenderEngine() = default;
	~gVKRenderEngine() override;

	bool beginFrame() override;
	void endFrame() override;

	void clear() override;
	void clearColor(int r, int g, int b, int a = 255) override;
	void clearColor(gColor color) override;

	void takeScreenshot(gImage& img, int x, int y, int width, int height) override;
	void takeScreenshot(gImage& img) override;

	void enableDepthTest() override;
	void enableDepthTest(int depthTestType) override;
	void setDepthTestFunc(int depthTestType) override;
	void disableDepthTest() override;
	bool isDepthTestEnabled() override;
	int getDepthTestType() override;

	void enableAlphaBlending() override;
	void disableAlphaBlending() override;
	bool isAlphaBlendingEnabled() override;
	void enableAlphaTest() override;
	void disableAlphaTest() override;
	bool isAlphaTestEnabled() override;

	/* -------------- gUbo ------------- */
	GLuint genBuffers() override;
	void deleteBuffer(GLuint& buffer) override;

	void bindBuffer(GLenum target, GLuint buffer) override;
	void unbindBuffer(GLenum target) override;

	void bufSubData(GLuint buffer, int offset, int size, const void* data) override;
	void setBufferData(GLuint buffer, const void* data, size_t size, int usage) override;
	void setBufferRange(int index, GLuint buffer, int offset, int size) override;

	/* -------------- gVbo --------------- */
	GLuint createVAO() override;
	void deleteVAO(GLuint& vao) override;

	void bindVAO(GLuint vao) override;
	void unbindVAO() override;

	void setVertexBufferData(GLuint vbo, size_t size, const void* data, int usage) override;
	void setIndexBufferData(GLuint ebo, size_t size, const void* data, int usage) override;

	void drawArrays(int drawMode, int count) override;
	void drawElements(int drawMode, int count) override;
	void drawArraysInstanced(int drawMode, int count, int instanceCount) override;
	void drawElementsInstanced(int drawMode, int count, int instanceCount) override;

	void enableVertexAttrib(int index) override;
	void disableVertexAttrib(int index) override;
	void setVertexAttribPointer(int index, int size, int type, bool normalized, int stride, const void* pointer) override;
	void setVertexAttribDivisor(int index, int divisor) override;
	void setViewport(int x, int y, int width, int height) override;

	/* -------------- gFbo --------------- */
	GLuint createFramebuffer() override;
	void deleteFramebuffer(GLuint& fbo) override;
	void bindFramebuffer(GLuint fbo) override;
	void checkFramebufferStatus() override;

	GLuint createRenderbuffer() override;
	void deleteRenderbuffer(GLuint& rbo) override;
	void bindRenderbuffer(GLuint rbo) override;
	void setRenderbufferStorage(GLenum format, int width, int height) override;

	void attachTextureToFramebuffer(GLenum attachment, GLenum textarget, GLuint texId, GLuint level = 0) override;
	void attachRenderbufferToFramebuffer(GLenum attachment, GLuint rbo) override;

	void setDrawBufferNone() override;
	void setReadBufferNone() override;

	void createFullscreenQuad(GLuint& vao, GLuint& vbo) override;
	void deleteFullscreenQuad(GLuint& vao, GLuint* vbo) override;

	/* -------------- gShader --------------- */
	// This function loads shaders without preproccesing them. Geometry source can be nullptr.
	GLuint loadProgram(const char* vertexSource, const char* fragmentSource, const char* geometrySource) override;
	void checkCompileErrors(GLuint shader, const std::string& type) override;
	void setBool(GLuint uniformloc, bool value) override;
	void setInt(GLuint uniformloc, int value) override;
	void setUnsignedInt(GLuint uniformloc, unsigned int value) override;
	void setFloat(GLuint uniformloc, float value) override;
	void setVec2(GLuint uniformloc, const glm::vec2& value) override;
	void setVec2(GLuint uniformloc, float x, float y) override;
	void setVec3(GLuint uniformloc, const glm::vec3& value) override;
	void setVec3(GLuint uniformloc, float x, float y, float z) override;
	void setVec4(GLuint uniformloc, const glm::vec4& value) override;
	void setVec4(GLuint uniformloc, float x, float y, float z, float w) override;
	void setMat2(GLuint uniformloc, const glm::mat2& mat) override;
	void setMat3(GLuint uniformloc, const glm::mat3& mat) override;
	void setMat4(GLuint uniformloc, const glm::mat4& mat) override;
	GLuint getUniformLocation(GLuint id, const std::string& name) override;

	void useShader(GLuint id) const override;
	void resetShader(GLuint id, bool loaded) const override;

	void attachUbo(GLuint id, int bindingpoint, const std::string& uboName) override {
		unsigned int blockIndex;
		G_CHECK_GL2(blockIndex, glGetUniformBlockIndex(id, uboName.c_str()));
		G_CHECK_GL(glUniformBlockBinding(id, blockIndex, bindingpoint));
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
	void clearScreen(bool color = true, bool depth = true) override;
	void bindQuadVAO() override;
	void drawFullscreenQuad() override;
	void bindDefaultFramebuffer() override;

	/* -------------- gGrid --------------- */
	void drawVbo(const gVbo& vbo) override;

	/* ---------------- gTexture ---------------- */
	GLuint createTextures() override;
	void bindTexture(GLuint texId) override;
	void bindTexture(GLuint texId, int textureSlotNo) override;
	void unbindTexture() override;
	void activateTexture(int textureSlotNo = 0) override;
	void resetTexture() override;
	void deleteTexture(GLuint& texId) override;

	void texImage2D(GLenum target, GLint internalFormat, int width, int height, GLint format, GLint type, void* data) override;
	void setWrapping(GLenum target, GLint wrapS, GLint wrapT) override;
	void setWrapping(GLenum target, GLint wrapS, GLint wrapT, GLint wrapR) override;

	void setFiltering(GLenum target, GLint minFilter, GLint magFilter) override;
	void setWrappingAndFiltering(GLenum target, GLint wrapS, GLint wrapT, GLint minFilter, GLint magFilter) override;
	void setWrappingAndFiltering(GLenum target, GLint wrapS, GLint wrapT, GLint wrapR, GLint minFilter, GLint magFilter) override;
	void setSwizzleMask(GLint swizzleMask[4]) override;

	void readTexturePixels(unsigned char* inPixels, GLuint textureId, int width, int height, GLenum format) override;
	void readTexturePixelsHDR(float* inPixels, GLuint textureId, int width, int height, GLenum format) override;

	void generateMipMap() override;

	/* ---------------- gSkybox ---------------- */
	void bindSkyTexture(GLuint texId) override;
	void bindSkyTexture(GLuint texId, int textureSlot) override;
	void unbindSkyTexture() override;
	void unbindSkyTexture(int textureSlotNo) override;
	void generateSkyMipMap() override;
	void enableDepthTestEqual() override;
	void createQuad(GLuint& inQuadVAO, GLuint& inQuadVBO) override;
	void enableCubeMap() override;

	/* ---------------- gRenderObject ---------------- */
	void pushMatrix() override;
	void popMatrix() override;

	/* ---------------- 2D draw path ---------------- */
	// Records coloured 2D geometry into the active frame (the Vulkan side of the
	// primitive meshes). drawMode is a GL primitive constant, the same one the mesh
	// would hand OpenGL. No-op if no frame is active.
	void drawColored2D(const glm::vec2* points, int count, const glm::vec4& color, const glm::mat4& mvp,
			int drawMode = GL_TRIANGLES) override;
	void drawMaterialMesh3D(const MeshVertex3D* vertices, int count, const glm::vec4& ambient,
			const glm::vec4& diffuse,
			GLuint textureId, const glm::mat4& mvp, int drawMode = GL_TRIANGLES) override;

	// Records a textured quad using the registered Vulkan texture for textureId
	// (the Vulkan side of gImage / gTexture::draw and drawSub). No-op if the id is
	// unknown. An unknown mask id draws unmasked rather than dropping the image.
	void drawTexturedRect2D(GLuint textureId, GLuint maskTextureId, const glm::vec4& tint,
			const glm::mat4& mvp,
			const glm::vec2& uvOffset = glm::vec2(0.0f), const glm::vec2& uvScale = glm::vec2(1.0f)) override;

	/* ---------------- Vulkan context ---------------- */
	// gVKContext is opaque here (its layout lives in the .cpp), so a developer can
	// reach the accessor rich context without this header ever pulling in
	// <vulkan/vulkan.h>. getContext() creates the context on first access when it
	// does not exist yet, so on a Vulkan-capable build it never returns null:
	// getContext()->setMinApiVersion(...) before init() is safe, and init() adopts
	// this same context. Whether the handles are actually populated is a separate
	// question - check isInitialized() before using them. (A build without Vulkan
	// support has no context type, so there it returns null.)
	gVKContext* getContext();

	// Takes ownership. Any context the engine already holds is torn down first -
	// its Vulkan objects destroyed and its memory freed through cleanupVulkan() -
	// so replacing a context never leaks the old one. The self assignment guard
	// avoids destroying the very context being handed in. The engine frees the
	// adopted context on cleanup.
	void setContext(gVKContext* context);

protected:
	void init() override;
	void cleanup() override;
private:
	mutable GLuint currentprogram = 0;
	bool initVulkan();
	void cleanupVulkan();
	gVKContext* vkcontext = nullptr;
	void updatePackUnpackAlignment(int i) override;

	// Registry backing the GLuint texture ids gTexture/gImage hand out: createTextures
	// mints an id, texImage2D fills the Vulkan texture for the currently bound id,
	// and deleteTexture frees it. boundtextureid mirrors the OpenGL bind state that
	// gTexture's upload path relies on.
	std::unordered_map<GLuint, gVKTexture*> vktextures;
	gVKTexture* defaultvktexture = nullptr;
	GLuint nextvktextureid = 1;
	GLuint boundtextureid = 0;
	// Synthetic vertex array ids. gVbo asserts that its VAO is not GL_NONE before
	// binding, so every gMesh needs a unique non-zero id even though Vulkan has no
	// such object.
	GLuint nextvkvaoid = 1;
	void destroyAllTextures();
	// The Vulkan texture behind the currently bound id, or null when there is none
	// yet - gTexture sets filtering and wrapping both before and after the upload.
	gVKTexture* getBoundVKTexture();

	// Shader hot reload. Development builds watch the .vert / .frag sources the 2D
	// pipelines are compiled from and rebuild them when one is saved, so a shader
	// can be tuned without restarting. Release builds compile nothing at runtime,
	// the timestamp stays 0 and this costs one comparison per frame.
	void checkShaderReload();
	long long shadersourcetimestamp = 0;
	int shaderpollcountdown = 0;
};

#endif
