/*
 * gRoundedRectangle.h
 *
 *  Created on: 15 Feb 2023
 *      Author: Umutcan Turkmen
 */

#ifndef GRAPHICS_PRIMITIVES_GROUNDEDRECTANGLE_H_
#define GRAPHICS_PRIMITIVES_GROUNDEDRECTANGLE_H_

#include "gMesh.h"

class gRoundedRectangle : public gMesh {
public:
	gRoundedRectangle();
	gRoundedRectangle(int x, int y, int w, int h, int radius, bool isFilled);
	~gRoundedRectangle() override;

	void draw() override;
	void draw(int x, int y, int w, int h, int radius, bool isFilled);
	void setRoundedRectanglePoints(int x, int y, int w, int h, int radius, bool isFilled);
private:
	//This is the default value of vertices of the created shape
	//Since it has as many vertices as its width or height (whichever is higher)
	//it will be seen as circular
	float numberOfVertices = 100; //
};

#endif /* GRAPHICS_PRIMITIVES_GROUNDEDRECTANGLE_H_ */
