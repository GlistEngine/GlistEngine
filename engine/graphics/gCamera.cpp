/*
 * gCamera.cpp
 *
 *  Created on: May 10, 2020
 *      Author: noyan
 */

#include "gCamera.h"

//gCamera::gCamera() {
//}

// Constructor with vectors
gCamera::gCamera(glm::vec3 camPosition) {
	fov = 60.0f;
	nearclip = 0.01f;
	farclip = 1000.0f;
	lookposition = glm::vec3(0.0f);
	lookorientation = glm::angleAxis(0.0f, glm::vec3(0.0f, 0.0f, 0.0f));
	lookscalevec = glm::vec3(1.0f);
	locallookmatrix = glm::mat4(1.0f);
	setPosition(camPosition.x, camPosition.y, camPosition.z);
}

// Constructor with scalar values
gCamera::gCamera(float camPosX, float camPosY, float camPosZ) {
	fov = 60.0f;
	nearclip = 0.01f;
	farclip = 1000.0f;
	lookposition = glm::vec3(0.0f);
	lookorientation = glm::angleAxis(0.0f, glm::vec3(0.0f, 0.0f, 0.0f));
	lookscalevec = glm::vec3(1.0f);
	locallookmatrix = glm::mat4(1.0f);
	setPosition(camPosX, camPosY, camPosZ);
}

gCamera::~gCamera() {
}

void gCamera::begin() {
	renderer->backupMatrices();
	renderer->setProjectionMatrix(glm::perspective(glm::radians(fov), (float)renderer->getWidth() / (float)renderer->getHeight(), nearclip, farclip));
	renderer->setViewMatrix(glm::inverse(locallookmatrix));
	renderer->setCameraPosition(position);
//	viewmatrix = GetViewMatrix();
}

void gCamera::end() {
	renderer->restoreMatrices();
}

void gCamera::setFov(float f) {
	fov = f;
}

void gCamera::setNearClip(float nearClip) {
	nearclip = nearClip;
}

void gCamera::setFarClip(float farClip) {
	farclip = farClip;
}

void gCamera::move(float dx, float dy, float dz) {
	gNode::move(dx, dy, dz);
	lookposition += glm::vec3(dx, dy, dz);
	processLookMatrix();
}

void gCamera::move(const glm::vec3 dv) {
	gNode::move(dv);
	lookposition += dv;
	processLookMatrix();
}

void gCamera::setPosition(float px, float py, float pz) {
	gNode::setPosition(px, py, pz);
	lookposition = glm::vec3(px, py, pz);
	processLookMatrix();
}

void gCamera::dolly(float distance) {
	gNode::dolly(distance);
	lookposition += normalize(glm::vec3(localtransformationmatrix[2])) * distance;
	processLookMatrix();
}

void gCamera::truck(float distance) {
	gNode::truck(distance);
	lookposition += normalize(glm::vec3(localtransformationmatrix[0])) * distance;
	processLookMatrix();
}

void gCamera::boom(float distance) {
	gNode::boom(distance);
	lookposition += normalize(glm::vec3(localtransformationmatrix[1])) * distance;
	processLookMatrix();
}

void gCamera::rotate(const glm::quat& o) {
	gNode::rotate(o);
	lookorientation = (const glm::quat&)lookorientation * o;
	processLookMatrix();
}

void gCamera::rotate(float angle, float ax, float ay, float az) {
	gNode::rotate(angle, ax, ay, az);
	lookorientation = (const glm::quat&)lookorientation * glm::angleAxis(angle, glm::vec3(ax, ay, az));
	processLookMatrix();
}

void gCamera::setOrientation(const glm::quat& o) {
	gNode::setOrientation(o);
	lookorientation = o;
	processLookMatrix();
}

void gCamera::setOrientation(const glm::vec3& angles) {
	gNode::setOrientation(angles);
	lookorientation = (const glm::quat&)lookorientation * glm::angleAxis(angles.x, glm::vec3(1, 0, 0)) * glm::angleAxis(angles.z, glm::vec3(0, 0, 1)) * glm::angleAxis(angles.y, glm::vec3(0, 1, 0));
	processLookMatrix();
}

void gCamera::tilt(float radians) {
	gNode::tilt(radians);
	lookorientation = (const glm::quat&)lookorientation * glm::angleAxis(radians, glm::vec3(1.0f, 0.0f, 0.0f));
	processLookMatrix();
}

void gCamera::pan(float radians) {
	gNode::pan(radians);
	lookorientation = (const glm::quat&)lookorientation * glm::angleAxis(radians, glm::vec3(0.0f, 1.0f, 0.0f));
	processLookMatrix();
}

void gCamera::roll(float radians) {
	gNode::roll(radians);
	lookorientation = (const glm::quat&)lookorientation * glm::angleAxis(radians, glm::vec3(0.0f, 0.0f, 1.0f));
	processLookMatrix();
}

void gCamera::scale(float sx, float sy, float sz) {
	gNode::scale(sx, sy, sz);
	lookscalevec.x *= sx;
	lookscalevec.y *= sy;
	lookscalevec.z *= sz;
	processLookMatrix();
}

void gCamera::scale(float s) {
	gNode::scale(s);
	lookscalevec.x *= s;
	lookscalevec.y *= s;
	lookscalevec.z *= s;
	processLookMatrix();
}

void gCamera::setScale(const glm::vec3& s) {
	gNode::setScale(s);
	lookscalevec = s;
	processLookMatrix();
}

void gCamera::setScale(float sx, float sy, float sz) {
	gNode::setScale(sx, sy, sz);
	lookscalevec = glm::vec3(sx, sy, sz);
	processLookMatrix();
}

void gCamera::setScale(float s) {
	gNode::setScale(s);
	lookscalevec = glm::vec3(s, s, s);
	processLookMatrix();
}

void gCamera::rotateLook(float angle, float ax, float ay, float az) {
	lookorientation = (const glm::quat&)lookorientation * glm::angleAxis(angle, glm::vec3(ax, ay, az));
	processLookMatrix();
}

void gCamera::resetLook() {
	lookorientation = orientation;
	processLookMatrix();
}

glm::mat4 gCamera::getLookMatrix() {
	return locallookmatrix;
}

glm::quat gCamera::getLookOrientation() {
	return lookorientation;
}


void gCamera::processLookMatrix() {
	locallookmatrix = glm::translate(glm::mat4(1.0f), lookposition);
	locallookmatrix = locallookmatrix * glm::toMat4((const glm::quat&)lookorientation);
	locallookmatrix = glm::scale(locallookmatrix, lookscalevec);
}

