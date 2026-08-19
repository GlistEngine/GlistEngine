/*
 * gBezierQuad.cpp
 *
 *  Created on: August 19, 2026
 *      Author: Ahmet Kahraman
 */

#include "gBezierQuad.h"
#include "gRenderer.h"

gBezierQuad::gBezierQuad() : resolution(50), is3D(false) {
}

gBezierQuad::~gBezierQuad() {
	points.clear();
}

void gBezierQuad::setPoint(const std::vector<glm::vec2>& pts) {
	points.clear();
	for (const auto& p : pts) points.push_back(glm::vec3(p.x, p.y, 0.0f));
	is3D = false;
}

void gBezierQuad::setPoint(const std::vector<glm::vec3>& pts) {
	points = pts;
	is3D = true;
}

void gBezierQuad::addPoint(glm::vec2 p) {
	points.push_back(glm::vec3(p.x, p.y, 0.0f));
}

void gBezierQuad::addPoint(glm::vec3 p) {
	points.push_back(p);
	is3D = true;
}

void gBezierQuad::setPointAtIndex(int index, glm::vec3 p) {
	if(index >= 0 && index < points.size()) {
		points[index] = p;
		is3D = true;
	}
}

void gBezierQuad::setPointAtIndex(int index, glm::vec2 p) {
	if(index >= 0 && index < points.size()) points[index] = glm::vec3(p.x, p.y, 0.0f);
}

glm::vec3 gBezierQuad::getPointAtIndex(int index) const {
	if(index >= 0 && index < points.size()) return points[index];
	return glm::vec3(0.0f);
}

glm::vec2 gBezierQuad::getPoint2DAtIndex(int index) const {
	if(index >= 0 && index < points.size()) return glm::vec2(points[index].x, points[index].y);
	return glm::vec2(0.0f);
}

std::vector<glm::vec3> gBezierQuad::getPoints() const {
	return points;
}

std::vector<glm::vec2> gBezierQuad::getPoints2D() const {
	std::vector<glm::vec2> pts2d;
	pts2d.reserve(points.size());
	for(const auto& p : points) pts2d.push_back(glm::vec2(p.x, p.y));
	return pts2d;
}

void gBezierQuad::clearPoints() {
	points.clear();
	is3D = false;
}

glm::vec3 gBezierQuad::getPoint(float t) const {
	if(points.empty()) return glm::vec3(0.0f);
	if(points.size() == 1) return points[0];
	if(points.size() == 2) return points[0] * (1.0f - t) + points[1] * t;

	// 3 noktalý durum korumasý (Quadratic)
	if(points.size() == 3) {
		float u = 1.0f - t;
		return (u * u * points[0]) + (2.0f * u * t * points[1]) + (t * t * points[2]);
	}

	// Standart 4 Noktalý (Cubic) Bezier Matematiði
	float u = 1.0f - t;
	return (u * u * u * points[0]) +
	       (3.0f * u * u * t * points[1]) +
	       (3.0f * u * t * t * points[2]) +
	       (t * t * t * points[3]);
}

glm::vec2 gBezierQuad::getPoint2D(float t) const {
	glm::vec3 p = getPoint(t);
	return glm::vec2(p.x, p.y);
}

void gBezierQuad::setResolution(int res) {
	if(res < 1) res = 1;
	resolution = res;
}

void gBezierQuad::draw() const {
	if (points.size() < 2) return;

	float step = 1.0f / (float)resolution;
	glm::vec3 previousPoint = getPoint(0.0f);

	for(float t = step; t <= 1.0f; t += step) {
		glm::vec3 currentPoint = getPoint(t);
		if (is3D) gDrawLine(previousPoint.x, previousPoint.y, previousPoint.z, currentPoint.x, currentPoint.y, currentPoint.z);
		else gDrawLine(previousPoint.x, previousPoint.y, currentPoint.x, currentPoint.y);
		previousPoint = currentPoint;
	}
}

