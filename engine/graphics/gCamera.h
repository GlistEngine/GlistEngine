/*
 * gCamera.h
 *
 *  Created on: May 10, 2020
 *      Author: noyan
 */

#ifndef ENGINE_GRAPHICS_GCAMERA_H_
#define ENGINE_GRAPHICS_GCAMERA_H_

//#include <GL/glew.h>
#include "gNode.h"
#include "gSkybox.h"

class gCamera : public gNode {
public:
//	gCamera();
    gCamera(glm::vec3 camPosition = glm::vec3(0.0f, 0.0f, 0.0f));
    gCamera(float posX, float posY, float posZ);
	virtual ~gCamera();

	void begin();
	void end();

	void setFov(float f);
	void setNearClip(float nearClip);
    void setFarClip(float farClip);

	void move(float dx, float dy, float dz);
	void move(const glm::vec3 dv);
	void rotate(const glm::quat& q);
	void rotate(float radians, float ax, float ay, float az);
	void rotateDeg(float angle, float ax, float ay, float az);
	void rotateAround(float radians, const glm::vec3& axis, const glm::vec3& point);
	void scale(float sx, float sy, float sz);
	void scale(float s);

	void setPosition(float px, float py, float pz);
	void setPosition(const glm::vec3 pv);
	void setOrientation(const glm::quat& o);
	void setOrientation(const glm::vec3& angles);
	void setScale(const glm::vec3& s);
	void setScale(float sx, float sy, float sz);
	void setScale(float s);

	void dolly(float distance);
	void truck(float distance);
	void boom(float distance);
	void tilt(float radians);
	void pan(float radians);
	void roll(float radians);
	void tiltDeg(float degrees);
	void panDeg(float degrees);
	void rollDeg(float degrees);

	void lookAt(const glm::vec3& point);
	void rotateLook(float angle, float ax, float ay, float az);
	void resetLook();
	glm::mat4 getLookMatrix();
	glm::quat getLookOrientation();

	void drawGizmos();

private:
    float fov, nearclip, farclip;

	glm::vec3 lookposition;
	glm::quat lookorientation;
	glm::vec3 lookscalevec;
    glm::mat4 locallookmatrix;
    void processLookMatrix();


    gSkybox* gizmos;

	void rotateGizmos(const glm::quat& o);
	void rotateGizmos(float radians, float ax, float ay, float az);
	void rotateDegGizmos(float angle, float ax, float ay, float az);
	void setOrientationGizmos(const glm::quat& o);

	void tiltGizmos(float radians);
	void panGizmos(float radians);
	void rollGizmos(float radians);

	void tiltDegGizmos(float degrees);
	void panDegGizmos(float degrees);
	void rollDegGizmos(float degrees);

};
#endif /* ENGINE_GRAPHICS_GCAMERA_H_ */
