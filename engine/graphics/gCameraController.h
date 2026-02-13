/*
 * gCameraController.h
 *
 *  Created on: Feb 5, 2026
 *      Author: Metehan Gezer
 */

#ifndef ENGINE_GRAPHICS_GCAMERACONTROLLER_H_
#define ENGINE_GRAPHICS_GCAMERACONTROLLER_H_

#include "gRenderObject.h"
#include "gCamera.h"
#include "gWindowEvents.h"
#include <string>

/**
 * First-person camera controller with configurable input bindings.
 *
 * Uses gInputManager player-based axes for movement, mouse look, and
 * gamepad right-stick look. All bindings can be rebound at runtime.
 *
 * Default keyboard bindings (player 0):
 *   AXIS_HORIZONTAL  : A(-) / D(+)
 *   AXIS_VERTICAL    : S(-) / W(+)
 *   AXIS_ELEVATION   : LShift(-) / Space(+)
 *   ACTION_TOGGLE    : ESC
 *
 * Default gamepad bindings (when gamepad assigned):
 *   AXIS_HORIZONTAL  : Left Stick X
 *   AXIS_VERTICAL    : Left Stick Y (inverted)
 *   AXIS_ELEVATION   : LB(-) / RB(+)
 *   AXIS_LOOK_HORIZONTAL : Right Stick X
 *   AXIS_LOOK_VERTICAL   : Right Stick Y (inverted)
 *   ACTION_TOGGLE    : Start
 *
 * Example - basic usage:
 * @code
 * gCamera camera;
 * gCameraController camcontroller;
 * camcontroller.setCamera(&camera);
 * // In update():
 * camcontroller.update();
 * @endcode
 *
 * Example - local co-op with gamepad:
 * @code
 * gCameraController camcontroller2;
 * camcontroller2.setPlayerIndex(1);
 * inputmanager->getPlayer(1)->setGamepadId(0);
 * camcontroller2.setCamera(&camera2);
 * @endcode
 *
 * Example - rebinding axes:
 * @code
 * inputmanager->rebindAxis(gCameraController::AXIS_HORIZONTAL, G_KEY_LEFT, G_KEY_RIGHT);
 * @endcode
 */
class gCameraController : public gRenderObject {
public:
	gCameraController();
	gCameraController(gCamera* camera);
	~gCameraController();

	void setCamera(gCamera* camera);
	gCamera* getCamera() const;

	/**
	 * Updates camera position and orientation based on current input state.
	 * Call this once per frame from your canvas update().
	 */
	void update();

	/**
	 * Enables or disables the controller. When enabled and the player has
	 * KB+M assigned, the cursor switches to relative mode for mouse look.
	 */
	void setEnabled(bool enabled);
	bool isEnabled() const;

	/**
	 * Which player index this controller reads input from. Default: 0.
	 * Set before calling update() for the first time, or call registerInputs()
	 * after changing to re-register default bindings on the new player.
	 */
	void setPlayerIndex(int playerIndex);
	int getPlayerIndex() const;

	/**
	 * Registers default keyboard and gamepad bindings on the current player.
	 * Called automatically in the constructor. Call manually after changing
	 * the player index if you want the defaults re-registered.
	 */
	void registerInputs();

	/**
	 * Movement speed in units per second. Default: 2.0
	 */
	void setMoveSpeed(float speed);
	float getMoveSpeed() const;

	/**
	 * Mouse look sensitivity multiplier. Default: 2.0
	 */
	void setMouseSensitivity(float sensitivity);
	float getMouseSensitivity() const;

	/**
	 * Gamepad right-stick look speed in radians per second at full deflection.
	 * Default: 3.0
	 */
	void setGamepadLookSensitivity(float sensitivity);
	float getGamepadLookSensitivity() const;

	static const std::string AXIS_HORIZONTAL;        ///< Strafe. Default: A(-)/D(+), Left Stick X
	static const std::string AXIS_VERTICAL;          ///< Forward/back. Default: S(-)/W(+), Left Stick Y
	static const std::string AXIS_ELEVATION;         ///< Up/down. Default: LShift(-)/Space(+), LB(-)/RB(+)
	static const std::string AXIS_LOOK_HORIZONTAL;   ///< Gamepad look horizontal. Default: Right Stick X
	static const std::string AXIS_LOOK_VERTICAL;     ///< Gamepad look vertical. Default: Right Stick Y
	static const std::string ACTION_TOGGLE;          ///< Toggle on/off. Default: ESC, Start

private:
	void updateCamera();

	gCamera* camera;
	bool enabled;
	int playerindex;
	float movespeed;
	float mousesensitivity;
	float gamepadlooksensitivity;
	float yaw;
	float pitch;
	glm::vec3 position;
};

#endif /* ENGINE_GRAPHICS_GCAMERACONTROLLER_H_ */
