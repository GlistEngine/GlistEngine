/*
 * gCone.cpp
 *
 *  Created on: 15 Mar 2023
 *      Author: Umutcan Türkmen
 */

#include "gCone.h"

gCone::gCone(int r, int h, int segments, glm::vec2 shiftdistance, bool isFilled) {
	std::vector<unsigned int> indicesb;
	std::vector<gVertex> verticesb;

	float angle = PI * 2 / segments;
	for(int i = 0; i < segments; i++) {
		//BOTTOM
		gVertex v;
		v.position.x = r * cos(angle * i);
		v.position.y = -h / 2;
		v.position.z = r * sin(angle * i);
		verticesb.push_back(v);
	}

	//TOP VERTEX
	gVertex v;
	v.position.x = shiftdistance.x;
	v.position.y = h / 2;
	v.position.z = shiftdistance.y;
	verticesb.push_back(v);

	for(int i = 0; i < segments; i++) {
		indicesb.push_back(i);
		indicesb.push_back(segments);
		indicesb.push_back(i + 1);
		indicesb.push_back(i);
	}

	setVertices(verticesb, indicesb);
	if(!isFilled) setDrawMode(gMesh::DRAWMODE_LINELOOP);
	else setDrawMode(gMesh::DRAWMODE_TRIANGLEFAN);
}

gCone::~gCone() {

}

