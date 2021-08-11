/*
 * gLight.h
 *
 *  Created on: Aug 28, 2020
 *      Author: noyan
 */

#ifndef ENGINE_GRAPHICS_GLIGHT_H_
#define ENGINE_GRAPHICS_GLIGHT_H_

#include "gNode.h"

/**
 * This class for the using light. Adding and removing light to the scene.
 * If light type is not setting, light type will be point light because of
 * default constructor. For example, changing ambientcolor, diffusecolor,
 * specularcolor. And many more, changing the light intensity.
 */
class gLight : public gNode {
public:
	static const int LIGHTTYPE_AMBIENT = 0;
	static const int LIGHTTYPE_DIRECTIONAL = 1;
	static const int LIGHTTYPE_POINT = 2;
	static const int LIGHTTYPE_SPOT = 3;

	gLight(int lightType = LIGHTTYPE_POINT);
	virtual ~gLight();

	/**
	 * Adding the light to the scene.
	 * This function should be using after camera.begin() funciton.
	 */
	void enable();

	/**
	 * Removing the light to the scene.
	 * This function should be using before camera.end() function.
	 */
	void disable();

	/**
	 * @return true if light is enable, false if not
	 */
	bool isEnabled();

	/**
	 * Changing type of light using id
	 *
	 * ambient light id number 0
	 * directional light id number 1
	 * point light id number 2
	 * spot light id number 3
	 *
	 * @param lightType setting light type id number
	 */
	void setType(int lightType);

	/**
 	 * Gives using which type of light id
 	 *
	 * ambient light id number 0
	 * directional light id number 1
	 * point light id number 2
	 * spot light id number 3
	 *
	 * @return light type id number
	 */
	int getType();

	/**
	 * Rotates light objects around the given vector
	 *
	 * The vector should be a unit vector eg.((1, 1, 1) or (1, 1, 0)...).
	 * Because the function multiplies the x, y, z values by the angle.
	 * For instance if we write "rotate(30, 0, 0, 3)", the object returns 90
	 * degrees around the z axis. All these x, y, z axis are relative to the
	 * object's pivot point.
	 *
	 * @param angle value of the angle for the rotate
	 *
	 * @param ax is the x value of the vector whose origin is the object's
	 * pivot point and rotate the object around.
	 *
	 * @param ay is the y value of the vector whose origin is the object's
	 * pivot point and rotate the object around.
     *
	 * @param az is the z value of the vector whose origin is the object's
	 * pivot point and rotate the object around.
	 */
	void rotate(float angle, float ax, float ay, float az);

	/**
	 * This function should be use in gLogi. For example, can be using
	 * function.x, function.y, function.z. This typing showing light x, y, z
	 * direction. If this function using more than 1, copy constructor called.
	 * Copy constructor that copies the existing object into the new object one
	 * byte at a time.
	 *
	 * @return Light direction with three dimensional floating point vector
	 */
	glm::vec3 getDirection();

	/**
	 * Ambient color changing with RGBA.
	 *
	 * @param r ambient color in red value. The range of the value [0,255]
	 * @param g ambient color in green value. The range of the value [0,255]
	 * @param b ambient color in blue value. The range of the value [0,255]
	 * @param a ambient color in alpha value. The range of the value [0,255]
	 */
	void setAmbientColor(int r, int g, int b, int a = 255);

	/**
	 * First, create gColor object. First, set color value and use parameter.
	 *
	 * @param color gColor object to use changing color
	 */
	void setAmbientColor(gColor* color);

	/**
	 * This function can be use two type. Using directly giving memory address.
	 * Other way, Typing like function.r, function.g, function.b, function.a
	 * giving value of color.
	 *
	 * @return color object memory address or RGBA value
	 */
	gColor* getAmbientColor();

	/**
	 * @return In light value of red in the ambient color. The range of the value [0,1]
	 */
	float getAmbientColorRed();

	/**
	 * @return In light value of green in the ambient color. The range of the value [0,1]
	 */
	float getAmbientColorGreen();

	/**
	 * @return In light value of blue in the ambient color. The range of the value [0,1]
	 */
	float getAmbientColorBlue();

	/**
	 * @return In light value of alpha in the ambient color. The range of the value [0,1]
	 */
	float getAmbientColorAlpha();

	/**
	 * Diffuse color changing with RGBA.
	 *
	 * @param r diffuse color in red value. The range of the value [0,255]
	 * @param g diffuse color in green value. The range of the value [0,255]
	 * @param b diffuse color in blue value. The range of the value [0,255]
	 * @param a diffuse color in alpha value. The range of the value [0,255]
	 */
	void setDiffuseColor(int r, int g, int b, int a = 255);

	/**
	 * First, create gColor object. Then, set color value and use parameter.
	 *
	 * @param color gColor object to use changing color
	 */
	void setDiffuseColor(gColor* color);

	/**
	 * Gives diffuse color RGBA value
	 *
	 * This function can be use two type. Using directly giving memory address.
	 * Other way, Typing like function.r, function.g, function.b, function.a
	 * giving value of color.
	 *
	 * @return color object memory address or RGBA value
	 */
	gColor* getDiffuseColor();

