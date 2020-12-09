/*
 * gLight.cpp
 *
 *  Created on: Aug 28, 2020
 *      Author: noyan
 */

#include "gLight.h"


gLight::gLight(int lightType) {
	type = lightType;
	isenabled = false;

	ambientcolor = gColor(1.0f, 1.0f, 1.0f, 1.0f);
	diffusecolor = gColor(1.0f, 1.0f, 1.0f, 1.0f);
	specularcolor = gColor(1.0f, 1.0f, 1.0f, 1.0f);
//	attenuation = glm::vec3(0.0001f, 0.0001f, 0.0001f);
	attenuation = glm::vec3(1.0f, 0.0014f, 0.000007f);
	spotcutoff = glm::vec2(15.0f, 5.0f);
	directioneuler = glm::vec3(0, 0, -1);
}

gLight::~gLight() {
}

void gLight::enable() {
	renderer->enableLighting();
	renderer->setLightingColor(ambientcolor.r, ambientcolor.g, ambientcolor.b, ambientcolor.a);
	renderer->setLightingPosition(position);
	renderer->addSceneLight(this);
	isenabled = true;
}

void gLight::disable() {
	renderer->removeSceneLight(this);
	renderer->disableLighting();
	isenabled = false;
}

bool gLight::isEnabled() {
	return isenabled;
}

void gLight::setType(int lightType) {
	type = lightType;
}

int gLight::getType() {
	return type;
}

void gLight::rotate(float angle, float ax, float ay, float az) {
	gNode::rotate(angle, ax, ay, az);
	directioneuler = glm::vec3(0, 0, -1) * orientation;
}

glm::vec3 gLight::getDirection() {
//	glm::vec3 euler = glm::vec3(0, 0, -1) * orientation;
//	logi("Euler x:" + str(euler.x), ", y: " + str(euler.y) + ", z:" + str(euler.z));
	return directioneuler;
}

void gLight::setAmbientColor(int r, int g, int b, int a) {
	ambientcolor.set(r, g, b, a);
}

void gLight::setAmbientColor(gColor* color) {
	ambientcolor.set(color->r, color->g, color->b, color->a);
}

gColor* gLight::getAmbientColor() {
	return &ambientcolor;
}

float gLight::getAmbientColorRed() {
	return ambientcolor.r;
}

float gLight::getAmbientColorGreen() {
	return ambientcolor.g;
}

float gLight::getAmbientColorBlue() {
	return ambientcolor.b;
}

float gLight::getAmbientColorAlpha() {
	return ambientcolor.a;
}

float gLight::getDiffuseColorRed() {
	return diffusecolor.r;
}

float gLight::getDiffuseColorGreen() {
	return diffusecolor.g;
}

float gLight::getDiffuseColorBlue() {
	return diffusecolor.b;
}

float gLight::getDiffuseColorAlpha() {
	return diffusecolor.a;
}


void gLight::setDiffuseColor(int r, int g, int b, int a) {
	diffusecolor.set(r, g, b, a);
}

void gLight::setDiffuseColor(gColor* color) {
	diffusecolor.set(color->r, color->g, color->b, color->a);
}

gColor* gLight::getDiffuseColor() {
	return &diffusecolor;
}

void gLight::setSpecularColor(int r, int g, int b, int a) {
	specularcolor.set(r, g, b, a);
}

void gLight::setSpecularColor(gColor* color) {
	specularcolor.set(color->r, color->g, color->b, color->a);
}

gColor* gLight::getSpecularColor() {
	return &specularcolor;
}

float gLight::getSpecularColorRed() {
	return specularcolor.r;
}

float gLight::getSpecularColorGreen() {
	return specularcolor.g;
}

float gLight::getSpecularColorBlue() {
	return specularcolor.b;
}

float gLight::getSpecularColorAlpha() {
	return specularcolor.a;
}

void gLight::setAttenuation(float constant, float linear, float quadratic) {
	attenuation.x = constant;
	attenuation.y = linear;
	attenuation.z = quadratic;
}

void gLight::setAttenuation(glm::vec3 attenuation) {
	this->attenuation = attenuation;
}

glm::vec3 gLight::getAttenuation() {
	return attenuation;
}

float gLight::getAttenuationConstant() {
	return attenuation.x;
}

float gLight::getAttenuationLinear() {
	return attenuation.y;
}

float gLight::getAttenuationQuadratic() {
	return attenuation.z;
}

void gLight::setSpotCutOff(glm::vec2 spotCutOff) {
	spotcutoff = spotCutOff;
}

glm::vec2 gLight::getSpotCutOff() {
	return spotcutoff;
}

void gLight::setSpotCutOffAngle(float cutOffAngle) {
	spotcutoff.x = cutOffAngle;
}

float gLight::getSpotCutOffAngle() {
	return spotcutoff.x;
}

float gLight::getSpotOuterCutOffAngle() {
	return spotcutoff.x + spotcutoff.y;
}

void gLight::setSpotCutOffSpread(float cutOffSpreadAngle) {
	spotcutoff.y = cutOffSpreadAngle;
}

float gLight::getSpotCutOffSpread() {
	return spotcutoff.y;
}


