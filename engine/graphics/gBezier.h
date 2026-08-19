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
#include <vector>

/**
 * @class gBezier
 * @brief Evaluates and renders a single quadratic Bezier curve requiring exactly 3 control points (two ends and a middle control point).
 *
 * Usage:
 * gBezier curve;
 * curve.addPoint(glm::vec3(0.0f, 0.0f, 0.0f));
 * curve.addPoint(glm::vec3(5.0f, 10.0f, 0.0f));
 * curve.addPoint(glm::vec3(10.0f, 0.0f, 0.0f));
 * curve.draw();
 */

class gBezier : public gRenderObject {
public:
	gBezier();
	virtual ~gBezier();
	//Sets curve control points, capped at 3
	void setPoint(const std::vector<glm::vec2>& pts);
	void setPoint(const std::vector<glm::vec3>& pts);

	// Adds a 2D control point to the end of the curve, up to 3. Z axis defaults to 0.0f.
	void addPoint(glm::vec2 p);

	//Adds a 3D control point to the end of the curve, up to 3.
	void addPoint(glm::vec3 p);

	// Updates a specific control point coordinate by its index safely.
	void setPointAtIndex(int index, glm::vec3 p);
	void setPointAtIndex(int index, glm::vec2 p);

	// Gets the current 3D coordinates of a specific control point by its index.
	glm::vec3 getPointAtIndex(int index) const;

	// Gets the current 2D coordinates (X, Y) of a specific control point.
	glm::vec2 getPoint2DAtIndex(int index) const;

	//Clears all points to reset or reuse the curve.
	void clearPoints();

	//Gets the interpolated point at time t [0.0 - 1.0]. Requires exactly 3 points; returns (0,0,0) otherwise.
	glm::vec3 getPoint(float t) const;

	glm::vec2 getPoint2D(float t) const;

	//Sets rendering resolution (number of segments).
	void setResolution(int res);

	//Draws the curve in 2D or 3D based on input points. No-op unless exactly 3 points are set.
	void draw() const;
private:
	std::vector<glm::vec3> points;
	int resolution;
	bool is3D;
};

#endif /* GRAPHICS_GBEZIER_H_ */
