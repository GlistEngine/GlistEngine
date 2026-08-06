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

void gCross::draw(float x, float y, float width, float height, float thickness, bool isFilled, float rotateAngle, float pivotx, float pivoty) {
	isprojection2d = true;
	if(isFilled == false) {
		drawNonFilled(x, y, width, height, thickness, rotateAngle, pivotx, pivoty);
		setDrawMode(gMesh::DRAWMODE_LINESTRIP);
	}
	else {
		drawFilled(x, y, width, height, thickness, rotateAngle, pivotx, pivoty);
		setDrawMode(gMesh::DRAWMODE_TRIANGLESTRIP);
	}
	gMesh::draw();
}

void gCross::drawNonFilled(float x, float y, float width, float height, float thickness, float rotateAngle, float pivotx, float pivoty) {
	// The corners are rebuilt on every call, so the previous ones have to go first;
	// without this the mesh grows by a full cross each frame.
	vertices.clear();
	indices.clear();

	// Pivot noktasý hesabý (Bounding Box: sol-üst köþe (x - thickness, y))
	float pivotPx = (x - thickness) + pivotx * width;
	float pivotPy = y + pivoty * height;

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

	float mry = (height / 2) - (1 - (width / 2 / (width - thickness))) * (height - thickness); //length (from origin to intersection on y axis)
	float mrx = ((1 - (height / 2 / (height - thickness))) * (width - thickness) - (width / 2)) * - 1; //length (from origin to intersection on x axis)

	// 'i' numbers represents vertices for cross.
	//i = 0
	vertices.push_back(transformPoint(x, y));
	//i = 1
	vertices.push_back(transformPoint(x - thickness, y + thickness));
	//i = 2
	vertices.push_back(transformPoint(x - thickness + (width / 2) - mrx, y + (height / 2)));
	//i = 3
	vertices.push_back(transformPoint(x - thickness, y - thickness + height));
	//i = 4
	vertices.push_back(transformPoint(x, y + height));
	//i = 5
	vertices.push_back(transformPoint(x - thickness + (width / 2), y + (height / 2) + mry));
	//i = 6
	vertices.push_back(transformPoint(x - (2 * thickness) + width, y + height));
	//i = 7
	vertices.push_back(transformPoint(x - thickness + width, y - thickness + height));
	//i = 8
	vertices.push_back(transformPoint(x - thickness + (width / 2) + mrx, y + (height / 2)));
	//i = 9
	vertices.push_back(transformPoint(x - thickness + width, y + thickness));
	//i = 10
	vertices.push_back(transformPoint(x - (2 * thickness) + width, y));
	//i = 11
	vertices.push_back(transformPoint(x - thickness + (width / 2), y + (height / 2) - mry));
	//i = 0
	vertices.push_back(transformPoint(x, y));

	auto verticesptr = std::make_shared<std::vector<gVertex>>(vertices);
	auto indicesptr = std::make_shared<std::vector<gIndex>>(indices);
	setVertices(verticesptr, indicesptr);
}

void gCross::drawFilled(float x, float y, float width, float height, float thickness, float rotateAngle, float pivotx, float pivoty) {
	vertices.clear();
	indices.clear();

	// Pivot noktasý hesabý
	float pivotPx = (x - thickness) + pivotx * width;
	float pivotPy = y + pivoty * height;

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

	float mry = (height / 2) - (1 - (width / 2 / (width - thickness))) * (height - thickness); //length (from origin to intersection on y axis)
	float mrx = ((1 - (height / 2 / (height - thickness))) * (width - thickness) - (width / 2)) * - 1; //length (from origin to intersection on x axis)

	// 'i' numbers represents vertices for cross.
	//i = 0
	vertices.push_back(transformPoint(x, y));
	//i = 1
	vertices.push_back(transformPoint(x - thickness, y + thickness));
	//i = 6
	vertices.push_back(transformPoint(x - (2 * thickness) + width, y + height));
	//i = 7
	vertices.push_back(transformPoint(x - thickness + width, y - thickness + height));
	//i = 0
	vertices.push_back(transformPoint(x, y));
	//i = 11
	vertices.push_back(transformPoint(x - thickness + (width / 2), y + (height / 2) - mry));
	//i = 8
	vertices.push_back(transformPoint(x - thickness + (width / 2) + mrx, y + (height / 2)));
	//i = 9
	vertices.push_back(transformPoint(x - thickness + width, y + thickness));
	//i = 10
	vertices.push_back(transformPoint(x - (2 * thickness) + width, y));
	//i = 3
	vertices.push_back(transformPoint(x - thickness, y - thickness + height));
	//i = 4
	vertices.push_back(transformPoint(x, y + height));
	//i = 9
	vertices.push_back(transformPoint(x - thickness + width, y + thickness));

	auto verticesptr = std::make_shared<std::vector<gVertex>>(vertices);
	auto indicesptr = std::make_shared<std::vector<gIndex>>(indices);
	setVertices(verticesptr, indicesptr);
}
