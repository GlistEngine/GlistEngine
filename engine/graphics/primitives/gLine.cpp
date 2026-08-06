/*
 * gLine.cpp
 *
 *  Created on: May 7, 2021
 *      Author: noyan
 */

#include "gLine.h"
#include <cmath>
#include <algorithm>


gLine::gLine() {
	thickness = 1.0f;
}

gLine::gLine(float x1, float y1, float x2, float y2, float rotateAngle, float pivotx, float pivoty) {
	isprojection2d = true;
	setLinePoints(x1, y1, 0.0f, x2, y2, 0.0f, rotateAngle, pivotx, pivoty);
}

gLine::gLine(float x1, float y1, float z1, float x2, float y2, float z2, float rotateAngle, float pivotx, float pivoty) {
	isprojection2d = false;
	setLinePoints(x1, y1, z1, x2, y2, z2, rotateAngle, pivotx, pivoty);
}

void gLine::setPoints(float x1, float y1, float x2, float y2, float rotateAngle, float pivotx, float pivoty) {
	isprojection2d = true;
	setLinePoints(x1, y1, 0.0f, x2, y2, 0.0f, rotateAngle, pivotx, pivoty);
}

void gLine::setPoints(float x1, float y1, float z1, float x2, float y2, float z2, float rotateAngle, float pivotx, float pivoty) {
	isprojection2d = false;
	setLinePoints(x1, y1, z1, x2, y2, z2, rotateAngle, pivotx, pivoty);
}

void gLine::draw() {
	gMesh::draw();
}

void gLine::draw(float x1, float y1, float x2, float y2, float rotateAngle, float pivotx, float pivoty) {
	isprojection2d = true;
	setLinePoints(x1, y1, 0.0f, x2, y2, 0.0f, rotateAngle, pivotx, pivoty);
	gMesh::draw();
}

void gLine::draw(float x1, float y1, float z1, float x2, float y2, float z2, float rotateAngle, float pivotx, float pivoty) {
	isprojection2d = false;
	setLinePoints(x1, y1, z1, x2, y2, z2, rotateAngle, pivotx, pivoty);
	gMesh::draw();
}

void gLine::setThickness(float value) {
	thickness = value;
}

void gLine::setLinePoints(float x1, float y1, float z1, float x2, float y2, float z2, float rotateAngle, float pivotx, float pivoty) {
	if(verticessb.size() > 0) {
		verticessb.clear();
		indicessb.clear();
	}

	// Bounding Box hesabý (pivot için)
	float minX = std::min(x1, x2);
	float maxX = std::max(x1, x2);
	float minY = std::min(y1, y2);
	float maxY = std::max(y1, y2);
	float width = maxX - minX;
	float height = maxY - minY;

	float pivotPx = minX + pivotx * width;
	float pivotPy = minY + pivoty * height;

	float cosA = std::cos(rotateAngle);
	float sinA = std::sin(rotateAngle);

	auto transformPoint = [&](float px, float py, float pz) -> glm::vec3 {
		if (rotateAngle != 0.0f) {
			float dx = px - pivotPx;
			float dy = py - pivotPy;
			float rx = dx * cosA - dy * sinA;
			float ry = dx * sinA + dy * cosA;
			return glm::vec3(pivotPx + rx, pivotPy + ry, pz);
		}
		return glm::vec3(px, py, pz);
	};

	if(thickness > 1.0f) {
		glm::vec3 v1 = glm::vec3(x1, y1, z1);
		glm::vec3 v2 = glm::vec3(x2, y2, z2);
		glm::vec3 d = glm::normalize(v2 - v1);
		glm::vec3 tangent = glm::cross(d, glm::vec3(0.0f, 0.0f, 1.0f));
		tangent = glm::normalize(tangent);

		float p1x = x1 + tangent.x * thickness;
		float p1y = y1 + tangent.y * thickness;
		float p1z = z1 + tangent.z * thickness;
		vertex1.position = transformPoint(p1x, p1y, p1z);
		verticessb.push_back(vertex1);

		float p2x = x2 + tangent.x * thickness;
		float p2y = y2 + tangent.y * thickness;
		float p2z = z2 + tangent.z * thickness;
		vertex2.position = transformPoint(p2x, p2y, p2z);
		verticessb.push_back(vertex2);

		float p3x = x1 - tangent.x * thickness;
		float p3y = y1 - tangent.y * thickness;
		float p3z = z1 - tangent.z * thickness;
		vertex1.position = transformPoint(p3x, p3y, p3z);
		verticessb.push_back(vertex1);

		float p4x = x2 - tangent.x * thickness;
		float p4y = y2 - tangent.y * thickness;
		float p4z = z2 - tangent.z * thickness;
		vertex2.position = transformPoint(p4x, p4y, p4z);
		verticessb.push_back(vertex2);

		indicessb = {0, 1, 3, 0, 2, 3};
		setDrawMode(gMesh::DRAWMODE_TRIANGLES);
	} else {
		vertex1.position = transformPoint(x1, y1, z1);
		verticessb.push_back(vertex1);

		vertex2.position = transformPoint(x2, y2, z2);
		verticessb.push_back(vertex2);
		setDrawMode(gMesh::DRAWMODE_LINES);
	}

	auto vertices = std::make_shared<std::vector<gVertex>>(verticessb);
	auto indices = std::make_shared<std::vector<gIndex>>(indicessb);
	setVertices(vertices, indices);
}
