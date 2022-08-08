/*
 * gArc.cpp
 *
 *  Created on: 27 Jul 2022
 *      Author: burakmeydan
 */

#include "gArc.h"

gArc::gArc() {

}

gArc::~gArc() {

}

void gArc::draw(float xCenter, float yCenter, float radius, bool isFilled, int numberOfSides, float degree, float rotate) {
	isprojection2d = true;
	setArcPoints(xCenter, yCenter, radius, isFilled, numberOfSides, degree, rotate);
	gMesh::draw();
}

void gArc::setArcPoints(float xCenter, float yCenter, float radius, bool isFilled,  int numberOfSides, float degree, float rotate) {
	if (!this->vertices.empty()) {
		this->vertices.clear();
		this->indices.clear();
	}
	if(degree >= 0) degree = std::fmod(degree, 360);
	else degree = std::fmod((std::fmod(degree, 360) + 360), 360);
	if(degree == 0) degree = 360;
	float angleradian = (float)degree / (float)numberOfSides * (PI / 180);
	float rotateradian = rotate * (PI / 180);
	float nextAngle;
	gVertex vertex;
	if(degree > -360 && degree < 360) {
		vertex.position.x = xCenter;
		vertex.position.y = yCenter;
		vertex.position.z = 0.0f;
		this->vertices.push_back(vertex);
	}
	for(int i = 0; i <= numberOfSides; i++) {
		nextAngle = rotateradian + (i * angleradian);
		vertex.position.x = radius * cos(nextAngle) + xCenter;
		vertex.position.y = radius * sin(nextAngle) + yCenter;
		vertex.position.z = 0.0f;
		this->vertices.push_back(vertex);
	}
	if(degree > -360 && degree < 360) {
		vertex.position.x = xCenter;
		vertex.position.y = yCenter;
		vertex.position.z = 0.0f;
		this->vertices.push_back(vertex);
	}
	this->setVertices(this->vertices, this->indices);
	if(isFilled == false) setDrawMode(gMesh::DRAWMODE_LINESTRIP);
		else setDrawMode(gMesh::DRAWMODE_TRIANGLEFAN);
}
