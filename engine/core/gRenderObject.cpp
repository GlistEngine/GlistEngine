/*
 * gRenderObject.cpp
 *
 *  Created on: May 9, 2020
 *      Author: noyan
 */

#include "gCross.h"
#include "gArc.h"
#include "gLight.h"
#include "gLine.h"
#include "gCircle.h"
#include "gRectangle.h"
#include "gBox.h"
#include "gSphere.h"
#include "gTriangle.h"
#include "gRoundedRectangle.h"
#include "gCylinder.h"
#include "gCone.h"
#include "gTube.h"
#include "gUbo.h"
#include "gShader.h"
#include "gCamera.h"
#include "gGrid.h"
#include "gRenderer.h"
#include "gGLRenderEngine.h"

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
	renderer->pushMatrix();
#endif
}

void gRenderObject::popMatrix() {
#if defined(WIN32) || defined(LINUX)
	renderer->popMatrix();
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
	renderer = new gGLRenderEngine();
	renderer->init();
}

void gRenderObject::destroyRenderer() {
	if(!renderer) return;
	renderer->cleanup();
	delete renderer;
	renderer = nullptr;
}
