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
	gRectangle(float x, float y, float w, float h, bool isFilled, float rotateAngle = 0.0f);
		~gRectangle() override;

		void setPoints(float x, float y, float w, float h, bool isFilled, float rotateAngle = 0.0f);

		void draw() override;
		void draw(float x, float y, float w, float h, bool isFilled, float rotateAngle = 0.0f);

private:
	std::vector<gVertex> verticessb;
	std::vector<gIndex> indicessb;

private:
	void setRectanglePoints(float x, float y, float w, float h, bool isFilled, float rotateAngle = 0.0f);
};


#endif /* GRAPHICS_PRIMITIVES_GRECTANGLE_H_ */
