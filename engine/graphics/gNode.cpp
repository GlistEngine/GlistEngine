/*
 * gTransformable.cpp
 *
 *  Created on: Aug 25, 2020
 *      Author: noyan
 */

#include "gNode.h"

int gNode::lastid = -1;

int gNode::getId() const {
	return id;
}

gNode::gNode() {
	lastid++;
	id = lastid;
	parent = nullptr;
	isenabled = true;
	localtransformationmatrix = glm::mat4(1.0f);
	position = glm::vec3(0.0f);
	orientation = glm::angleAxis(0.0f, glm::vec3(0.0f, 0.0f, 0.0f));
	scalevec = glm::vec3(1.0f);
	processTransformationMatrix();
}

gNode::~gNode() {
	removeParent();
}

void gNode::setParent(gNode* parent) {
	removeParent();
	this->parent = parent;
}

gNode* gNode::getParent() const {
	return parent;
}

void gNode::removeParent() {
	if (this->parent) {
		this->parent = nullptr;
//		delete this->parent;
	}
}

void gNode::move(float dx, float dy, float dz) {
	position += glm::vec3(dx, dy, dz);
	processTransformationMatrix();
}

void gNode::move(const glm::vec3& dv) {
	position += dv;
	processTransformationMatrix();
}

void gNode::setPosition(float px, float py, float pz) {
	position = glm::vec3(px, py, pz);
	processTransformationMatrix();
}

void gNode::setPosition(const glm::vec3& pv) {
	position = pv;
	processTransformationMatrix();
}

void gNode::dolly(float distance) {
	position += normalize(glm::vec3(localtransformationmatrix[2])) * distance;
	processTransformationMatrix();
}

void gNode::truck(float distance) {
	position += normalize(glm::vec3(localtransformationmatrix[0])) * distance;
	processTransformationMatrix();
}

void gNode::boom(float distance) {
	position += normalize(glm::vec3(localtransformationmatrix[1])) * distance;
	processTransformationMatrix();
}

float gNode::getPosX() const {
	return position.x;
}

float gNode::getPosY() const {
	return position.y;
}

float gNode::getPosZ() const {
	return position.z;
}

const glm::vec3& gNode::getPosition() const {
	return position;
}

void gNode::rotate(const glm::quat& o) {
	orientation = (const glm::quat&)orientation * o;
	processTransformationMatrix();
}

void gNode::rotate(float radians, float ax, float ay, float az) {
	orientation = (const glm::quat&)orientation * glm::angleAxis(radians, glm::vec3(ax, ay, az));
	processTransformationMatrix();
}

void gNode::rotateDeg(float degrees, float ax, float ay, float az) {
	orientation = (const glm::quat&)orientation * glm::angleAxis((degrees * 3.141592f) / 180.0f, glm::vec3(ax, ay, az));
	processTransformationMatrix();
}

void gNode::rotateAround(float radians, const glm::vec3& axis, const glm::vec3& point) {
	position = glm::angleAxis(radians, axis) * (position - point) + point;
	processTransformationMatrix();
}

void gNode::rotateAroundDeg(float degrees, const glm::vec3& axis, const glm::vec3& point) {
	position = glm::angleAxis((degrees * 3.141592f) / 180.0f, axis) * (position - point) + point;
	processTransformationMatrix();
}

void gNode::setOrientation(const glm::quat& o) {
	orientation = o;
	processTransformationMatrix();
}

void gNode::setOrientation(const glm::vec3& angles) {
	orientation = (const glm::quat&)orientation * glm::angleAxis(angles.x, glm::vec3(1, 0, 0)) * glm::angleAxis(angles.z, glm::vec3(0, 0, 1)) * glm::angleAxis(angles.y, glm::vec3(0, 1, 0));
	processTransformationMatrix();
}

const glm::quat& gNode::getOrientation() const {
	return orientation;
}

void gNode::tilt(float radians) {
	orientation = (const glm::quat&)orientation * glm::angleAxis(radians, glm::vec3(1.0f, 0.0f, 0.0f));
	processTransformationMatrix();
}

void gNode::tiltDeg(float degrees) {
	orientation = (const glm::quat&)orientation * glm::angleAxis((degrees * 3.141592f) / 180.0f, glm::vec3(1.0f, 0.0f, 0.0f));
	processTransformationMatrix();

}

void gNode::pan(float radians) {
	orientation = (const glm::quat&)orientation * glm::angleAxis(radians, glm::vec3(0.0f, 1.0f, 0.0f));
	processTransformationMatrix();
}

void gNode::panDeg(float degrees) {
	orientation = (const glm::quat&)orientation * glm::angleAxis((degrees * 3.141592f) / 180.0f, glm::vec3(0.0f, 1.0f, 0.0f));
	processTransformationMatrix();
}

void gNode::roll(float radians) {
	orientation = (const glm::quat&)orientation * glm::angleAxis(radians, glm::vec3(0.0f, 0.0f, 1.0f));
	processTransformationMatrix();
}

void gNode::rollDeg(float degrees) {
	orientation = (const glm::quat&)orientation * glm::angleAxis((degrees * 3.141592f) / 180.0f, glm::vec3(0.0f, 0.0f, 1.0f));
	processTransformationMatrix();
}

void gNode::scale(float sx, float sy, float sz) {
	scalevec.x *= sx;
	scalevec.y *= sy;
	scalevec.z *= sz;
	processTransformationMatrix();
}

void gNode::scale(float s) {
	scalevec.x *= s;
	scalevec.y *= s;
	scalevec.z *= s;
	processTransformationMatrix();
}

void gNode::setScale(const glm::vec3& s) {
	scalevec = s;
	processTransformationMatrix();
}

void gNode::setScale(float sx, float sy, float sz) {
	scalevec = glm::vec3(sx, sy, sz);
	processTransformationMatrix();
}

void gNode::setScale(float s) {
	scalevec = glm::vec3(s, s, s);
	processTransformationMatrix();
}

const glm::vec3& gNode::getScale() const{
	return scalevec;
}

glm::vec3 gNode::getScalarDirectionX() const {
	return glm::vec3(localtransformationmatrix[0]);
}

glm::vec3 gNode::getScalarDirectionY() const {
	return glm::vec3(localtransformationmatrix[1]);
}

glm::vec3 gNode::getScalarDirectionZ() const {
	return glm::vec3(localtransformationmatrix[2]);
}


void gNode::pushMatrix() const {
#if defined(WIN32) || defined(LINUX)
	glPushMatrix();
#endif
}

void gNode::popMatrix() const {
#if defined(WIN32) || defined(LINUX)
	glPushMatrix();
#endif
}

void gNode::processTransformationMatrix() {
	localtransformationmatrix = glm::translate(glm::mat4(1.0f), position);
	localtransformationmatrix = localtransformationmatrix * glm::toMat4((const glm::quat&)orientation);
	localtransformationmatrix = glm::scale(localtransformationmatrix, scalevec);
}

void gNode::setEnabled(bool isEnabled) {
	isenabled = isEnabled;
}

bool gNode::isEnabled() const {
	return isenabled;
}

void gNode::setTransformationMatrix(const glm::mat4& transformationMatrix) {
	localtransformationmatrix = transformationMatrix;
}

const glm::mat4& gNode::getTransformationMatrix() const {
	return localtransformationmatrix;
}
