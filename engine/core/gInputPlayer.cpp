/*
 * gInputPlayer.cpp
 *
 *  Created on: Feb 9, 2026
 *      Author: Metehan Gezer
 */

#include "gInputPlayer.h"
#include "gInputManager.h"
#include <algorithm>
#include <cmath>

const std::vector<int> gInputPlayer::emptybindings;
const std::vector<gInputPlayer::AxisBinding> gInputPlayer::emptyaxisbindings;

gInputPlayer::gInputPlayer(gInputManager* manager, int playerindex)
	: inputmanager(manager), playerindex(playerindex),
	  keyboardmouseassigned(false), gamepadid(-1), deadzone(0.1f) {
}

gInputPlayer::~gInputPlayer() {
}

int gInputPlayer::getPlayerIndex() const {
	return playerindex;
}

// ---- Device assignment ----

void gInputPlayer::setKeyboardMouseAssigned(bool assigned) {
	keyboardmouseassigned = assigned;
}

bool gInputPlayer::isKeyboardMouseAssigned() const {
	return keyboardmouseassigned;
}

void gInputPlayer::setGamepadId(int id) {
	gamepadid = id;
}

int gInputPlayer::getGamepadId() const {
	return gamepadid;
}

bool gInputPlayer::hasGamepad() const {
	return gamepadid >= 0;
}

void gInputPlayer::setDeadzone(float dz) {
	deadzone = dz;
}

float gInputPlayer::getDeadzone() const {
	return deadzone;
}

float gInputPlayer::applyDeadzone(float value) const {
	if(deadzone <= 0.0f) return value;
	float abs = value < 0.0f ? -value : value;
	if(abs < deadzone) return 0.0f;
	float sign = value < 0.0f ? -1.0f : 1.0f;
	return sign * (abs - deadzone) / (1.0f - deadzone);
}

void gInputPlayer::addActionBinding(const std::string& action, int keyCode) {
	std::vector<int>& bindings = actionbindings[action];
	for(int i = 0; i < (int)bindings.size(); i++) {
		if(bindings[i] == keyCode) return;
	}
	bindings.push_back(keyCode);
}

void gInputPlayer::removeActionBinding(const std::string& action, int keyCode) {
	auto it = actionbindings.find(action);
	if(it == actionbindings.end()) return;
	std::vector<int>& bindings = it->second;
	bindings.erase(std::remove(bindings.begin(), bindings.end(), keyCode), bindings.end());
}

void gInputPlayer::clearActionBindings(const std::string& action) {
	auto it = actionbindings.find(action);
	if(it != actionbindings.end()) it->second.clear();
}

void gInputPlayer::rebindAction(const std::string& action, int keyCode) {
	actionbindings[action].clear();
	actionbindings[action].push_back(keyCode);
}

void gInputPlayer::addGamepadActionBinding(const std::string& action, int buttonId) {
	std::vector<int>& bindings = gamepadactionbindings[action];
	for(int i = 0; i < (int)bindings.size(); i++) {
		if(bindings[i] == buttonId) return;
	}
	bindings.push_back(buttonId);
}

void gInputPlayer::removeGamepadActionBinding(const std::string& action, int buttonId) {
	auto it = gamepadactionbindings.find(action);
	if(it == gamepadactionbindings.end()) return;
	std::vector<int>& bindings = it->second;
	bindings.erase(std::remove(bindings.begin(), bindings.end(), buttonId), bindings.end());
}

void gInputPlayer::clearGamepadActionBindings(const std::string& action) {
	auto it = gamepadactionbindings.find(action);
	if(it != gamepadactionbindings.end()) it->second.clear();
}

// ---- Keyboard axis bindings ----

void gInputPlayer::addAxisBinding(const std::string& axis, int negativeKey, int positiveKey) {
	std::vector<AxisBinding>& bindings = axisbindings[axis];
	for(int i = 0; i < (int)bindings.size(); i++) {
		if(bindings[i].negativekey == negativeKey && bindings[i].positivekey == positiveKey) return;
	}
	AxisBinding ab;
	ab.negativekey = negativeKey;
	ab.positivekey = positiveKey;
	bindings.push_back(ab);
}

