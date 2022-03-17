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


class gMaterial : public gRenderObject {
public:
	gMaterial();
	virtual ~gMaterial();

	bool isPBR() const;

	void setAmbientColor(int r, int g, int b, int a = 255);
	void setAmbientColor(float r, float g, float b, float a = 1.0f);
	void setAmbientColor(gColor* color);
	const gColor& getAmbientColor() const;

	void setDiffuseColor(int r, int g, int b, int a = 255);
	void setDiffuseColor(float r, float g, float b, float a = 1.0f);
	void setDiffuseColor(gColor* color);
	const gColor& getDiffuseColor() const;

	void setSpecularColor(int r, int g, int b, int a = 255);
	void setSpecularColor(float r, float g, float b, float a = 1.0f);
	void setSpecularColor(gColor* color);
	const gColor& getSpecularColor() const;

	void setShininess(float shininess);
	float getShininess() const;

	void loadDiffuseMap(const std::string& texturePath);
	void setDiffuseMap(gTexture* diffuseMap);
	const gTexture* getDiffuseMap() const;
	void bindDiffuseMap() const;
	void unbindDiffuseMap() const;
	void setDiffuseMapEnabled(bool enableDiffuseMap);
	bool isDiffuseMapEnabled() const;

	void loadSpecularMap(const std::string& texturePath);
	void setSpecularMap(gTexture* specularMap);
	const gTexture* getSpecularMap() const;
	void bindSpecularMap() const;
	void unbindSpecularMap() const;
	void setSpecularMapEnabled(bool enableSpecularMap);
	bool isSpecularMapEnabled() const;

	void loadNormalMap(const std::string& texturePath);
	void setNormalMap(gTexture* normalMap);
	const gTexture* getNormalMap() const;
	void bindNormalMap() const;
	void unbindNormalMap() const;
	void setNormalMapEnabled(bool enableNormalMap);
	bool isNormalMapEnabled() const;

	void loadHeightMap(const std::string& texturePath);
	void setHeightMap(gTexture* heightMap);
	const gTexture* getHeightMap() const;
	void bindHeightMap() const;
	void unbindHeightMap() const;
	void setHeightMapEnabled(bool enableHeightMap);
	bool isHeightMapEnabled() const;

	void loadAlbedoMap(const std::string& texturePath);
	void setAlbedoMap(gTexture* albedoMap);
	const gTexture* getAlbedoMap() const;
	void bindAlbedoMap(int slotNo = 3) const;
	void unbindAlbedoMap() const;
	void setAlbedoMapEnabled(bool enableNormalMap);
	bool isAlbedoMapEnabled() const;

	void loadRoughnessMap(const std::string& texturePath);
	void setRoughnessMap(gTexture* roughnessMap);
	const gTexture* getRoughnessMap() const;
	void bindRoughnessMap(int slotNo = 6) const;
	void unbindRoughnessMap() const;
	void setRoughnessMapEnabled(bool enableRoughnessMap);
	bool isRoughnessMapEnabled() const;

	void loadMetalnessMap(const std::string& texturePath);
	void setMetalnessMap(gTexture* metalnessMap);
	const gTexture* getMetalnessMap() const;
	void bindMetalnessMap(int slotNo = 5);
	void unbindMetalnessMap() const;
	void setMetalnessMapEnabled(bool enableMetalnessMap);
	bool isMetalnessMapEnabled() const;

	void loadPbrNormalMap(const std::string& texturePath);
	void setPbrNormalMap(gTexture* pbrNormalMap);
	const gTexture* getPbrNormalMap() const;
	void bindPbrNormalMap(int slotNo = 4) const;
	void unbindPbrNormalMap() const;
	void setPbrNormalMapEnabled(bool enablePbrNormalMap);
	bool isPbrNormalMapEnabled() const;

	void loadAOMap(const std::string& texturePath);
	void setAOMap(gTexture* aoMap);
	const gTexture* getAOMap() const;
	void bindAOMap(int slotNo = 7) const;
	void unbindAOMap() const;
	void setAOMapEnabled(bool enableAOMap);
	bool isAOMapEnabled() const;

private:
    gColor ambient;
    gColor diffuse;
    gColor specular;
    float shininess;
    gTexture* diffusemap, *normalmap, *specularmap, *heightmap;
    bool diffusemapenabled, specularmapenabled, normalmapenabled, heightmapenabled;
    bool diffusemapown, normalmapown, specularmapown, heightmapown;
    bool ispbr;
    gTexture* albedomap, *roughnessmap, *metalnessmap, *pbrnormalmap, *aomap;
    bool albedomapenabled, roughnessmapenabled, metalnessmapenabled, pbrnormalmapenabled, aomapenabled;
    bool albedomapown, roughnessmapown, metalnessmapown, pbrnormalmapown, aomapown;
};

#endif /* GRAPHICS_GMATERIAL_H_ */
