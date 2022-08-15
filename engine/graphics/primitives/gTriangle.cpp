/*
 * gTriangle.cpp
 *
 *  Created on: Jul 28, 2022
 *      Author: emirhantasdeviren
 */

#include "gTriangle.h"

gTriangle::gTriangle() {}

gTriangle::~gTriangle() {}

void gTriangle::draw(float px, float py, float qx, float qy, float rx, float ry, bool is_filled) {
	this->setPoints(px, py, qx, qy, rx, ry, is_filled);
	gMesh::draw();
}

void gTriangle::setPoints(float px, float py, float qx, float qy, float rx, float ry, bool is_filled) {
	this->isprojection2d = true;

	if (!this->vertices.empty()) {
		this->vertices.clear();
		this->indices.clear();
	}

	this->vertex1.position.x = px;
	this->vertex1.position.y = py;
	this->vertex1.position.z = 0.0f;
	this->vertices.push_back(vertex1);

	this->vertex2.position.x = qx;
	this->vertex2.position.y = qy;
	this->vertex2.position.z = 0.0f;
	this->vertices.push_back(vertex2);

	this->vertex3.position.x = rx;
	this->vertex3.position.y = ry;
	this->vertex3.position.z = 0.0f;
	this->vertices.push_back(vertex3);

	indices.push_back(0);
	indices.push_back(1);
	indices.push_back(2);
	indices.push_back(0);

	this->setVertices(this->vertices, this->indices);

	if (!is_filled) {
		this->setDrawMode(gMesh::DRAWMODE_LINESTRIP);
	} else {
		this->setDrawMode(gMesh::DRAWMODE_TRIANGLESTRIP);
	}
}
