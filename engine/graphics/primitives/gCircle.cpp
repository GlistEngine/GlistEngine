/*
 * gCircle.cpp
 *
 *  Created on: 9 Tem 2021
 *      Author: YavuzBilginoglu
 */

#include "gCircle.h"

gCircle::gCircle() {

}

gCircle::gCircle(float xCenter, float yCenter, float radius, bool isFilled, float numberOfSides) {
	isprojection2d = true;
	setCirclePoints(xCenter, yCenter, radius, isFilled, numberOfSides);
}

gCircle::~gCircle() {

}

void gCircle::setPoints(float xCenter, float yCenter, float radius, bool isFilled,  float numberOfSides) {
	isprojection2d = true;
	setCirclePoints(xCenter, yCenter, radius, isFilled, numberOfSides);
}

void gCircle::draw(){
	isprojection2d = true;
	gMesh::draw();
}

void gCircle::draw(float xCenter, float yCenter, float radius, bool isFilled, float numberOfSides){
	isprojection2d = true;
	setCirclePoints(xCenter, yCenter, radius, isFilled, numberOfSides);
	gMesh::draw();
}

void gCircle::setCirclePoints(float xCenter, float yCenter, float radius, bool isFilled,  float numberOfSides) {
	float angle     = PI * 2 / numberOfSides;
	std::vector<gVertex> verticessb;

	for(int i = 0; i <= numberOfSides; i++) {
		float nextAngle = angle * i;
		gVertex vertex;
		vertex.position.x = radius * cos(nextAngle) + xCenter;
		vertex.position.y = radius * sin(nextAngle) + yCenter;
		vertex.position.z = 0.0f;
		verticessb.push_back(vertex);
	}

	setVertices(verticessb);
	if(isFilled == false) setDrawMode(gMesh::DRAWMODE_LINESTRIP);
	else setDrawMode(gMesh::DRAWMODE_TRIANGLEFAN);
}


