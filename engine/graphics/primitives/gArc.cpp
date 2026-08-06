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

void gArc::draw(float xCenter, float yCenter, float radius, bool isFilled, int numberOfSides, float degree, float rotate, float rotateAngle, float pivotx, float pivoty) {
	isprojection2d = true;
	setArcPoints(xCenter, yCenter, radius, isFilled, numberOfSides, degree, rotate, rotateAngle, pivotx, pivoty);
	gMesh::draw();
}

void gArc::setArcPoints(float xCenter, float yCenter, float radius, bool isFilled, int numberOfSides, float degree, float rotate, float rotateAngle, float pivotx, float pivoty) {
	if (!this->vertices.empty()) {
		this->vertices.clear();
		this->indices.clear();
	}
	if(degree >= 0)
	    degree = std::fmod(degree, 360.0f);
	else
	    degree = std::fmod((std::fmod(degree, 360.0f) + 360), 360.0f);
	if(degree == 0)
	    degree = 360;

	float angleradian = (float)degree / (float)numberOfSides * (PI / 180);
	float rotateradian = rotate * (PI / 180);
	float nextAngle;

	// Pivot noktasý hesabý (Bounding Box: sol-üst köþe (xCenter - radius, yCenter - radius))
	float boxX = xCenter - radius;
	float boxY = yCenter - radius;
	float boxW = 2.0f * radius;
	float boxH = 2.0f * radius;

	float pivotPx = boxX + pivotx * boxW;
	float pivotPy = boxY + pivoty * boxH;

	float cosA = cos(rotateAngle);
	float sinA = sin(rotateAngle);

	auto transformPoint = [&](float px, float py) -> gVertex {
		gVertex v;
		if (rotateAngle != 0.0f) {
			float relX = px - pivotPx;
			float relY = py - pivotPy;
			v.position.x = relX * cosA - relY * sinA + pivotPx;
			v.position.y = relX * sinA + relY * cosA + pivotPy;
		} else {
			v.position.x = px;
			v.position.y = py;
		}
		v.position.z = 0.0f;
		return v;
	};

	// Dolgulu modda (TRIANGLE_FAN) merkez nokta MUTLAKA ilk vertex olmalýdýr (derece fark etmeksizin).
	if (isFilled) {
		this->vertices.push_back(transformPoint(xCenter, yCenter));
	}

	// Çevre üzerindeki yay noktalarý
	for(int i = 0; i <= numberOfSides; i++) {
		nextAngle = rotateradian + (i * angleradian);
		float vx = radius * cos(nextAngle) + xCenter;
		float vy = radius * sin(nextAngle) + yCenter;
		this->vertices.push_back(transformPoint(vx, vy));
	}

	auto verticesptr = std::make_shared<std::vector<gVertex>>(this->vertices);
	auto indicesptr = std::make_shared<std::vector<gIndex>>(this->indices);
	setVertices(verticesptr, indicesptr);

	if(isFilled == false)
		setDrawMode(gMesh::DRAWMODE_LINESTRIP);
	else
		setDrawMode(gMesh::DRAWMODE_TRIANGLEFAN);
}
