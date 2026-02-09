/*
 * gMaterial.cpp
 *
 *  Created on: Oct 29, 2020
 *      Author: noyan
 */

#include "gMaterial.h"

bool gMaterial::isPBRType(gTexture::TextureType type) {
	return type == gTexture::TEXTURETYPE_PBR_ALBEDO
		|| type == gTexture::TEXTURETYPE_PBR_ROUGHNESS
		|| type == gTexture::TEXTURETYPE_PBR_METALNESS;
}

void gMaterial::cleanupSlot(TextureSlot& slot) {
	if(slot.enabled && slot.owned) {
		delete slot.texture;
	}
	slot.texture = nullptr;
	slot.enabled = false;
	slot.owned = false;
}

gMaterial::gMaterial()
	: ispbr(false), shininess(0.5f) {
}

gMaterial::gMaterial(const gMaterial& other)
	: ispbr(other.ispbr), ambient(other.ambient), diffuse(other.diffuse),
	  specular(other.specular), shininess(other.shininess), slots(other.slots) {
	for(auto& slot : slots) {
		slot.owned = false;
	}
}

gMaterial& gMaterial::operator=(const gMaterial& other) {
	if(this != &other) {
		for(auto& slot : slots) {
			cleanupSlot(slot);
		}
		ambient = other.ambient;
		diffuse = other.diffuse;
		specular = other.specular;
		shininess = other.shininess;
		ispbr = other.ispbr;
		slots = other.slots;
		for(auto& slot : slots) {
			slot.owned = false;
		}
	}
	return *this;
}

gMaterial::~gMaterial() {
	for(auto& slot : slots) {
		cleanupSlot(slot);
	}
}

bool gMaterial::isPBR() const {
	return ispbr;
}

void gMaterial::loadMap(gTexture::TextureType type, const std::string& texturePath) {
	TextureSlot& slot = slots[type];
	cleanupSlot(slot);
	slot.texture = new gTexture();
	slot.texture->loadTexture(texturePath);
	if(slot.texture->getWidth() > 0) {
		slot.enabled = true;
		if(isPBRType(type)) ispbr = true;
	}
	slot.owned = true;
}

void gMaterial::setMap(gTexture::TextureType type, gTexture* texture) {
	TextureSlot& slot = slots[type];
	cleanupSlot(slot);
	slot.texture = texture;
	if(texture && texture->getWidth() > 0) {
		slot.enabled = true;
		if(isPBRType(type)) ispbr = true;
	}
	slot.owned = false;
}

gTexture* gMaterial::getMap(gTexture::TextureType type) const {
	return slots[type].texture;
}

void gMaterial::bindMap(gTexture::TextureType type) const {
	const TextureSlot& slot = slots[type];
	if(slot.enabled && slot.texture) {
		slot.texture->bind();
	}
}

void gMaterial::bindMap(gTexture::TextureType type, int slotNo) const {
	const TextureSlot& slot = slots[type];
	if(slot.enabled && slot.texture) {
		slot.texture->bind(slotNo);
	}
}

void gMaterial::unbindMap(gTexture::TextureType type) const {
	const TextureSlot& slot = slots[type];
	if(slot.texture) {
		slot.texture->unbind();
	}
}

void gMaterial::setMapEnabled(gTexture::TextureType type, bool enabled) {
	slots[type].enabled = enabled;
}

bool gMaterial::isMapEnabled(gTexture::TextureType type) const {
	return slots[type].enabled;
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

float gMaterial::getShininess() const {
	return shininess;
}
