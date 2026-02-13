/*
 * gInputManager.h
 *
 *  Created on: Feb 9, 2026
 *      Author: Metehan Gezer
 */

#ifndef ENGINE_CORE_GINPUTMANAGER_H_
#define ENGINE_CORE_GINPUTMANAGER_H_

#include "gEventHook.h"
#include "gWindowEvents.h"
#include "gInputPlayer.h"
#include <map>
#include <vector>
#include <string>

/**
 * Central input manager that tracks raw device state and manages per-player
 * input contexts. Supports keyboard+mouse and up to 16 gamepads.
 *
 * Player 0 is created automatically and assigned to keyboard+mouse.
 * Additional players can be created for local co-op.
 *
 * Example - single player with keyboard + gamepad fallback:
 * @code
 * gInputPlayer* p = inputmanager->getPlayer(0);
 * p->setGamepadId(0);
 * p->addAxisBinding("horizontal", G_KEY_A, G_KEY_D);
 * p->addGamepadAxisBinding("horizontal", G_GAMEPAD_AXIS_LEFT_X);
 * @endcode
 *
 * Example - local co-op:
 * @code
 * gInputPlayer* p1 = inputmanager->getPlayer(1);
 * p1->setGamepadId(0);
 * p1->addGamepadAxisBinding("move", G_GAMEPAD_AXIS_LEFT_X);
 * p1->addGamepadActionBinding("jump", G_GAMEPAD_BUTTON_A);
 * @endcode
 *
 */
class gInputManager : public gEventHook {
public:
	static const int MAXGAMEPADS = 16;
	static const int MAXGAMEPADBUTTONS = 15;
	static const int MAXGAMEPADAXES = 6;

	typedef gInputPlayer::AxisBinding AxisBinding;

	gInputManager();
	~gInputManager();

	// Player management

	/**
	 * Returns the player at the given index. Creates it if it doesn't exist.
	 * Player 0 always exists and has KB+M assigned by default.
	 */
	gInputPlayer* getPlayer(int playerIndex);

	/**
	 * Returns the number of players currently created.
	 */
	int getPlayerCount() const;

	// Raw device state

	bool isKeyPressed(int keyCode) const;
	bool isKeyJustPressed(int keyCode) const;
	void getMouseDelta(float& dx, float& dy) const;

	bool isGamepadConnected(int gamepadId) const;
	bool isGamepadButtonPressed(int gamepadId, int buttonId) const;
	bool isGamepadButtonJustPressed(int gamepadId, int buttonId) const;
	float getGamepadAxisValue(int gamepadId, int axisId) const;

	/**
	 * Resets per-frame state and polls gamepads.
	 * Called by gAppManager at the start of each tick.
	 */
	void update();

private:
	void onEvent(gEvent& event) override;
	bool onKeyPressed(gKeyPressedEvent& event);
	bool onKeyReleased(gKeyReleasedEvent& event);
	bool onMouseMoved(gMouseMovedEvent& event);
	bool onJoystickConnect(gJoystickConnectEvent& event);
	bool onJoystickDisconnect(gJoystickDisconnectEvent& event);

	void pollGamepads();

	// Keyboard + mouse state
	bool keys[512];
	bool keysjustpressed[512];
	float mousedeltax;
	float mousedeltay;

	// Gamepad state
	struct GamepadState {
		bool connected;
		bool buttons[MAXGAMEPADBUTTONS];
		bool previousbuttons[MAXGAMEPADBUTTONS];
		bool buttonsjustpressed[MAXGAMEPADBUTTONS];
		float axes[MAXGAMEPADAXES];
	};
	GamepadState gamepadstate[MAXGAMEPADS];

	// Players
	std::vector<gInputPlayer*> players;
};

extern gInputManager* inputmanager;

#endif /* ENGINE_CORE_GINPUTMANAGER_H_ */
