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
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale, glm::perspective
#include <glm/gtx/quaternion.hpp>

#include <deque>
#include "gShader.h"
#include "gColor.h"
#include "gConstants.h"
class gLight;
class gImage;
//#include "gLight.h"

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


class gRenderer: public gObject {
public:
	static const int SCREENSCALING_NONE, SCREENSCALING_MIPMAP, SCREENSCALING_AUTO;
	static const int DEPTHTESTTYPE_LESS, DEPTHTESTTYPE_ALWAYS;
	static const int FOGMODE_LINEAR, FOGMODE_EXP;

	gRenderer();
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

	//grid
	void drawGrid();
	void drawGridYZ();
	void drawGridXY();
	void drawGridXZ();
	void enableGrid();
	void disableGrid();
	void setGridEnableAxis(bool xy, bool yz, bool xz);
	void setGridEnableXY(bool xy), setGridEnableYZ(bool yz), setGridEnableXZ(bool xz);
	void setGridMaxLength(float length);
	float getGridMaxLength();
	void setGridLineInterval(float intervalvalue);
	float getGridLineInterval();
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
	bool isGridEnabled(), isGridXYEnabled(), isGridYZEnabled(), isGridXZEnabled();



	static void setCurrentResolution(int resolution);
	static void setCurrentResolution(int screenWidth, int screenHeight);
	static void setUnitResolution(int resolution);
	static void setUnitResolution(int screenWidth, int screenHeight);
	static int getResolution(int screenWidth, int screenHeight);
	int getCurrentResolution();
	int getUnitResolution();
	static int scaleX(int x);
	static int scaleY(int y);

	void setColor(int r, int g, int b, int a = 255);
	void setColor(float r, float g, float b, float a = 1.0f);
	void setColor(gColor color);
	void setColor(gColor* color);
	gColor* getColor();

	void clear();
	void clearColor(int r, int g, int b, int a = 255);
	void clearColor(gColor color);

	void enableLighting();
	void disableLighting();
	bool isLightingEnabled();
	void setLightingColor(int r, int g, int b, int a = 255);
	void setLightingColor(gColor* color);
	gColor* getLightingColor();
	void setLightingPosition(glm::vec3 lightingPosition);
	glm::vec3 getLightingPosition();
	void setGlobalAmbientColor(int r, int g, int b, int a = 255);
	void setGlobalAmbientColor(gColor color);
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
	gLight* getSceneLight(int lightNo);
	int getSceneLightNum();
	void removeSceneLight(gLight* light);
	void removeAllSceneLights();

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

	void setProjectionMatrix(glm::mat4 projectionMatrix);
	void setProjectionMatrix2d(glm::mat4 projectionMatrix2d);
	void setViewMatrix(glm::mat4 viewMatrix);
	void setCameraPosition(glm::vec3 cameraPosition);
	const glm::mat4& getProjectionMatrix() const;
	const glm::mat4& getProjectionMatrix2d() const;
	const glm::mat4& getViewMatrix() const;
	const glm::vec3& getCameraPosition() const;
	void backupMatrices();
	void restoreMatrices();

	/*
	 * Takes Screen Shot of the current Rendered Screen and returns it as an gImage class
	 */
	gImage takeScreenshot();

	/*
	 * Takes Screen Shot of the part of current Rendered Screen and returns it as an gImage class
	 */
	gImage takeScreenshot(int x, int y, int width, int height);

private:
	static int width, height;
	static int unitwidth, unitheight;
	static int screenscaling;
	static int currentresolution, unitresolution;
	//grid
	int gridmaxvalue;
	float gridlineinterval;
	bool isgridenable, isgridxzenable, isgridxyenable, isgridyzenable;
	gColor gridxzcolor, gridxzmargincolor, gridxycolor,gridxymargincolor, gridyzcolor, gridyzmargincolor;//default
	//grid - END
	gColor* rendercolor;

	gColor* lightingcolor;
	bool islightingenabled;

	bool isfogenabled;
	int fogno;
	gColor fogcolor;
	float fogdensity;
	float foggradient;
	int fogmode;
	float foglinearstart;
	float foglinearend;

	glm::vec3 lightingposition;
	gColor* globalambientcolor;
	int li;
	std::deque<gLight*> scenelights;


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

	glm::mat4 projectionmatrix;
	glm::mat4 projectionmatrixold;
	glm::mat4 projectionmatrix2d;
	glm::mat4 viewmatrix;
	glm::mat4 viewmatrixold;
	glm::vec3 cameraposition;

	const std::string getShaderSrcColorVertex();
	const std::string getShaderSrcColorFragment();
	const std::string getShaderSrcTextureVertex();
	const std::string getShaderSrcTextureFragment();
	const std::string getShaderSrcImageVertex();
	const std::string getShaderSrcImageFragment();
	const std::string getShaderSrcFontVertex();
	const std::string getShaderSrcFontFragment();
	const std::string getShaderSrcSkyboxVertex();
	const std::string getShaderSrcSkyboxFragment();
	const std::string getShaderSrcShadowmapVertex();
	const std::string getShaderSrcShadowmapFragment();
	const std::string getShaderSrcPbrVertex();
	const std::string getShaderSrcPbrFragment();
	const std::string getShaderSrcCubemapVertex();
	const std::string getShaderSrcEquirectangularFragment();
	const std::string getShaderSrcIrradianceFragment();
	const std::string getShaderSrcPrefilterFragment();
	const std::string getShaderSrcBrdfVertex();
	const std::string getShaderSrcBrdfFragment();
	const std::string getShaderSrcFboVertex();
	const std::string getShaderSrcFboFragment();
};

#endif /* CORE_GRENDERER_H_ */
