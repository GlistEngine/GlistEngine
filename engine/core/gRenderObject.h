/*
 * gRenderObject.h
 *
 *  Created on: May 9, 2020
 *      Author: noyan
 */

#pragma once

#ifndef ENGINE_BASE_GRENDEROBJECT_H_
#define ENGINE_BASE_GRENDEROBJECT_H_

#include "gRenderer.h"

#ifdef ANDROID
using gIndex = unsigned short;
#define G_INDEX_SIZE GL_UNSIGNED_SHORT
#else
using gIndex = unsigned int;
#define G_INDEX_SIZE GL_UNSIGNED_INT
#endif

class gRenderObject : public gObject {
public:
	gRenderObject();

	static void setScreenSize(int screenWidth, int screenHeight);
	static void setUnitScreenSize(int unitWidth, int unitHeight);
	static void setScreenScaling(int screenScaling);
	int getScreenWidth();
	int getScreenHeight();

	void pushMatrix();
	void popMatrix();

	static void enableShadowMapping();
	static void disableShadowMapping();
	static bool isShadowMappingEnabled();

	static gRenderer* getRenderer();

	static void destroyRenderer();
	static void createRenderer();
protected:
	friend class gRenderer;
	static gRenderer* renderer;
	static bool isshadowmappingenabled;
};

#endif /* ENGINE_BASE_GRENDEROBJECT_H_ */
