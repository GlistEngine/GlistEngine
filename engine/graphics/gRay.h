/*
 * gRay.h
 *
 *  Created on: Mar 16, 2021
 *      Author: noyan
 */

#ifndef GRAPHICS_GRAY_H_
#define GRAPHICS_GRAY_H_

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>


class gRay {
public:
	gRay();
	gRay(const glm::vec3& originPoint, const glm::vec3& directionVector);
	virtual ~gRay();

	void setOrigin(const glm::vec3& originPoint);
	void setDirection(const glm::vec3& directionVector);
	glm::vec3 getOrigin();
	glm::vec3 getDirection();

private:
	glm::vec3 origin, direction;
};

#endif /* GRAPHICS_GRAY_H_ */
