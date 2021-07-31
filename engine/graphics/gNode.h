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
	void move(const glm::vec3 dv);
	void rotate(const glm::quat& q);
	void rotateDeg(float angle, float ax, float ay, float az);
	void rotate(float radians, float ax, float ay, float az);
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
	 * Rotation around local x axis
	 *
	 * @param radians rotation amount in gl units
	 */
	void tilt(float radians);

	/**
	 *
	 */
	void tiltDeg(float angle);
	/*
	 * Rotation around local y axis
	 *
	 * @param radians rotation amount in gl units
	 */
	void pan(float radians);

	/**
	 *
	 */
	void panDeg(float angle);
	/*
	 * Rotation around local z axis
	 *
	 * @param radians rotation amount in gl units
	 */
	void roll(float radians);

	/**
	 *
	 */
	void rollDeg(float angle);

	float getPosX();
	float getPosY();
	float getPosZ();
	glm::vec3 getPosition();
	glm::quat getOrientation();
	glm::vec3 getScale();

	/**
	 * Returns truck direction as a scalar vec3.
	 */
	glm::vec3 getScalarDirectionX();

	/**
	 * Returns boom direction as a scalar vec3.
	 */
	glm::vec3 getScalarDirectionY();

	/**
	 * Returns dolly direction as a scalar vec3.
	 */
	glm::vec3 getScalarDirectionZ();

	void setTransformationMatrix(glm::mat4 transformationMatrix);
	glm::mat4 getTransformationMatrix();

	int getId();

	void setParent(gNode* parent);
	gNode* getParent();
	void removeParent();

	void setEnabled(bool isEnabled);
	bool isEnabled();

	void pushMatrix();
	void popMatrix();

protected:
	gNode* parent;
	glm::mat4 localtransformationmatrix;

	bool isenabled;

	glm::vec3 position;
	glm::quat orientation;
	glm::vec3 scalevec;

private:
	static int lastid;
	int id;
	void processTransformationMatrix();

/*
	inline const glm::vec3 & toGlm(const glm::vec3 & v){
		return *reinterpret_cast<const glm::vec3*>(&v);
	}
*/
};

#endif /* ENGINE_BASE_GNODE_H_ */
