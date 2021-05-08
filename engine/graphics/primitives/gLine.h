/*
 * gLine.h
 *
 *  Created on: May 7, 2021
 *      Author: noyan
 */

#ifndef GRAPHICS_PRIMITIVES_GLINE_H_
#define GRAPHICS_PRIMITIVES_GLINE_H_

#include "gMesh.h"


class gLine: public gMesh {
public:
	gLine();
	gLine(float x1, float y1, float x2, float y2);
	gLine(float x1, float y1, float z1, float x2, float y2, float z2);
	virtual ~gLine();

	void setPoints(float x1, float y1, float x2, float y2);
	void setPoints(float x1, float y1, float z1, float x2, float y2, float z2);

	void draw();
	void draw(float x1, float y1, float x2, float y2);
	void draw(float x1, float y1, float z1, float x2, float y2, float z2);

private:
	std::vector<gVertex> verticessb;
	void setLinePoints(float x1, float y1, float z1, float x2, float y2, float z2);
	gVertex vertex1, vertex2;
};

#endif /* GRAPHICS_PRIMITIVES_GLINE_H_ */
