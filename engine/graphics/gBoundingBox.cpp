/*
 * gBoundingBox.cpp
 *
 *  Created on: Mar 15, 2021
 *      Author: noyan
 */

#include "gBoundingBox.h"
#include "gRay.h"
#include "gRenderer.h"


gBoundingBox::gBoundingBox() : minf(glm::vec3(0.0f)), maxf(glm::vec3(0.0f)), width(0), height(0), depth(0), widthhalf(0), heighthalf(0), depthhalf(0), origin(glm::vec3(0.0f)) {}

gBoundingBox::gBoundingBox(float minX, float minY, float minZ, float maxX, float maxY, float maxZ) : minf(glm::vec3(minX, minY, minZ)), maxf(glm::vec3(maxX, maxY, maxZ)), width(maxX - minX), height(maxY - minY), depth(maxZ - minZ), widthhalf(width / 2.0f), heighthalf(height / 2.0f), depthhalf(depth / 2.0f), origin(glm::vec3((maxX + minX) / 2.0f, (maxY + minY) / 2.0f, (maxZ + minZ) / 2.0f)) {}

gBoundingBox::gBoundingBox(float minX, float minY, float minZ, float maxX, float maxY, float maxZ, glm::mat4 transformationMatrix) : minf(glm::vec3(minX, minY, minZ)), maxf(glm::vec3(maxX, maxY, maxZ)), width(maxX - minX), height(maxY - minY), depth(maxZ - minZ), widthhalf(width / 2.0f), heighthalf(height / 2.0f), depthhalf(depth / 2.0f), origin(glm::vec3((maxX + minX) / 2.0f, (maxY + minY) / 2.0f, (maxZ + minZ) / 2.0f)), transformationmatrix(transformationMatrix) {}

gBoundingBox::gBoundingBox(const gBoundingBox& b) : minf(glm::vec3(b.minX(), b.minY(), b.minZ())), maxf(glm::vec3(b.maxX(), b.maxY(), b.maxZ())), width(b.getWidth()), height(b.getHeight()), depth(b.getDepth()), widthhalf(width / 2.0f), heighthalf(height / 2.0f), depthhalf(depth / 2.0f), origin(b.getOrigin()) {}

gBoundingBox::~gBoundingBox() {}

void gBoundingBox::set(float minX, float minY, float minZ, float maxX, float maxY, float maxZ) {
	minf = glm::vec3(minX, minY, minZ);
	maxf = glm::vec3(maxX, maxY, maxZ);
	width = maxX - minX;
	height = maxY - minY;
	depth = maxZ - minZ;
	widthhalf = width / 2.0f;
	heighthalf = height / 2.0f;
	depthhalf = depth / 2.0f;
	origin = glm::vec3((maxX + minX) / 2.0f, (maxY + minY) / 2.0f, (maxZ + minZ) / 2.0f);
}

void gBoundingBox::set(const gBoundingBox& b) {
	minf = glm::vec3(b.minX(), b.minY(), b.minZ());
	maxf = glm::vec3(b.maxX(), b.maxY(), b.maxZ());
	width = b.getWidth();
	height = b.getHeight();
	depth = b.getDepth();
	widthhalf = width / 2.0f;
	heighthalf = height / 2.0f;
	depthhalf = depth / 2.0f;
	origin = b.getOrigin();
}

bool gBoundingBox::intersects(float minX1, float minY1, float minZ1, float maxX1, float maxY1, float maxZ1, float minX2, float minY2, float minZ2, float maxX2, float maxY2, float maxZ2) {
	return minX1 <= maxX2 && maxX1 >= minX2 && minY1 <= maxY2 && maxY1 >= minY2 && minZ1 <= maxZ2 && maxZ1 >= minZ2;
}

bool gBoundingBox::intersects(const gBoundingBox& b1, const gBoundingBox& b2) {
	return intersects(b1.minX(), b1.minY(), b1.minZ(), b1.maxX(), b1.maxY(), b1.maxZ(), b2.minX(), b2.minY(), b2.minZ(), b2.maxX(), b2.maxY(), b2.maxZ());
}

bool gBoundingBox::intersects(float minX, float minY, float minZ, float maxX, float maxY, float maxZ) {
	return intersects(minf[0], minf[1], minf[2], maxf[0], maxf[1], maxf[2], minX, minY, minZ, maxX, maxY, maxZ);
}

