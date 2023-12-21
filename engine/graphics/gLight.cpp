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
	ischanged = true;

	ambientcolor = gColor(1.0f, 1.0f, 1.0f, 1.0f);
	diffusecolor = gColor(1.0f, 1.0f, 1.0f, 1.0f);
	specularcolor = gColor(1.0f, 1.0f, 1.0f, 1.0f);
//	attenuation = glm::vec3(0.0001f, 0.0001f, 0.0001f);
	attenuation = glm::vec3(1.0f, 0.0014f, 0.000007f);
	spotcutoff = glm::vec2(15.0f, 5.0f);
	directioneuler = glm::vec3(0, 0, -1);
	renderer->addSceneLight(this);
}

gLight::~gLight() {
	renderer->removeSceneLight(this);
}

void gLight::enable() {
	isenabled = true;
	renderer->updateLights();
}

void gLight::disable() {
	isenabled = false;
	renderer->updateLights();
}

bool gLight::isEnabled() const {
	return isenabled;
}

bool gLight::isChanged() const {
	return ischanged;
}

void gLight::setChanged(bool isChanged) {
	ischanged = isChanged;
}

void gLight::setType(int lightType) {
	type = lightType;
	ischanged = true;
}

int gLight::getType() const {
	return type;
}

void gLight::rotate(float angle, float ax, float ay, float az) {
	gNode::rotate(angle, ax, ay, az);
	directioneuler = glm::vec3(0, 0, -1) * orientation;
	ischanged = true;
}

const glm::vec3& gLight::getDirection() const {
//	glm::vec3 euler = glm::vec3(0, 0, -1) * orientation;
//	logi("Euler x:" + str(euler.x), ", y: " + str(euler.y) + ", z:" + str(euler.z));
	return directioneuler;
}

void gLight::setAmbientColor(int r, int g, int b, int a) {
	ambientcolor.set(r, g, b, a);
	ischanged = true;
}

void gLight::setAmbientColor(gColor* color) {
	ambientcolor.set(color->r, color->g, color->b, color->a);
	ischanged = true;
}

gColor* gLight::getAmbientColor() {
	return &ambientcolor;
}

float gLight::getAmbientColorRed() const {
	return ambientcolor.r;
}

float gLight::getAmbientColorGreen() const {
	return ambientcolor.g;
}

float gLight::getAmbientColorBlue() const {
	return ambientcolor.b;
}

float gLight::getAmbientColorAlpha() const {
	return ambientcolor.a;
}

float gLight::getDiffuseColorRed() const {
	return diffusecolor.r;
}

float gLight::getDiffuseColorGreen() const {
	return diffusecolor.g;
}

float gLight::getDiffuseColorBlue() const {
	return diffusecolor.b;
}

float gLight::getDiffuseColorAlpha() const {
	return diffusecolor.a;
}


void gLight::setDiffuseColor(int r, int g, int b, int a) {
	diffusecolor.set(r, g, b, a);
	ischanged = true;
}

void gLight::setDiffuseColor(gColor* color) {
	diffusecolor.set(color->r, color->g, color->b, color->a);
	ischanged = true;
}

gColor* gLight::getDiffuseColor() {
	return &diffusecolor;
}

void gLight::setSpecularColor(int r, int g, int b, int a) {
	specularcolor.set(r, g, b, a);
	ischanged = true;
}

void gLight::setSpecularColor(gColor* color) {
	specularcolor.set(color->r, color->g, color->b, color->a);
	ischanged = true;
}

gColor* gLight::getSpecularColor() {
	return &specularcolor;
}

float gLight::getSpecularColorRed() const {
	return specularcolor.r;
}

float gLight::getSpecularColorGreen() const {
	return specularcolor.g;
}

float gLight::getSpecularColorBlue() const {
	return specularcolor.b;
}

float gLight::getSpecularColorAlpha() const {
	return specularcolor.a;
}

void gLight::setAttenuation(float constant, float linear, float quadratic) {
	attenuation.x = constant;
	attenuation.y = linear;
	attenuation.z = quadratic;
	ischanged = true;
}

void gLight::setAttenuation(glm::vec3 attenuation) {
	this->attenuation = attenuation;
	ischanged = true;
}

const glm::vec3& gLight::getAttenuation() const {
	return attenuation;
}

float gLight::getAttenuationConstant() const {
	return attenuation.x;
}

float gLight::getAttenuationLinear() const {
	return attenuation.y;
}

float gLight::getAttenuationQuadratic() const {
	return attenuation.z;
}

void gLight::setSpotCutOff(glm::vec2 spotCutOff) {
	spotcutoff = spotCutOff;
	ischanged = true;
}

const glm::vec2& gLight::getSpotCutOff() const {
	return spotcutoff;
}

void gLight::setSpotCutOffAngle(float cutOffAngle) {
	spotcutoff.x = cutOffAngle;
	ischanged = true;
}

float gLight::getSpotCutOffAngle() const {
	return spotcutoff.x;
}

float gLight::getSpotOuterCutOffAngle() const {
	return spotcutoff.x + spotcutoff.y;
}

void gLight::setSpotCutOffSpread(float cutOffSpreadAngle) {
	spotcutoff.y = cutOffSpreadAngle;
	ischanged = true;
}

float gLight::getSpotCutOffSpread() const {
	return spotcutoff.y;
}

void gLight::processTransformationMatrix() {
	gNode::processTransformationMatrix();
	ischanged = true;
}

