/*
 * gRoundedRectangle.cpp
 *
 *  Created on: 15 Feb 2023
 *      Author: Umutcan Turkmen
 */

#include "gRoundedRectangle.h"
#include <cmath>
#include <algorithm>

gRoundedRectangle::gRoundedRectangle() {

}

gRoundedRectangle::gRoundedRectangle(int x, int y, int w, int h, int radius, bool isFilled, float rotateAngle, float pivotx, float pivoty) {
	isprojection2d = true;
	setRoundedRectanglePoints(x, y, w, h, radius, isFilled, rotateAngle, pivotx, pivoty);
}

gRoundedRectangle::~gRoundedRectangle() {

}

void gRoundedRectangle::draw() {
	isprojection2d = true;
	gMesh::draw();
}

void gRoundedRectangle::draw(int x, int y, int w, int h, int radius, bool isFilled, float rotateAngle, float pivotx, float pivoty) {
	isprojection2d = true;
	setRoundedRectanglePoints(x, y, w, h, radius, isFilled, rotateAngle, pivotx, pivoty);
	gMesh::draw();
}

void gRoundedRectangle::setRoundedRectanglePoints(int x, int y, int w, int h, int radius, bool isFilled, float rotateAngle, float pivotx, float pivoty) {
	isprojection2d = true;
	if (radius < 0) radius = 0;
	if (radius > w / 2) radius = w / 2;
	if (radius > h / 2) radius = h / 2;

	std::vector<gVertex> vertices;
	float numberOfVertices = std::max(std::max(100, w), h);
	float angle = PI * 2 / numberOfVertices;

	// Pivot noktasý hesabý (Bounding Box: x, y, w, h)
	float px = x + pivotx * w;
	float py = y + pivoty * h;

	float cosA = std::cos(rotateAngle);
	float sinA = std::sin(rotateAngle);

	auto transformPoint = [&](float vx, float vy) -> gVertex {
		gVertex vertex;
		if (rotateAngle != 0.0f) {
			float dx = vx - px;
			float dy = vy - py;
			vertex.position.x = px + (dx * cosA - dy * sinA);
			vertex.position.y = py + (dx * sinA + dy * cosA);
		} else {
			vertex.position.x = vx;
			vertex.position.y = vy;
		}
		vertex.position.z = 0.0f;
		return vertex;
	};

	for (int i = 0; i <= numberOfVertices / 4; i++) {
	    float nextAngle = angle * i;
	    float vx = radius * cos(nextAngle) + x + w - radius;
	    float vy = radius * sin(nextAngle) + y + h - radius;
	    vertices.push_back(transformPoint(vx, vy));
	}

	for (int i = numberOfVertices / 4; i <= numberOfVertices / 2; i++) {
	    float nextAngle = angle * i;
	    float vx = radius * cos(nextAngle) + x + radius;
	    float vy = radius * sin(nextAngle) + y + h - radius;
	    vertices.push_back(transformPoint(vx, vy));
	}

	for (int i = numberOfVertices / 2; i <= 3 * numberOfVertices / 4; i++) {
	    float nextAngle = angle * i;
	    float vx = radius * cos(nextAngle) + x + radius;
	    float vy = radius * sin(nextAngle) + y + radius;
	    vertices.push_back(transformPoint(vx, vy));
	}

	for (int i = 3 * numberOfVertices / 4; i <= numberOfVertices; i++) {
	    float nextAngle = angle * i;
	    float vx = radius * cos(nextAngle) + x + w - radius;
	    float vy = radius * sin(nextAngle) + y + radius;
	    vertices.push_back(transformPoint(vx, vy));
	}

	float startX = x + w;
	float startY = y + h - radius;
	vertices.push_back(transformPoint(startX, startY));

	auto verticesptr = std::make_shared<std::vector<gVertex>>(vertices);
	setVertices(verticesptr);
	if (isFilled == false)
	    setDrawMode(gMesh::DRAWMODE_LINESTRIP);
	else
	    setDrawMode(gMesh::DRAWMODE_TRIANGLEFAN);
}
