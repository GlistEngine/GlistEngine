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

gBezier::gBezier() : p0(glm::vec2(0.0f)), p1(glm::vec2(0.0f)), p2(glm::vec2(0.0f)), p3(glm::vec2(0.0f)), resolution(50) {

}

gBezier::gBezier(glm::vec2 p0, glm::vec2 p1, glm::vec2 p2, glm::vec2 p3) : p0(p0), p1(p1), p2(p2), p3(p3), resolution(50) {

}

gBezier::~gBezier() {

}

void gBezier::setPoint(glm::vec2 p0, glm::vec2 p1, glm::vec2 p2, glm::vec2 p3) {
	this->p0 = p0;
	this->p1 = p1;
	this->p2 = p2;
	this->p3 = p3;
}

void gBezier::setResolution(int res) {
	if(res < 1) res = 1;
	resolution = res;
}

glm::vec2 gBezier::getPoint(float t) const {
	if(t < 0.0f) t = 0.0f;
	if(t > 1.0f) t = 1.0f;

	float u = 1.0f - t;
	float tt = t * t;
	float uu = u * u;
	float uuu = uu * u;
	float ttt = tt * t;

	glm::vec2 result;
	result.x = (uuu * p0.x) + (3.0f * uu * t * p1.x) + (3.0f * u * tt * p2.x) + (ttt * p3.x);
	result.y = (uuu * p0.y) + (3.0f * uu * t * p1.y) + (3.0f * u * tt * p2.y) + (ttt * p3.y);

	return result;
}

void gBezier::draw() const {
	float step = 1.0f / (float)resolution;
	glm::vec2 previousPoint = getPoint(0.0f);

	for(float t = step; t <= 1.0f; t += step) {
		glm::vec2 currentPoint = getPoint(t);
		gDrawLine(previousPoint.x, previousPoint.y, currentPoint.x, currentPoint.y);
		previousPoint = currentPoint;
	}
}



