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
#ifdef WIN32
	glPushMatrix();
#endif
}

void gRenderObject::popMatrix() {
#ifdef WIN32
	glPopMatrix();
#endif
}


