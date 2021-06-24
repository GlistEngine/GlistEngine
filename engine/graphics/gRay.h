/*
 * Copyright (C) 2016 Nitra Games Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef GRAPHICS_GRAY_H_
#define GRAPHICS_GRAY_H_

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

class gMesh;
class gBoundingBox;


class gRay {
public:
	gRay();
	gRay(const glm::vec3& originPoint, const glm::vec3& directionVector);
	virtual ~gRay();

	void setOrigin(const glm::vec3& originPoint);
	void setDirection(const glm::vec3& directionVector);
	glm::vec3 getOrigin();
	glm::vec3 getDirection();

	bool intersects(gBoundingBox& boundingBox);
	float distance(gBoundingBox& boundingBox);

	void link(gMesh* mesh, float rayScale = 1.0f, bool isNormalized = false, float dx = 0.0f, float dy = 0.0f, float dz = 0.0f);
	void unlink();

private:
	glm::vec3 origin, direction;
	bool islinked;
	gMesh* linkedmesh;
	float scale;
	bool isnormalized;
	glm::vec3 pdiff;
};

#endif /* GRAPHICS_GRAY_H_ */
