//
// Created by sadettin on 13.08.2025.
//

#pragma once

#ifndef CORE_GVKRENDERENGINE_H
#define CORE_GVKRENDERENGINE_H
#include "gRenderer.h"
#include "gUbo.h"
#include <unordered_map>
#include <vector>

// All Vulkan objects live behind these opaque types, so <vulkan/vulkan.h> is not
// pushed into every translation unit that includes this header and the class
// still compiles on platforms that have no Vulkan headers.
struct gVKContext;
struct gVKTexture;
struct gVKFramebuffer;
struct gVKMeshBuffer;

// What a vertex array id stands for: the pair of buffer ids that were bound while
// it was current, 0 meaning nothing was bound. Vulkan has no vertex array object,
// but gVbo binds one around every upload and every draw, so remembering the pairing
// is what lets a draw find the buffers it is about. Defined here rather than in
// gVKMeshBuffer.h because it holds no Vulkan types and so needs no Vulkan headers.
struct gVKVertexArray {
	GLuint vertexbuffer = 0;
	GLuint indexbuffer = 0;
	// A vertex array that is drawn instanced has a second array buffer holding one
	// model matrix per instance. gVbo uploads it through the same entry points as
	// the vertex data, so the backend tells them apart by order: the first array
	// buffer a vertex array sees is its vertices, a later different one is the
	// instance data. setVertexAttribDivisor confirms it.
	GLuint instancebuffer = 0;
};

class gVKRenderEngine : public gRenderer {
public:
	gVKRenderEngine() = default;
	~gVKRenderEngine() override;

	bool beginFrame() override;
	void endFrame() override;
	void flushQueuedDraws() override;

	void clear() override;
	void clearColor(int r, int g, int b, int a = 255) override;
	void clearColor(gColor color) override;

	// Takes the camera's OpenGL-style projection and stores the Vulkan equivalent.
	// See gVKRenderEngine.cpp for what the correction is and why the Y axis is not
	// part of it.
	void setProjectionMatrix(glm::mat4 projectionMatrix) override;

	void takeScreenshot(gImage& img, int x, int y, int width, int height) override;
	void takeScreenshot(gImage& img) override;

	void enableDepthTest() override;
	void enableDepthTest(int depthTestType) override;
	void setDepthTestFunc(int depthTestType) override;
	void disableDepthTest() override;
	bool isDepthTestEnabled() override;
	int getDepthTestType() override;
	void enableCulling() override;
	void disableCulling() override;
	void setCullFace(int face) override;
	void setCullingDirection(int direction) override;

	void enableAlphaBlending() override;
	void setBlendMode(int blendMode) override;
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
	void drawVbo(const gVbo& vbo, const glm::mat4& model, const gMeshSurface& surface) override;

	/* ---------------- gTexture ---------------- */
	GLuint createTextures() override;
	void bindTexture(GLuint texId) override;
	void bindTexture(GLuint texId, int textureSlotNo) override;
	void unbindTexture() override;
	void activateTexture(int textureSlotNo = 0) override;
	void resetTexture() override;
	void deleteTexture(GLuint& texId) override;

	void texImage2D(GLenum target, GLint internalFormat, int width, int height, GLint format, GLint type, void* data, GLint level = 0) override;
	void setTextureMaxLevel(GLenum target, int maxLevel) override;
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

	// Records a textured quad using the registered Vulkan texture for textureId
	// (the Vulkan side of gImage / gTexture::draw and drawSub). No-op if the id is
	// unknown. An unknown mask id draws unmasked rather than dropping the image.
	void drawMesh3D(GLuint vertexArrayId, int vertexCount, int indexCount,
			const glm::mat4& model, const gMeshSurface& surface,
			int drawMode = GL_TRIANGLES, int instanceCount = 1) override;

	bool drawSkyboxFace(GLuint textureId, const float* xyzuv, int vertexCount,
			const glm::mat4& viewProjection) override;

	bool allocateShadowMap(int width, int height) override;
	void releaseShadowMap() override;
	bool beginShadowPass() override;
	void endShadowPass() override;
	bool isShadowPassActive() const override;
	void setShadowMapState(bool enabled, const glm::mat4& lightMatrix,
			const glm::vec3& lightPosition, bool softShadows) override;

	// Drops the gathered scene block so the next 3D draw rebuilds it; see the base
	// declaration for why the two backends cannot share one moment for this.
	void updateLights() override;

	// Rebuilds the swapchain with the present mode the new setting asks for.
	void setVsync(bool enabled) override;

	// Rebuilds the screen render pass, its attachments and every pipeline at the new
	// sample count. Like setVsync, the work is deferred to the next frame boundary,
	// because this can be called from anywhere - including mid-frame with a command
	// buffer already recording.
	void setMultiSampling(int samples) override;
	int getMultiSampling() const override;

	void drawTexturedRect2D(GLuint textureId, GLuint maskTextureId, const glm::vec4& tint,
			const glm::mat4& mvp,
			const glm::vec2& uvOffset = glm::vec2(0.0f), const glm::vec2& uvScale = glm::vec2(1.0f)) override;
	void drawTexturedTriangles2D(GLuint textureId, const glm::vec4& tint,
			const glm::mat4& mvp, const float* xyuv, int vertexCount) override;

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
	GLuint nextvktextureid = 1;
	GLuint boundtextureid = 0;
	// Offscreen render targets, one per gFbo. Each owns a render pass and a
	// framebuffer of its own, the same shape the shadow map already uses, because
	// this backend renders through render passes rather than dynamic rendering.
	std::unordered_map<GLuint, gVKFramebuffer*> vkframebuffers;
	GLuint nextvkframebufferid = 1;
	// Renderbuffers have no Vulkan counterpart: gFbo is steered onto a sampleable
	// depth texture instead, so these ids only have to be unique and non-zero.
	GLuint nextvkrenderbufferid = 1;
	// Ends whatever offscreen pass is open and hands its attachments back to the
	// shader as sampleable textures. Safe to call when none is open.
	void endOffscreenPass();
	void destroyAllFramebuffers();
	// Builds a target's render pass and framebuffer on first use.
	bool ensureFramebufferPass(gVKFramebuffer* target);
	// Vertex array ids. Vulkan has no such object, but gVbo binds one before every
	// upload and every draw, so the id is what tells the backend which pair of
	// buffers a draw is about. See gVKMeshBuffer.h.
	std::unordered_map<GLuint, gVKVertexArray> vkvertexarrays;
	GLuint nextvkvaoid = 1;
	GLuint boundvaoid = 0;

