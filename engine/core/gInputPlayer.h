/*
 * gInputPlayer.h
 *
 *  Created on: Feb 9, 2026
 *      Author: Metehan Gezer
 */

#ifndef ENGINE_CORE_GINPUTPLAYER_H_
#define ENGINE_CORE_GINPUTPLAYER_H_

#include <map>
#include <vector>
#include <string>

class gInputManager;

/**
 * Per-player input context with device assignment and bindings.
 *
 * Each player can be assigned keyboard+mouse, a specific gamepad, or both.
 * Bindings are per-player, so multiple players can use the same action/axis
 * names with different devices.
 *
 * Example - single player with keyboard + gamepad:
 * @code
 * gInputPlayer* p = inputmanager->getPlayer(0);
 * p->addAxisBinding("horizontal", G_KEY_A, G_KEY_D);
 * p->addGamepadAxisBinding("horizontal", G_GAMEPAD_AXIS_LEFT_X);
 * float h = p->getAxisValue("horizontal");
 * @endcode
 *
 * Example - local co-op setup:
 * @code
 * gInputPlayer* p0 = inputmanager->getPlayer(0);
 * p0->addAxisBinding("move", G_KEY_A, G_KEY_D);
 *
 * gInputPlayer* p1 = inputmanager->getPlayer(1);
 * p1->setGamepadId(0);
 * p1->addGamepadAxisBinding("move", G_GAMEPAD_AXIS_LEFT_X);
 * @endcode
 */
class gInputPlayer {
public:
	struct AxisBinding {
		int negativekey;
		int positivekey;
	};

	struct GamepadAxisBinding {
		int axisid;
		bool inverted;
	};

	struct GamepadButtonAxisBinding {
		int negativebuttonid;
		int positivebuttonid;
	};

	gInputPlayer(gInputManager* manager, int playerindex);
	~gInputPlayer();

	int getPlayerIndex() const;

	// Device assignment

	/**
	 * Assigns or unassigns keyboard+mouse to this player.
	 * Only player 0 has KB+M assigned by default.
	 */
	void setKeyboardMouseAssigned(bool assigned);
	bool isKeyboardMouseAssigned() const;

	/**
	 * Assigns a gamepad by GLFW joystick ID (0-15). Pass -1 to unassign.
	 */
	void setGamepadId(int id);
	int getGamepadId() const;
	bool hasGamepad() const;

	// Deadzone

	/**
	 * Analog stick deadzone. Values below this threshold read as 0.
	 * Default: 0.1
	 */
	void setDeadzone(float dz);
	float getDeadzone() const;

	// Keyboard action bindings

	void addActionBinding(const std::string& action, int keyCode);
	void removeActionBinding(const std::string& action, int keyCode);
	void clearActionBindings(const std::string& action);
	void rebindAction(const std::string& action, int keyCode);

	// Gamepad button action bindings

	void addGamepadActionBinding(const std::string& action, int buttonId);
	void removeGamepadActionBinding(const std::string& action, int buttonId);
	void clearGamepadActionBindings(const std::string& action);

	// Keyboard axis bindings (neg/pos key pairs)

	void addAxisBinding(const std::string& axis, int negativeKey, int positiveKey);
	void removeAxisBinding(const std::string& axis, int negativeKey, int positiveKey);
	void clearAxisBindings(const std::string& axis);
	void rebindAxis(const std::string& axis, int negativeKey, int positiveKey);

	// Gamepad analog axis bindings

	/**
	 * Binds a gamepad analog axis (stick or trigger) to a named axis.
	 * Set inverted=true to negate the value (e.g. Y-axis where up is negative).
	 */
	void addGamepadAxisBinding(const std::string& axis, int gamepadAxisId, bool inverted = false);
	void removeGamepadAxisBinding(const std::string& axis, int gamepadAxisId);
	void clearGamepadAxisBindings(const std::string& axis);

	// Gamepad button axis bindings (d-pad as axis)

	void addGamepadButtonAxisBinding(const std::string& axis, int negativeButtonId, int positiveButtonId);
	void removeGamepadButtonAxisBinding(const std::string& axis, int negativeButtonId, int positiveButtonId);
	void clearGamepadButtonAxisBindings(const std::string& axis);

	// Unified queries

	/**
	 * Returns true if the action is pressed via any bound source
	 * (keyboard key if KB+M assigned, gamepad button if gamepad assigned).
	 */
	bool isActionPressed(const std::string& action) const;

	/**
	 * Returns true if the action was first pressed this frame.
	 */
	bool isActionJustPressed(const std::string& action) const;

	/**
	 * Returns combined axis value in [-1, +1] from all binding types.
	 * Keyboard keys contribute +/-1, gamepad analog uses deadzone,
	 * gamepad buttons contribute +/-1.
	 */
	float getAxisValue(const std::string& axis) const;

	/**
	 * Returns mouse delta if this player has KB+M assigned, otherwise (0, 0).
	 */
	void getMouseDelta(float& dx, float& dy) const;

	// Binding getters

	const std::vector<int>& getActionBindings(const std::string& action) const;
	const std::vector<AxisBinding>& getAxisBindings(const std::string& axis) const;

private:
	gInputManager* inputmanager;
	int playerindex;
	bool keyboardmouseassigned;
	int gamepadid;
	float deadzone;

	std::map<std::string, std::vector<int>> actionbindings;
	std::map<std::string, std::vector<int>> gamepadactionbindings;
	std::map<std::string, std::vector<AxisBinding>> axisbindings;
	std::map<std::string, std::vector<GamepadAxisBinding>> gamepadaxisbindings;
	std::map<std::string, std::vector<GamepadButtonAxisBinding>> gamepadbuttonaxisbindings;

	static const std::vector<int> emptybindings;
	static const std::vector<AxisBinding> emptyaxisbindings;

	float applyDeadzone(float value) const;
};

#endif /* ENGINE_CORE_GINPUTPLAYER_H_ */
