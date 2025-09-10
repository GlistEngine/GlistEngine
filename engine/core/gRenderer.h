/*
 * gRenderManager.h
 *
 *  Created on: 4 Ara 2020
 *      Author: Acer
 */

#pragma once

#ifndef CORE_GRENDERER_H_
#define CORE_GRENDERER_H_

#include "gObject.h"

#if defined(WIN32) || defined(LINUX)
//#include <GL/glext.h>
#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>
#endif
#if TARGET_OS_OSX
#include <GL/glew.h>
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#endif
#ifdef ANDROID
#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>
#include <GLES3/gl3platform.h>
#endif
#if TARGET_OS_IPHONE || TARGET_OS_SIMULATOR
#	include <OpenGLES/ES3/gl.h>
#	include <OpenGLES/ES3/glext.h>
#	include <OpenGLES/gltypes.h>
#endif
#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include <glm/glm.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale, glm::perspective
#include <glm/gtx/quaternion.hpp>

#include "gColor.h"
#include "gConstants.h"
#include <deque>

#ifndef GLIST_MAX_LIGHTS
// amount of maximum lights, this is used to allocate memory for the light uniform buffer
#define GLIST_MAX_LIGHTS 8
#endif

// You can define ENGINE_OPENGL_CHECKS to enable OpenGL checks
// without debugging.
#if defined(DEBUG) || defined(ENGINE_OPENGL_CHECKS)
#define G_CHECK_GL(a) gCheckGLErrorAndPrint("Previously Unhandled ", __PRETTY_FUNCTION__, __LINE__); a; gCheckGLErrorAndPrint("", __PRETTY_FUNCTION__, __LINE__)
#define G_CHECK_GL2(value, fn) gCheckGLErrorAndPrint("Previously Unhandled ", __PRETTY_FUNCTION__, __LINE__); value = fn; gCheckGLErrorAndPrint("", __PRETTY_FUNCTION__, __LINE__)
#else
#define G_CHECK_GL(fn) fn
#define G_CHECK_GL2(value, fn) value = fn
#endif

void gCheckGLErrorAndPrint(const std::string& prefix, const std::string& func, int line);

void gEnableCulling();
void gDisableCulling();
bool gIsCullingEnabled();
void gCullFace(int cullingFace);
int gGetCullFace();
void gSetCullingDirection(int cullingDirection);
int gGetCullingDirection();

