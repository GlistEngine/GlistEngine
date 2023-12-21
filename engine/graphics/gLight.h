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
 * Glist Engine has four light types: Ambient, Directional, Point, and Spot.
 * Ambient lights have a small amount of light.
 * Directional lights are sun like light.
 * Point lights are bulb like light.
 * Spot lights emit light from a single point.
 * If light type is not changed, light type will be point light because of
 * default constructor.
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
	 * This function should be using after camera.begin() function.
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
	bool isEnabled() const;

	/**
	 *
	 * @return true if light data is changed and not uploaded to the GPU yet, false if not
	 */
	bool isChanged() const;

	void setChanged(bool isChanged);

	/**
	 * Changing type of light using id
	 *
	 * @param lightType light id no
	 */
	void setType(int lightType);

	/**
 	 * Gives using type of light id
 	 *
	 * @return light type id no
	 */
	int getType() const;

	/**
	 * Rotates light objects around the given vector
	 *
	 * The vector should be a unit vector eg.((1, 1, 1) or (1, 1, 0)...).
	 * Because the function multiplies the x, y, z values by the angle.
	 * For instance if we write "rotate(30, 0, 0, 3)", the object returns 90
	 * degrees around the z axis. All these x, y, z axis are relative to the
	 * object's pivot point.
	 *
	 * @param angle the amount of rotation
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
	 * Gives light direction x, y, z coordinate.
	 * For example, can be using function.x, function.y, function.z.
	 * This typing showing light x, y, z direction. If this function
	 * using more than 1, copy constructor called.
	 *
	 * @return Light direction with three dimensional floating point vector
	 */
	const glm::vec3& getDirection() const;

	/**
	 * Ambient color changing with RGBA.
	 * The range of the value [0,255]
	 *
	 * @param r ambient color red component.
	 * @param g ambient color green component.
	 * @param b ambient color blue component.
	 * @param a ambient color alpha component.
	 */
	void setAmbientColor(int r, int g, int b, int a = 255);

	/**
	 * Ambient color changing with gColor object.
	 *
	 * @param color gColor object to use changing color
	 */
	void setAmbientColor(gColor* color);

	/**
	 * Gives ambient color RGBA component.
	 * This function can be use two type. Using directly giving memory address.
	 * Other way, Typing like function.r, function.g, function.b, function.a
	 * giving value of color component.
	 *
	 * @return color object memory address or RGBA value
	 */
	gColor* getAmbientColor();

	/**
	 * Gives red component value range is [0,1].
	 *
	 * @return ambient color red component.
	 */
	float getAmbientColorRed() const;

	/**
	 * Gives green component value range is [0,1].
	 *
	 * @return ambient color green component.
	 */
	float getAmbientColorGreen() const;

	/**
	 * Gives blue component value range is [0,1].
	 *
	 * @return ambient color blue component.
	 */
	float getAmbientColorBlue() const;

	/**
	 * Gives alpha component value range is [0,1].
	 *
	 * @return ambient color alpha component.
	 */
	float getAmbientColorAlpha() const;

	/**
	 * Diffuse color changing with RGBA.
	 * The range of the value [0,255].
	 *
	 * @param r diffuse color red component.
	 * @param g diffuse color green component.
	 * @param b diffuse color blue component.
	 * @param a diffuse color alpha component.
	 */
	void setDiffuseColor(int r, int g, int b, int a = 255);

	/**
	 * Diffuse color changing with gColor object.
	 *
	 * @param color gColor object to use changing color
	 */
	void setDiffuseColor(gColor* color);

	/**
	 * Gives diffuse color RGBA value.
	 *
	 * This function can be use two type. Using directly giving memory address.
	 * Other way, Typing like function.r, function.g, function.b, function.a
	 * giving value of color component.
	 *
	 * @return color object memory address or RGBA value
	 */
	gColor* getDiffuseColor();

	/**
	 * Gives red component value range is [0,1].
	 *
	 * @return diffuse color red component.
	 */
	float getDiffuseColorRed() const;

	/**
	 * Gives green component value range is [0,1].
	 *
	 * @return diffuse color green component.
	 */
	float getDiffuseColorGreen() const;

	/**
	 * Gives blue component value the range is
	 *
	 * @return diffuse color blue component.
	 */
	float getDiffuseColorBlue() const;

	/**
	 * Gives alpha component value range is [0,1].
	 *
	 * @return diffuse color alpha component.
	 */
	float getDiffuseColorAlpha() const;

	/**
	 * Specular color changing with RGBA.
	 * The range of the value [0,255]
	 *
	 * @param r specular color red component.
	 * @param g specular color green component.
	 * @param b specular color blue component.
	 * @param a specular color alpha component.
	 */
	void setSpecularColor(int r, int g, int b, int a = 255);

	/**
	 * Specular color changing with gColor object.
	 *
	 * @param color gColor object to use changing color
	 */
	void setSpecularColor(gColor* color);

	/**
	 * Gives specular color RGBA value.
	 *
	 * This function can be use two type. Using directly giving memory address.
	 * Other way, Typing like function.r, function.g, function.b, function.a
	 * giving value of color component.
	 *
	 * @return color object memory address or RGBA value
	 */
	gColor* getSpecularColor();

	/**
	 * Gives red component value range is [0,1].
	 *
	 * @return specular color red component.
	 */
	float getSpecularColorRed() const;

	/**
	 * Gives green component value range is [0,1].
	 *
	 * @return specular color green component.
	 */
	float getSpecularColorGreen() const;

	/**
	 * Gives blue component value range is [0,1].
	 *
	 * @return specular color blue component.
	 */
	float getSpecularColorBlue() const;

	/**
	 * Gives alpha component value range is [0,1].
	 *
	 * @return specular color alpha component.
	 */
	float getSpecularColorAlpha() const;

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
	 * For example, can be using function.x, function.y, function.z.
	 * This typing showing light constant, linear, quadratic values.
	 * If this function using more than 1, copy constructor called.
	 *
	 * @return attenuation with three dimensional floating point vector
	 */
	const glm::vec3& getAttenuation() const;

	/**
	 * @return light attenuation constant value
	 */
	float getAttenuationConstant() const;

	/**
	 * @return light attenuation linear value
	 */
	float getAttenuationLinear() const;

	/**
	 * @return light attenuation quadratic value
	 */
	float getAttenuationQuadratic() const;

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
	 * For example, can be using function.x and function.y. This typing showing
	 * light spot cutoff angle. If this function using more than 1,
	 * copy constructor called.
	 *
	 * @return spot cut off angles
	 */
	const glm::vec2& getSpotCutOff() const;

	/**
	 * @param cutOffAngle changing minimum angle
	 */
	void setSpotCutOffAngle(float cutOffAngle);

	/**
	 * @return spot angle between minimum angle
	 */
	float getSpotCutOffAngle() const;

	/**
	 * @return spot angle between minimum angle and maximum angle
	 */
	float getSpotOuterCutOffAngle() const;

	/**
	 * @param cutOffSpreadAngle changing maximum angle
	 */
	void setSpotCutOffSpread(float cutOffSpreadAngle);

	/**
	 * @return spot angle between maximum angle
	 */
	float getSpotCutOffSpread() const;
protected:

	void processTransformationMatrix() override;

private:
	int type;
	gColor ambientcolor, diffusecolor, specularcolor;
	bool isenabled;
	bool ischanged;
	glm::vec3 attenuation;
	glm::vec2 spotcutoff;
	glm::vec3 directioneuler;
};

#endif /* ENGINE_GRAPHICS_GLIGHT_H_ */
