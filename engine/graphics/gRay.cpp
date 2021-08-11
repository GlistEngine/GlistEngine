/*
 * gRay.cpp
 *
 *  Created on: Mar 16, 2021
 *      Author: noyan
 */

#include "gRay.h"
#include "gMesh.h"
#include "gBoundingBox.h"


gRay::gRay() : origin(glm::vec3(0.0f)), direction(glm::vec3(0.0f)) {
	islinked = false;
	scale = 1.0f;
	isnormalized = false;
	pdiff = glm::vec3(0.0f);
	length = 0.0f;
}

gRay::gRay(const glm::vec3& originPoint, const glm::vec3& directionVector) : origin(originPoint), direction(directionVector) {
	islinked = false;
	scale = 1.0f;
	isnormalized = false;
	pdiff = glm::vec3(0.0f);
	length = glm::length(directionVector);
}

gRay::~gRay() {}

void gRay::setOrigin(const glm::vec3& originPoint) {
	origin = originPoint;
}

void gRay::setDirection(const glm::vec3& directionVector) {
	direction = directionVector;
	length = glm::length(directionVector);
}

glm::vec3 gRay::getOrigin() {
	if(islinked) return linkedmesh->getPosition() + pdiff;
	return origin;
}

glm::vec3 gRay::getDirection() {
	if(islinked) {
		if(isnormalized) return normalize(direction) * scale;
		return direction * scale;
	}
	return direction;
}

float gRay::getLength() {
	return length;
}

bool gRay::intersects(gBoundingBox& boundingBox) {
	return boundingBox.intersects(this);
}

float gRay::distance(gBoundingBox& boundingBox) {
	return boundingBox.distance(this);
}


void gRay::link(gMesh* mesh, float rayScale, bool isNormalized, float dx, float dy, float dz) {
	linkedmesh = mesh;
	islinked = true;
	scale = rayScale;
	isnormalized = isNormalized;
	pdiff = glm::vec3(dx, dy, dz);
}

void gRay::unlink() {
	islinked = false;
}