void gDrawLine(float x1, float y1, float x2, float y2, float thickness = 1.0f);
void gDrawLine(float x1, float y1, float z1, float x2, float y2, float z2, float thickness = 1.0f);
void gDrawTriangle(float px, float py, float qx, float qy, float rx, float ry, bool is_filled = true);
void gDrawCircle(float xCenter, float yCenter, float radius, bool isFilled = false, float numberOfSides = 64.0f);
void gDrawCross(float x, float y, float width, float height, float thickness, bool isFilled);
void gDrawArc(float xCenter, float yCenter, float radius, bool isFilled = true, int numberOfSides = 60, float degree = 360.0f, float rotate = 360.0f);
void gDrawArrow(float x1, float y1, float length, float angle, float tipLength, float tipAngle);
void gDrawRectangle(float x, float y, float w, float h, bool isFilled = false);
void gDrawRoundedRectangle(float x, float y, float w, float h, int radius, bool isFilled);
void gDrawBox(float x, float y, float z, float w = 1.0f, float h = 1.0f, float d = 1.0f, bool isFilled = true);
void gDrawBox(glm::mat4 transformationMatrix, bool isFilled = true);
void gDrawSphere(float xPos, float yPos, float zPos, glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f), int xSegmentNum = 64, int ySegmentNum = 32, bool isFilled = true);
void gDrawCylinder(float x, float y, float z, int r, int h, glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f), int segmentnum = 32, bool isFilled = true);
void gDrawCylinderOblique(float x, float y, float z, int r, int h, glm::vec2 shiftdistance, glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f), int segmentnum = 32, bool isFilled = true);
void gDrawCylinderTrapezodial(float x, float y, float z, int r1, int r2, int h, glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f), int segmentnum = 32, bool isFilled = true);
void gDrawCylinderObliqueTrapezodial(float x, float y, float z, int r1, int r2, int h, glm::vec2 shiftdistance, glm::vec3 scale = glm::vec3(1.0, 1.0, 1.0), int segmentnum = 32, bool isFilled = true);
void gDrawCone(float x, float y, float z, int r, int h, glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f), int segmentnum = 32, bool isFilled = true);
void gDrawConeOblique(float x, float y, float z, int r, int h, glm::vec2 shiftdistance, glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f), int segmentnum = 32, bool isFilled = true);
void gDrawPyramid(float x, float y, float z, int r, int h, glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f), int numberofsides = 4, bool isFilled = true);
void gDrawPyramidOblique(float x, float y, float z, int r, int h, glm::vec2 shiftdistance, glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f), int numberofsides = 4, bool isFilled = true);
void gDrawTube(float x, float y, float z, int outerradius,int innerradious, int h, glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f), int segmentnum = 32, bool isFilled = true);
void gDrawTubeOblique(float x, float y, float z, int outerradius,int innerradious, int h, glm::vec2 shiftdistance, glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f), int segmentnum = 32, bool isFilled = true);
void gDrawTubeTrapezodial(float x, float y, float z, int topouterradius,int topinnerradious, int buttomouterradious, int buttominnerradious, int h, glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f), int segmentnum = 32, bool isFilled = true);
void gDrawTubeObliqueTrapezodial(float x, float y, float z, int topouterradius,int topinnerradious, int buttomouterradious, int buttominnerradious, int h, glm::vec2 shiftdistance, glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f), int segmentnum = 32, bool isFilled = true);

class gVbo;

template<typename T>
class gUbo;
class gLight;
class gImage;
class gShader;
class gCamera;
class gGrid;
class gLine;
class gTriangle;
class gCircle;
class gCross;
class gArc;
class gRectangle;
class gRoundedRectangle;
class gBox;

class gRenderer : public gObject {
public:
	static const int SCREENSCALING_NONE, SCREENSCALING_MIPMAP, SCREENSCALING_AUTO;
	static const int DEPTHTESTTYPE_LESS, DEPTHTESTTYPE_ALWAYS;
	static const int FOGMODE_LINEAR, FOGMODE_EXP;
	struct alignas(16) gSceneLightData {
		alignas(4) int type;
		alignas(16) glm::vec3 position;
		alignas(16) glm::vec3 direction;
		alignas(16) glm::vec4 ambient;
		alignas(16) glm::vec4 diffuse;
		alignas(16) glm::vec4 specular;

		alignas(4) float constant;
		alignas(4) float linear;
		alignas(4) float quadratic;

		alignas(4) float spotcutoffangle;
		alignas(4) float spotoutercutoffangle;
	};

	struct alignas(16) gSceneLights {
		alignas(4) int lightnum = 0;
		// bitwise enabled lights, 1 means enabled, 0 means disabled, 32-bit integer
		// supports only 32 max lights, make sure to change this if max lights is changed to be something above 32
		alignas(4) int enabledlights;
		alignas(16) glm::vec4 globalambientcolor;
		gSceneLightData lights[GLIST_MAX_LIGHTS];
	};

	enum SceneDataFlags {
		ENABLE_SSAO = 0b0001,
		ENABLE_FOG = 0b0010
	};

	struct alignas(16) gSceneFogData {
		alignas(16) glm::vec3 color;
		alignas(4) float linearStart;
		alignas(4) float linearEnd;
		alignas(4) float density;
		alignas(4) float gradient;
		alignas(4) int mode;
	};
	struct alignas(16) gSceneData {
		alignas(16) gColor rendercolor;
		alignas(16) glm::vec3 viewpos;
		alignas(16) glm::mat4 viewmatrix;
		alignas(4) float ssaobias;
		alignas(4) unsigned int flags;
		gSceneFogData fog;
	};

	gRenderer() = default;
	virtual ~gRenderer();

