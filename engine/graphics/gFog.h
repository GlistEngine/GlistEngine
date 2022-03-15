/*
 * gFog.h
 *
 *  Created on: 15 Aðu 2021
 *      Author: furka
 */

#ifndef GRAPHICS_GFOG_H_
#define GRAPHICS_GFOG_H_


#include <gRenderObject.h>
#include <gColor.h>

// This is a class that allows creating fog

class gFog : public gRenderObject{
public:
	gFog();

	/* Activates the fog.This function creates a fog by giving objects a visibility index and
	 * multiplying this visibility index by the fog color thanks to these two formula
	 * visibility = exp(-pow((distance * fogdensity), foggradient))
	 * objectColor = mix(vec4(fogColor, 1.0), FragColor, visibility)
	 */
	void enable();

	// Deactivates the fog.
	void disable();

	/* Sets fogColor.
	 * @param r = red value of fogColor.
	 * @param g = green value of fogColor.
	 * @param b = blue value of fogColor.
	*/
	void setColor(float r, float g, float b);

	/* Sets density of fog
	 * @param d = density value.
	 */
	void setDensity(float d);

	/* Sets the gradient curve that distributes the fog in the field.
	 * @param g = Gradient value.
	 */
	void setGradient(float g);

	//Returns fogColor.
	const gColor& getColor() const;

	//Returns density value.
	float getDensity() const;

	//Returns fog value.
	float getGradient() const;

private:

};


#endif /* GRAPHICS_GFOG_H_ */
