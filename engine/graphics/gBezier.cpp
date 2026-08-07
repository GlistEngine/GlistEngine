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

glm::vec3 gBezier::getPoint(float t) const {
	if(points.empty()) return glm::vec3(0.0f);
	if(points.size() == 1) return points[0];

	if(t < 0.0f) t = 0.0f;
	if(t > 1.0f) t = 1.0f;

	std::vector<glm::vec3> temp = points;
	int n = temp.size();

	for (int k = 1; k < n; ++k) {
		for (int i = 0; i < n - k; ++i) {
			temp[i] = temp[i] * (1.0f - t) + temp[i + 1] * t;
		}
	}
	return temp[0];
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



