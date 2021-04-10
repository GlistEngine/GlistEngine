/*
 * gRenderObject.cpp
 *
 *  Created on: May 9, 2020
 *      Author: noyan
 */

#include "gRenderObject.h"
#include <iostream>


gRenderer* gRenderObject::renderer;

bool gRenderObject::isrendermaterialsloaded = false;
bool gRenderObject::isshadowmappingenabled = false;


gRenderObject::gRenderObject() {
	if(!isrendermaterialsloaded) loadRenderMaterials();
}

gRenderObject::~gRenderObject() {
}

void gRenderObject::loadRenderMaterials() {
	renderer = new gRenderer();

	isrendermaterialsloaded = true;
}

void gRenderObject::setScreenSize(int screenWidth, int screenHeight) {
	renderer->setScreenSize(screenWidth, screenHeight);
}

void gRenderObject::pushMatrix() {
#if defined(WIN32) || defined(LINUX)
	glPushMatrix();
#endif
}

void gRenderObject::popMatrix() {
#if defined(WIN32) || defined(LINUX)
	glPopMatrix();
#endif
}

void gRenderObject::enableShadowMapping() {
	isshadowmappingenabled = true;
}

void gRenderObject::disableShadowMapping() {
	isshadowmappingenabled = false;
}

bool gRenderObject::isShadowMappingEnabled() {
	return isshadowmappingenabled;
}



