/*
 * gFog.cpp
 *
 *  Created on: 12 Ağu 2021
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
	renderer->density = d;
}

void gFog::setGradient(float g) {
	renderer->gradient = g;
}

gColor gFog::getColor() {
    return renderer->fogcolor;
}

float gFog::getDensity() {
    return renderer->density;
}

float gFog::getGradient() {
    return renderer->gradient;
}


