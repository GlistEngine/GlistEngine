/*
 * gRectangle.h
 *
 *  Created on: 13 Tem 2021
 *      Author: oznur
 */

#ifndef GRAPHICS_PRIMITIVES_GRECTANGLE_H_
#define GRAPHICS_PRIMITIVES_GRECTANGLE_H_

#include "gMesh.h"

class gRectangle: public gMesh {
public:
	gRectangle();
	gRectangle(float x, float y, float w, float h, bool isFilled);
	~gRectangle() override;

	void setPoints(float x, float y, float w, float h, bool isFilled);

	void draw() override;
	void draw(float x, float y, float w, float h, bool isFilled);

private:
	std::vector<gVertex> verticessb;
	std::vector<gIndex> indicessb;

private:
	void setRectanglePoints(float x, float y, float w, float h, bool isFilled);
};


#endif /* GRAPHICS_PRIMITIVES_GRECTANGLE_H_ */
