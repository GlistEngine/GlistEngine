/*
 * gCircleSlice.cpp
 *
 *  Created on: 28 Tem 2022
 *      Author: buraks
 */

#include "gCircleSlice.h"

gCircleSlice::gCircleSlice() {

}

gCircleSlice::~gCircleSlice() {

}

void gCircleSlice::draw(float xCenter, float yCenter, float radius, bool isFilled, float numberOfSides, int degree) {
	isprojection2d = true;
	setCirclePoints(xCenter, yCenter, radius, isFilled, numberOfSides, degree);
	gMesh::draw();
}

void gCircleSlice::setCirclePoints(float xCenter, float yCenter, float radius, bool isFilled,  float numberOfSides, int degree) {
	float angle     = PI * 2 / numberOfSides;
	float ratio = (float)degree / 360;
	float nextAngle;
	gVertex vertex;
	std::vector<gVertex> verticessb;

	vertex.position.x = xCenter;
	vertex.position.y = yCenter;
	vertex.position.z = 0.0f;
	verticessb.push_back(vertex);
	setVertices(verticessb);
	for(int i = 0; i <= numberOfSides * ratio; i++) {
		nextAngle = angle * i;
		vertex.position.x = radius * cos(nextAngle) + xCenter;
		vertex.position.y = radius * sin(nextAngle) + yCenter;
		vertex.position.z = 0.0f;
		verticessb.push_back(vertex);
		setVertices(verticessb);
	}
	vertex.position.x = xCenter;
	vertex.position.y = yCenter;
	vertex.position.z = 0.0f;
	verticessb.push_back(vertex);
	setVertices(verticessb);
	gLogi("gCircleSlice.h") << "parameters -> " << xCenter << ", " << yCenter << ", " << radius << ", " << isFilled << ", " << numberOfSides << ", " << degree << ", " << ratio;
	if(isFilled == false) setDrawMode(gMesh::DRAWMODE_LINESTRIP);
		else setDrawMode(gMesh::DRAWMODE_TRIANGLEFAN);
}