	// Registry backing the GLuint buffer names gVbo hands out. genBuffers mints an
	// id, setVertexBufferData / setIndexBufferData fill it, deleteBuffer frees it.
	// boundarraybufferid mirrors the OpenGL bind state, which is what lets a vertex
	// array learn which buffer belongs to it.
	std::unordered_map<GLuint, gVKMeshBuffer*> vkmeshbuffers;
	GLuint nextvkbufferid = 1;
	GLuint boundarraybufferid = 0;
	GLuint boundelementbufferid = 0;

	// Fetches the buffer behind an id, or null if the id was never minted. Uploading
	// creates the VkBuffer lazily, so the entry can exist while its handle is still
	// VK_NULL_HANDLE.
	gVKMeshBuffer* getMeshBuffer(GLuint id);
	void destroyAllMeshBuffers();

	// A 1x1 white texture bound wherever a mesh has no map of that kind. The 3D
	// shader samples its map bindings unconditionally from Vulkan's point of view -
	// the "use this map" flag only decides what happens to the result - so every
	// binding has to point at something valid. White is the identity for the
	// multiply that follows, which keeps an unmapped material's colours untouched.
	GLuint whitetextureid = 0;
	bool ensureWhiteTexture();

	// One identity matrix, bound as the instance buffer of a draw that is not
	// instanced. The 3D shader always reads a per-instance model matrix, so this is
	// what lets a single pipeline and a single shader serve both kinds of draw:
	// multiplying by the identity leaves the mesh where its own model matrix put it.
	gVKMeshBuffer* identityinstancebuffer = nullptr;
	bool ensureIdentityInstanceBuffer();

	// What gShadowMap last told the backend. Fed into the scene uniform block, and
	// into the depth pass's push constant while the shadow pass is being recorded.
	bool shadowmapenabled = false;
	bool shadowsoft = false;
	glm::mat4 shadowlightmatrix{1.0f};
	glm::vec3 shadowlightposition{0.0f};


	// Fills the frame's scene uniform block from the renderer's camera and lights.
	// Declared unconditionally: this header does not pull in the Vulkan guard, and
	// the definition is what is compiled out on a build without Vulkan.
	void updateSceneUniforms();
	// Whether this frame's scene block has been written yet. Reset in beginFrame, so
	// the camera and lights are gathered once per frame instead of once per mesh.
	bool sceneuniformswritten = false;

	// Consecutive opaque mesh calls are safe to combine because their relative
	// order is preserved.  Transparent meshes and caller-provided instancing stay
	// immediate: changing either would alter the public draw semantics.
	struct QueuedMeshDraw {
		GLuint vertexarrayid = 0;
		int vertexcount = 0;
		int indexcount = 0;
		int drawmode = GL_TRIANGLES;
		int instancecount = 1;
		glm::mat4 model{1.0f};
		gMeshSurface surface{};
		glm::vec4 tint{1.0f};
		bool depthtest = false;
		int depthtesttype = DEPTHTESTTYPE_LESS;
		bool culling = false;
		int cullface = GL_BACK;
		int cullingdirection = GL_CCW;
		// A digest of everything canMergeQueuedDraws compares, filled when the draw
		// is queued. flushQueuedDraws sorts by it so that draws which could merge but
		// arrived apart end up next to each other; the exact comparison still decides
		// whether they really do, so a hash collision costs a missed merge and
		// nothing else.
		uint64_t mergekey = 0;
	};
	std::vector<QueuedMeshDraw> queuedmeshdraws;
	bool flushingqueueddraws = false;
	static uint64_t gvkQueuedDrawKey(const QueuedMeshDraw& draw);
	bool canMergeQueuedDraws(const QueuedMeshDraw& first, const QueuedMeshDraw& next) const;
	void recordQueuedDrawGroup(size_t first, size_t count);
	void destroyAllTextures();
	// The Vulkan texture behind the currently bound id, or null when there is none
	// yet - gTexture sets filtering and wrapping both before and after the upload.
	gVKTexture* getBoundVKTexture();

	// Applies a pending setMultiSampling at a frame boundary: everything the sample
	// count is baked into - the render pass, the depth and MSAA attachments, the
	// framebuffers and both pipeline builds - is destroyed and rebuilt. Returns
	// straight away when nothing was requested, which is the normal case.
	void applyPendingSampleCount();
	// The sample count an app asked for before the context existed, and the pending
	// request applyPendingSampleCount acts on. 0 means "nothing pending".
	int pendingsamplecount = 0;

	// Shader hot reload. Development builds watch the .vert / .frag sources the 2D
	// pipelines are compiled from and rebuild them when one is saved, so a shader
	// can be tuned without restarting. Release builds compile nothing at runtime,
	// the timestamp stays 0 and this costs one comparison per frame.
	void checkShaderReload();
	long long shadersourcetimestamp = 0;
	int shaderpollcountdown = 0;
};

#endif
