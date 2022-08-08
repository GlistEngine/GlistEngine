/*
 * gArc.h
 *
 *  Created on: 27 Tem 2022
 *      Author: burakmeydan
 */

#ifndef GRAPHICS_PRIMITIVES_gArc_H_
#define GRAPHICS_PRIMITIVES_gArc_H_

#include "gMesh.h"

class gArc: public gMesh {
public:
	gArc();
	virtual ~gArc();

	void draw(float xCenter, float yCenter, float radius, bool isFilled, int numberOfSides = 60, float degree = 360, float rotate = 360);

private:
	void setArcPoints(float xCenter, float yCenter, float radius, bool isFilled, int numberOfSides = 60, float degree = 360, float rotate = 360);
	std::vector<gVertex> vertices;
	std::vector<unsigned int> indices;
};

#endif /* GRAPHICS_PRIMITIVES_gArc_H_ */
