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

	void setPoints(float xCenter, float yCenter, float radius, bool isFilled, int numberOfSides = 60, float degree = 360.0f, float rotate = 0.0f, float rotateAngle = 0.0f, float pivotx = 0.5f, float pivoty = 0.5f) {
	isprojection2d = true;
	setArcPoints(xCenter, yCenter, radius, isFilled, numberOfSides, degree, rotate, rotateAngle, pivotx, pivoty);
}
	void draw(float xCenter, float yCenter, float radius, bool isFilled, int numberOfSides = 60, float degree = 360.0f, float rotate = 0.0f, float rotateAngle = 0.0f, float pivotx = 0.5f, float pivoty = 0.5f);

	private:
		void setArcPoints(float xCenter, float yCenter, float radius, bool isFilled, int numberOfSides = 60, float degree = 360.0f, float rotate = 0.0f, float rotateAngle = 0.0f, float pivotx = 0.5f, float pivoty = 0.5f);
	std::vector<gVertex> vertices;
	std::vector<gIndex> indices;
};

#endif /* GRAPHICS_PRIMITIVES_gArc_H_ */