	static void setScreenSize(int screenWidth, int screenHeight);
	static void setUnitScreenSize(int unitWidth, int unitHeight);
	static void setScreenScaling(int screenScaling);

	int getWidth();
	int getHeight();
	int getScreenWidth();
	int getScreenHeight();
	int getUnitWidth();
	int getUnitHeight();
	static int getScreenScaling();

	static void setCurrentResolution(int resolution);
	static void setCurrentResolution(int screenWidth, int screenHeight);
	static void setUnitResolution(int resolution);
	static void setUnitResolution(int screenWidth, int screenHeight);
	static int getResolution(int screenWidth, int screenHeight);
	int getCurrentResolution();
	int getUnitResolution();
	static int scaleX(int x);
	static int scaleY(int y);

	//grid
	void drawGrid();
	void drawGridYZ();
	void drawGridXY();
	void drawGridXZ();
	void enableGrid();
	void disableGrid();
	void setGridEnableAxis(bool xy, bool yz, bool xz);
	void setGridEnableXY(bool xy);
	void setGridEnableYZ(bool yz);
	void setGridEnableXZ(bool xz);
	void setGridColorofAxisXZ(int r, int g, int b, int a);
	void setGridColorofAxisYZ(int r, int g, int b, int a);
	void setGridColorofAxisXY(int r, int g, int b, int a);
	void setGridColorofAxisXZ(gColor* color);
	void setGridColorofAxisYZ(gColor* color);
	void setGridColorofAxisXY(gColor* color);
	void setGridColorofAxisWireFrameXZ(int r, int g, int b, int a);
	void setGridColorofAxisWireFrameYZ(int r, int g, int b, int a);
	void setGridColorofAxisWireFrameXY(int r, int g, int b, int a);
	void setGridColorofAxisWireFrameXZ(gColor* color);
	void setGridColorofAxisWireFrameYZ(gColor* color);
	void setGridColorofAxisWireFrameXY(gColor* color);
	bool isGridEnabled();
	bool isGridXYEnabled();
	bool isGridYZEnabled();
	bool isGridXZEnabled();
	void setGridMaxLength(float length);
	float getGridMaxLength();
	void setGridLineInterval(float intervalvalue);
	float getGridLineInterval();

	gGrid* getGrid() const;
	void setGrid(gGrid* newgrid);

	void setColor(int r, int g, int b, int a = 255);
	void setColor(float r, float g, float b, float a = 1.0f);
	void setColor(const gColor& color);
	void setColor(gColor* color);
	gColor* getColor();

	virtual void clear() = 0;
	virtual void clearColor(int r, int g, int b, int a = 255) = 0;
	virtual void clearColor(gColor color) = 0;

	void enableLighting();
	void disableLighting();
	bool isLightingEnabled();
	void setLightingColor(int r, int g, int b, int a = 255);
	void setLightingColor(gColor* color) { setLightingColor(color->r, color->g, color->b, color->a); }
	gColor* getLightingColor();
	void setLightingPosition(glm::vec3 lightingPosition);
	glm::vec3 getLightingPosition();

	void setGlobalAmbientColor(int r, int g, int b, int a = 255);
	void setGlobalAmbientColor(gColor color) { setGlobalAmbientColor(color.r, color.g, color.b, color.a); }
	gColor* getGlobalAmbientColor();

	void enableFog();
	void disableFog();
	void setFogNo(int no);
	void setFogColor(float r, float g, float b);
	void setFogColor(const gColor& color);
	void setFogMode(int fogMode);
	void setFogDensity(float value);
	void setFogGradient(float value);
	void setFogLinearStart(float value);
	void setFogLinearEnd(float value);

	bool isFogEnabled();
	int getFogNo() const;
	const gColor& getFogColor() const;
	int getFogMode() const;
	float getFogDensity() const;
	float getFogGradient() const;
	float getFogLinearStart() const;
	float getFogLinearEnd() const;

	// add and remove functions are called by gLight class automatically,
	// so you don't need to
	void addSceneLight(gLight* light);
	void removeSceneLight(gLight* light);
	gLight* getSceneLight(int lightNo);
	int getSceneLightNum();
	void removeAllSceneLights();
	void updateLights();

