/*
 * gRenderer.h
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
#if defined(EMSCRIPTEN)
#include <GLES3/gl3.h>
#include <GLES3/gl2ext.h> // well, okay?
#include <GLES3/gl3platform.h>
#endif
#if defined(ANDROID)
#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>
#include <GLES3/gl3platform.h>
#endif
#if defined(__APPLE__)
#include <TargetConditionals.h>
#endif
#if TARGET_OS_OSX
#include <GL/glew.h>
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
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
#include <memory>
#include <vector>

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

// --- 2D / 3D Line Overloads
void gDrawLine(float x1, float y1, float x2, float y2);
void gDrawLine(float x1, float y1, float x2, float y2, float thickness);
void gDrawLine(float x1, float y1, float z1, float x2, float y2, float z2, float thickness = 1.0f);
void gDrawLine(float x1, float y1, float x2, float y2, float thickness, float rotateAngle, float pivotx, float pivoty);

void gDrawTriangle(float px, float py, float qx, float qy, float rx, float ry, bool is_filled = true, float rotateAngle = 0.0f, float pivotx = 0.5f, float pivoty = 0.5f);
void gDrawCircle(float xCenter, float yCenter, float radius, bool isFilled = false, float numberOfSides = 64.0f, float rotateAngle = 0.0f, float pivotx = 0.5f, float pivoty = 0.5f);
void gDrawCross(float x, float y, float width, float height, float thickness, bool isFilled, float rotateAngle = 0.0f, float pivotx = 0.5f, float pivoty = 0.5f);
void gDrawArc(float xCenter, float yCenter, float radius, bool isFilled = true, int numberOfSides = 60, float degree = 360.0f, float rotate = 360.0f, float rotateAngle = 0.0f, float pivotx = 0.5f, float pivoty = 0.5f);
void gDrawArrow(float x1, float y1, float length, float angle, float tipLength, float tipAngle, float thickness = 1.0f, float rotateAngle = 0.0f, float pivotx = 0.5f, float pivoty = 0.5f);
void gDrawRectangle(float x, float y, float w, float h, bool isFilled = false, float rotateAngle = 0.0f, float pivotx = 0.5f, float pivoty = 0.5f);
void gDrawRoundedRectangle(float x, float y, float w, float h, int radius, bool isFilled, float rotateAngle = 0.0f, float pivotx = 0.5f, float pivoty = 0.5f);
void gDrawBox(float x, float y, float z, float w = 1.0f, float h = 1.0f, float d = 1.0f, bool isFilled = true);
void gDrawBox(float x, float y, float z, float w, float h, float d, float rotateAngle, float axisX, float axisY, float axisZ, bool isFilled = true);
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
class gFbo;

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
//class gArrow;

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
		alignas(4) int enabledlights;
		alignas(16) glm::vec4 globalambientcolor;
		gSceneLightData lights[GLIST_MAX_LIGHTS];
	};

	enum SceneDataFlags {
		ENABLE_SSAO = 0b0001,
		ENABLE_FOG = 0b0010,
		ENABLE_GAMMA = 0b0100,
		ENABLE_HDR = 0b1000,
		ENABLE_SOFT_SHADOWS = 0b10000
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
		alignas(4) unsigned int flags;
		gSceneFogData fog;
	};

	gRenderer() = default;
	virtual ~gRenderer();

	static void setScreenSize(int screenWidth, int screenHeight);
	static void setUnitScreenSize(int unitWidth, int unitHeight);
	static void setScreenScaling(int screenScaling);
	static void updateProjectionMatrix2d();

	int getWidth();
	int getHeight();
	int getScreenWidth();
	int getScreenHeight();
	int getUnitWidth();
	int getUnitHeight();
	static int getScreenScaling();
	unsigned int getFullscreenQuadVAO() const;

	static void setCurrentResolution(int resolution);
	static void setCurrentResolution(int screenWidth, int screenHeight);
	static void setUnitResolution(int resolution);
	static void setUnitResolution(int screenWidth, int screenHeight);
	static int getResolution(int screenWidth, int screenHeight);
	int getCurrentResolution();
	int getUnitResolution();
	static float getScaleMultiplier();
	static int scaleX(int x);
	static int scaleY(int y);
	static int unscaleX(int x);
	static int unscaleY(int y);

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

	virtual bool beginFrame() { return true; }
	virtual void endFrame() {}
	virtual void flushQueuedDraws() {}

	int getRenderEngineType() const { return renderenginetype; }
	bool isVulkan() const { return renderenginetype == G_RENDERER_VK; }

	virtual void drawColored2D(const glm::vec2* points, int count, const glm::vec4& color, const glm::mat4& mvp,
			int drawMode = GL_TRIANGLES) {}

	virtual void drawTexturedRect2D(GLuint textureId, GLuint maskTextureId, const glm::vec4& tint,
			const glm::mat4& mvp,
			const glm::vec2& uvOffset = glm::vec2(0.0f), const glm::vec2& uvScale = glm::vec2(1.0f)) {}

	// Backend hook for an already-expanded textured triangle list. xyuv contains
	// four floats per vertex: screen-space x/y followed by texture u/v. This is
	// used by batched text, where all glyph quads share one atlas texture and can
	// therefore be recorded as one draw instead of one draw per glyph.
	virtual void drawTexturedTriangles2D(GLuint textureId, const glm::vec4& tint,
			const glm::mat4& mvp, const float* xyuv, int vertexCount) {}

	// What a backend needs about a mesh's surface in order to shade it. Mirrors the
	// non-map part of gMaterial; the texture maps join it once the Vulkan path can
	// sample them.
	struct gMeshSurface {
		glm::vec4 ambient{1.0f};
		glm::vec4 diffuse{1.0f};
		glm::vec4 specular{1.0f};
		float shininess = 0.5f;
		// Texture ids as gTexture hands them out, or 0 for "this mesh has no map of
		// that kind". A diffuse map replaces the ambient and diffuse colours rather
		// than tinting them, matching what the OpenGL shader does.
		GLuint diffusemapid = 0;
		GLuint specularmapid = 0;
		// Tangent-space normal map. When set, the surface normal comes from the
		// texture and the lighting is computed in tangent space.
		GLuint normalmapid = 0;

		// PBR takes a different shader entirely: a metallic-roughness workflow whose
		// maps replace the colours above rather than sitting alongside them. Backends
		// switch on ispbr rather than on which ids happen to be set, because a PBR
		// material with no maps at all is still PBR and has its own defaults.
		bool ispbr = false;
		GLuint albedomapid = 0;
		GLuint pbrnormalmapid = 0;
		GLuint metallicmapid = 0;
		GLuint roughnessmapid = 0;
		GLuint aomapid = 0;
	};

	// Backend hook for a 3D mesh. OpenGL draws these through its own shader and vbo
	// binding and leaves this a no-op; Vulkan looks the vertex array id up in its
	// registry and records a draw from the buffers already on the device.
	//
	// vertexArrayId is gVbo's VAO name, which is what the backend keys its buffers
	// off. indexCount is 0 for a non-indexed mesh, in which case vertexCount is
	// used. Only the model matrix is passed: the camera and the lights are scene
	// state the backend already has, and sending them per mesh would mean pushing
	// far more than the 128 bytes Vulkan guarantees for push constants.
	//
	// instanceCount above 1 draws the mesh that many times, reading a model matrix
	// per instance from the buffer gVbo::setInstanceData uploaded. The transforms
	// themselves are not passed here: they already live on the device by the time
	// this is called, which is the point of uploading them.
	virtual void drawMesh3D(GLuint vertexArrayId, int vertexCount, int indexCount,
			const glm::mat4& model, const gMeshSurface& surface,
			int drawMode = GL_TRIANGLES, int instanceCount = 1) {}

	// Backend hooks for shadow mapping. OpenGL leaves both alone: gShadowMap drives
	// it there directly, through an FBO and shader uniforms it owns. Vulkan needs
	// the render target created up front and the light's transform handed over,
	// because the depth pass is a separate render pass with its own pipeline.
	//
	// allocateShadowMap returns false when the backend cannot provide one, which is
	// what makes gShadowMap fall back to drawing the scene unshadowed.
	// Backend hook for one face of the skybox. OpenGL draws the sky through its own
	// cubemap shader and leaves this a no-op; Vulkan has no cube map here and draws
	// six quads instead, one per face, each sampling a plain 2D texture.
	//
	// xyzuv holds five floats per vertex - three of position in world space, two of
	// texture coordinate. Returns false when the backend cannot draw it, which is
	// what tells gSkybox to fall back to its own path.
	virtual bool drawSkyboxFace(GLuint textureId, const float* xyzuv, int vertexCount,
			const glm::mat4& viewProjection) { return false; }

	virtual bool allocateShadowMap(int width, int height) { return false; }
	virtual void releaseShadowMap() {}

	// Opens and closes the depth-only pass the shadow map is drawn into. Called by
	// the frame loop around the first of the two scene draws; OpenGL does the
	// equivalent inside gShadowMap::enable() and leaves these alone.
	virtual bool beginShadowPass() { return false; }
	virtual void endShadowPass() {}
	virtual bool isShadowPassActive() const { return false; }

	// lightMatrix is lightProjection * lightView; lightPosition is where the caster
	// sits, used for the depth bias. enabled false means shade without shadows.
	virtual void setShadowMapState(bool enabled, const glm::mat4& lightMatrix,
			const glm::vec3& lightPosition, bool softShadows) {}

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

	void addSceneLight(gLight* light);
	void removeSceneLight(gLight* light);
	gLight* getSceneLight(int lightNo);
	int getSceneLightNum();
	void removeAllSceneLights();
	// Virtual because the two backends publish light state at different moments.
	// OpenGL writes its uniform block right here, the instant a light is enabled or
	// its colour changes. The Vulkan backend gathers the whole scene block once per
	// render pass instead, so it overrides this to mark that block stale - a canvas
	// that enables its light just before drawing and disables it after, which is
	// what gLight's API invites, would otherwise be shaded with the state left over
	// from the previous frame, when the light was off.
	virtual void updateLights();

	// Presentation pacing. OpenGL sets it on the window through glfwSwapInterval
	// and needs nothing here; Vulkan expresses it as the swapchain's present mode,
	// so the backend overrides this to rebuild the swapchain with the new one.
	virtual void setVsync(bool enabled) {}

	// Multisample anti-aliasing, in samples per pixel: 1 (the default) is off, and 2,
	// 4 or 8 ask for that many coverage samples on the screen pass. Shaped like
	// setVsync for the same reason - OpenGL takes it from the window's framebuffer
	// while Vulkan bakes it into the render pass and every pipeline, so only the
	// Vulkan backend overrides these.
	//
	// The request is capped at what the device supports for colour *and* depth
	// attachments, so getMultiSampling() is the value that was actually achieved and
	// may be lower than what was asked for - 1 on a device that offers no
	// multisampled combination at all. Off by default: an existing application sees
	// no change in cost or appearance until it asks.
	virtual void setMultiSampling(int samples) {}
	virtual int getMultiSampling() const { return 1; }

	void updateScene();

	void gPushMatrix();
	void gPopMatrix();
	virtual void enableDepthTest() = 0;
	virtual void enableDepthTest(int depthTestType) = 0;
	virtual void setDepthTestFunc(int depthTestType) = 0;
	virtual void disableDepthTest() = 0;
	virtual bool isDepthTestEnabled() = 0;
	virtual int getDepthTestType() = 0;
	virtual void enableCulling() { iscullingenabled = true; }
	virtual void disableCulling() { iscullingenabled = false; }
	virtual bool isCullingEnabled() const { return iscullingenabled; }
	virtual void setCullFace(int face) { cullface = face; }
	virtual int getCullFace() const { return cullface; }
	virtual void setCullingDirection(int direction) { cullingdirection = direction; }
	virtual int getCullingDirection() const { return cullingdirection; }

	virtual void enableAlphaBlending() = 0;
	virtual void disableAlphaBlending() = 0;
	virtual bool isAlphaBlendingEnabled() = 0;

	// How a blended draw combines with what is already in the framebuffer. Only
	// meaningful while alpha blending is on; enabling it resets the mode to ALPHA,
	// which is what it has always done.
	//
	// This exists because there was no way to ask for anything but the standard
	// over operator. A game wanting an additive effect - a muzzle flash, a glow, a
	// spark - had to reach past the renderer and call glBlendFunc itself, and that
	// call means nothing to a backend that is not OpenGL: on Vulkan blending is
	// baked into the pipeline, so the effect silently came out composited instead
	// of added.
	enum BlendMode {
		BLENDMODE_ALPHA,     // src * a + dst * (1 - a). Layers one image over another.
		BLENDMODE_ADDITIVE,  // src * a + dst. Only brightens, so black adds nothing.
	};
	virtual void setBlendMode(int blendMode) { blendmode = blendMode; }
	virtual int getBlendMode() const { return blendmode; }
	virtual void enableAlphaTest() = 0;
	virtual void disableAlphaTest() = 0;
	virtual bool isAlphaTestEnabled() = 0;

	bool isSSAOEnabled();
	void enableSSAO();
	void disableSSAO();
	void setSSAOBias(float value);
	float getSSAOBias();
	void setSSAORadius(float value);
	float getSSAORadius();
	void setSSAOStrength(float value);
	float getSSAOStrength();
	void setSSAODebug(bool enabled);
	bool isSSAODebug();
	bool isSSAOAllocated();
	void beginSSAO();
	void endSSAO();

	bool isGammaCorrectionEnabled();
	void enableGammaCorrection();
	void disableGammaCorrection();

	bool isHDREnabled();
	void enableHDR();
	void disableHDR();

	bool isSoftShadowsEnabled();
	void enableSoftShadows();
	void disableSoftShadows();

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

	// Virtual because the two backends do not agree on what a projection matrix is.
	// OpenGL clips depth to -1..1, Vulkan to 0..1, so the Vulkan backend folds a
	// correction into whatever the camera hands it. Everything downstream - gMesh,
	// gGrid, gSkybox - passes getProjectionMatrix() straight to a shader, so putting
	// the fix here keeps it out of every one of those call sites.
	virtual void setProjectionMatrix(glm::mat4 projectionMatrix);
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

	virtual void takeScreenshot(gImage& img) = 0;
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
	virtual void drawArraysInstanced(int drawMode, int count, int instanceCount) = 0;
	virtual void drawElementsInstanced(int drawMode, int count, int instanceCount) = 0;

	virtual void enableVertexAttrib(int index) = 0;
	virtual void disableVertexAttrib(int index) = 0;
	virtual void setVertexAttribPointer(int index, int size, int type, bool normalized, int stride, const void* pointer) = 0;
	virtual void setVertexAttribDivisor(int index, int divisor) = 0;

	virtual void setViewport(int x, int y, int width, int height) = 0;
	void getViewport(int& x, int& y, int& width, int& height) const;

	/* -------------- gFbo --------------- */
	virtual GLuint createFramebuffer() = 0;
	virtual void deleteFramebuffer(GLuint& fbo) = 0;
	virtual void bindFramebuffer(GLuint fbo) = 0;
	GLuint getBoundFramebuffer() const;
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
	virtual void drawVbo(const gVbo& vbo, const glm::mat4& model, const gMeshSurface& surface) = 0;

	/* ---------------- gTexture ---------------- */
	virtual GLuint createTextures() = 0;
	virtual void bindTexture(GLuint texId) = 0;
	virtual void bindTexture(GLuint texId, int textureSlotNo) = 0;
	virtual void unbindTexture() = 0;
	virtual void activateTexture(int textureSlotNo = 0) = 0;
	virtual void resetTexture() = 0;
	virtual void deleteTexture(GLuint& texId) = 0;

	virtual void texImage2D(GLenum target, GLint internalFormat, int width, int height, GLint format, GLint type, void* data, GLint level = 0) = 0;
	virtual void setTextureMaxLevel(GLenum target, int maxLevel) = 0;
	virtual void setWrapping(GLenum target, GLint wrapS, GLint wrapT) = 0;
	virtual void setWrapping(GLenum target, GLint wrapS, GLint wrapT, GLint wrapR) = 0;

	virtual void setFiltering(GLenum target, GLint minFilter, GLint magFilter) = 0;
	virtual void setWrappingAndFiltering(GLenum target, GLint wrapS, GLint wrapT, GLint minFilter, GLint magFilter) = 0;
	virtual void setWrappingAndFiltering(GLenum target, GLint wrapS, GLint wrapT, GLint wrapR, GLint minFilter, GLint magFilter) = 0;
	virtual void setSwizzleMask(GLint swizzleMask[4]) = 0;

	virtual void readTexturePixels(unsigned char* inPixels, GLuint textureId, int width, int height, GLenum format) = 0;
	virtual void readTexturePixelsHDR(float* inPixels, GLuint textureId, int width, int height, GLenum format) = 0;

	virtual void generateMipMap() = 0;

	/* ---------------- gSkybox ---------------- */
	virtual void bindSkyTexture(GLuint texId) = 0;
	virtual void bindSkyTexture(GLuint texId, int textureSlot) = 0;
	virtual void unbindSkyTexture() = 0;
	virtual void unbindSkyTexture(int textureSlotNo) = 0;
	virtual void generateSkyMipMap() = 0;
	virtual void enableDepthTestEqual() = 0;
	virtual void createQuad(GLuint& inQuadVAO, GLuint& inQuadVBO) = 0;
	virtual void enableCubeMap() = 0;

	/* ---------------- gRenderObject ---------------- */
	virtual void pushMatrix() = 0;
	virtual void popMatrix() = 0;

	/* ---------------- Utilities ---------------- */
	void drawLine(float x1, float y1, float x2, float y2);
	void drawLine(float x1, float y1, float x2, float y2, float thickness);
	void drawLine(float x1, float y1, float z1, float x2, float y2, float z2, float thickness = 1.0f);
	void drawLine(float x1, float y1, float x2, float y2, float thickness, float rotateAngle, float pivotx, float pivoty);

	void drawTriangle(float px, float py, float qx, float qy, float rx, float ry, bool is_filled = true, float rotateAngle = 0.0f, float pivotx = 0.5f, float pivoty = 0.5f);
	void drawCircle(float xCenter, float yCenter, float radius, bool isFilled = false, float numberOfSides = 64.0f, float rotateAngle = 0.0f, float pivotx = 0.5f, float pivoty = 0.5f);
	void drawCross(float x, float y, float width, float height, float thickness, bool isFilled, float rotateAngle = 0.0f, float pivotx = 0.5f, float pivoty = 0.5f);
	void drawArc(float xCenter, float yCenter, float radius, bool isFilled = true, int numberOfSides = 60, float degree = 360.0f, float rotate = 360.0f, float rotateAngle = 0.0f, float pivotx = 0.5f, float pivoty = 0.5f);
	void drawArrow(float x1, float y1, float length, float angle, float tipLength, float tipAngle, float thickness = 1.0f, float rotateAngle = 0.0f, float pivotx = 0.5f, float pivoty = 0.5f);
	void drawRectangle(float x, float y, float w, float h, bool isFilled = false, float rotateAngle = 0.0f, float pivotx = 0.5f, float pivoty = 0.5f);
	void drawRoundedRectangle(float x, float y, float w, float h, int radius, bool isFilled, float rotateAngle = 0.0f, float pivotx = 0.5f, float pivoty = 0.5f);
	void drawBox(float x, float y, float z, float w = 1.0f, float h = 1.0f, float d = 1.0f, bool isFilled = true);
	void drawBox(float x, float y, float z, float w, float h, float d, float rotateAngle, float axisX, float axisY, float axisZ, bool isFilled = true);
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
	friend class gRenderObject;
	friend class gAppManager;

	static int width, height;
	static int unitwidth, unitheight;
	static int screenscaling;
	static int currentresolution, unitresolution;

	gColor* rendercolor = nullptr;

	int renderenginetype = G_RENDERER_GL;

	bool isfogenabled = false;
	int fogno = -1;
	gColor fogcolor;
	float fogdensity = 0.3f;
	float foggradient = 2.0f;
	int fogmode = FOGMODE_EXP;
	float foglinearstart = 0.0f;
	float foglinearend = 1.0f;

	std::deque<gLight*> scenelights;
	gUbo<gSceneLights>* lightsubo = nullptr;
	gUbo<gSceneData>* sceneubo = nullptr;
	bool islightingenabled = true;
	glm::vec3 lightingposition;
	gColor lightingcolor;
	gColor globalambientcolor;
	bool isglobalambientcolorchanged = true;

	bool isdepthtestenabled = false;
	int depthtesttype = 0;
	bool iscullingenabled = false;
	int cullface = GL_BACK;
	int cullingdirection = GL_CCW;
	unsigned int depthtesttypeid[2];
	bool isalphablendingenabled = false, isalphatestenabled = false;
	int blendmode = BLENDMODE_ALPHA;

	GLuint boundframebuffer = 0;
	int viewportx = 0, viewporty = 0, viewportwidth = 0, viewportheight = 0;

	bool isssaoenabled;
	float ssaobias;
	float ssaoradius;
	float ssaostrength;
	bool isssaoallocated;
	bool isssaodebug;
	gFbo* ssaofbo;
	gFbo* ssaoresultfbo;
	gShader* ssaoshader;
	gShader* ssaoblurshader;
	int ssaorealdefaultfbo;
	GLuint ssaoprevframebuffer;
	int ssaoprevviewport[4];
	bool isssaorendering;
	void initSSAOResources();
	void cleanupSSAOResources();
	void setupSSAODepthSampling();
	bool isgammacorrectionenabled = false;
	bool ishdrenabled = false;
	bool issoftshadowsenabled = false;

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

	unsigned int fullscreenquadvao;
	unsigned int fullscreenquadvbo;

	std::unique_ptr<gLine> linemesh, linemesh2, linemesh3;
	std::unique_ptr<gTriangle> trianglemesh;
	std::unique_ptr<gCircle> circlemesh;
	std::unique_ptr<gCross> crossmesh;
	std::unique_ptr<gArc> arcmesh;
	std::unique_ptr<gRectangle> rectanglemesh;
	std::unique_ptr<gRoundedRectangle> roundedrectanglemesh;
	std::unique_ptr<gBox> boxmesh;
	//std::unique_ptr<gArrow> arrowmesh;

	virtual void init();
	virtual void cleanup();
	virtual void updatePackUnpackAlignment(int i) = 0;

	void createPrimitiveMeshes();
	void destroyPrimitiveMeshes();

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
	static const std::string& getShaderSrcSSAOVertex();
	static const std::string& getShaderSrcSSAOFragment();
	static const std::string& getShaderSrcSSAOBlurFragment();
};

#endif /* CORE_GRENDERER_H_ */
