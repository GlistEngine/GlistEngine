/*
 * gCameraController.cpp
 *
 *  Created on: Feb 5, 2026
 *      Author: Metehan Gezer
 */

#include "gCameraController.h"
#include "gAppManager.h"
#include "gInputManager.h"

const std::string gCameraController::AXIS_HORIZONTAL = "camera_horizontal";
const std::string gCameraController::AXIS_VERTICAL = "camera_vertical";
const std::string gCameraController::AXIS_ELEVATION = "camera_elevation";
const std::string gCameraController::AXIS_LOOK_HORIZONTAL = "camera_look_horizontal";
const std::string gCameraController::AXIS_LOOK_VERTICAL = "camera_look_vertical";
const std::string gCameraController::ACTION_TOGGLE = "camera_toggle";

gCameraController::gCameraController() {
	camera = nullptr;
	enabled = false;
	playerindex = 0;
	movespeed = 2.0f;
	mousesensitivity = 2.0f;
	gamepadlooksensitivity = 3.0f;
	yaw = 0.0f;
	pitch = 0.0f;
	position = glm::vec3(0.0f);
	registerInputs();
}

gCameraController::gCameraController(gCamera* camera) : gCameraController() {
	setCamera(camera);
}

gCameraController::~gCameraController() {
}

void gCameraController::registerInputs() {
	if(!inputmanager) return;
	gInputPlayer* player = inputmanager->getPlayer(playerindex);

	// Keyboard bindings
	player->addAxisBinding(AXIS_HORIZONTAL, G_KEY_A, G_KEY_D);
	player->addAxisBinding(AXIS_VERTICAL, G_KEY_S, G_KEY_W);
	player->addAxisBinding(AXIS_ELEVATION, G_KEY_LEFT_SHIFT, G_KEY_SPACE);
	player->addActionBinding(ACTION_TOGGLE, G_KEY_ESC);

	// Gamepad left stick for movement
	player->addGamepadAxisBinding(AXIS_HORIZONTAL, G_GAMEPAD_AXIS_LEFT_X);
	player->addGamepadAxisBinding(AXIS_VERTICAL, G_GAMEPAD_AXIS_LEFT_Y, true);

	// Gamepad bumpers for elevation
	player->addGamepadButtonAxisBinding(AXIS_ELEVATION, G_GAMEPAD_BUTTON_LEFT_BUMPER, G_GAMEPAD_BUTTON_RIGHT_BUMPER);

	// Gamepad right stick for look
	player->addGamepadAxisBinding(AXIS_LOOK_HORIZONTAL, G_GAMEPAD_AXIS_RIGHT_X);
	player->addGamepadAxisBinding(AXIS_LOOK_VERTICAL, G_GAMEPAD_AXIS_RIGHT_Y, true);

	// Gamepad Start as toggle
	player->addGamepadActionBinding(ACTION_TOGGLE, G_GAMEPAD_BUTTON_START);
}

void gCameraController::setCamera(gCamera* camera) {
	this->camera = camera;
	if(camera) {
		position = camera->getPosition();
	}
}

gCamera* gCameraController::getCamera() const {
	return camera;
}

void gCameraController::update() {
	if(!camera || !inputmanager) return;

	gInputPlayer* player = inputmanager->getPlayer(playerindex);

	// Toggle check (works even when disabled)
	if(player->isActionJustPressed(ACTION_TOGGLE)) {
		setEnabled(!enabled);
	}

	if(!enabled) return;

	float dt = appmanager->getElapsedTime();
	float speed = movespeed * dt;

	float fx = std::sin(yaw);
	float fz = -std::cos(yaw);
	float rx = std::cos(yaw);
	float rz = std::sin(yaw);

	float h = player->getAxisValue(AXIS_HORIZONTAL);
	float v = player->getAxisValue(AXIS_VERTICAL);
	float e = player->getAxisValue(AXIS_ELEVATION);

	position.x += (fx * v + rx * h) * speed;
	position.z += (fz * v + rz * h) * speed;
	position.y += e * speed;

	// Mouse look
	float mdx, mdy;
	player->getMouseDelta(mdx, mdy);
	if(mdx != 0.0f || mdy != 0.0f) {
		yaw -= mdx * mousesensitivity;
		pitch += mdy * mousesensitivity;
	}

	// Gamepad right stick look
	float lookh = player->getAxisValue(AXIS_LOOK_HORIZONTAL);
	float lookv = player->getAxisValue(AXIS_LOOK_VERTICAL);
	if(lookh != 0.0f || lookv != 0.0f) {
		yaw -= lookh * gamepadlooksensitivity * dt;
		pitch += lookv * gamepadlooksensitivity * dt;
	}

	if(pitch > 1.5f) pitch = 1.5f;
	if(pitch < -1.5f) pitch = -1.5f;

	updateCamera();
}

void gCameraController::setEnabled(bool enabled) {
	this->enabled = enabled;
	if(!inputmanager) return;
	gInputPlayer* player = inputmanager->getPlayer(playerindex);
	if(player->isKeyboardMouseAssigned()) {
		if(enabled) {
			appmanager->setCursorMode(CURSORMODE_RELATIVE);
		} else {
			appmanager->setCursorMode(CURSORMODE_NORMAL);
		}
	}
}

bool gCameraController::isEnabled() const {
	return enabled;
}

void gCameraController::setPlayerIndex(int playerIndex) {
	playerindex = playerIndex;
}

int gCameraController::getPlayerIndex() const {
	return playerindex;
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

void gCameraController::setGamepadLookSensitivity(float sensitivity) {
	gamepadlooksensitivity = sensitivity;
}

float gCameraController::getGamepadLookSensitivity() const {
	return gamepadlooksensitivity;
}

void gCameraController::updateCamera() {
	if(!camera) return;

	camera->setPosition(position);

	glm::vec3 target;
	target.x = position.x + std::sin(yaw) * std::cos(pitch);
	target.y = position.y + std::sin(pitch);
	target.z = position.z - std::cos(yaw) * std::cos(pitch);
	camera->lookAt(target);
}
