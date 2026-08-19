/*
 * gBezierQuad.h
 *
 *  Created on: August 7, 2026
 *      Author: Ahmet Kahraman
 */

#ifndef GRAPHICS_GBEZIERQUAD_H_
#define GRAPHICS_GBEZIERQUAD_H_

#include "gMesh.h"
#include <glm/glm.hpp>
#include <vector>

// Evaluates and renders a standard 4-point (Cubic) Bezier curve.
class gBezierQuad : public gMesh {
public:
	gBezierQuad();
	virtual ~gBezierQuad();

	// Sets all control points using 2D vectors.
	void setPoint(const std::vector<glm::vec2>& pts);

	// Sets all control points using 3D vectors.
	void setPoint(const std::vector<glm::vec3>& pts);

	// Adds a 2D control point to the curve.
	void addPoint(glm::vec2 p);

	// Adds a 3D control point to the curve.
	void addPoint(glm::vec3 p);

	// Updates a 3D control point safely by its index.
	void setPointAtIndex(int index, glm::vec3 p);

	// Updates a 2D control point safely by its index.
	void setPointAtIndex(int index, glm::vec2 p);

	// Gets the 3D coordinates of a point by its index.
	glm::vec3 getPointAtIndex(int index) const;

	// Gets the 2D coordinates of a point by its index.
	glm::vec2 getPoint2DAtIndex(int index) const;

	// Returns all control points in 3D.
	std::vector<glm::vec3> getPoints() const;

	// Returns all control points in 2D.
	std::vector<glm::vec2> getPoints2D() const;

	// Clears all existing control points.
	void clearPoints();

	// Evaluates the 4-point curve at normalized time t [0.0 - 1.0] in 3D.
	glm::vec3 getPoint(float t) const;

	// Evaluates the 4-point curve at normalized time t [0.0 - 1.0] in 2D.
	glm::vec2 getPoint2D(float t) const;

	// Sets the drawing resolution (number of line segments).
	void setResolution(int res);

	// Renders the 4-point curve to the screen automatically.
	void draw() const;

private:
	std::vector<glm::vec3> points;
	int resolution;
	bool is3D;
};

#endif /* GRAPHICS_GBEZIERQUAD_H_ */
