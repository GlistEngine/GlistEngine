/*
 * gRectangle.cpp
 *
 *  Created on: 13 Tem 2021
 *      Author: oznur
 */

#include "gRectangle.h"

/*
 * Draws a rectangle.
 *
 *
 * @param x x point where the rectangle starts.
 * @param y y point where the rectangle starts.
 * @param w Width of rectangle.
 * @param h Height of rectangle.
 * @param isFilled Specifies whether the rectangle is filled or empty.
 */

gRectangle::gRectangle() {

}

gRectangle::gRectangle(float x, float y, float w, float h, bool isFilled) {
	isprojection2d = true;
	setRectanglePoints(x, y, w, h, isFilled);
}

void gRectangle::setPoints(float x, float y, float w, float h, bool isFilled) {
	isprojection2d = true;
	setRectanglePoints(x, y, w, h, isFilled);
}

void gRectangle::draw() {
	isprojection2d = true;
	gMesh::draw();
}

void gRectangle::draw(float x, float y, float w, float h, bool isFilled) {
	isprojection2d = true;
	setRectanglePoints(x, y, w, h, isFilled);
	gMesh::draw();
}

void gRectangle::setRectanglePoints(float x, float y, float w, float h, bool isFilled) {
	if(!verticessb.empty()) {
		verticessb.clear();
		indicessb.clear();
	}

	verticessb.push_back({{x, y, 0.0f}});
	verticessb.push_back({{x + w, y, 0.0f}});
	verticessb.push_back({{x + w, y + h, 0.0f}});
	verticessb.push_back({{x, y + h, 0.0f}});

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
