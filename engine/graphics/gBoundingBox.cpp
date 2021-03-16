/*
 * gBoundingBox.cpp
 *
 *  Created on: Mar 15, 2021
 *      Author: noyan
 */

#include <gBoundingBox.h>

gBoundingBox::gBoundingBox() : minx(0), miny(0), minz(0), maxx(0), maxy(0), maxz(0), width(0), height(0), depth(0) {}

gBoundingBox::gBoundingBox(float minX, float minY, float minZ, float maxX, float maxY, float maxZ) : minx(minX), miny(minY), minz(minZ), maxx(maxX), maxy(maxY), maxz(maxZ), width(maxX - minX), height(maxY - minY), depth(maxZ - minZ) {}

gBoundingBox::gBoundingBox(const gBoundingBox& b) : minx(b.minX()), miny(b.minY()), minz(b.minZ()), maxx(b.maxX()), maxy(b.maxY()), maxz(b.maxZ()), width(b.maxX() - b.minX()), height(b.maxY() - b.minY()), depth(b.maxZ() - b.minZ()) {}

gBoundingBox::~gBoundingBox() {}

void gBoundingBox::set(float minX, float minY, float minZ, float maxX, float maxY, float maxZ) {
	minx = minX;
	miny = minY;
	minz = minZ;
	maxx = maxX;
	maxy = maxY;
	maxz = maxZ;
	width = maxX - minX;
	height = maxY - minY;
	depth = maxZ - minZ;
}

void gBoundingBox::set(const gBoundingBox& b) {
	minx = b.minX();
	miny = b.minY();
	minz = b.minZ();
	maxx = b.maxX();
	maxy = b.maxY();
	maxz = b.maxZ();
	width = b.getWidth();
	height = b.getHeight();
	depth = b.getDepth();
}

bool gBoundingBox::intersects(float minX1, float minY1, float minZ1, float maxX1, float maxY1, float maxZ1, float minX2, float minY2, float minZ2, float maxX2, float maxY2, float maxZ2) {
	return minX1 <= maxX2 && maxX1 >= minX2 && minY1 <= maxY2 && maxY1 >= minY2 && minZ1 <= maxZ2 && maxZ1 >= minZ2;
}

bool gBoundingBox::intersects(const gBoundingBox& b1, const gBoundingBox& b2) {
	return intersects(b1.minX(), b1.minY(), b1.minZ(), b1.maxX(), b1.maxY(), b1.maxZ(), b2.minX(), b2.minY(), b2.minZ(), b2.maxX(), b2.maxY(), b2.maxZ());
}

bool gBoundingBox::intersects(float minX, float minY, float minZ, float maxX, float maxY, float maxZ) {
	return intersects(minx, miny, minz, maxx, maxy, maxz, minX, minY, minZ, maxX, maxY, maxZ);
}

bool gBoundingBox::intersects(const gBoundingBox& b) {
	return intersects(minx, miny, minz, maxx, maxy, maxz, b.minX(), b.minY(), b.minZ(), b.maxX(), b.maxY(), b.maxZ());
}

bool gBoundingBox::contains(float minX1, float minY1, float minZ1, float maxX1, float maxY1, float maxZ1, float minX2, float minY2, float minZ2, float maxX2, float maxY2, float maxZ2) {
	return minX1 <= minX2 && maxX1 >= maxX2 && minY1 <= minY2 && maxY1 >= maxY2 && minZ1 <= minZ2 && maxZ1 >= maxZ2;
}

bool gBoundingBox::contains(const gBoundingBox& b1, const gBoundingBox& b2) {
	return contains(b1.minX(), b1.minY(), b1.minZ(), b1.maxX(), b1.maxY(), b1.maxZ(), b2.minX(), b2.minY(), b2.minZ(), b2.maxX(), b2.maxY(), b2.maxZ());
}

bool gBoundingBox::contains(float minX, float minY, float minZ, float maxX, float maxY, float maxZ) {
	return contains(minx, miny, minz, maxx, maxy, maxz, minX, minY, minZ, maxX, maxY, maxZ);
}

bool gBoundingBox::contains(const gBoundingBox& b) {
	return contains(minx, miny, minz, maxx, maxy, maxz, b.minX(), b.minY(), b.minZ(), b.maxX(), b.maxY(), b.maxZ());
}

bool gBoundingBox::contains(float x, float y, float z) {
	return contains(minx, miny, minz, maxx, maxy, maxz, x, y, z, x, y, z);
}

gBoundingBox gBoundingBox::merge(gBoundingBox& other) {
	if (width == 0.0f && height == 0.0f && depth == 0.0f) return other;
	else if (other.getWidth() == 0.0f && other.getHeight() == 0.0f && other.getDepth() == 0.0f) return *this;

	return gBoundingBox(
			std::min(minx, other.minX()),
			std::min(miny, other.minY()),
			std::min(minz, other.minZ()),
			std::max(maxx, other.maxX()),
			std::max(maxy, other.maxY()),
			std::max(maxz, other.maxZ())
	);
}

float gBoundingBox::minX() const {
	return minx;
}

float gBoundingBox::minY() const {
	return miny;
}

float gBoundingBox::minZ() const {
	return minz;
}

float gBoundingBox::maxX() const {
	return maxx;
}

float gBoundingBox::maxY() const {
	return maxy;
}

float gBoundingBox::maxZ() const {
	return maxz;
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

