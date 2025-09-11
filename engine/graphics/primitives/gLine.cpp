/*
 * gLine.cpp
 *
 *  Created on: May 7, 2021
 *      Author: noyan
 */

#include "gLine.h"


gLine::gLine() {
	thickness = 1.0f;
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

void gLine::setThickness(float value) {
	thickness = value;
}

void gLine::setLinePoints(float x1, float y1, float z1, float x2, float y2, float z2) {
	if(verticessb.size() > 0) {
		verticessb.clear();
		indicessb.clear();
	}

	if(thickness > 1.0f) {
		glm::vec3 v1 = glm::vec3(x1, y1, z1);
		glm::vec3 v2 = glm::vec3(x2, y2, z2);
		glm::vec3 d = glm::normalize(v2 - v1);
		glm::vec3 tangent = glm::cross(d, glm::vec3(0.0f, 0.0f, 1.0f));
		tangent = glm::normalize(tangent);

		vertex1.position.x = x1 + tangent.x * thickness;
		vertex1.position.y = y1 + tangent.y * thickness;
		vertex1.position.z = z1 + tangent.z * thickness;
		verticessb.push_back(vertex1);

		vertex2.position.x = x2 + tangent.x * thickness;
		vertex2.position.y = y2 + tangent.y * thickness;
		vertex2.position.z = z2 + tangent.z * thickness;
		verticessb.push_back(vertex2);

		vertex1.position.x = x1 - tangent.x * thickness;
		vertex1.position.y = y1 - tangent.y * thickness;
		vertex1.position.z = z1 - tangent.z * thickness;
		verticessb.push_back(vertex1);

		vertex2.position.x = x2 - tangent.x * thickness;
		vertex2.position.y = y2 - tangent.y * thickness;
		vertex2.position.z = z2 - tangent.z * thickness;
		verticessb.push_back(vertex2);

		indicessb = {0, 1, 3, 0, 2, 3};
		setDrawMode(gMesh::DRAWMODE_TRIANGLES);
	} else {
		vertex1.position.x = x1;
		vertex1.position.y = y1;
		vertex1.position.z = z1;
		verticessb.push_back(vertex1);

		vertex2.position.x = x2;
		vertex2.position.y = y2;
		vertex2.position.z = z2;
		verticessb.push_back(vertex2);
		setDrawMode(gMesh::DRAWMODE_LINES);
	}

	auto vertices = std::make_shared<std::vector<gVertex>>(verticessb);
	auto indices = std::make_shared<std::vector<gIndex>>(indicessb);
	setVertices(vertices, indices);
}
