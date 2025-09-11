/*
 * gCross.cpp
 *
 *  Created on: 24 Aug 2022
 *      Author: burakmeydan
 */

#include "gCross.h"

gCross::gCross() {

}

gCross::~gCross() {

}

void gCross::draw(float x, float y, float width, float height, float thickness, bool isFilled) {
	isprojection2d = true;
	if(isFilled == false) {
		drawNonFilled(x, y, width, height, thickness);
		setDrawMode(gMesh::DRAWMODE_LINESTRIP);
	}
	else {
		drawFilled(x, y, width, height, thickness);
		setDrawMode(gMesh::DRAWMODE_TRIANGLESTRIP);
	}
	gMesh::draw();
}

void gCross::drawNonFilled(float x, float y, float width, float height, float thickness) {
	float mry = (height / 2) - (1 - (width / 2 / (width - thickness))) * (height - thickness); //length (from origin to intersection on y axis)
	float mrx = ((1 - (height / 2 / (height - thickness))) * (width - thickness) - (width / 2)) * - 1; //length (from origin to intersection on x axis)
	gVertex vertex;
	// 'i' numbers represents vertices for cross.
	//i = 0
	vertex.position.x = x;
	vertex.position.y = y;
	vertex.position.z = 0.0f;
	vertices.push_back(vertex);
	//i = 1
	vertex.position.x = x - thickness;
	vertex.position.y = y + thickness;
	vertices.push_back(vertex);
	//i = 2
	vertex.position.x = x - thickness + (width / 2) - mrx;
	vertex.position.y = y + (height / 2);
	vertices.push_back(vertex);
	//i = 3
	vertex.position.x = x - thickness;
	vertex.position.y = y - thickness + height;
	vertices.push_back(vertex);
	//i = 4
	vertex.position.x = x;
	vertex.position.y = y + height;
	vertices.push_back(vertex);
	//i = 5
	vertex.position.x = x - thickness + (width / 2);
	vertex.position.y = y + (height / 2) + mry;
	vertices.push_back(vertex);
	//i = 6
	vertex.position.x = x - (2 * thickness) + width;
	vertex.position.y = y + height;
	vertices.push_back(vertex);
	//i = 7
	vertex.position.x = x - thickness + width;
	vertex.position.y = y - thickness + height;
	vertices.push_back(vertex);
	//i = 8
	vertex.position.x = x - thickness + (width / 2) + mrx;
	vertex.position.y = y + (height / 2);
	vertices.push_back(vertex);
	//i = 9
	vertex.position.x = x - thickness + width;
	vertex.position.y = y + thickness;
	vertices.push_back(vertex);
	//i = 10
	vertex.position.x = x - (2 * thickness) + width;
	vertex.position.y = y;
	vertices.push_back(vertex);
	//i = 11
	vertex.position.x = x - thickness + (width / 2);
	vertex.position.y = y + (height / 2) - mry;
	vertices.push_back(vertex);
	//i = 0
	vertex.position.x = x;
	vertex.position.y = y;
	vertices.push_back(vertex);
	auto verticesptr = std::make_shared<std::vector<gVertex>>(vertices);
	auto indicesptr = std::make_shared<std::vector<gIndex>>(indices);
	setVertices(verticesptr, indicesptr);
}

void gCross::drawFilled(float x, float y, float width, float height, float thickness) {
	float mry = (height / 2) - (1 - (width / 2 / (width - thickness))) * (height - thickness); //length (from origin to intersection on y axis)
	float mrx = ((1 - (height / 2 / (height - thickness))) * (width - thickness) - (width / 2)) * - 1; //length (from origin to intersection on x axis)
	gVertex vertex;
	// 'i' numbers represents vertices for cross.
	//i = 0
	vertex.position.x = x;
	vertex.position.y = y;
	vertex.position.z = 0.0f;
	vertices.push_back(vertex);
	//i = 1
	vertex.position.x = x - thickness;
	vertex.position.y = y + thickness;
	vertices.push_back(vertex);
	//i = 6
	vertex.position.x = x - (2 * thickness) + width;
	vertex.position.y = y + height;
	vertices.push_back(vertex);
	//i = 7
	vertex.position.x = x - thickness + width;
	vertex.position.y = y - thickness + height;
	vertices.push_back(vertex);
	//i = 0
	vertex.position.x = x;
	vertex.position.y = y;
	vertices.push_back(vertex);
	//i = 11
	vertex.position.x = x - thickness + (width / 2);
	vertex.position.y = y + (height / 2) - mry;
	vertices.push_back(vertex);
	//i = 8
	vertex.position.x = x - thickness + (width / 2) + mrx;
	vertex.position.y = y + (height / 2);
	vertices.push_back(vertex);
	//i = 9
	vertex.position.x = x - thickness + width;
	vertex.position.y = y + thickness;
	vertices.push_back(vertex);
	//i = 10
	vertex.position.x = x - (2 * thickness) + width;
	vertex.position.y = y;
	vertices.push_back(vertex);
	//i = 3
	vertex.position.x = x - thickness;
	vertex.position.y = y - thickness + height;
	vertices.push_back(vertex);
	//i = 4
	vertex.position.x = x;
	vertex.position.y = y + height;
	vertices.push_back(vertex);
	//i = 9
	vertex.position.x = x - thickness + width;
	vertex.position.y = y + thickness;
	vertices.push_back(vertex);
	auto verticesptr = std::make_shared<std::vector<gVertex>>(vertices);
	auto indicesptr = std::make_shared<std::vector<gIndex>>(indices);
	setVertices(verticesptr, indicesptr);
}
