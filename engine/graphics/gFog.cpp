/*
 * gFog.cpp
 *
 *  Created on: Aug 15, 2021
 *      Author: furka
 */

#include "gFog.h"

int gFog::fognum = 0;

gFog::gFog() {
	color.set(0.3f, 0.3f, 0.3f);
	mode = gRenderer::FOGMODE_EXP;
	density = 0.3f;
	gradient = 2.0f;
	linearstart = 0.0f;
	linearend = 1.0f;
	fogno = fognum++;
}

void gFog::enable() {
	if(renderer->getFogNo() == fogno) {
		renderer->enableFog();
		return;
	}

	renderer->setFogNo(fogno);
	renderer->setFogColor(&color);
	renderer->setFogMode(mode);
	renderer->setFogDensity(density);
	renderer->setFogGradient(gradient);
	renderer->setFogLinearStart(linearstart);
	renderer->setFogLinearEnd(linearend);
	renderer->enableFog();
}

void gFog::disable() {
	renderer->disableFog();
}

void gFog::setColor(float r, float g, float b) {
	color.set(r, g, b);
	if(renderer->getFogNo() == fogno) renderer->setFogColor(r, g, b);
}

void gFog::setMode(int value) {
	mode = value;
	if(renderer->getFogNo() == fogno) renderer->setFogMode(value);
}

void gFog::setDensity(float value) {
	density = value;
	if(renderer->getFogNo() == fogno) renderer->setFogDensity(value);
}

void gFog::setGradient(float value) {
	density = value;
	if(renderer->getFogNo() == fogno) renderer->setFogGradient(value);
}

void gFog::setLinearStart(float value) {
	linearstart = value;
	if(renderer->getFogNo() == fogno) renderer->setFogLinearStart(value);
}

void gFog::setLinearEnd(float value) {
	linearend = value;
	if(renderer->getFogNo() == fogno) renderer->setFogLinearEnd(value);
}

const gColor& gFog::getColor() const {
	return color;
}

int gFog::getMode() const {
	return mode;
}

float gFog::getDensity() const {
	return density;
}

float gFog::getGradient() const {
	return gradient;
}

float gFog::getLinearStart() const {
	return linearstart;
}

float gFog::getLinearEnd() const {
	return linearend;
}


