/*
 * gLine.cpp
 *
 *  Created on: May 7, 2021
 *      Author: noyan
 */

#include "gLine.h"


gLine::gLine() {
}

gLine::gLine(float x1, float y1, float x2, float y2) {
	isprojection2d = true;
	setLinePoints(x1, y1, 0.0f, x2, y2, 0.0f);
}

gLine::gLine(float x1, float y1, float z1, float x2, float y2, float z2) {
	isprojection2d = false;
	setLinePoints(x1, y1, z1, x2, y2, z2);
}

void gLine::setPoints(float x1, float y1, float x2, float y2) {
	isprojection2d = true;
	setLinePoints(x1, y1, 0.0f, x2, y2, 0.0f);
}

void gLine::setPoints(float x1, float y1, float z1, float x2, float y2, float z2) {
	isprojection2d = false;
	setLinePoints(x1, y1, z1, x2, y2, z2);
}

void gLine::draw() {
	gMesh::draw();
}

void gLine::draw(float x1, float y1, float x2, float y2) {
	isprojection2d = true;
	setLinePoints(x1, y1, 0.0f, x2, y2, 0.0f);
	gMesh::draw();
}

void gLine::draw(float x1, float y1, float z1, float x2, float y2, float z2) {
	isprojection2d = false;
	setLinePoints(x1, y1, z1, x2, y2, z2);
	gMesh::draw();
}


void gLine::setLinePoints(float x1, float y1, float z1, float x2, float y2, float z2) {
	if(verticessb.size() > 0)
        verticessb.clear();

	vertex1.position.x = x1;
	vertex1.position.y = y1;
	vertex1.position.z = z1;
	verticessb.push_back(vertex1);

	vertex2.position.x = x2;
	vertex2.position.y = y2;
	vertex2.position.z = z2;
	verticessb.push_back(vertex2);

	setVertices(verticessb);
	setDrawMode(gMesh::DRAWMODE_LINES);
}
