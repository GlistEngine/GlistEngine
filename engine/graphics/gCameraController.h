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

class gCameraController : public gRenderObject {
public:
	gCameraController();
	gCameraController(gCamera* camera);
	~gCameraController();

	void setCamera(gCamera* camera);
	gCamera* getCamera() const;

	void update();

	void onEvent(gEvent& event);
	bool onMouseMoved(gMouseMovedEvent& event);
	void onKeyPressed(int key);
	void onKeyReleased(int key);

	void setEnabled(bool enabled);
	bool isEnabled() const;

	void setMoveSpeed(float speed);
	float getMoveSpeed() const;

	void setMouseSensitivity(float sensitivity);
	float getMouseSensitivity() const;

private:
	gCamera* camera;
	bool enabled;
	float movespeed;
	float mousesensitivity;
	float yaw;
	float pitch;
	glm::vec3 position;
	bool keys[512];

	void updateCamera();
};

#endif /* ENGINE_GRAPHICS_GCAMERACONTROLLER_H_ */
