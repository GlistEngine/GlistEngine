/*
 * gCircleSlice.h
 *
 *  Created on: 28 Tem 2022
 *      Author: buraks
 */

#ifndef GRAPHICS_PRIMITIVES_GCIRCLESLICE_H_
#define GRAPHICS_PRIMITIVES_GCIRCLESLICE_H_

#include "gMesh.h"

class gCircleSlice: public gMesh {
public:
	gCircleSlice();
	virtual ~gCircleSlice();

	void draw(float xCenter, float yCenter, float radius, bool isFilled, float numberOfSides = 64.0f, int degree = 360);

private:
	void setCirclePoints(float xCenter, float yCenter, float radius, bool isFilled, float numberOfSides = 64.0f, int degree = 360);
};

#endif /* GRAPHICS_PRIMITIVES_GCIRCLESLICE_H_ */
