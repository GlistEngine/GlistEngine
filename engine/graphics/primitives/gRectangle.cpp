/*
 * gRectangle.cpp
 *
 *  Created on: 13 Tem 2021
 *      Author: oznur
 */

#include "gRectangle.h"
#include <cmath>

/*
 * Draws a rectangle.
 *
 *
 * @param x x point where the rectangle starts.
 * @param y y point where the rectangle starts.
 * @param w Width of rectangle.
 * @param h Height of rectangle.
 * @param isFilled Specifies whether the rectangle is filled or empty.
 * @param rotateAngle Rotation angle in radians, applied around the rectangle's center.
 */

gRectangle::gRectangle() {

}

gRectangle::gRectangle(float x, float y, float w, float h, bool isFilled, float rotateAngle) {
	isprojection2d = true;
	setRectanglePoints(x, y, w, h, isFilled, rotateAngle);
}

void gRectangle::setPoints(float x, float y, float w, float h, bool isFilled, float rotateAngle) {
	isprojection2d = true;
	setRectanglePoints(x, y, w, h, isFilled, rotateAngle);
}

void gRectangle::draw() {
	isprojection2d = true;
	gMesh::draw();
}

void gRectangle::draw(float x, float y, float w, float h, bool isFilled, float rotateAngle) {
	isprojection2d = true;
	setRectanglePoints(x, y, w, h, isFilled, rotateAngle);
	gMesh::draw();
}

void gRectangle::setRectanglePoints(float x, float y, float w, float h, bool isFilled, float rotateAngle) {
	if(!verticessb.empty()) {
		verticessb.clear();
		indicessb.clear();
	}

	// Rectangle's center, rotation pivot point
	float cx = x + w * 0.5f;
	float cy = y + h * 0.5f;

	float cosA = std::cos(rotateAngle);
	float sinA = std::sin(rotateAngle);

	// Rotates a point around the rectangle's center by rotateAngle (radians)
	auto rotatePoint = [&](float px, float py) -> glm::vec3 {
		float dx = px - cx;
		float dy = py - cy;
		float rx = dx * cosA - dy * sinA;
		float ry = dx * sinA + dy * cosA;
		return glm::vec3(cx + rx, cy + ry, 0.0f);
	};

	verticessb.push_back({rotatePoint(x, y)});
	verticessb.push_back({rotatePoint(x + w, y)});
	verticessb.push_back({rotatePoint(x + w, y + h)});
	verticessb.push_back({rotatePoint(x, y + h)});

	if (isFilled) {
		indicessb.push_back(0);
		indicessb.push_back(3);
		indicessb.push_back(2);
        indicessb.push_back(1);
		setDrawMode(gMesh::DRAWMODE_TRIANGLEFAN);
	} else {
		indicessb.push_back(0);
		indicessb.push_back(1);
		indicessb.push_back(2);
		indicessb.push_back(3);
		indicessb.push_back(0);
		setDrawMode(gMesh::DRAWMODE_LINESTRIP);
	}
	auto vertices = std::make_shared<std::vector<gVertex>>(verticessb);
	auto indices = std::make_shared<std::vector<gIndex>>(indicessb);
	setVertices(vertices, indices);
}

gRectangle::~gRectangle() {

}