bool gBoundingBox::intersects(const gBoundingBox& b) {
	return intersects(minf[0], minf[1], minf[2], maxf[0], maxf[1], maxf[2], b.minX(), b.minY(), b.minZ(), b.maxX(), b.maxY(), b.maxZ());
}

bool gBoundingBox::contains(float minX1, float minY1, float minZ1, float maxX1, float maxY1, float maxZ1, float minX2, float minY2, float minZ2, float maxX2, float maxY2, float maxZ2) {
	return minX1 <= minX2 && maxX1 >= maxX2 && minY1 <= minY2 && maxY1 >= maxY2 && minZ1 <= minZ2 && maxZ1 >= maxZ2;
}

bool gBoundingBox::contains(const gBoundingBox& b1, const gBoundingBox& b2) {
	return contains(b1.minX(), b1.minY(), b1.minZ(), b1.maxX(), b1.maxY(), b1.maxZ(), b2.minX(), b2.minY(), b2.minZ(), b2.maxX(), b2.maxY(), b2.maxZ());
}

bool gBoundingBox::contains(float minX, float minY, float minZ, float maxX, float maxY, float maxZ) {
	return contains(minf[0], minf[1], minf[2], maxf[0], maxf[1], maxf[2], minX, minY, minZ, maxX, maxY, maxZ);
}

bool gBoundingBox::contains(const gBoundingBox& b) {
	return contains(minf[0], minf[1], minf[2], maxf[0], maxf[1], maxf[2], b.minX(), b.minY(), b.minZ(), b.maxX(), b.maxY(), b.maxZ());
}

bool gBoundingBox::contains(float x, float y, float z) {
	return contains(minf[0], minf[1], minf[2], maxf[0], maxf[1], maxf[2], x, y, z, x, y, z);
}

gBoundingBox gBoundingBox::merge(const gBoundingBox& other) {
	if (width == 0.0f && height == 0.0f && depth == 0.0f) return other;
	else if (other.getWidth() == 0.0f && other.getHeight() == 0.0f && other.getDepth() == 0.0f) return *this;

	return gBoundingBox(
			std::min(minf[0], other.minX()),
			std::min(minf[1], other.minY()),
			std::min(minf[2], other.minZ()),
			std::max(maxf[0], other.maxX()),
			std::max(maxf[1], other.maxY()),
			std::max(maxf[2], other.maxZ())
	);
}

bool gBoundingBox::intersects(gRay& ray) {
	raydist = distance(ray);
	return raydist == std::numeric_limits<float>::min() ? false : true && raydist <= glm::length(ray.getDirection());
}

float gBoundingBox::distance(gRay& ray) {
	ro = ray.getOrigin();
	rd = ray.getDirection();
	dmin = std::numeric_limits<float>::min();
	dmax = std::numeric_limits<float>::max();

	for (di = 0; di < componentnum; di++) {
		dimlo = (minf[di] - ro[di]) / rd[di];
		dimhi = (maxf[di] - ro[di]) / rd[di];

		if (dimlo > dimhi) {
			dimtemp = dimlo;
			dimlo = dimhi;
			dimhi = dimtemp;
		}

		if (dimhi < dmin || dimlo > dmax) {
			return std::numeric_limits<float>::min();
		}

		if (dimlo > dmin) dmin = dimlo;
		if (dimhi < dmax) dmax = dimhi;
	}

	return dmin > dmax ? std::numeric_limits<float>::min() : dmin;
}

bool gBoundingBox::intersects(gRay* ray) {
	raydist = distance(ray);
	return raydist == std::numeric_limits<float>::min() ? false : true && raydist <= ray->getLength();
}

float gBoundingBox::distance(gRay* ray) {
	ro = ray->getOrigin();
	rd = ray->getDirection();
	dmin = std::numeric_limits<float>::min();
	dmax = std::numeric_limits<float>::max();

	for (di = 0; di < componentnum; di++) {
		dimlo = (minf[di] - ro[di]) / rd[di];
		dimhi = (maxf[di] - ro[di]) / rd[di];

		if (dimlo > dimhi) {
			dimtemp = dimlo;
			dimlo = dimhi;
			dimhi = dimtemp;
		}

		if (dimhi < dmin || dimlo > dmax) {
			return std::numeric_limits<float>::min();
		}

		if (dimlo > dmin) dmin = dimlo;
		if (dimhi < dmax) dmax = dimhi;
	}

	return dmin > dmax ? std::numeric_limits<float>::min() : dmin;
}

