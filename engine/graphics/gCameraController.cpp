/*
 * gCameraController.cpp
 *
 *  Created on: Feb 5, 2026
 *      Author: Metehan Gezer
 */

#include "gCameraController.h"
#include "gAppManager.h"

gCameraController::gCameraController() {
	camera = nullptr;
	enabled = false;
	movespeed = 2.0f;
	mousesensitivity = 2.0f;
	yaw = 0.0f;
	pitch = 0.0f;
	position = glm::vec3(0.0f);
	for (int i = 0; i < 512; i++) keys[i] = false;
}

gCameraController::gCameraController(gCamera* camera) : gCameraController() {
	setCamera(camera);
}

gCameraController::~gCameraController() {
}

void gCameraController::setCamera(gCamera* camera) {
	this->camera = camera;
	if (camera) {
		position = camera->getPosition();
	}
}

gCamera* gCameraController::getCamera() const {
	return camera;
}

void gCameraController::update() {
	if (!camera || !enabled) return;

	float dt = appmanager->getElapsedTime();
	float speed = movespeed * dt;

	float fx = std::sin(yaw);
	float fz = -std::cos(yaw);
	float rx = std::cos(yaw);
	float rz = std::sin(yaw);

	if (keys[G_KEY_W]) { position.x += fx * speed; position.z += fz * speed; }
	if (keys[G_KEY_S]) { position.x -= fx * speed; position.z -= fz * speed; }
	if (keys[G_KEY_A]) { position.x -= rx * speed; position.z -= rz * speed; }
	if (keys[G_KEY_D]) { position.x += rx * speed; position.z += rz * speed; }
	if (keys[G_KEY_SPACE]) position.y += speed;
	if (keys[G_KEY_LEFT_SHIFT]) position.y -= speed;

	updateCamera();
}

void gCameraController::onEvent(gEvent& event) {
	gEventDispatcher dispatcher(event);
	dispatcher.dispatch<gMouseMovedEvent>(G_BIND_FUNCTION(onMouseMoved));
}

bool gCameraController::onMouseMoved(gMouseMovedEvent& event) {
	if (!enabled) return false;

	yaw -= event.getX() * mousesensitivity;
	pitch += event.getY() * mousesensitivity;

	if (pitch > 1.5f) pitch = 1.5f;
	if (pitch < -1.5f) pitch = -1.5f;

	updateCamera();
	return false;
}

void gCameraController::onKeyPressed(int key) {
	if (key < 512) keys[key] = true;

	if (key == G_KEY_ESC) {
		enabled = !enabled;
		if (enabled) {
			appmanager->setCursorMode(CURSORMODE_RELATIVE);
		} else {
			appmanager->setCursorMode(CURSORMODE_NORMAL);
		}
	}
}

void gCameraController::onKeyReleased(int key) {
	if (key < 512) keys[key] = false;
}

void gCameraController::setEnabled(bool enabled) {
	this->enabled = enabled;
	if (enabled) {
		appmanager->setCursorMode(CURSORMODE_RELATIVE);
	} else {
		appmanager->setCursorMode(CURSORMODE_NORMAL);
	}
}

bool gCameraController::isEnabled() const {
	return enabled;
}

void gCameraController::setMoveSpeed(float speed) {
	movespeed = speed;
}

float gCameraController::getMoveSpeed() const {
	return movespeed;
}

void gCameraController::setMouseSensitivity(float sensitivity) {
	mousesensitivity = sensitivity;
}

float gCameraController::getMouseSensitivity() const {
	return mousesensitivity;
}

void gCameraController::updateCamera() {
	if (!camera) return;

	camera->setPosition(position);

	glm::vec3 target;
	target.x = position.x + std::sin(yaw) * std::cos(pitch);
	target.y = position.y + std::sin(pitch);
	target.z = position.z - std::cos(yaw) * std::cos(pitch);
	camera->lookAt(target);
}