	void updateScene();

	virtual void enableDepthTest() = 0;
	virtual void enableDepthTest(int depthTestType) = 0;
	virtual void setDepthTestFunc(int depthTestType) = 0;
	virtual void disableDepthTest() = 0;
	virtual bool isDepthTestEnabled() = 0;
	virtual int getDepthTestType() = 0;

	virtual void enableAlphaBlending() = 0;
	virtual void disableAlphaBlending() = 0;
	virtual bool isAlphaBlendingEnabled() = 0;
	virtual void enableAlphaTest() = 0;
	virtual void disableAlphaTest() = 0;
	virtual bool isAlphaTestEnabled() = 0;

	bool isSSAOEnabled();
	void enableSSAO();
	void disableSSAO();
	void setSSAOBias(float value);
	float getSSAOBias();

	gShader* getColorShader();
	gShader* getTextureShader();
	gShader* getFontShader();
	gShader* getImageShader();
	gShader* getSkyboxShader();
	gShader* getShadowmapShader();
	gShader* getPbrShader();
	gShader* getEquirectangularShader();
	gShader* getIrradianceShader();
	gShader* getPrefilterShader();
	gShader* getBrdfShader();
	gShader* getFboShader();
	gShader* getGridShader();

	void setProjectionMatrix(glm::mat4 projectionMatrix);
	void setProjectionMatrix2d(glm::mat4 projectionMatrix2d);
	void setViewMatrix(glm::mat4 viewMatrix);
	void setCameraPosition(glm::vec3 cameraPosition);
	void setCamera(gCamera* camera);
	const glm::mat4& getProjectionMatrix() const;
	const glm::mat4& getProjectionMatrix2d() const;
	const glm::mat4& getViewMatrix() const;
	const glm::vec3& getCameraPosition() const;
	const gCamera* getCamera() const;
	void backupMatrices();
	void restoreMatrices();

	/*
	 * Takes Screen Shot of the current Rendered Screen and returns it as an gImage class
	 */
	virtual void takeScreenshot(gImage& img) = 0;

	/*
	 * Takes Screen Shot of the part of current Rendered Screen and returns it as an gImage class
	 */
	virtual void takeScreenshot(gImage& img, int x, int y, int width, int height) = 0;

	/* -------------- gUbo ------------- */
	virtual GLuint genBuffers() = 0;
	virtual void deleteBuffer(GLuint& buffer) = 0;

	virtual void bindBuffer(GLenum target, GLuint buffer) = 0;
	virtual void unbindBuffer(GLenum target) = 0;

	virtual void bufSubData(GLuint buffer, int offset, int size, const void* data) = 0;
	virtual void setBufferData(GLuint buffer, const void* data, size_t size, int usage) = 0;
	virtual void setBufferRange(int index, GLuint buffer, int offset, int size) = 0;

	virtual void attachUbo(GLuint id, int bindingpoint, const std::string& uboName) = 0;

	/* -------------- gVbo --------------- */
	virtual GLuint createVAO() = 0;
	virtual void deleteVAO(GLuint& vao) = 0;

	virtual void bindVAO(GLuint vao) = 0;
	virtual void unbindVAO() = 0;

	virtual void setVertexBufferData(GLuint vbo, size_t size, const void* data, int usage) = 0;
	virtual void setIndexBufferData(GLuint ebo, size_t size, const void* data, int usage) = 0;

	virtual void drawArrays(int drawMode, int count) = 0;
	virtual void drawElements(int drawMode, int count) = 0;

	virtual void enableVertexAttrib(int index) = 0;
	virtual void disableVertexAttrib(int index) = 0;
	virtual void setVertexAttribPointer(int index, int size, int type, bool normalized, int stride, const void* pointer) = 0;

	/* -------------- gFbo --------------- */
	virtual GLuint createFramebuffer() = 0;
	virtual void deleteFramebuffer(GLuint& fbo) = 0;
	virtual void bindFramebuffer(GLuint fbo) = 0;
	virtual void checkFramebufferStatus() = 0;

