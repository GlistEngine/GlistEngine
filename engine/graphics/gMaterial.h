/*
 * gMaterial.h
 *
 *  Created on: Oct 29, 2020
 *      Author: noyan
 */

#pragma once

#ifndef GRAPHICS_GMATERIAL_H_
#define GRAPHICS_GMATERIAL_H_

#include "gRenderObject.h"
#include "gTexture.h"
#include <array>


class gMaterial : public gRenderObject {
public:
	gMaterial();
	gMaterial(const gMaterial& other);
	gMaterial& operator=(const gMaterial& other);
	~gMaterial();

	bool isPBR() const;

	// Texture slot stuff
	void loadMap(gTexture::TextureType type, const std::string& texturePath);
	void setMap(gTexture::TextureType type, gTexture* texture);
	gTexture* getMap(gTexture::TextureType type) const;
	void bindMap(gTexture::TextureType type) const;
	void bindMap(gTexture::TextureType type, int slotNo) const;
	void unbindMap(gTexture::TextureType type) const;
	void setMapEnabled(gTexture::TextureType type, bool enabled);
	bool isMapEnabled(gTexture::TextureType type) const;

	// Color properties
	void setAmbientColor(int r, int g, int b, int a = 255);
	void setAmbientColor(float r, float g, float b, float a = 1.0f);
	void setAmbientColor(gColor* color);
	gColor* getAmbientColor();

	void setDiffuseColor(int r, int g, int b, int a = 255);
	void setDiffuseColor(float r, float g, float b, float a = 1.0f);
	void setDiffuseColor(gColor* color);
	gColor* getDiffuseColor();

	void setSpecularColor(int r, int g, int b, int a = 255);
	void setSpecularColor(float r, float g, float b, float a = 1.0f);
	void setSpecularColor(gColor* color);
	gColor* getSpecularColor();

	void setShininess(float shininess);
	float getShininess() const;

	// Backwards Compatability

	void loadDiffuseMap(const std::string& texturePath) { loadMap(gTexture::TEXTURETYPE_DIFFUSE, texturePath); }
	void setDiffuseMap(gTexture* diffuseMap) { setMap(gTexture::TEXTURETYPE_DIFFUSE, diffuseMap); }
	gTexture* getDiffuseMap() { return getMap(gTexture::TEXTURETYPE_DIFFUSE); }
	void bindDiffuseMap() const { bindMap(gTexture::TEXTURETYPE_DIFFUSE); }
	void unbindDiffuseMap() const { unbindMap(gTexture::TEXTURETYPE_DIFFUSE); }
	void setDiffuseMapEnabled(bool enabled) { setMapEnabled(gTexture::TEXTURETYPE_DIFFUSE, enabled); }
	bool isDiffuseMapEnabled() const { return isMapEnabled(gTexture::TEXTURETYPE_DIFFUSE); }

	void loadSpecularMap(const std::string& texturePath) { loadMap(gTexture::TEXTURETYPE_SPECULAR, texturePath); }
	void setSpecularMap(gTexture* specularMap) { setMap(gTexture::TEXTURETYPE_SPECULAR, specularMap); }
	gTexture* getSpecularMap() { return getMap(gTexture::TEXTURETYPE_SPECULAR); }
	void bindSpecularMap() const { bindMap(gTexture::TEXTURETYPE_SPECULAR); }
	void unbindSpecularMap() const { unbindMap(gTexture::TEXTURETYPE_SPECULAR); }
	void setSpecularMapEnabled(bool enabled) { setMapEnabled(gTexture::TEXTURETYPE_SPECULAR, enabled); }
	bool isSpecularMapEnabled() const { return isMapEnabled(gTexture::TEXTURETYPE_SPECULAR); }

	void loadNormalMap(const std::string& texturePath) { loadMap(gTexture::TEXTURETYPE_NORMAL, texturePath); }
	void setNormalMap(gTexture* normalMap) { setMap(gTexture::TEXTURETYPE_NORMAL, normalMap); }
	gTexture* getNormalMap() { return getMap(gTexture::TEXTURETYPE_NORMAL); }
	void bindNormalMap() const { bindMap(gTexture::TEXTURETYPE_NORMAL); }
	void unbindNormalMap() const { unbindMap(gTexture::TEXTURETYPE_NORMAL); }
	void setNormalMapEnabled(bool enabled) { setMapEnabled(gTexture::TEXTURETYPE_NORMAL, enabled); }
	bool isNormalMapEnabled() const { return isMapEnabled(gTexture::TEXTURETYPE_NORMAL); }

	void loadHeightMap(const std::string& texturePath) { loadMap(gTexture::TEXTURETYPE_HEIGHT, texturePath); }
	void setHeightMap(gTexture* heightMap) { setMap(gTexture::TEXTURETYPE_HEIGHT, heightMap); }
	gTexture* getHeightMap() { return getMap(gTexture::TEXTURETYPE_HEIGHT); }
	void bindHeightMap() const { bindMap(gTexture::TEXTURETYPE_HEIGHT); }
	void unbindHeightMap() const { unbindMap(gTexture::TEXTURETYPE_HEIGHT); }
	void setHeightMapEnabled(bool enabled) { setMapEnabled(gTexture::TEXTURETYPE_HEIGHT, enabled); }
	bool isHeightMapEnabled() const { return isMapEnabled(gTexture::TEXTURETYPE_HEIGHT); }

