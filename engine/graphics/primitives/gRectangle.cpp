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
 * @param rotateAngle Rotation angle in radians.
 * @param pivotx Normalized pivot point on x-axis (0.0 to 1.0).
 * @param pivoty Normalized pivot point on y-axis (0.0 to 1.0).
 */

gRectangle::gRectangle() {

}

gRectangle::gRectangle(float x, float y, float w, float h, bool isFilled, float rotateAngle, float pivotx, float pivoty) {
	isprojection2d = true;
	setRectanglePoints(x, y, w, h, isFilled, rotateAngle, pivotx, pivoty);
}

void gRectangle::setPoints(float x, float y, float w, float h, bool isFilled, float rotateAngle, float pivotx, float pivoty) {
	isprojection2d = true;
	setRectanglePoints(x, y, w, h, isFilled, rotateAngle, pivotx, pivoty);
}

void gRectangle::draw() {
	isprojection2d = true;
	gMesh::draw();
}

void gRectangle::draw(float x, float y, float w, float h, bool isFilled, float rotateAngle, float pivotx, float pivoty) {
	isprojection2d = true;
	setRectanglePoints(x, y, w, h, isFilled, rotateAngle, pivotx, pivoty);
	gMesh::draw();
}

void gRectangle::setRectanglePoints(float x, float y, float w, float h, bool isFilled, float rotateAngle, float pivotx, float pivoty) {
	if(!verticessb.empty()) {
		verticessb.clear();
		indicessb.clear();
	}

	// Rectangle's rotation pivot point
	float px = x + pivotx * w;
	float py = y + pivoty * h;

	float cosA = std::cos(rotateAngle);
	float sinA = std::sin(rotateAngle);

	// Rotates a point around the pivot point by rotateAngle
	auto rotatePoint = [&](float vx, float vy) -> glm::vec3 {
		float dx = vx - px;
		float dy = vy - py;
		float rx = dx * cosA - dy * sinA;
		float ry = dx * sinA + dy * cosA;
		return glm::vec3(px + rx, py + ry, 0.0f);
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
