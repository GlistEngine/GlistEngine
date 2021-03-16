/*
 * gRay.cpp
 *
 *  Created on: Mar 16, 2021
 *      Author: noyan
 */

#include "gRay.h"

gRay::gRay() : origin(glm::vec3(0.0f)), direction(glm::vec3(0.0f)) {}

gRay::gRay(const glm::vec3& originPoint, const glm::vec3& directionVector) : origin(originPoint), direction(directionVector) {}

gRay::~gRay() {}

void gRay::setOrigin(const glm::vec3& originPoint) {
	origin = originPoint;
}

void gRay::setDirection(const glm::vec3& directionVector) {
	direction = directionVector;
}

glm::vec3 gRay::getOrigin() {
	return origin;
}

glm::vec3 gRay::getDirection() {
	return direction;
}

