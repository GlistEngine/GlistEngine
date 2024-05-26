/*
 * gTransformable.h
 *
 *  Created on: Aug 25, 2020
 *      Author: noyan
 */

#ifndef ENGINE_BASE_GNODE_H_
#define ENGINE_BASE_GNODE_H_

#include "gRenderObject.h"


class gNode : public gRenderObject {
public:
	gNode();
	virtual ~gNode();

	void move(float dx, float dy, float dz);
	void move(const glm::vec3& dv);
	void rotate(const glm::quat& q);
	void rotate(float radians, float ax, float ay, float az);
	void rotateDeg(float degrees, float ax, float ay, float az);
	void rotateAround(float radians, const glm::vec3& axis, const glm::vec3& point);
	void rotateAroundDeg(float degrees, const glm::vec3& axis, const glm::vec3& point);
	void scale(float sx, float sy, float sz);
	void scale(float s);

	void setPosition(float px, float py, float pz);
	void setPosition(const glm::vec3& pv);
	void setOrientation(const glm::quat& o);
	void setOrientation(const glm::vec3& angles);
	void setScale(const glm::vec3& s);
	void setScale(float sx, float sy, float sz);
	void setScale(float s);

	/*
	 * Movement right-left along x axis
	 *
	 * @param distance movement amount in gl units
	 */
	void truck(float distance);

	/*
	 * Movement up-down along y axis
	 *
	 * @param distance movement amount in gl units
	 */
	void boom(float distance);

	/*
	 * Movement forward-backward along z axis
	 *
	 * @param distance movement amount in gl units
	 */
	void dolly(float distance);

	/*
	 * Rotation around local x axis in radians
	 *
	 * @param angle in radians
	 */
	void tilt(float radians);

	/*
	 * Rotation around local x axis in degrees
	 *
	 * @param angle in degrees
	 */
	void tiltDeg(float degrees);

	/*
	 * Rotation around local y axis in radians
	 *
	 * @param angle in radians
	 */
	void pan(float radians);

	/*
	 * Rotation around local y axis in degrees
	 *
	 * @param angle in degrees
	 */
	void panDeg(float degrees);

	/*
	 * Rotation around local z axis in radians
	 *
	 * @param angle in radians
	 */
	void roll(float radians);

	/*
	 * Rotation around local z axis in degrees
	 *
	 * @param angle in degrees
	 */
	void rollDeg(float degrees);

	float getPosX() const;
	float getPosY() const;
	float getPosZ() const;
	const glm::vec3& getPosition() const;
	const glm::quat& getOrientation() const;
	const glm::vec3& getScale() const;

	/**
	 * Returns truck direction as a scalar vec3.
	 */
	glm::vec3 getScalarDirectionX() const;

	/**
	 * Returns boom direction as a scalar vec3.
	 */
	glm::vec3 getScalarDirectionY() const;

	/**
	 * Returns dolly direction as a scalar vec3.
	 */
	glm::vec3 getScalarDirectionZ() const;

	void setTransformationMatrix(const glm::mat4& transformationMatrix);
	const glm::mat4& getTransformationMatrix() const;

	int getId() const;

	void setParent(gNode* parent);
	gNode* getParent() const;
	void removeParent();

	void setEnabled(bool isEnabled);
	bool isEnabled() const;

	void pushMatrix() const;
	void popMatrix() const;

protected:
	gNode* parent;
	glm::mat4 localtransformationmatrix;

	bool isenabled;

	glm::vec3 position;
	glm::quat orientation;
	glm::vec3 scalevec;

	virtual void processTransformationMatrix();

private:
	static int lastid;
	int id;
};

#endif /* ENGINE_BASE_GNODE_H_ */
