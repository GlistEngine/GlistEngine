/*
 * gPolygon.h
 *
 *  Created on: 19 Aðu 2026
 *      Author: merve
 */

#ifndef GRAPHICS_PRIMITIVES_GPOLYGON_H_
#define GRAPHICS_PRIMITIVES_GPOLYGON_H_

#include "gRenderObject.h"
#include <vector>

class gPolygon : public gRenderObject {
public:
	gPolygon();
	virtual ~gPolygon();

	void addPoint(float x, float y);
	void clearPoints();

	void draw();
	void draw(float x, float y);

private:
	std::vector<float> pointsx;
	std::vector<float> pointsy;

	void drawPolygon(float x, float y);
};

#endif /* GRAPHICS_PRIMITIVES_GPOLYGON_H_ */
