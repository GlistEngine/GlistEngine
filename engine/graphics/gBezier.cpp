/*
 *
 * gBezier.cpp
 *
 *  Created On: August 7, 2026
 *  Author: Ahmet Kahraman
 *
 */
#include "gBezier.h"
#include "gRenderer.h"

gBezier::gBezier() : resolution(50), is3D(false) {
}

gBezier::~gBezier() {
	points.clear();
}

void gBezier::setPoint(const std::vector<glm::vec2>& pts) {
	points.clear();
	for (const auto& p : pts) {
		points.push_back(glm::vec3(p.x, p.y, 0.0f));
	}
	is3D = false;
}

void gBezier::setPoint(const std::vector<glm::vec3>& pts) {
	points = pts;
	is3D = true;
}

void gBezier::addPoint(glm::vec2 p) {
	points.push_back(glm::vec3(p.x, p.y, 0.0f));
}

void gBezier::addPoint(glm::vec3 p) {
	points.push_back(p);
	is3D = true;
}

void gBezier::clearPoints() {
	points.clear();
	is3D = false;
}

void gBezier::setResolution(int res) {
	if(res < 1) res = 1;
	resolution = res;
}

void gBezier::setPointAtIndex(int index, glm::vec3 p) {
	if(index >= 0 && index < points.size()) {
		points[index] = p;
		is3D = true;
	}
}

void gBezier::setPointAtIndex(int index, glm::vec2 p) {
	if(index >= 0 && index < points.size()) {
		points[index] = glm::vec3(p.x, p.y, 0.0f);
	}
}

glm::vec2 gBezier::getPoint2DAtIndex(int index) const {
	if(index >= 0 && index < points.size()) {
		return glm::vec2(points[index].x, points[index].y);
	}
	return glm::vec2(0.0f);
}

glm::vec3 gBezier::getPointAtIndex(int index) const {
	if(index >= 0 && index < points.size()) {
		return points[index];
	}
	return glm::vec3(0.0f);
}

glm::vec3 gBezier::getPoint(float t) const {

	if(points.size() < 3) {
		if(points.empty()) return glm::vec3(0.0f);
		if(points.size() == 1) return points[0];
		return points[0] * (1.0f - t) + points[1] * t;
	}

	if(t <= 0.0f) return points.front();
	if(t >= 1.0f) return points.back();

	int numSegments = (points.size() - 1) / 2;

	float scaled_t = t * numSegments;

	int segment = (int)scaled_t;
	if(segment >= numSegments) segment = numSegments - 1;

	float local_t = scaled_t - segment;

	int startIndex = segment * 2;

	glm::vec3 p0 = points[startIndex];
	glm::vec3 p1 = points[startIndex + 1];
	glm::vec3 p2 = points[startIndex + 2];

	float u = 1.0f - local_t;
	return (u * u * p0) + (2.0f * u * local_t * p1) + (local_t * local_t * p2);
}

glm::vec2 gBezier::getPoint2D(float t) const {
    glm::vec3 p = getPoint(t);
    return glm::vec2(p.x, p.y);
}

void gBezier::draw() const {
	if (points.size() < 2) return;

	float step = 1.0f / (float)resolution;
	glm::vec3 previousPoint = getPoint(0.0f);

	for(float t = step; t <= 1.0f; t += step) {
		glm::vec3 currentPoint = getPoint(t);

		if (is3D) {
			gDrawLine(previousPoint.x, previousPoint.y, previousPoint.z, currentPoint.x, currentPoint.y, currentPoint.z);
		} else {
			gDrawLine(previousPoint.x, previousPoint.y, currentPoint.x, currentPoint.y);
		}

		previousPoint = currentPoint;
	}
}