	virtual GLuint createRenderbuffer() = 0;
	virtual void deleteRenderbuffer(GLuint& rbo) = 0;
	virtual void bindRenderbuffer(GLuint rbo) = 0;
	virtual void setRenderbufferStorage(GLenum format, int width, int height) = 0;

	virtual void attachTextureToFramebuffer(GLenum attachment, GLenum textarget, GLuint texId, GLuint level = 0) = 0;
	virtual void attachRenderbufferToFramebuffer(GLenum attachment, GLuint rbo) = 0;

	virtual void setDrawBufferNone() = 0;
	virtual void setReadBufferNone() = 0;

	virtual void createFullscreenQuad(GLuint& vao, GLuint& vbo) = 0;
	virtual void deleteFullscreenQuad(GLuint& vao, GLuint* vbo) = 0;

	/* -------------- gShader --------------- */
	// This function loads shaders without preproccesing them. Geometry source can be nullptr.
	virtual GLuint loadProgram(const char* vertexSource, const char* fragmentSource, const char* geometrySource) = 0;
	virtual void checkCompileErrors(GLuint shader, const std::string& type) = 0;
	virtual void setBool(GLuint uniformloc, bool value) = 0;
	virtual void setInt(GLuint uniformloc, int value) = 0;
	virtual void setUnsignedInt(GLuint uniformloc, unsigned int value) = 0;
	virtual void setFloat(GLuint uniformloc, float value) = 0;
	virtual void setVec2(GLuint uniformloc, const glm::vec2& value) = 0;
	virtual void setVec2(GLuint uniformloc, float x, float y) = 0;
	virtual void setVec3(GLuint uniformloc, const glm::vec3& value) = 0;
	virtual void setVec3(GLuint uniformloc, float x, float y, float z) = 0;
	virtual void setVec4(GLuint uniformloc, const glm::vec4& value) = 0;
	virtual void setVec4(GLuint uniformloc, float x, float y, float z, float w) = 0;
	virtual void setMat2(GLuint uniformloc, const glm::mat2& mat) = 0;
	virtual void setMat3(GLuint uniformloc, const glm::mat3& mat) = 0;
	virtual void setMat4(GLuint uniformloc, const glm::mat4& mat) = 0;
	virtual GLuint getUniformLocation(GLuint id, const std::string& name) = 0;

	virtual void useShader(GLuint id) const = 0;
	virtual void resetShader(GLuint id, bool loaded) const = 0;

	/* ------------ gPostProcessManager ------------- */
	virtual void clearScreen(bool color = true, bool depth = true) = 0;
	virtual void bindQuadVAO() = 0;
	virtual void drawFullscreenQuad() = 0;
	virtual void bindDefaultFramebuffer() = 0;

	/* -------------- gGrid --------------- */
	virtual void drawVbo(const gVbo& vbo) = 0;

	/* ---------------- gTexture ---------------- */
	virtual GLuint createTextures() = 0;
	virtual void bindTexture(GLuint texId) = 0;
	virtual void bindTexture(GLuint texId, int textureSlotNo) = 0;
	virtual void unbindTexture() = 0;
	virtual void activateTexture(int textureSlotNo = 0) = 0;
	virtual void resetTexture() = 0;
	virtual void deleteTexture(GLuint& texId) = 0;

	virtual void texImage2D(GLenum target, GLint internalFormat, int width, int height, GLint format, GLint type, void* data) = 0;
	virtual void setWrapping(GLenum target, GLint wrapS, GLint wrapT) = 0;
	virtual void setWrapping(GLenum target, GLint wrapS, GLint wrapT, GLint wrapR) = 0;

	virtual void setFiltering(GLenum target, GLint minFilter, GLint magFilter) = 0;
	virtual void setWrappingAndFiltering(GLenum target, GLint wrapS, GLint wrapT, GLint minFilter, GLint magFilter) = 0;
	virtual void setWrappingAndFiltering(GLenum target, GLint wrapS, GLint wrapT, GLint wrapR, GLint minFilter, GLint magFilter) = 0;
	virtual void setSwizzleMask(GLint swizzleMask[4]) = 0;

