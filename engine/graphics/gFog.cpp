/*
 * gFog.cpp
 *
 *  Created on: 15 Aðu 2021
 *      Author: furka
 */

#include "gFog.h"

gFog::gFog() {
	
}

gFog::~gFog() {
}

void gFog::enable() {
   renderer->enableFog();
}

void gFog::disable() {
	renderer->disableFog();
}

void gFog::setColor(float r, float g, float b) {
	renderer->setFogColor(r, g, b);
}

void gFog::setDensity(float d) {
	renderer->fogdensity = d;
}

void gFog::setGradient(float g) {
	renderer->foggradient = g;
}

gColor gFog::getColor() {
    return renderer->fogcolor;
}

float gFog::getDensity() {
    return renderer->fogdensity;
}

float gFog::getGradient() {
    return renderer->foggradient;
}



