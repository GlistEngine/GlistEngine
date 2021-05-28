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

	bool isPBR();

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
	float getShininess();

	void loadDiffuseMap(std::string texturePath);
	void setDiffuseMap(gTexture* diffuseMap);
	gTexture* getDiffuseMap();
	void bindDiffuseMap();
	void unbindDiffuseMap();
	void setDiffuseMapEnabled(bool enableDiffuseMap);
	bool isDiffuseMapEnabled();

	void loadSpecularMap(std::string texturePath);
	void setSpecularMap(gTexture* specularMap);
	gTexture* getSpecularMap();
	void bindSpecularMap();
	void unbindSpecularMap();
	void setSpecularMapEnabled(bool enableSpecularMap);
	bool isSpecularMapEnabled();

	void loadNormalMap(std::string texturePath);
	void setNormalMap(gTexture* normalMap);
	gTexture* getNormalMap();
	void bindNormalMap();
	void unbindNormalMap();
	void setNormalMapEnabled(bool enableNormalMap);
	bool isNormalMapEnabled();

	void loadHeightMap(std::string texturePath);
	void setHeightMap(gTexture* heightMap);
	gTexture* getHeightMap();
	void bindHeightMap();
	void unbindHeightMap();
	void setHeightMapEnabled(bool enableHeightMap);
	bool isHeightMapEnabled();

	void loadAlbedoMap(std::string texturePath);
	void setAlbedoMap(gTexture* albedoMap);
	gTexture* getAlbedoMap();
	void bindAlbedoMap(int slotNo = 3);
	void unbindAlbedoMap();
	void setAlbedoMapEnabled(bool enableNormalMap);
	bool isAlbedoMapEnabled();

	void loadRoughnessMap(std::string texturePath);
	void setRoughnessMap(gTexture* roughnessMap);
	gTexture* getRoughnessMap();
	void bindRoughnessMap(int slotNo = 6);
	void unbindRoughnessMap();
	void setRoughnessMapEnabled(bool enableRoughnessMap);
	bool isRoughnessMapEnabled();

	void loadMetalnessMap(std::string texturePath);
	void setMetalnessMap(gTexture* metalnessMap);
	gTexture* getMetalnessMap();
	void bindMetalnessMap(int slotNo = 5);
	void unbindMetalnessMap();
	void setMetalnessMapEnabled(bool enableMetalnessMap);
	bool isMetalnessMapEnabled();

	void loadPbrNormalMap(std::string texturePath);
	void setPbrNormalMap(gTexture* pbrNormalMap);
	gTexture* getPbrNormalMap();
	void bindPbrNormalMap(int slotNo = 4);
	void unbindPbrNormalMap();
	void setPbrNormalMapEnabled(bool enablePbrNormalMap);
	bool isPbrNormalMapEnabled();

	void loadAOMap(std::string texturePath);
	void setAOMap(gTexture* aoMap);
	gTexture* getAOMap();
	void bindAOMap(int slotNo = 7);
	void unbindAOMap();
	void setAOMapEnabled(bool enableAOMap);
	bool isAOMapEnabled();

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