	virtual void readTexturePixels(unsigned char* inPixels, GLuint textureId, int width, int height, GLenum format) = 0;

	virtual void generateMipMap() = 0;

	/* ---------------- gSkybox ---------------- */
	virtual void bindSkyTexture(GLuint texId) = 0;
	virtual void bindSkyTexture(GLuint texId, int textureSlot) = 0;
	virtual void unbindSkyTexture() = 0;
	virtual void unbindSkyTexture(int textureSlotNo) = 0;
	virtual void generateSkyMipMap() = 0;
	virtual void enableDepthTestEqual() = 0;
	virtual void createQuad(GLuint& inQuadVAO, GLuint& inQuadVBO) = 0;
	virtual void enableCubeMapSeemless() = 0;
	virtual void checkEnableCubeMap4Android() = 0;

	/* ---------------- gRenderObject ---------------- */
	virtual void pushMatrix() = 0;
	virtual void popMatrix() = 0;

	/* ---------------- Utilities ---------------- */
	void drawLine(float x1, float y1, float x2, float y2, float thickness = 1.0f);
	void drawLine(float x1, float y1, float z1, float x2, float y2, float z2, float thickness = 1.0f);
	void drawTriangle(float px, float py, float qx, float qy, float rx, float ry, bool is_filled = true);
	void drawCircle(float xCenter, float yCenter, float radius, bool isFilled = false, float numberOfSides = 64.0f);
	void drawCross(float x, float y, float width, float height, float thickness, bool isFilled);
	void drawArc(float xCenter, float yCenter, float radius, bool isFilled = true, int numberOfSides = 60, float degree = 360.0f, float rotate = 360.0f);
	void drawArrow(float x1, float y1, float length, float angle, float tipLength, float tipAngle);
	void drawRectangle(float x, float y, float w, float h, bool isFilled = false);
	void drawRoundedRectangle(float x, float y, float w, float h, int radius, bool isFilled);
	void drawBox(float x, float y, float z, float w = 1.0f, float h = 1.0f, float d = 1.0f, bool isFilled = true);
	void drawBox(glm::mat4 transformationMatrix, bool isFilled = true);
	void drawSphere(float xPos, float yPos, float zPos, glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f), int xSegmentNum = 64, int ySegmentNum = 32, bool isFilled = true);
	void drawCylinder(float x, float y, float z, int r, int h, glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f), int segmentnum = 32, bool isFilled = true);
	void drawCylinderOblique(float x, float y, float z, int r, int h, glm::vec2 shiftdistance, glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f), int segmentnum = 32, bool isFilled = true);
	void drawCylinderTrapezodial(float x, float y, float z, int r1, int r2, int h, glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f), int segmentnum = 32, bool isFilled = true);
	void drawCylinderObliqueTrapezodial(float x, float y, float z, int r1, int r2, int h, glm::vec2 shiftdistance, glm::vec3 scale = glm::vec3(1.0, 1.0, 1.0), int segmentnum = 32, bool isFilled = true);
	void drawCone(float x, float y, float z, int r, int h, glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f), int segmentnum = 32, bool isFilled = true);
	void drawConeOblique(float x, float y, float z, int r, int h, glm::vec2 shiftdistance, glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f), int segmentnum = 32, bool isFilled = true);
	void drawPyramid(float x, float y, float z, int r, int h, glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f), int numberofsides = 4, bool isFilled = true);
	void drawPyramidOblique(float x, float y, float z, int r, int h, glm::vec2 shiftdistance, glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f), int numberofsides = 4, bool isFilled = true);
	void drawTube(float x, float y, float z, int outerradius,int innerradious, int h, glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f), int segmentnum = 32, bool isFilled = true);
	void drawTubeOblique(float x, float y, float z, int outerradius,int innerradious, int h, glm::vec2 shiftdistance, glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f), int segmentnum = 32, bool isFilled = true);
	void drawTubeTrapezodial(float x, float y, float z, int topouterradius,int topinnerradious, int buttomouterradious, int buttominnerradious, int h, glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f), int segmentnum = 32, bool isFilled = true);
	void drawTubeObliqueTrapezodial(float x, float y, float z, int topouterradius,int topinnerradious, int buttomouterradious, int buttominnerradious, int h, glm::vec2 shiftdistance, glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f), int segmentnum = 32, bool isFilled = true);

