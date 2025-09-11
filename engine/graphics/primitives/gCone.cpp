/*
 * gCone.cpp
 *
 *  Created on: 15 Mar 2023
 *      Author: Umutcan Turkmen
 */

#include "gCone.h"

gCone::gCone(int r, int h, glm::vec2 shiftdistance, int segmentnum, bool isFilled) {
	std::vector<gIndex> indicesb;
	std::vector<gVertex> verticesb;

	float angle = PI * 2 / segmentnum;
	for(int i = 0; i < segmentnum; i++) {
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

	for(int i = 0; i < segmentnum; i++) {
		indicesb.push_back(i);
		indicesb.push_back(segmentnum);
		indicesb.push_back(i + 1);
		indicesb.push_back(i);
	}

	auto verticesptr = std::make_shared<std::vector<gVertex>>(verticesb);
	auto indicesptr = std::make_shared<std::vector<gIndex>>(indicesb);
	setVertices(verticesptr, indicesptr);
	if(!isFilled) setDrawMode(gMesh::DRAWMODE_LINELOOP);
	else setDrawMode(gMesh::DRAWMODE_TRIANGLEFAN);
}

gCone::~gCone() {

}

