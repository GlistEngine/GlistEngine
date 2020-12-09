/*
 * gMaterial.cpp
 *
 *  Created on: Oct 29, 2020
 *      Author: noyan
 */

#include "gMaterial.h"

gMaterial::gMaterial() {
    shininess = 0.5f;
    diffusemapenabled = false;
    specularmapenabled = false;
    normalmapenabled = false;
}

gMaterial::~gMaterial() {
}

void gMaterial::setAmbientColor(int r, int g, int b, int a) {
	ambient.set(r, g, b, a);
}

void gMaterial::setAmbientColor(float r, float g, float b, float a) {
	ambient.set(r, g, b, a);
}

void gMaterial::setAmbientColor(gColor* color) {
	ambient.set(color);
}

gColor* gMaterial::getAmbientColor() {
	return &ambient;
}

void gMaterial::setDiffuseColor(int r, int g, int b, int a) {
	diffuse.set(r, g, b, a);
}

void gMaterial::setDiffuseColor(float r, float g, float b, float a) {
	diffuse.set(r, g, b, a);
}

void gMaterial::setDiffuseColor(gColor* color) {
	diffuse.set(color);
}

gColor* gMaterial::getDiffuseColor() {
	return &diffuse;
}

void gMaterial::setSpecularColor(int r, int g, int b, int a) {
	specular.set(r, g, b, a);
}

void gMaterial::setSpecularColor(float r, float g, float b, float a) {
	specular.set(r, g, b, a);
}

void gMaterial::setSpecularColor(gColor* color) {
	specular.set(color);
}

gColor* gMaterial::getSpecularColor() {
	return &specular;
}

void gMaterial::setShininess(float shininess) {
	this->shininess = shininess;
}

float gMaterial::getShininess() {
	return shininess;
}

void gMaterial::setDiffuseMap(gTexture* diffuseMap) {
	diffusemap = *diffuseMap;
	diffusemapenabled = true;
}

gTexture* gMaterial::getDiffuseMap() {
	return &diffusemap;
}

void gMaterial::bindDiffuseMap() {
	diffusemap.bind();
}

void gMaterial::unbindDiffuseMap() {
	diffusemap.unbind();
}

void gMaterial::setDiffuseMapEnabled(bool enableDiffuseMap) {
	diffusemapenabled = enableDiffuseMap;
}

bool gMaterial::isDiffuseMapEnabled() {
	return diffusemapenabled;
}

void gMaterial::setSpecularMap(gTexture* specularMap) {
	specularmap = *specularMap;
	specularmapenabled = true;
}

gTexture* gMaterial::getSpecularMap() {
	return &specularmap;
}

void gMaterial::bindSpecularMap() {
	specularmap.bind();
}

void gMaterial::unbindSpecularMap() {
	specularmap.unbind();
}

void gMaterial::setSpecularMapEnabled(bool enableSpecularMap) {
	specularmapenabled = enableSpecularMap;
}

bool gMaterial::isSpecularMapEnabled() {
	return specularmapenabled;
}

void gMaterial::setNormalMap(gTexture* normalMap) {
	normalmap = *normalMap;
	normalmapenabled = true;
}

gTexture* gMaterial::getNormalMap() {
	return &normalmap;
}

void gMaterial::bindNormalMap() {
	normalmap.bind();
}

void gMaterial::unbindNormalMap() {
	normalmap.unbind();
}

void gMaterial::setNormalMapEnabled(bool enableNormalMap) {
	normalmapenabled = enableNormalMap;
}

bool gMaterial::isNormalMapEnabled() {
	return normalmapenabled;
}


