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
    diffusemapown = false;
    normalmapown = false;
    specularmapown = false;
    heightmapown = false;
    albedomapown = false;
    roughnessmapown = false;
    metalnessmapown = false;
    pbrnormalmapown = false;
    aomapown = false;
}

gMaterial::~gMaterial() {
	if(diffusemapenabled && diffusemapown) delete diffusemap;
	if(normalmapenabled && normalmapown) delete normalmap;
	if(specularmapenabled && specularmapown) delete specularmap;
	if(heightmapenabled && heightmapown) delete heightmap;
	if(albedomapenabled && albedomapown) delete albedomap;
	if(roughnessmapenabled && roughnessmapown) delete roughnessmap;
	if(metalnessmapenabled && metalnessmapown) delete metalnessmap;
	if(pbrnormalmapenabled && pbrnormalmapown) delete pbrnormalmap;
	if(aomapenabled && aomapown) delete aomap;
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

void gMaterial::loadDiffuseMap(std::string texturePath) {
	if(diffusemapenabled && diffusemapown) delete diffusemap;
	diffusemap = new gTexture();
	diffusemap->loadTexture(texturePath);
	if(diffusemap->getWidth() > 0) diffusemapenabled = true;
	diffusemapown = true;
}

void gMaterial::setDiffuseMap(gTexture* diffuseMap) {
	if(diffusemapenabled && diffusemapown) delete diffusemap;
	diffusemap = diffuseMap;
	if(diffusemap->getWidth() > 0) diffusemapenabled = true;
	diffusemapown = false;
}

gTexture* gMaterial::getDiffuseMap() {
	return diffusemap;
}

void gMaterial::bindDiffuseMap() {
	diffusemap->bind();
}

void gMaterial::unbindDiffuseMap() {
	diffusemap->unbind();
}

void gMaterial::setDiffuseMapEnabled(bool enableDiffuseMap) {
	diffusemapenabled = enableDiffuseMap;
}

bool gMaterial::isDiffuseMapEnabled() {
	return diffusemapenabled;
}

void gMaterial::loadSpecularMap(std::string texturePath) {
	if(specularmapenabled && specularmapown) delete specularmap;
	specularmap = new gTexture();
	specularmap->loadTexture(texturePath);
	if(specularmap->getWidth() > 0) specularmapenabled = true;
	specularmapown = true;
}

void gMaterial::setSpecularMap(gTexture* specularMap) {
	if(specularmapenabled && specularmapown) delete specularmap;
	specularmap = specularMap;
	if(specularmap->getWidth() > 0) specularmapenabled = true;
	specularmapown = false;
}

gTexture* gMaterial::getSpecularMap() {
	return specularmap;
}

void gMaterial::bindSpecularMap() {
	specularmap->bind();
}

void gMaterial::unbindSpecularMap() {
	specularmap->unbind();
}

void gMaterial::setSpecularMapEnabled(bool enableSpecularMap) {
	specularmapenabled = enableSpecularMap;
}

bool gMaterial::isSpecularMapEnabled() {
	return specularmapenabled;
}

void gMaterial::loadNormalMap(std::string texturePath) {
	if(normalmapenabled && normalmapown) delete normalmap;
	normalmap = new gTexture();
	normalmap->loadTexture(texturePath);
	if(normalmap->getWidth() > 0) normalmapenabled = true;
	normalmapown = true;
}

void gMaterial::setNormalMap(gTexture* normalMap) {
	if(normalmapenabled && normalmapown) delete normalmap;
	normalmap = normalMap;
	if(normalmap->getWidth() > 0) normalmapenabled = true;
	normalmapown = false;
}

gTexture* gMaterial::getNormalMap() {
	return normalmap;
}

void gMaterial::bindNormalMap() {
	normalmap->bind();
}

void gMaterial::unbindNormalMap() {
	normalmap->unbind();
}

void gMaterial::setNormalMapEnabled(bool enableNormalMap) {
	normalmapenabled = enableNormalMap;
}

bool gMaterial::isNormalMapEnabled() {
	return normalmapenabled;
}

void gMaterial::loadHeightMap(std::string texturePath) {
	if(heightmapenabled && heightmapown) delete heightmap;
	heightmap = new gTexture();
	heightmap->loadTexture(texturePath);
	if(heightmap->getWidth() > 0) heightmapenabled = true;
	heightmapown = true;
}

void gMaterial::setHeightMap(gTexture* heightMap) {
	if(heightmapenabled && heightmapown) delete heightmap;
	heightmap = heightMap;
	if(heightmap->getWidth() > 0) heightmapenabled = true;
	heightmapown = false;
}

gTexture* gMaterial::getHeightMap() {
	return heightmap;
}

void gMaterial::bindHeightMap() {
	heightmap->bind();
}

void gMaterial::unbindHeightMap() {
	heightmap->unbind();
}

void gMaterial::setHeightMapEnabled(bool enableHeightMap) {
	heightmapenabled = enableHeightMap;
}

bool gMaterial::isHeightMapEnabled() {
	return heightmapenabled;
}

void gMaterial::loadAlbedoMap(std::string texturePath) {
	if(albedomapenabled && albedomapown) delete albedomap;
	albedomap = new gTexture();
	albedomap->loadTexture(texturePath);
	if(albedomap->getWidth() > 0) {
		albedomapenabled = true;
		ispbr = true;
	}
	albedomapown = true;
}

void gMaterial::setAlbedoMap(gTexture* albedoMap) {
	if(albedomapenabled && albedomapown) delete albedomap;
	albedomap = albedoMap;
	if(albedomap->getWidth() > 0) {
		albedomapenabled = true;
		ispbr = true;
	}
	albedomapown = false;
}

gTexture* gMaterial::getAlbedoMap() {
	return albedomap;
}

void gMaterial::bindAlbedoMap(int slotNo) {
	if(albedomapenabled) albedomap->bind(slotNo);
}

void gMaterial::unbindAlbedoMap() {
	albedomap->unbind();
}

void gMaterial::loadRoughnessMap(std::string texturePath) {
	if(roughnessmapenabled && roughnessmapown) delete roughnessmap;
	roughnessmap = new gTexture();
	roughnessmap->loadTexture(texturePath);
	if(roughnessmap->getWidth() > 0) {
		roughnessmapenabled = true;
		ispbr = true;
	}
	roughnessmapown = true;
}

void gMaterial::setRoughnessMap(gTexture* roughnessMap) {
	if(roughnessmapenabled && roughnessmapown) delete roughnessmap;
	roughnessmap = roughnessMap;
	if(roughnessmap->getWidth() > 0) {
		roughnessmapenabled = true;
		ispbr = true;
	}
	roughnessmapown = false;
}

gTexture* gMaterial::getRoughnessMap() {
	return roughnessmap;
}

void gMaterial::bindRoughnessMap(int slotNo) {
	roughnessmap->bind(slotNo);
}

void gMaterial::unbindRoughnessMap() {
	roughnessmap->unbind();
}

void gMaterial::setRoughnessMapEnabled(bool enableRoughnessMap) {
	roughnessmapenabled = enableRoughnessMap;
}

bool gMaterial::isRoughnessMapEnabled() {
	return roughnessmapenabled;
}

void gMaterial::loadMetalnessMap(std::string texturePath) {
	if(metalnessmapenabled && metalnessmapown) delete metalnessmap;
	metalnessmap = new gTexture();
	metalnessmap->loadTexture(texturePath);
	if(metalnessmap->getWidth() > 0) {
		metalnessmapenabled = true;
		ispbr = true;
	}
	metalnessmapown = true;
}

void gMaterial::setMetalnessMap(gTexture* metalnessMap) {
	if(metalnessmapenabled && metalnessmapown) delete metalnessmap;
	metalnessmap = metalnessMap;
	if(metalnessmap->getWidth() > 0) {
		metalnessmapenabled = true;
		ispbr = true;
	}
	metalnessmapown = false;
}

gTexture* gMaterial::getMetalnessMap() {
	return metalnessmap;
}

void gMaterial::bindMetalnessMap(int slotNo) {
	metalnessmap->bind(slotNo);
}

void gMaterial::unbindMetalnessMap() {
	metalnessmap->unbind();
}

void gMaterial::setMetalnessMapEnabled(bool enableMetalnessMap) {
	metalnessmapenabled = enableMetalnessMap;
}

bool gMaterial::isMetalnessMapEnabled() {
	return metalnessmapenabled;
}

void gMaterial::loadPbrNormalMap(std::string texturePath) {
	if(pbrnormalmapenabled && pbrnormalmapown) delete pbrnormalmap;
	pbrnormalmap = new gTexture();
	pbrnormalmap->loadTexture(texturePath);
	if(pbrnormalmap->getWidth() > 0) pbrnormalmapenabled = true;
	pbrnormalmapown = true;
}

void gMaterial::setPbrNormalMap(gTexture* pbrNormalMap) {
	if(pbrnormalmapenabled && pbrnormalmapown) delete pbrnormalmap;
	pbrnormalmap = pbrNormalMap;
	if(pbrnormalmap->getWidth() > 0) pbrnormalmapenabled = true;
	pbrnormalmapown = false;
}

gTexture* gMaterial::getPbrNormalMap() {
	return pbrnormalmap;
}

void gMaterial::bindPbrNormalMap(int slotNo) {
	pbrnormalmap->bind(slotNo);
}

void gMaterial::unbindPbrNormalMap() {
	pbrnormalmap->unbind();
}

void gMaterial::setPbrNormalMapEnabled(bool enablePbrNormalMap) {
	pbrnormalmapenabled = enablePbrNormalMap;
}

bool gMaterial::isPbrNormalMapEnabled() {
	return pbrnormalmapenabled;
}

void gMaterial::loadAOMap(std::string texturePath) {
	if(aomapenabled && aomapown) delete aomap;
	aomap = new gTexture();
	aomap->loadTexture(texturePath);
	if(aomap->getWidth() > 0) aomapenabled = true;
	aomapown = true;
}

void gMaterial::setAOMap(gTexture* aoMap) {
	if(aomapenabled && aomapown) delete aomap;
	aomap = aoMap;
	if(aomap->getWidth() > 0) aomapenabled = true;
	aomapown = false;
}

gTexture* gMaterial::getAOMap() {
	return aomap;
}

void gMaterial::bindAOMap(int slotNo) {
	aomap->bind(slotNo);
}

void gMaterial::unbindAOMap() {
	aomap->unbind();
}

void gMaterial::setAOMapEnabled(bool enableAOMap) {
	aomapenabled = enableAOMap;
}

bool gMaterial::isAOMapEnabled() {
	return aomapenabled;
}