glm::vec3 gBoundingBox::getMin() {
	return minf;
}

glm::vec3 gBoundingBox::getMax() {
	return maxf;
}

float gBoundingBox::minX() const {
	return minf[0];
}

float gBoundingBox::minY() const {
	return minf[1];
}

float gBoundingBox::minZ() const {
	return minf[2];
}

float gBoundingBox::maxX() const {
	return maxf[0];
}

float gBoundingBox::maxY() const {
	return maxf[1];
}

float gBoundingBox::maxZ() const {
	return maxf[2];
}

float gBoundingBox::getWidth() const {
	return width;
}

float gBoundingBox::getHeight() const {
	return height;
}

float gBoundingBox::getDepth() const {
	return depth;
}

glm::vec3 gBoundingBox::getOrigin() const {
	return origin;
}

void gBoundingBox::setTransformationMatrix(glm::mat4 matrix) {
	transformationmatrix = matrix;
}

bool gBoundingBox::intersectsOBB(gRay* ray) {
	raydist = distanceOBB(ray);
	return raydist == 0.0f ? false : true && raydist <= ray->getLength();
}

float gBoundingBox::distanceOBB(gRay* ray) {
    float tMin = 0.0f;
    float tMax = 100000.0f;
    float tTemp;

    glm::vec3 OBBposition_worldspace(transformationmatrix[3].x, transformationmatrix[3].y, transformationmatrix[3].z);
    glm::vec3 delta = OBBposition_worldspace - ray->getOrigin();

    {
        glm::vec3 xaxis(transformationmatrix[0].x, transformationmatrix[0].y, transformationmatrix[0].z);
        float e = glm::dot(xaxis, delta);
        float f = glm::dot(ray->getDirection(), xaxis);

        if (fabs(f) > 0.001f) {
            float t1 = (e + minX()) / f;
            float t2 = (e + maxX()) / f;

            if (t1 > t2) {
            	tTemp = t1;
            	t1 = t2;
            	t2 = tTemp;
            }

            if (t2 < tMax) tMax = t2;
            if (t1 > tMin) tMin = t1;
            if (tMin > tMax) return {};

        }
        else {
            if (-e + minX() > 0.0f || -e + maxX() < 0.0f) return {};
        }
    }


    {
        glm::vec3 yaxis(transformationmatrix[1].x, transformationmatrix[1].y, transformationmatrix[1].z);
        float e = glm::dot(yaxis, delta);
        float f = glm::dot(ray->getDirection(), yaxis);

        if (fabs(f) > 0.001f) {

            float t1 = (e + minY()) / f;
            float t2 = (e + maxY()) / f;

            if (t1 > t2) {
            	tTemp = t1;
            	t1 = t2;
            	t2 = tTemp;
            }

            if (t2 < tMax) tMax = t2;
            if (t1 > tMin) tMin = t1;
            if (tMin > tMax) return {};

        }
        else {
            if (-e + minY() > 0.0f || -e + maxY() < 0.0f) return {};
        }
    }


    {
        glm::vec3 zaxis(transformationmatrix[2].x, transformationmatrix[2].y, transformationmatrix[2].z);
        float e = glm::dot(zaxis, delta);
        float f = glm::dot(ray->getDirection(), zaxis);

        if (fabs(f) > 0.001f) {

            float t1 = (e + minZ()) / f;
            float t2 = (e + maxZ()) / f;

            if (t1 > t2) {
            	tTemp = t1;
            	t1 = t2;
            	t2 = tTemp;
            }

            if (t2 < tMax) tMax = t2;
            if (t1 > tMin) tMin = t1;
            if (tMin > tMax) return {};

        }
        else {
            if (-e + minZ() > 0.0f || -e + maxZ() < 0.0f) return {};
        }
    }

    return tMin;
}

void gBoundingBox::draw() {
	gColor oldrendercolor = *renderer->getColor();
	renderer->setColor(0, 255, 0);
	gDrawBox(origin.x, origin.y, origin.z, widthhalf, heighthalf, depthhalf, false);
	renderer->setColor(oldrendercolor);
}

void gBoundingBox::drawOBB() {
	gColor oldrendercolor = *renderer->getColor();
	renderer->setColor(0, 255, 0);
	gDrawBox(transformationmatrix, false);
	renderer->setColor(oldrendercolor);
}