void gInputPlayer::removeAxisBinding(const std::string& axis, int negativeKey, int positiveKey) {
	auto it = axisbindings.find(axis);
	if(it == axisbindings.end()) return;
	std::vector<AxisBinding>& bindings = it->second;
	for(int i = 0; i < (int)bindings.size(); i++) {
		if(bindings[i].negativekey == negativeKey && bindings[i].positivekey == positiveKey) {
			bindings.erase(bindings.begin() + i);
			return;
		}
	}
}

void gInputPlayer::clearAxisBindings(const std::string& axis) {
	auto it = axisbindings.find(axis);
	if(it != axisbindings.end()) it->second.clear();
}

void gInputPlayer::rebindAxis(const std::string& axis, int negativeKey, int positiveKey) {
	axisbindings[axis].clear();
	AxisBinding ab;
	ab.negativekey = negativeKey;
	ab.positivekey = positiveKey;
	axisbindings[axis].push_back(ab);
}

void gInputPlayer::addGamepadAxisBinding(const std::string& axis, int gamepadAxisId, bool inverted) {
	std::vector<GamepadAxisBinding>& bindings = gamepadaxisbindings[axis];
	for(int i = 0; i < (int)bindings.size(); i++) {
		if(bindings[i].axisid == gamepadAxisId) return;
	}
	GamepadAxisBinding gab;
	gab.axisid = gamepadAxisId;
	gab.inverted = inverted;
	bindings.push_back(gab);
}

void gInputPlayer::removeGamepadAxisBinding(const std::string& axis, int gamepadAxisId) {
	auto it = gamepadaxisbindings.find(axis);
	if(it == gamepadaxisbindings.end()) return;
	std::vector<GamepadAxisBinding>& bindings = it->second;
	for(int i = 0; i < (int)bindings.size(); i++) {
		if(bindings[i].axisid == gamepadAxisId) {
			bindings.erase(bindings.begin() + i);
			return;
		}
	}
}

void gInputPlayer::clearGamepadAxisBindings(const std::string& axis) {
	auto it = gamepadaxisbindings.find(axis);
	if(it != gamepadaxisbindings.end()) it->second.clear();
}

void gInputPlayer::addGamepadButtonAxisBinding(const std::string& axis, int negativeButtonId, int positiveButtonId) {
	std::vector<GamepadButtonAxisBinding>& bindings = gamepadbuttonaxisbindings[axis];
	for(int i = 0; i < (int)bindings.size(); i++) {
		if(bindings[i].negativebuttonid == negativeButtonId && bindings[i].positivebuttonid == positiveButtonId) return;
	}
	GamepadButtonAxisBinding gbab;
	gbab.negativebuttonid = negativeButtonId;
	gbab.positivebuttonid = positiveButtonId;
	bindings.push_back(gbab);
}

void gInputPlayer::removeGamepadButtonAxisBinding(const std::string& axis, int negativeButtonId, int positiveButtonId) {
	auto it = gamepadbuttonaxisbindings.find(axis);
	if(it == gamepadbuttonaxisbindings.end()) return;
	std::vector<GamepadButtonAxisBinding>& bindings = it->second;
	for(int i = 0; i < (int)bindings.size(); i++) {
		if(bindings[i].negativebuttonid == negativeButtonId && bindings[i].positivebuttonid == positiveButtonId) {
			bindings.erase(bindings.begin() + i);
			return;
		}
	}
}

void gInputPlayer::clearGamepadButtonAxisBindings(const std::string& axis) {
	auto it = gamepadbuttonaxisbindings.find(axis);
	if(it != gamepadbuttonaxisbindings.end()) it->second.clear();
}

