/*
 * gCylinder.cpp
 *
 *  Created on: 13 Mar 2023
 *      Author: Umutcan Turkmen
 */

#include "gCylinder.h"

gCylinder::gCylinder(int r1, int r2, int h, glm::vec2 shiftdistance, int segmentnum, bool isFilled) {

	std::vector<Index> indicesb;
	std::vector<gVertex> verticesb;

	float angle = PI * 2 / segmentnum;
	for(int i = 0; i < segmentnum; i++) {
		//TOP
		gVertex v;
		v.position.x = shiftdistance.x + r1 * cos(angle * i);
		v.position.y = h / 2;
		v.position.z = shiftdistance.y + r1 * sin(angle * i);
		verticesb.push_back(v);
	}

	for(int i = 0; i < segmentnum; i++) {
		//BOTTOM
		gVertex v;
		v.position.x = -shiftdistance.x + r2 * cos(angle * i);
		v.position.y = -h / 2;
		v.position.z = -shiftdistance.y + r2 * sin(angle * i);
		verticesb.push_back(v);
	}

	for(int i = 0; i < segmentnum - 1; i++) {
		indicesb.push_back(i);
		indicesb.push_back(i + segmentnum);
		indicesb.push_back(i + segmentnum + 1);
		indicesb.push_back(i + 1);
		indicesb.push_back(i);
	}

	setVertices(verticesb, indicesb);
	if(!isFilled) setDrawMode(gMesh::DRAWMODE_LINELOOP);
	else setDrawMode(gMesh::DRAWMODE_TRIANGLEFAN);
}

gCylinder::~gCylinder() {

}
