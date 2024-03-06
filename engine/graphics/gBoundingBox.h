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

#ifndef GRAPHICS_GBOUNDINGBOX_H_
#define GRAPHICS_GBOUNDINGBOX_H_

#include <algorithm>
#include "gRenderObject.h"
#include "gRay.h"
#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>


/**
 * gBoundingBox represents an axis aligned box (AABB) defined by 6 parameters for the minimum and maximum
 * values of its corners. Abounding box has 3 size parameters: width, height and depth; that are calculated
 * automatically after setting min and max values. One can use gBoundingBox in calculating broad distance
 * intersection(rejection test) of the objects.
 */
class gBoundingBox : public gRenderObject {
public:
	gBoundingBox();
	gBoundingBox(float minX, float minY, float minZ, float maxX, float maxY, float maxZ);
	gBoundingBox(float minX, float minY, float minZ, float maxX, float maxY, float maxZ, glm::mat4 transformationMatrix);
	gBoundingBox(const gBoundingBox& b);
	virtual ~gBoundingBox();

	void set(float minX, float minY, float minZ, float maxX, float maxY, float maxZ);
	void set(const gBoundingBox& b);

	static bool intersects(float minX1, float minY1, float minZ1, float maxX1, float maxY1, float maxZ1, float minX2, float minY2, float minZ2, float maxX2, float maxY2, float maxZ2);
	static bool intersects(const gBoundingBox& b1, const gBoundingBox& b2);

	bool intersects(float minX, float minY, float minZ, float maxX, float maxY, float maxZ);
	bool intersects(const gBoundingBox& b);

	static bool contains(float minX1, float minY1, float minZ1, float maxX1, float maxY1, float maxZ1, float minX2, float minY2, float minZ2, float maxX2, float maxY2, float maxZ2);
	static bool contains(const gBoundingBox& b1, const gBoundingBox& b2);

	bool contains(float minX, float minY, float minZ, float maxX, float maxY, float maxZ);
	bool contains(const gBoundingBox& b);
	bool contains(float x, float y, float z);

	gBoundingBox merge(const gBoundingBox& other);

	bool intersects(gRay& ray);
	float distance(gRay& ray);
	bool intersects(gRay* ray);
	float distance(gRay* ray);

	const glm::vec3& getMin() const;
	const glm::vec3& getMax() const;
	float minX() const;
	float minY() const;
	float minZ() const;
	float maxX() const;
	float maxY() const;
	float maxZ() const;
	float getWidth()  const;
	float getHeight() const;
	float getDepth() const;
	const glm::vec3& getOrigin() const;

	void setTransformationMatrix(const glm::mat4& matrix);
	bool intersectsOBB(gRay* ray);
	float distanceOBB(gRay* ray);

	void draw();
	void drawOBB();

private:
	static const int componentnum = 3;
	glm::vec3 minf, maxf;
	float width, height, depth;
	float widthhalf, heighthalf, depthhalf;
	glm::vec3 origin;
	glm::mat4 transformationmatrix;

	float inverted, direction1, direction2, tempdirection;
	float dimlo, dimhi, dimtemp;
	int di;
	glm::vec3 ro, rd;
	float dmin, dmax;
	float raydist;
};

#endif /* GRAPHICS_GBOUNDINGBOX_H_ */
