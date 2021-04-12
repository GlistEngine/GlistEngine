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
//#include "gColor.h"


class gRenderObject : public gObject {
public:
	gRenderObject();
	virtual ~gRenderObject();

	static void setScreenSize(int screenWidth, int screenHeight);
	static void setUnitScreenSize(int unitWidth, int unitHeight);
	static void setScreenScaling(int screenScaling);

	void pushMatrix();
	void popMatrix();

	static void enableShadowMapping();
	static void disableShadowMapping();
	static bool isShadowMappingEnabled();


protected:
	static gRenderer* renderer;
	static bool isshadowmappingenabled;

private:
	static bool isrendermaterialsloaded;

	static void loadRenderMaterials();
};

#endif /* ENGINE_BASE_GRENDEROBJECT_H_ */
