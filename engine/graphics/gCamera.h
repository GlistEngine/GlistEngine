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
#include "gBoundingBox.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

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
    float getNearClip() const {return nearclip;} // needed for grid
    float getFarClip() const {return farclip;} // needed for grid
    float getFov() const {return fov;}

	void move(float dx, float dy, float dz);
	void move(const glm::vec3 dv);
	void rotate(const glm::quat& q);
	void rotate(float radians, float ax, float ay, float az);
	void rotateDeg(float angle, float ax, float ay, float az);
	void rotateAround(float radians, const glm::vec3& axis, const glm::vec3& point);
	void rotateAroundDeg(float degrees, const glm::vec3& axis, const glm::vec3& point);
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
	const glm::mat4& getLookMatrix() const;
	const glm::quat& getLookOrientation() const;

	void drawGizmos();

	bool isInFrustum(const gBoundingBox& box) const;

protected:
    float fov, nearclip, farclip;

	struct Plane {
		glm::vec3 normal;
		float distance;

		Plane() = default;

		Plane(const glm::vec3& p1, const glm::vec3& norm)
			: normal(glm::normalize(norm)),
			  distance(glm::dot(normal, p1)) {
		}

		float getSignedDistanceToPlane(const glm::vec3& point) const {
			return glm::dot(normal, point) - distance;
		}

		bool checkAABB(const gBoundingBox& aabb) const {
			// Compute the projection interval radius of b onto L(t) = b.c + t * p.n
			const float r = aabb.getWidth() * std::abs(normal.x) + aabb.getHeight() * std::abs(normal.y) +
							aabb.getDepth() * std::abs(normal.z);
			return -r <= getSignedDistanceToPlane(aabb.getOrigin());
		}
	};
	struct Frustum {
		Plane topFace;
		Plane bottomFace;

		Plane rightFace;
		Plane leftFace;

		Plane farFace;
		Plane nearFace;
	};

	Frustum frustum;

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
