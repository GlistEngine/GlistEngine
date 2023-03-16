/*
 * gRoundedRectangle.cpp
 *
 *  Created on: 15 Feb 2023
 *      Author: Umutcan Turkmen
 */

#include "gRoundedRectangle.h"
gRoundedRectangle::gRoundedRectangle() {

}

gRoundedRectangle::gRoundedRectangle(int x, int y, int w, int h, int radius, bool isFilled) {
	isprojection2d = true;
	setRoundedRectanglePoints(x, y, w, h, radius, isFilled);
}

gRoundedRectangle::~gRoundedRectangle() {

}

void gRoundedRectangle::draw() {
	isprojection2d = true;
	gMesh::draw();
}

void gRoundedRectangle::draw(int x, int y, int w, int h, int radius, bool isFilled) {
	isprojection2d = true;
	setRoundedRectanglePoints(x, y, w, h, radius, isFilled);
	gMesh::draw();
}

void gRoundedRectangle::setRoundedRectanglePoints(int x, int y, int w, int h, int radius, bool isFilled) {
	isprojection2d = true;
	if (radius < 0) radius = 0;
	if (radius > w / 2) radius = w / 2;
	if (radius > h / 2) radius = h / 2;
	std::vector<gVertex> vertices;
	float numberOfVertices = std::max(std::max(100, w), h);
	float angle = PI * 2 / numberOfVertices;
	for (int i = 0; i <= numberOfVertices / 4; i++) {
	    float nextAngle = angle * i;
	    gVertex vertex;
	    vertex.position.x = radius * cos(nextAngle) + x + w - radius;
	    vertex.position.y = radius * sin(nextAngle) + y + h - radius;
	    vertex.position.z = 0.0f;
	    vertices.push_back(vertex);
	}

	for (int i = numberOfVertices / 4; i <= numberOfVertices / 2; i++) {
	    float nextAngle = angle * i;
	    gVertex vertex;
	    vertex.position.x = radius * cos(nextAngle) + x + radius;
	    vertex.position.y = radius * sin(nextAngle) + y + h - radius;
	    vertex.position.z = 0.0f;
	    vertices.push_back(vertex);
	}

	for (int i = numberOfVertices / 2; i <= 3 * numberOfVertices / 4; i++) {
	    float nextAngle = angle * i;
	    gVertex vertex;
	    vertex.position.x = radius * cos(nextAngle) + x + radius;
	    vertex.position.y = radius * sin(nextAngle) + y + radius;
	    vertex.position.z = 0.0f;
	    vertices.push_back(vertex);
	}

	for (int i = 3 * numberOfVertices / 4; i <= numberOfVertices; i++) {
	    float nextAngle = angle * i;
	    gVertex vertex;
	    vertex.position.x = radius * cos(nextAngle) + x + w - radius;
	    vertex.position.y = radius * sin(nextAngle) + y + radius;
	    vertex.position.z = 0.0f;
	    vertices.push_back(vertex);
	}

	gVertex startPoint;
	startPoint.position.x = x + w;
	startPoint.position.y = y + h - radius;
	startPoint.position.z = 0.0f;
	vertices.push_back(startPoint);
	setVertices(vertices);
	if (isFilled == false)
	    setDrawMode(gMesh::DRAWMODE_LINESTRIP);
	else
	    setDrawMode(gMesh::DRAWMODE_TRIANGLEFAN);
}

