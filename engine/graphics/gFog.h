/*
 * gFog.h
 *
 *  Created on: Aug 15, 2021
 *      Author: furka
 */

#ifndef GRAPHICS_GFOG_H_
#define GRAPHICS_GFOG_H_


#include <gRenderObject.h>
#include <gColor.h>

/**
 * Represents a fog object that can be enabled and configured.
 * Inherits from gRenderObject.
*/
class gFog : public gRenderObject {
public:
	static int fognum;

	gFog();

	/**
	 * Sets the current fog as active. You can have multiple fog instances with different properties.
	 */
	void enable();

	/**
	 * Deactivates the fog
	 */
	void disable();

	/**
	 * Sets the color of the fog.
	 *
	 * @param r = red value of fogColor.
	 * @param g = green value of fogColor.
	 * @param b = blue value of fogColor.
	 */
	void setColor(float r, float g, float b);

	/**
	 * Sets mode of the fog
	 * @param value New fog mode. Possible values are:
	 * - gRenderer::FOGMODE_LINEAR -> Linear Fog (only linear start/end affects visuals)
	 * - gRenderer::FOGMODE_EXP -> Exponential Fog (only density and gradient affects visuals)
	 */
	void setMode(int value);

	/**
 	 * Sets density of the fog, increasing this will make it appear closer to the camera.
 	 * @param value Density value.
	 */
	void setDensity(float value);

	/**
	 * Sets the rate at which the fog density increases. A higher value will cause the fog to become denser more quickly.
	 *
	 * @param rate The rate of fog density increase. Default is 2.0
	 */
	void setGradient(float value);

	/**
	 * Sets the start value of the linear fog.
	 * @param value Start value of the linear mode.
	 */
	void setLinearStart(float value);

	/**
	 * Sets the end value of the linear fog.
	 * @param value End value of the linear mode.
	 */
	void setLinearEnd(float value);

	/**
	 * Gets the fog color.
	 * @return The fog color.
	 */
	const gColor& getColor() const;

	/**
	 * Gets the fog mode.
	 * @return The fog mode. Possible values are:
	 * - gRenderer::FOGMODE_LINEAR -> Linear Fog (only linear start/end affects visuals)
	 * - gRenderer::FOGMODE_EXP -> Exponential Fog (only density and gradient affects visuals)
	 */
	int getMode() const;

	/**
	 * Gets the fog density.
	 * @return The fog density.
	 */
	float getDensity() const;

	/**
	 * Gets the fog gradient.
	 * @return The fog gradient.
	 */
	float getGradient() const;

	/**
	 * Gets the start value of the linear mode.
	 * @return The start value of the linear mode. Default is 0.0.
	 */
	float getLinearStart() const;

	/**
	 * Gets the end value of the linear mode.
	 * @return The end value of the linear mode. Default is 1.0.
	 */
	float getLinearEnd() const;

private:
	int fogno;
	gColor color;
	float density;
	float gradient;
	int mode;
	float linearstart, linearend;
};


#endif /* GRAPHICS_GFOG_H_ */
