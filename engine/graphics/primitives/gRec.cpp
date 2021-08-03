/*
 * gRec.cpp
 *
 *  Created on: 13 Tem 2021
 *      Author: oznur
 */

#include "gRec.h"

/*
 * Draws a rectangle.
 *
 *
 * @param x x point where the rectangle starts.
 * @param y y point where the rectangle starts.
 * @param w Width of rectangle.
 * @param h Height of rectangle.
 * @param isFilled Specifies whether the rectangle is filled or empty.
 */

gRec::gRec() {

}

gRec::gRec(float x, float y, float w, float h, bool isFilled) {
	isprojection2d = true;
	setRecPoints(x, y, w, h, isFilled);
}

void gRec::setPoints(float x, float y, float w, float h, bool isFilled) {
	isprojection2d = true;
	setRecPoints(x, y, w, h, isFilled);
}

void gRec::draw() {
	gMesh::draw();
}

void gRec::draw(float x, float y, float w, float h, bool isFilled) {
	isprojection2d = true;
	setRecPoints(x, y, w, h, isFilled);
	gMesh::draw();
}

void gRec::setRecPoints(float x, float y, float w, float h, bool isFilled) {
//	verticessb.clear();

	vertex1.position.x = x;
	vertex1.position.y = y;
	verticessb.push_back(vertex1);

	vertex2.position.x = x + w;
	vertex2.position.y = y;
	verticessb.push_back(vertex2);

	vertex3.position.x = x + w;
	vertex3.position.y = y + h;
	verticessb.push_back(vertex3);

	vertex4.position.x = x;
	vertex4.position.y = y + h;
	verticessb.push_back(vertex4);

	indicessb.push_back(0);
	indicessb.push_back(1);
	indicessb.push_back(2);
	indicessb.push_back(3);
	indicessb.push_back(0);


	setVertices(verticessb, indicessb);
    if(isFilled == false) setDrawMode(gMesh::DRAWMODE_LINESTRIP);
    else setDrawMode(gMesh::DRAWMODE_TRIANGLESTRIP);

}

gRec::~gRec() {

}
