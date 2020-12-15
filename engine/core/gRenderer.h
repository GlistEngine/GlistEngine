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
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale, glm::perspective
#include <glm/gtx/quaternion.hpp>

#include <vector>
#include "gShader.h"
#include "gColor.h"
class gLight;
//#include "gLight.h"


class gRenderer: public gObject {
public:
	static const int DEPTHTESTTYPE_LESS, DEPTHTESTTYPE_ALWAYS;

	gRenderer();
	virtual ~gRenderer();

	static void setScreenSize(int screenWidth, int screenHeight);

	int getWidth();
	int getHeight();
	int getScreenWidth();
	int getScreenHeight();

	void setColor(int r, int g, int b, int a = 255);
	void setColor(gColor color);
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
	void enableAlphaTest();
	void disableAlphaTest();

	gShader* getColorShader();
	gShader* getTextureShader();
	gShader* getFontShader();
	gShader* getImageShader();

	void setProjectionMatrix(glm::mat4 projectionMatrix);
	void setProjectionMatrix2d(glm::mat4 projectionMatrix2d);
	void setViewMatrix(glm::mat4 viewMatrix);
	glm::mat4 getProjectionMatrix();
	glm::mat4 getProjectionMatrix2d();
	glm::mat4 getViewMatrix();
	void backupMatrices();
	void restoreMatrices();


private:
	static int width, height;
	gColor *rendercolor;

	gColor *lightingcolor;
	bool islightingenabled;
	glm::vec3 lightingposition;
	gColor *globalambientcolor;
	int li;
	std::vector<gLight*> scenelights;

	bool isdepthtestenabled;
	int depthtesttype;
	unsigned int depthtesttypeid[2];

	gShader *colorshader;
	gShader *textureshader;
	gShader *fontshader;
	gShader *imageshader;

	glm::mat4 projectionmatrix;
	glm::mat4 projectionmatrixold;
	glm::mat4 projectionmatrix2d;
	glm::mat4 viewmatrix;
	glm::mat4 viewmatrixold;

	const std::string getShaderSrcColorVertex();
	const std::string getShaderSrcColorFragment();
	const std::string getShaderSrcTextureVertex();
	const std::string getShaderSrcTextureFragment();
	const std::string getShaderSrcImageVertex();
	const std::string getShaderSrcImageFragment();
	const std::string getShaderSrcFontVertex();
	const std::string getShaderSrcFontFragment();

};

#endif /* CORE_GRENDERER_H_ */
