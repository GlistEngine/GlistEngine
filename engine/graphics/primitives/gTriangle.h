/*
 * gTriangle.h
 *
 *  Created on: Jul 28, 2022
 *      Author: emirhantasdeviren
 */

#ifndef GRAPHICS_PRIMITIVES_GTRIANGLE_H_
#define GRAPHICS_PRIMITIVES_GTRIANGLE_H_

#include "gMesh.h"

class gTriangle: public gMesh {
public:
	gTriangle();
	~gTriangle() override;

	void draw(float px, float py, float qx, float qy, float rx, float ry, bool is_filled);

private:
	void setPoints(float px, float py, float qx, float qy, float rx, float ry, bool is_filled);

	std::vector<gVertex> vertices;
	std::vector<gIndex> indices;
	gVertex vertex1;
	gVertex vertex2;
	gVertex vertex3;
};

#endif /* GRAPHICS_PRIMITIVES_GTRIANGLE_H_ */
