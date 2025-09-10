/*
 * gCross.h
 *
 *  Created on: 24 Aug 2022
 *      Author: burakmeydan
 */

#ifndef GRAPHICS_PRIMITIVES_GCROSS_H_
#define GRAPHICS_PRIMITIVES_GCROSS_H_

#include "gMesh.h"

class gCross: public gMesh {
public:
	gCross();
	~gCross() override;

	void draw(float x, float y, float width, float height, float thickness, bool isFilled);

private:
	void drawNonFilled(float x, float y, float width, float height, float thickness);
	void drawFilled(float x, float y, float width, float height, float thickness);

	std::vector<gVertex> vertices;
	std::vector<gIndex> indices;
};

#endif /* GRAPHICS_PRIMITIVES_GCROSS_H_ */