	/**
	 * @return In light value of red in the diffuse color. The range of the value [0,1]
	 */
	float getDiffuseColorRed();

	/**
	 * @return In light value of green in the diffuse color. The range of the value [0,1]
	 */
	float getDiffuseColorGreen();

	/**
	 * @return In light value of blue in the diffuse color. The range of the value [0,1]
	 */
	float getDiffuseColorBlue();

	/**
	 * @return In light value of alpha in the diffuse color. The range of the value [0,1]
	 */
	float getDiffuseColorAlpha();

	/**
	 * Specular color changing with RGBA.
	 *
	 * @param r specular color in red value. The range of the value [0,255]
	 * @param g specular color in green value. The range of the value [0,255]
	 * @param b specular color in blue value. The range of the value [0,255]
	 * @param a specular color in alpha value. The range of the value [0,255]
	 */
	void setSpecularColor(int r, int g, int b, int a = 255);

	/**
	 * First, create gColor object. Then, set color value and use parameter.
	 *
	 * @param color gColor object to use changing color
	 */
	void setSpecularColor(gColor* color);

	/**
	 * Gives specular color RGBA value
	 *
	 * This function can be use two type. Using directly giving memory address.
	 * Other way, Typing like function.r, function.g, function.b, function.a
	 * giving value of color.
	 *
	 * @return color object memory address or RGBA value
	 */
	gColor* getSpecularColor();

	/**
	 * @return In light value of red in the specular color. The range of the value [0,1]
	 */
	float getSpecularColorRed();

	/**
	 * @return In light value of green in the specular color. The range of the value [0,1]
	 */
	float getSpecularColorGreen();

	/**
	 * @return In light value of blue in the specular color. The range of the value [0,1]
	 */
	float getSpecularColorBlue();

	/**
	 * @return In light value of alpha in the specular color. The range of the value [0,1]
	 */
	float getSpecularColorAlpha();

	/**
	 * When blending between these three types of attenuation, set the
	 * proportions of each attenuation type against the other two types.
	 * To avoid confusion, a proportion of 3:6:1 gives the exact same,
	 * which can easily be translated to 30%, 60% and 10%.
	 *
	 * @param constant The intensity, represented by the brigtness setting
	 * of the light entity, will be totally unaffected by distance, continuing
	 * until it hits a surface, theoretically capable of illuminating
	 * an infinite area.
	 *
	 * @param linear Light intensity (set by its brightness) will diminish
	 * (at a fixed rate) as it travels from its source.
	 *
	 * @param quadratic Mathematically, the attenuation of a 100%
	 * quadratic light is exponential (quadratic), expressed as
	 * "I = 1/d^2", meaning that the further the light travels
	 * from its source, the more it will be diminished.
	 *
	 */
	void setAttenuation(float constant, float linear, float quadratic);

	/**
	 * Alternative version to blending between these three types of attenuation,
	 * using three dimensional of the vector type.
	 *
	 * @param attenuation three dimensional floating point vector
	 */
	void setAttenuation(glm::vec3 attenuation);

	/**
	 * This function should be use in gLogi. For example, can be using
	 * function.x, function.y, function.z. This typing showing light
	 * constant, linear, quadratic values. If this function using more than 1,
	 * copy constructor called. Copy constructor that copies the existing
	 * object into the new object one byte at a time.
	 *
	 * @return attenuation with three dimensional floating point vector
	 */
	glm::vec3 getAttenuation();

	/**
	 * @return light attenuation constant value
	 */
	float getAttenuationConstant();

	/**
	 * @return light attenuation linear value
	 */
	float getAttenuationLinear();

	/**
	 * @return light attenuation quadratic value
	 */
	float getAttenuationQuadratic();

	/**
	 * Value that specifies the maximum spread angle of a light source.
	 * Only values in the range 0 90 and the special value 180 are accepted.
	 * If the angle between the direction of the light and the direction from
	 * the light to the vertex being lighted is greater than the spot cutoff
	 * angle, the light is completely masked. Otherwise, its intensity is
	 * controlled by the spot exponent and the attenuation factors. The initial
	 * spot cutoff is 180, resulting in uniform light distribution.
	 *
	 * @param spotCutOff Two dimensional of the vector type
	 */
	void setSpotCutOff(glm::vec2 spotCutOff);

	/**
	 * This function should be use in gLogi. For example, can be using
	 * function.x and function.y. This typing showing light spot cutoff angle.
	 * If this function using more than 1, copy constructor called.
	 * Copy constructor that copies the existing object into the new object one
	 * byte at a time.
	 *
	 * @return spot cut off angles
	 */
	glm::vec2 getSpotCutOff();

	/**
	 * @param cutOffAngle changing minimum angle
	 */
	void setSpotCutOffAngle(float cutOffAngle);

	/**
	 * @return spot angle between minimum angle
	 */
	float getSpotCutOffAngle();

	/**
	 * @return spot angle between minimum angle and maximum angle
	 */
	float getSpotOuterCutOffAngle();

	/**
	 * @param cutOffSpreadAngle changing maximum angle
	 */
	void setSpotCutOffSpread(float cutOffSpreadAngle);

	/**
	 * @return spot angle between maximum angle
	 */
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