	void loadAlbedoMap(const std::string& texturePath) { loadMap(gTexture::TEXTURETYPE_PBR_ALBEDO, texturePath); }
	void setAlbedoMap(gTexture* albedoMap) { setMap(gTexture::TEXTURETYPE_PBR_ALBEDO, albedoMap); }
	gTexture* getAlbedoMap() { return getMap(gTexture::TEXTURETYPE_PBR_ALBEDO); }
	void bindAlbedoMap(int slotNo = 3) const { bindMap(gTexture::TEXTURETYPE_PBR_ALBEDO, slotNo); }
	void unbindAlbedoMap() const { unbindMap(gTexture::TEXTURETYPE_PBR_ALBEDO); }
	void setAlbedoMapEnabled(bool enabled) { setMapEnabled(gTexture::TEXTURETYPE_PBR_ALBEDO, enabled); }
	bool isAlbedoMapEnabled() const { return isMapEnabled(gTexture::TEXTURETYPE_PBR_ALBEDO); }

	void loadRoughnessMap(const std::string& texturePath) { loadMap(gTexture::TEXTURETYPE_PBR_ROUGHNESS, texturePath); }
	void setRoughnessMap(gTexture* roughnessMap) { setMap(gTexture::TEXTURETYPE_PBR_ROUGHNESS, roughnessMap); }
	gTexture* getRoughnessMap() { return getMap(gTexture::TEXTURETYPE_PBR_ROUGHNESS); }
	void bindRoughnessMap(int slotNo = 6) const { bindMap(gTexture::TEXTURETYPE_PBR_ROUGHNESS, slotNo); }
	void unbindRoughnessMap() const { unbindMap(gTexture::TEXTURETYPE_PBR_ROUGHNESS); }
	void setRoughnessMapEnabled(bool enabled) { setMapEnabled(gTexture::TEXTURETYPE_PBR_ROUGHNESS, enabled); }
	bool isRoughnessMapEnabled() const { return isMapEnabled(gTexture::TEXTURETYPE_PBR_ROUGHNESS); }

	void loadMetalnessMap(const std::string& texturePath) { loadMap(gTexture::TEXTURETYPE_PBR_METALNESS, texturePath); }
	void setMetalnessMap(gTexture* metalnessMap) { setMap(gTexture::TEXTURETYPE_PBR_METALNESS, metalnessMap); }
	gTexture* getMetalnessMap() { return getMap(gTexture::TEXTURETYPE_PBR_METALNESS); }
	void bindMetalnessMap(int slotNo = 5) const { bindMap(gTexture::TEXTURETYPE_PBR_METALNESS, slotNo); }
	void unbindMetalnessMap() const { unbindMap(gTexture::TEXTURETYPE_PBR_METALNESS); }
	void setMetalnessMapEnabled(bool enabled) { setMapEnabled(gTexture::TEXTURETYPE_PBR_METALNESS, enabled); }
	bool isMetalnessMapEnabled() const { return isMapEnabled(gTexture::TEXTURETYPE_PBR_METALNESS); }

	void loadPbrNormalMap(const std::string& texturePath) { loadMap(gTexture::TEXTURETYPE_PBR_NORMAL, texturePath); }
	void setPbrNormalMap(gTexture* pbrNormalMap) { setMap(gTexture::TEXTURETYPE_PBR_NORMAL, pbrNormalMap); }
	gTexture* getPbrNormalMap() { return getMap(gTexture::TEXTURETYPE_PBR_NORMAL); }
	void bindPbrNormalMap(int slotNo = 4) const { bindMap(gTexture::TEXTURETYPE_PBR_NORMAL, slotNo); }
	void unbindPbrNormalMap() const { unbindMap(gTexture::TEXTURETYPE_PBR_NORMAL); }
	void setPbrNormalMapEnabled(bool enabled) { setMapEnabled(gTexture::TEXTURETYPE_PBR_NORMAL, enabled); }
	bool isPbrNormalMapEnabled() const { return isMapEnabled(gTexture::TEXTURETYPE_PBR_NORMAL); }

	void loadAOMap(const std::string& texturePath) { loadMap(gTexture::TEXTURETYPE_PBR_AO, texturePath); }
	void setAOMap(gTexture* map) { setMap(gTexture::TEXTURETYPE_PBR_AO, map); }
	gTexture* getAOMap() { return getMap(gTexture::TEXTURETYPE_PBR_AO); }
	void bindAOMap(int slotNo = 7) const { bindMap(gTexture::TEXTURETYPE_PBR_AO, slotNo); }
	void unbindAOMap() const { unbindMap(gTexture::TEXTURETYPE_PBR_AO); }
	void setAOMapEnabled(bool enabled) { setMapEnabled(gTexture::TEXTURETYPE_PBR_AO, enabled); }
	bool isAOMapEnabled() const { return isMapEnabled(gTexture::TEXTURETYPE_PBR_AO); }

private:
	struct TextureSlot {
		gTexture* texture = nullptr;
		bool enabled = false;
		bool owned = false;
	};

	static bool isPBRType(gTexture::TextureType type);
	void cleanupSlot(TextureSlot& slot);

	std::array<TextureSlot, gTexture::TEXTURETYPE_COUNT> slots;
	bool ispbr;

    gColor ambient;
    gColor diffuse;
    gColor specular;
    float shininess;
};

#endif /* GRAPHICS_GMATERIAL_H_ */
