/*
 * gInputManager.cpp
 *
 *  Created on: Feb 9, 2026
 *      Author: Metehan Gezer
 */

#include "gInputManager.h"
#include "gAppManager.h"
#include <algorithm>

gInputManager* inputmanager = nullptr;

gInputManager::gInputManager() {
	for(int i = 0; i < 512; i++) {
		keys[i] = false;
		keysjustpressed[i] = false;
	}
	mousedeltax = 0.0f;
	mousedeltay = 0.0f;

	for(int gp = 0; gp < MAXGAMEPADS; gp++) {
		gamepadstate[gp].connected = false;
		for(int b = 0; b < MAXGAMEPADBUTTONS; b++) {
			gamepadstate[gp].buttons[b] = false;
			gamepadstate[gp].previousbuttons[b] = false;
			gamepadstate[gp].buttonsjustpressed[b] = false;
		}
		for(int a = 0; a < MAXGAMEPADAXES; a++) {
			gamepadstate[gp].axes[a] = 0.0f;
		}
	}

	// Player 0: keyboard + mouse by default
	gInputPlayer* p0 = new gInputPlayer(this, 0);
	p0->setKeyboardMouseAssigned(true);
	players.push_back(p0);
}

gInputManager::~gInputManager() {
	for(int i = 0; i < (int)players.size(); i++) {
		delete players[i];
	}
	players.clear();
}

gInputPlayer* gInputManager::getPlayer(int playerIndex) {
	while((int)players.size() <= playerIndex) {
		players.push_back(new gInputPlayer(this, (int)players.size()));
	}
	return players[playerIndex];
}

int gInputManager::getPlayerCount() const {
	return (int)players.size();
}

bool gInputManager::isKeyPressed(int keyCode) const {
	if(keyCode < 0 || keyCode >= 512) return false;
	return keys[keyCode];
}

bool gInputManager::isKeyJustPressed(int keyCode) const {
	if(keyCode < 0 || keyCode >= 512) return false;
	return keysjustpressed[keyCode];
}

void gInputManager::getMouseDelta(float& dx, float& dy) const {
	dx = mousedeltax;
	dy = mousedeltay;
}

bool gInputManager::isGamepadConnected(int gamepadId) const {
	if(gamepadId < 0 || gamepadId >= MAXGAMEPADS) return false;
	return gamepadstate[gamepadId].connected;
}

bool gInputManager::isGamepadButtonPressed(int gamepadId, int buttonId) const {
	if(gamepadId < 0 || gamepadId >= MAXGAMEPADS) return false;
	if(buttonId < 0 || buttonId >= MAXGAMEPADBUTTONS) return false;
	return gamepadstate[gamepadId].buttons[buttonId];
}

bool gInputManager::isGamepadButtonJustPressed(int gamepadId, int buttonId) const {
	if(gamepadId < 0 || gamepadId >= MAXGAMEPADS) return false;
	if(buttonId < 0 || buttonId >= MAXGAMEPADBUTTONS) return false;
	return gamepadstate[gamepadId].buttonsjustpressed[buttonId];
}

float gInputManager::getGamepadAxisValue(int gamepadId, int axisId) const {
	if(gamepadId < 0 || gamepadId >= MAXGAMEPADS) return 0.0f;
	if(axisId < 0 || axisId >= MAXGAMEPADAXES) return 0.0f;
	return gamepadstate[gamepadId].axes[axisId];
}

void gInputManager::update() {
	// Reset per-frame keyboard/mouse state
	for(int i = 0; i < 512; i++) {
		keysjustpressed[i] = false;
	}
	mousedeltax = 0.0f;
	mousedeltay = 0.0f;

	// Poll gamepads
	pollGamepads();
}

void gInputManager::pollGamepads() {
	for(int gp = 0; gp < MAXGAMEPADS; gp++) {
		if(!gamepadstate[gp].connected) continue;

		// Save previous button state
		for(int b = 0; b < MAXGAMEPADBUTTONS; b++) {
			gamepadstate[gp].previousbuttons[b] = gamepadstate[gp].buttons[b];
		}

		// Poll buttons
		for(int b = 0; b < MAXGAMEPADBUTTONS; b++) {
			gamepadstate[gp].buttons[b] = appmanager->isGamepadButtonPressed(gp, b);
			gamepadstate[gp].buttonsjustpressed[b] =
				gamepadstate[gp].buttons[b] && !gamepadstate[gp].previousbuttons[b];
		}

		// Poll axes
		const float* axes = appmanager->getJoystickAxes(gp);
		if(axes) {
			int axiscount = appmanager->getJoystickAxesCount(gp);
			for(int a = 0; a < MAXGAMEPADAXES && a < axiscount; a++) {
				gamepadstate[gp].axes[a] = axes[a];
			}
		}
	}
}

// Event handling

void gInputManager::onEvent(gEvent& event) {
	gEventDispatcher dispatcher(event);
	dispatcher.dispatch<gKeyPressedEvent>(G_BIND_FUNCTION(onKeyPressed));
	dispatcher.dispatch<gKeyReleasedEvent>(G_BIND_FUNCTION(onKeyReleased));
	dispatcher.dispatch<gMouseMovedEvent>(G_BIND_FUNCTION(onMouseMoved));
	dispatcher.dispatch<gJoystickConnectEvent>(G_BIND_FUNCTION(onJoystickConnect));
	dispatcher.dispatch<gJoystickDisconnectEvent>(G_BIND_FUNCTION(onJoystickDisconnect));
}

bool gInputManager::onKeyPressed(gKeyPressedEvent& event) {
	int key = event.getKeyCode();
	if(key >= 0 && key < 512) {
		keys[key] = true;
		keysjustpressed[key] = true;
	}
	return false;
}

bool gInputManager::onKeyReleased(gKeyReleasedEvent& event) {
	int key = event.getKeyCode();
	if(key >= 0 && key < 512) {
		keys[key] = false;
	}
	return false;
}

bool gInputManager::onMouseMoved(gMouseMovedEvent& event) {
	mousedeltax += event.getX();
	mousedeltay += event.getY();
	return false;
}

bool gInputManager::onJoystickConnect(gJoystickConnectEvent& event) {
	int id = event.getJoystickId();
	if(id >= 0 && id < MAXGAMEPADS) {
		gamepadstate[id].connected = true;
		for(int b = 0; b < MAXGAMEPADBUTTONS; b++) {
			gamepadstate[id].buttons[b] = false;
			gamepadstate[id].previousbuttons[b] = false;
			gamepadstate[id].buttonsjustpressed[b] = false;
		}
		for(int a = 0; a < MAXGAMEPADAXES; a++) {
			gamepadstate[id].axes[a] = 0.0f;
		}
	}
	return false;
}

bool gInputManager::onJoystickDisconnect(gJoystickDisconnectEvent& event) {
	int id = event.getJoystickId();
	if(id >= 0 && id < MAXGAMEPADS) {
		gamepadstate[id].connected = false;
	}
	return false;
}

