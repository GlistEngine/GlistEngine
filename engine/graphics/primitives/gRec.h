/*
 * gRec.h
 *
 *  Created on: 13 Tem 2021
 *      Author: oznur
 */

#ifndef GRAPHICS_PRIMITIVES_GREC_H_
#define GRAPHICS_PRIMITIVES_GREC_H_

#include "gMesh.h"

class gRec: public gMesh {
public:
	gRec();
	gRec(float x, float y, float w, float h, bool isFilled);
	virtual ~gRec();

	void setPoints(float x, float y, float w, float h, bool isFilled);

	void draw();
	void draw(float x, float y, float w, float h, bool isFilled);

	private:
	std::vector<gVertex> verticessb;
	void setRecPoints(float x, float y, float w, float h, bool isFilled);
	gVertex vertex1, vertex2, vertex3, vertex4;
	std::vector<unsigned int> indicessb;
};


#endif /* GRAPHICS_PRIMITIVES_GREC_H_ */
