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
	virtual ~gRectangle();

	void setPoints(float x, float y, float w, float h, bool isFilled);

	void draw();
	void draw(float x, float y, float w, float h, bool isFilled);

	private:
	std::vector<gVertex> verticessb;
	void setRectanglePoints(float x, float y, float w, float h, bool isFilled);
	gVertex vertex1, vertex2, vertex3, vertex4;
	std::vector<unsigned int> indicessb;
};


#endif /* GRAPHICS_PRIMITIVES_GRECTANGLE_H_ */
