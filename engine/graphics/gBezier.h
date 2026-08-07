/*
 * gBezier.h
 *
 *  Created On: August 7, 2026
 *  Author: Ahmet Kahraman
 *
 */
#ifndef GRAPHICS_GBEZIER_H_
#define GRAPHICS_GBEZIER_H_

#include "gRenderObject.h"
#include <glm/glm.hpp>

/**
 * @class gBezier
 * @brief Evaluates and renders 2D/3D Cubic Bezier curves using Bernstein polynomials.
 *
 * @usage
 * gBezier curve(p0, p1, p2, p3);
 * curve.setResolution(100); // Optional: higher is smoother (default: 50)
 * curve.draw(); // Call inside the draw() loop
 */

class gBezier : public gRenderObject {
public:
	gBezier();
	// Creates a 2D curve. Z axis will be 0.0f.
	gBezier(glm::vec2 p0, glm::vec2 p1, glm::vec2 p2, glm::vec2 p3);
	//Creates a 3D curve in world coordinates.
	gBezier(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, glm::vec3 p3);
	virtual ~gBezier();

	void setPoint(glm::vec2 p0, glm::vec2 p1, glm::vec2 p2, glm::vec2 p3);
	void setPoint(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, glm::vec3 p3);

	//Returns the exact 2D coordinates at time t [0.0 - 1.0].
	glm::vec2 getPoint2D(float t) const;
	//Returns the exact 3D coordinates at time t [0.0 - 1.0].
	glm::vec3 getPoint3D(float t) const;
	//Sets the number of line segments used for drawing.
	void setResolution(int res);
	//Draws the curve to the screen automatically in 2D or 3D.
	void draw() const;
private:
	glm::vec3 p0, p1, p2, p3;
	int resolution;
	bool is3D;
};

#endif /* GRAPHICS_GBEZIER_H_ */
