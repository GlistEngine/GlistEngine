/*
 * gArc.h
 *
 *  Created on: 27 Jul 2022
 *      Author: burakmeydan
 */

#ifndef GRAPHICS_PRIMITIVES_gArc_H_
#define GRAPHICS_PRIMITIVES_gArc_H_

#include "gMesh.h"

class gArc: public gMesh {
public:
	gArc();
	~gArc() override;

	void draw(float xCenter, float yCenter, float radius, bool isFilled, int numberOfSides = 60, float degree = 360.0f, float rotate = 360.0f);

private:
	void setArcPoints(float xCenter, float yCenter, float radius, bool isFilled, int numberOfSides = 60, float degree = 360.0f, float rotate = 360.0f);
	std::vector<gVertex> vertices;
	std::vector<gIndex> indices;
};

#endif /* GRAPHICS_PRIMITIVES_gArc_H_ */
