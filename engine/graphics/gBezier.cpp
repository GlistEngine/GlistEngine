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

gBezier::gBezier() : p0(glm::vec3(0.0f)), p1(glm::vec3(0.0f)), p2(glm::vec3(0.0f)), p3(glm::vec3(0.0f)), resolution(50), is3D(false) {

}

gBezier::gBezier(glm::vec2 p0, glm::vec2 p1, glm::vec2 p2, glm::vec2 p3) : p0(glm::vec3(p0, 0.0f)), p1(glm::vec3(p1, 0.0f)), p2(glm::vec3(p2, 0.0f)), p3(glm::vec3(p3, 0.0f)), resolution(50), is3D(false) {

}

gBezier::gBezier(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, glm::vec3 p3) : p0(p0), p1(p1), p2(p2), p3(p3), resolution(50), is3D(true) {

}

gBezier::~gBezier() {

}

void gBezier::setPoint(glm::vec2 p0, glm::vec2 p1, glm::vec2 p2, glm::vec2 p3) {
	this->p0 = glm::vec3(p0, 0.0f);
	this->p1 = glm::vec3(p1, 0.0f);
	this->p2 = glm::vec3(p2, 0.0f);
	this->p3 = glm::vec3(p3, 0.0f);
	is3D = false;
}

void gBezier::setPoint(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, glm::vec3 p3) {
	this->p0 = p0;
	this->p1 = p1;
	this->p2 = p2;
	this->p3 = p3;
	is3D = true;
}

void gBezier::setResolution(int res) {
	if(res < 1) res = 1;
	resolution = res;
}

glm::vec2 gBezier::getPoint2D(float t) const {
	glm::vec3 pt = getPoint3D(t);
	return glm::vec2(pt.x, pt.y);
}

glm::vec3 gBezier::getPoint3D(float t) const {
	if(t < 0.0f) t = 0.0f;
	if(t > 1.0f) t = 1.0f;

	float u = 1.0f - t;
	float tt = t * t;
	float uu = u * u;
	float uuu = uu * u;
	float ttt = tt * t;

	glm::vec3 result;
	result.x = (uuu * p0.x) + (3.0f * uu * t * p1.x) + (3.0f * u * tt * p2.x) + (ttt * p3.x);
	result.y = (uuu * p0.y) + (3.0f * uu * t * p1.y) + (3.0f * u * tt * p2.y) + (ttt * p3.y);
	result.z = (uuu * p0.z) + (3.0f * uu * t * p1.z) + (3.0f * u * tt * p2.z) + (ttt * p3.z);

	return result;
}

void gBezier::draw() const {
	float step = 1.0f / (float)resolution;

	if (is3D) {
		glm::vec3 previousPoint = getPoint3D(0.0f);
		for(float t = step; t <= 1.0f; t += step) {
			glm::vec3 currentPoint = getPoint3D(t);
			gDrawLine(previousPoint.x, previousPoint.y, previousPoint.z, currentPoint.x, currentPoint.y, currentPoint.z);
			previousPoint = currentPoint;
		}
	} else {
		glm::vec2 previousPoint = getPoint2D(0.0f);
		for(float t = step; t <= 1.0f; t += step) {
			glm::vec2 currentPoint = getPoint2D(t);
			gDrawLine(previousPoint.x, previousPoint.y, currentPoint.x, currentPoint.y);
			previousPoint = currentPoint;
		}
	}
}