protected:
	friend class gRenderObject; // this is where renderer->init() is called from

	static int width, height;
	static int unitwidth, unitheight;
	static int screenscaling;
	static int currentresolution, unitresolution;

	gColor* rendercolor;

	bool isfogenabled;
	int fogno;
	gColor fogcolor;
	float fogdensity;
	float foggradient;
	int fogmode;
	float foglinearstart;
	float foglinearend;

	std::deque<gLight*> scenelights;
	gUbo<gSceneLights>* lightsubo;
	gUbo<gSceneData>* sceneubo;
	bool islightingenabled;
	glm::vec3 lightingposition;
	gColor lightingcolor;
	gColor globalambientcolor;
	bool isglobalambientcolorchanged;

	bool isdepthtestenabled;
	int depthtesttype;
	unsigned int depthtesttypeid[2];
	bool isalphablendingenabled, isalphatestenabled;

	bool isssaoenabled;
	float ssaobias;

	gShader* colorshader;
	gShader* textureshader;
	gShader* fontshader;
	gShader* imageshader;
	gShader* skyboxshader;
	gShader* shadowmapshader;
	gShader* pbrshader;
	gShader* equirectangularshader;
	gShader* irradianceshader;
	gShader* prefiltershader;
	gShader* brdfshader;
	gShader* fboshader;
	gShader* gridshader;

	glm::mat4 projectionmatrix;
	glm::mat4 projectionmatrixold;
	glm::mat4 projectionmatrix2d;
	glm::mat4 viewmatrix;
	glm::mat4 viewmatrixold;
	glm::vec3 cameraposition;
	gCamera* camera;

	gGrid* grid;
	gGrid* originalgrid;

	// std::unique_ptr is automatically deletes the underlying object when this gRenderer object is deleted
	std::unique_ptr<gLine> linemesh, linemesh2, linemesh3;
	std::unique_ptr<gTriangle> trianglemesh;
	std::unique_ptr<gCircle> circlemesh;
	std::unique_ptr<gCross> crossmesh;
	std::unique_ptr<gArc> arcmesh;
	std::unique_ptr<gRectangle> rectanglemesh;
	std::unique_ptr<gRoundedRectangle> roundedrectanglemesh;
	std::unique_ptr<gBox> boxmesh;

	virtual void init();
	virtual void cleanup();
	virtual void updatePackUnpackAlignment(int i) = 0;

	static const std::string& getShaderSrcGridVertex();
	static const std::string& getShaderSrcGridFragment();
	static const std::string& getShaderSrcColorVertex();
	static const std::string& getShaderSrcColorFragment();
	static const std::string& getShaderSrcTextureVertex();
	static const std::string& getShaderSrcTextureFragment();
	static const std::string& getShaderSrcImageVertex();
	static const std::string& getShaderSrcImageFragment();
	static const std::string& getShaderSrcFontVertex();
	static const std::string& getShaderSrcFontFragment();
	static const std::string& getShaderSrcSkyboxVertex();
	static const std::string& getShaderSrcSkyboxFragment();
	static const std::string& getShaderSrcShadowmapVertex();
	static const std::string& getShaderSrcShadowmapFragment();
	static const std::string& getShaderSrcPbrVertex();
	static const std::string& getShaderSrcPbrFragment();
	static const std::string& getShaderSrcCubemapVertex();
	static const std::string& getShaderSrcEquirectangularFragment();
	static const std::string& getShaderSrcIrradianceFragment();
	static const std::string& getShaderSrcPrefilterFragment();
	static const std::string& getShaderSrcBrdfVertex();
	static const std::string& getShaderSrcBrdfFragment();
	static const std::string& getShaderSrcFboVertex();
	static const std::string& getShaderSrcFboFragment();

};

#endif /* CORE_GRENDERER_H_ */
