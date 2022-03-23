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

void gRenderObject::loadRenderMaterials() {
	renderer = new gRenderer();

	isrendermaterialsloaded = true;
}

void gRenderObject::setScreenSize(int screenWidth, int screenHeight) {
	renderer->setScreenSize(screenWidth, screenHeight);
}

void gRenderObject::setUnitScreenSize(int unitWidth, int unitHeight) {
	renderer->setUnitScreenSize(unitWidth, unitHeight);
}

void gRenderObject::setScreenScaling(int screenScaling) {
	renderer->setScreenScaling(screenScaling);
}

int gRenderObject::getScreenWidth() {
	return renderer->getScreenWidth();
}

int gRenderObject::getScreenHeight() {
	return renderer->getScreenHeight();
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

gRenderer* gRenderObject::getRenderer() {
	return renderer;
}

