/*
 * gRenderObject.cpp
 *
 *  Created on: May 9, 2020
 *      Author: noyan
 */

#include "gRenderObject.h"
#include <iostream>


gRenderer* gRenderObject::renderer = nullptr;

bool gRenderObject::isshadowmappingenabled = false;


gRenderObject::gRenderObject() {
	// Renderer should be created by the gAppManager
}

void gRenderObject::setScreenSize(int screenWidth, int screenHeight) {
#ifdef DEBUG
	assert(renderer);
#endif
	renderer->setScreenSize(screenWidth, screenHeight);
}

void gRenderObject::setUnitScreenSize(int unitWidth, int unitHeight) {
#ifdef DEBUG
	assert(renderer);
#endif
	renderer->setUnitScreenSize(unitWidth, unitHeight);
}

void gRenderObject::setScreenScaling(int screenScaling) {
#ifdef DEBUG
	assert(renderer);
#endif
	renderer->setScreenScaling(screenScaling);
}

int gRenderObject::getScreenWidth() {
#ifdef DEBUG
	assert(renderer);
#endif
	return renderer->getScreenWidth();
}

int gRenderObject::getScreenHeight() {
#ifdef DEBUG
	assert(renderer);
#endif
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
#ifdef DEBUG
	assert(renderer);
#endif
	return isshadowmappingenabled;
}

gRenderer* gRenderObject::getRenderer() {
	return renderer;
}

void gRenderObject::createRenderer() {
	destroyRenderer(); // Delete the previous renderer if exists. If renderer is null, this will have no effect.
	renderer = new gRenderer();
	renderer->init();
}

void gRenderObject::destroyRenderer() {
	delete renderer;
	renderer = nullptr;
}