bool gInputPlayer::isActionPressed(const std::string& action) const {
	// Check keyboard keys
	if(keyboardmouseassigned) {
		auto it = actionbindings.find(action);
		if(it != actionbindings.end()) {
			const std::vector<int>& bindings = it->second;
			for(int i = 0; i < (int)bindings.size(); i++) {
				if(inputmanager->isKeyPressed(bindings[i])) return true;
			}
		}
	}

	// Check gamepad buttons
	if(hasGamepad() && inputmanager->isGamepadConnected(gamepadid)) {
		auto it = gamepadactionbindings.find(action);
		if(it != gamepadactionbindings.end()) {
			const std::vector<int>& bindings = it->second;
			for(int i = 0; i < (int)bindings.size(); i++) {
				if(inputmanager->isGamepadButtonPressed(gamepadid, bindings[i])) return true;
			}
		}
	}

	return false;
}

bool gInputPlayer::isActionJustPressed(const std::string& action) const {
	// Check keyboard keys
	if(keyboardmouseassigned) {
		auto it = actionbindings.find(action);
		if(it != actionbindings.end()) {
			const std::vector<int>& bindings = it->second;
			for(int i = 0; i < (int)bindings.size(); i++) {
				if(inputmanager->isKeyJustPressed(bindings[i])) {
					return true;
				}
			}
		}
	}

	// Check gamepad buttons
	if(hasGamepad() && inputmanager->isGamepadConnected(gamepadid)) {
		auto it = gamepadactionbindings.find(action);
		if(it != gamepadactionbindings.end()) {
			const std::vector<int>& bindings = it->second;
			for(int i = 0; i < (int)bindings.size(); i++) {
				if(inputmanager->isGamepadButtonJustPressed(gamepadid, bindings[i])) {
					return true;
				}
			}
		}
	}

	return false;
}

float gInputPlayer::getAxisValue(const std::string& axis) const {
	float value = 0.0f;

	// Keyboard key pairs
	if(keyboardmouseassigned) {
		auto it = axisbindings.find(axis);
		if(it != axisbindings.end()) {
			const std::vector<AxisBinding>& bindings = it->second;
			for(int i = 0; i < (int)bindings.size(); i++) {
				if(inputmanager->isKeyPressed(bindings[i].negativekey)) value -= 1.0f;
				if(inputmanager->isKeyPressed(bindings[i].positivekey)) value += 1.0f;
			}
		}
	}

	if(hasGamepad() && inputmanager->isGamepadConnected(gamepadid)) {
		// Gamepad analog axes
		{
			auto it = gamepadaxisbindings.find(axis);
			if(it != gamepadaxisbindings.end()) {
				const std::vector<GamepadAxisBinding>& bindings = it->second;
				for(int i = 0; i < (int)bindings.size(); i++) {
					float raw = inputmanager->getGamepadAxisValue(gamepadid, bindings[i].axisid);
					float v = applyDeadzone(raw);
					if(bindings[i].inverted) v = -v;
					value += v;
				}
			}
		}

		// Gamepad button pairs (d-pad)
		{
			auto it = gamepadbuttonaxisbindings.find(axis);
			if(it != gamepadbuttonaxisbindings.end()) {
				const std::vector<GamepadButtonAxisBinding>& bindings = it->second;
				for(int i = 0; i < (int)bindings.size(); i++) {
					if(inputmanager->isGamepadButtonPressed(gamepadid, bindings[i].negativebuttonid)) value -= 1.0f;
					if(inputmanager->isGamepadButtonPressed(gamepadid, bindings[i].positivebuttonid)) value += 1.0f;
				}
			}
		}
	}

	if(value < -1.0f) value = -1.0f;
	if(value > 1.0f) value = 1.0f;
	return value;
}

void gInputPlayer::getMouseDelta(float& dx, float& dy) const {
	if(keyboardmouseassigned) {
		inputmanager->getMouseDelta(dx, dy);
	} else {
		dx = 0.0f;
		dy = 0.0f;
	}
}

// Binding getters

const std::vector<int>& gInputPlayer::getActionBindings(const std::string& action) const {
	auto it = actionbindings.find(action);
	if(it == actionbindings.end()) return emptybindings;
	return it->second;
}

const std::vector<gInputPlayer::AxisBinding>& gInputPlayer::getAxisBindings(const std::string& axis) const {
	auto it = axisbindings.find(axis);
	if(it == axisbindings.end()) return emptyaxisbindings;
	return it->second;
}
