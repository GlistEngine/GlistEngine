/*
 * gRectangle.cpp
 *
 *  Created on: 13 Tem 2021
 *      Author: oznur
 */

#include "gRectangle.h"

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

gRectangle::gRectangle() {

}

gRectangle::gRectangle(float x, float y, float w, float h, bool isFilled) {
	isprojection2d = true;
	setRectanglePoints(x, y, w, h, isFilled);
}

void gRectangle::setPoints(float x, float y, float w, float h, bool isFilled) {
	isprojection2d = true;
	setRectanglePoints(x, y, w, h, isFilled);
}

void gRectangle::draw() {
	isprojection2d = true;
	gMesh::draw();
}

void gRectangle::draw(float x, float y, float w, float h, bool isFilled) {
	isprojection2d = true;
	setRectanglePoints(x, y, w, h, isFilled);
	gMesh::draw();
}

void gRectangle::setRectanglePoints(float x, float y, float w, float h, bool isFilled) {
//	verticessb.clear();

	vertex1.position.x = x;
	vertex1.position.y = y;
	vertex1.position.z = 0.0f;
	verticessb.push_back(vertex1);

	vertex2.position.x = x + w;
	vertex2.position.y = y;
	vertex2.position.z = 0.0f;
	verticessb.push_back(vertex2);

	vertex3.position.x = x + w;
	vertex3.position.y = y + h;
	vertex3.position.z = 0.0f;
	verticessb.push_back(vertex3);

	vertex4.position.x = x;
	vertex4.position.y = y + h;
	vertex4.position.z = 0.0f;
	verticessb.push_back(vertex4);

	indicessb.push_back(0);
	indicessb.push_back(1);
	indicessb.push_back(2);
	indicessb.push_back(3);
	indicessb.push_back(0);


	setVertices(verticessb, indicessb);
    if(!isFilled) setDrawMode(gMesh::DRAWMODE_LINESTRIP);
    else setDrawMode(gMesh::DRAWMODE_TRIANGLESTRIP);

}

gRectangle::~gRectangle() {

}
