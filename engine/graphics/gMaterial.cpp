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
    heightmapenabled = false;
    albedomapenabled = false;
    roughnessmapenabled = false;
    metalnessmapenabled = false;
    pbrnormalmapenabled = false;
    aomapenabled = false;
    ispbr = false;
}

gMaterial::~gMaterial() {
}

bool gMaterial::isPBR() {
	return ispbr;
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

void gMaterial::loadDiffuseMap(std::string texturePath, bool isHDR) {
	diffusemap.loadTexture(texturePath, isHDR);
	if(diffusemap.getWidth() > 0) diffusemapenabled = true;
}

void gMaterial::setDiffuseMap(gTexture* diffuseMap) {
	diffusemap = *diffuseMap;
	if(diffusemap.getWidth() > 0) diffusemapenabled = true;
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

void gMaterial::loadSpecularMap(std::string texturePath, bool isHDR) {
	specularmap.loadTexture(texturePath, isHDR);
	if(specularmap.getWidth() > 0) specularmapenabled = true;
}

void gMaterial::setSpecularMap(gTexture* specularMap) {
	specularmap = *specularMap;
	if(specularmap.getWidth() > 0) specularmapenabled = true;
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

void gMaterial::loadNormalMap(std::string texturePath, bool isHDR) {
	normalmap.loadTexture(texturePath, isHDR);
	if(normalmap.getWidth() > 0) normalmapenabled = true;
}

void gMaterial::setNormalMap(gTexture* normalMap) {
	normalmap = *normalMap;
	if(normalmap.getWidth() > 0) normalmapenabled = true;
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

void gMaterial::loadHeightMap(std::string texturePath, bool isHDR) {
	heightmap.loadTexture(texturePath, isHDR);
	if(heightmap.getWidth() > 0) heightmapenabled = true;
}

void gMaterial::setHeightMap(gTexture* heightMap) {
	heightmap = *heightMap;
	if(heightmap.getWidth() > 0) heightmapenabled = true;
}

gTexture* gMaterial::getHeightMap() {
	return &heightmap;
}

void gMaterial::bindHeightMap() {
	heightmap.bind();
}

void gMaterial::unbindHeightMap() {
	heightmap.unbind();
}

void gMaterial::setHeightMapEnabled(bool enableHeightMap) {
	heightmapenabled = enableHeightMap;
}

bool gMaterial::isHeightMapEnabled() {
	return heightmapenabled;
}

void gMaterial::loadAlbedoMap(std::string texturePath, bool isHDR) {
	albedomap.loadTexture(texturePath, isHDR);
	if(albedomap.getWidth() > 0) {
		albedomapenabled = true;
		ispbr = true;
	}
}

void gMaterial::setAlbedoMap(gTexture* albedoMap) {
	albedomap = *albedoMap;
	if(albedomap.getWidth() > 0) {
		albedomapenabled = true;
		ispbr = true;
	}
}

gTexture* gMaterial::getAlbedoMap() {
	return &albedomap;
}

void gMaterial::bindAlbedoMap(int slotNo) {
	if(albedomapenabled) albedomap.bind(slotNo);
}

void gMaterial::unbindAlbedoMap() {
	albedomap.unbind();
}

void gMaterial::loadRoughnessMap(std::string texturePath, bool isHDR) {
	roughnessmap.loadTexture(texturePath, isHDR);
	if(roughnessmap.getWidth() > 0) {
		roughnessmapenabled = true;
		ispbr = true;
	}
}

void gMaterial::setRoughnessMap(gTexture* roughnessMap) {
	roughnessmap = *roughnessMap;
	if(roughnessmap.getWidth() > 0) {
		roughnessmapenabled = true;
		ispbr = true;
	}
}

gTexture* gMaterial::getRoughnessMap() {
	return &roughnessmap;
}

void gMaterial::bindRoughnessMap(int slotNo) {
	roughnessmap.bind(slotNo);
}

void gMaterial::unbindRoughnessMap() {
	roughnessmap.unbind();
}

void gMaterial::setRoughnessMapEnabled(bool enableRoughnessMap) {
	roughnessmapenabled = enableRoughnessMap;
}

bool gMaterial::isRoughnessMapEnabled() {
	return roughnessmapenabled;
}

void gMaterial::loadMetalnessMap(std::string texturePath, bool isHDR) {
	metalnessmap.loadTexture(texturePath, isHDR);
	if(metalnessmap.getWidth() > 0) {
		metalnessmapenabled = true;
		ispbr = true;
	}
}

void gMaterial::setMetalnessMap(gTexture* metalnessMap) {
	metalnessmap = *metalnessMap;
	if(metalnessmap.getWidth() > 0) {
		metalnessmapenabled = true;
		ispbr = true;
	}
}

gTexture* gMaterial::getMetalnessMap() {
	return &metalnessmap;
}

void gMaterial::bindMetalnessMap(int slotNo) {
	metalnessmap.bind(slotNo);
}

void gMaterial::unbindMetalnessMap() {
	metalnessmap.unbind();
}

void gMaterial::setMetalnessMapEnabled(bool enableMetalnessMap) {
	metalnessmapenabled = enableMetalnessMap;
}

bool gMaterial::isMetalnessMapEnabled() {
	return metalnessmapenabled;
}

void gMaterial::loadPbrNormalMap(std::string texturePath, bool isHDR) {
	pbrnormalmap.loadTexture(texturePath, isHDR);
	if(pbrnormalmap.getWidth() > 0) pbrnormalmapenabled = true;
}

void gMaterial::setPbrNormalMap(gTexture* pbrNormalMap) {
	pbrnormalmap = *pbrNormalMap;
	if(pbrnormalmap.getWidth() > 0) pbrnormalmapenabled = true;
}

gTexture* gMaterial::getPbrNormalMap() {
	return &pbrnormalmap;
}

void gMaterial::bindPbrNormalMap(int slotNo) {
	pbrnormalmap.bind(slotNo);
}

void gMaterial::unbindPbrNormalMap() {
	pbrnormalmap.unbind();
}

void gMaterial::setPbrNormalMapEnabled(bool enablePbrNormalMap) {
	pbrnormalmapenabled = enablePbrNormalMap;
}

bool gMaterial::isPbrNormalMapEnabled() {
	return pbrnormalmapenabled;
}

void gMaterial::loadAOMap(std::string texturePath, bool isHDR) {
	aomap.loadTexture(texturePath, isHDR);
	if(aomap.getWidth() > 0) aomapenabled = true;
}

void gMaterial::setAOMap(gTexture* aoMap) {
	aomap = *aoMap;
	if(aomap.getWidth() > 0) aomapenabled = true;
}

gTexture* gMaterial::getAOMap() {
	return &aomap;
}

void gMaterial::bindAOMap(int slotNo) {
	aomap.bind(slotNo);
}

void gMaterial::unbindAOMap() {
	aomap.unbind();
}

void gMaterial::setAOMapEnabled(bool enableAOMap) {
	aomapenabled = enableAOMap;
}

bool gMaterial::isAOMapEnabled() {
	return aomapenabled;
}

