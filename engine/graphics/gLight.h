/*
 * gLight.h
 *
 *  Created on: Aug 28, 2020
 *      Author: noyan
 */

#ifndef ENGINE_GRAPHICS_GLIGHT_H_
#define ENGINE_GRAPHICS_GLIGHT_H_

#include "gNode.h"

class gLight : public gNode {
public:
	static const int LIGHTTYPE_AMBIENT = 0;
	static const int LIGHTTYPE_DIRECTIONAL = 1;
	static const int LIGHTTYPE_POINT = 2;
	static const int LIGHTTYPE_SPOT = 3;

	gLight(int lightType = LIGHTTYPE_POINT);
	virtual ~gLight();

	void enable();
	void disable();
	bool isEnabled();

	void setType(int lightType);
	int getType();

	void rotate(float angle, float ax, float ay, float az);
	glm::vec3 getDirection();

	void setAmbientColor(int r, int g, int b, int a = 255);
	void setAmbientColor(gColor* color);
	gColor* getAmbientColor();
	float getAmbientColorRed();
	float getAmbientColorGreen();
	float getAmbientColorBlue();
	float getAmbientColorAlpha();

	void setDiffuseColor(int r, int g, int b, int a = 255);
	void setDiffuseColor(gColor* color);
	gColor* getDiffuseColor();
	float getDiffuseColorRed();
	float getDiffuseColorGreen();
	float getDiffuseColorBlue();
	float getDiffuseColorAlpha();

	void setSpecularColor(int r, int g, int b, int a = 255);
	void setSpecularColor(gColor* color);
	gColor* getSpecularColor();
	float getSpecularColorRed();
	float getSpecularColorGreen();
	float getSpecularColorBlue();
	float getSpecularColorAlpha();

	void setAttenuation(float constant, float linear, float quadratic);
	void setAttenuation(glm::vec3 attenuation);
	glm::vec3 getAttenuation();
	float getAttenuationConstant();
	float getAttenuationLinear();
	float getAttenuationQuadratic();

	void setSpotCutOff(glm::vec2 spotCutOff);
	glm::vec2 getSpotCutOff();
	void setSpotCutOffAngle(float cutOffAngle);
	float getSpotCutOffAngle();
	float getSpotOuterCutOffAngle();
	void setSpotCutOffSpread(float cutOffSpreadAngle);
	float getSpotCutOffSpread();

private:
	int type;
	gColor ambientcolor, diffusecolor, specularcolor;
	bool isenabled;
	glm::vec3 attenuation;
	glm::vec2 spotcutoff;
	glm::vec3 directioneuler;
};

#endif /* ENGINE_GRAPHICS_GLIGHT_H_ */
