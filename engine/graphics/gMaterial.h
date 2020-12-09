/*
 * gMaterial.h
 *
 *  Created on: Oct 29, 2020
 *      Author: noyan
 */

#ifndef GRAPHICS_GMATERIAL_H_
#define GRAPHICS_GMATERIAL_H_

#include "gRenderObject.h"
#include "gTexture.h"


class gMaterial : public gRenderObject {
public:
	gMaterial();
	virtual ~gMaterial();

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

	void setDiffuseMap(gTexture* diffuseMap);
	gTexture* getDiffuseMap();
	void bindDiffuseMap();
	void unbindDiffuseMap();
	void setDiffuseMapEnabled(bool enableDiffuseMap);
	bool isDiffuseMapEnabled();

	void setSpecularMap(gTexture* specularMap);
	gTexture* getSpecularMap();
	void bindSpecularMap();
	void unbindSpecularMap();
	void setSpecularMapEnabled(bool enableSpecularMap);
	bool isSpecularMapEnabled();

	void setNormalMap(gTexture* normalMap);
	gTexture* getNormalMap();
	void bindNormalMap();
	void unbindNormalMap();
	void setNormalMapEnabled(bool enableNormalMap);
	bool isNormalMapEnabled();

private:
    gColor ambient;
    gColor diffuse;
    gColor specular;
    float shininess;
    gTexture diffusemap, specularmap, normalmap;
    bool diffusemapenabled, specularmapenabled, normalmapenabled;
};

#endif /* GRAPHICS_GMATERIAL_H_ */
