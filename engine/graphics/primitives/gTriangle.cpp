/*
 * gTriangle.cpp
 *
 *  Created on: Jul 28, 2022
 *      Author: emirhantasdeviren
 */

#include "gTriangle.h"
#include <cmath>
#include <algorithm>

gTriangle::gTriangle() {}

gTriangle::~gTriangle() {}

void gTriangle::draw(float px, float py, float qx, float qy, float rx, float ry, bool is_filled) {
	this->draw(px, py, qx, qy, rx, ry, is_filled, 0.0f, 0.5f, 0.5f);
}

void gTriangle::draw(float px, float py, float qx, float qy, float rx, float ry, bool is_filled, float rotateAngle, float pivotx, float pivoty) {
	this->setPoints(px, py, qx, qy, rx, ry, is_filled, rotateAngle, pivotx, pivoty);
	gMesh::draw();
}

void gTriangle::setPoints(float px, float py, float qx, float qy, float rx, float ry, bool is_filled, float rotateAngle, float pivotx, float pivoty) {
	this->isprojection2d = true;

	if (!this->vertices.empty()) {
		this->vertices.clear();
		this->indices.clear();
	}

	float pivotPX;
	float pivotPY;

	// EÐER (0.5, 0.5) ÝSE: Üçgenin tam kütle/geometrik aðýrlýk merkezini al (yalpalanmadan döner)
	if (pivotx == 0.5f && pivoty == 0.5f) {
		pivotPX = (px + qx + rx) / 3.0f;
		pivotPY = (py + qy + ry) / 3.0f;
	} else {
		// ÖZEL PÝVOT DEÐERÝ GÝRÝLDÝYSE: Bounding Box (Sýnýrlayýcý Kutu) oranlarýný kullan
		float minX = std::min({px, qx, rx});
		float maxX = std::max({px, qx, rx});
		float minY = std::min({py, qy, ry});
		float maxY = std::max({py, qy, ry});

		float width = maxX - minX;
		float height = maxY - minY;

		pivotPX = minX + pivotx * width;
		pivotPY = minY + pivoty * height;
	}

	float cosA = std::cos(rotateAngle);
	float sinA = std::sin(rotateAngle);

	auto rotatePoint = [&](float vx, float vy) -> glm::vec3 {
		float dx = vx - pivotPX;
		float dy = vy - pivotPY;
		float rx_rot = dx * cosA - dy * sinA;
		float ry_rot = dx * sinA + dy * cosA;
		return glm::vec3(pivotPX + rx_rot, pivotPY + ry_rot, 0.0f);
	};

	this->vertex1.position = rotatePoint(px, py);
	this->vertices.push_back(this->vertex1);

	this->vertex2.position = rotatePoint(qx, qy);
	this->vertices.push_back(this->vertex2);

	this->vertex3.position = rotatePoint(rx, ry);
	this->vertices.push_back(this->vertex3);

	indices.push_back(0);
	indices.push_back(1);
	indices.push_back(2);
	indices.push_back(0);

	auto verticesptr = std::make_shared<std::vector<gVertex>>(this->vertices);
	auto indicesptr = std::make_shared<std::vector<gIndex>>(this->indices);
	this->setVertices(verticesptr, indicesptr);

	if (!is_filled) {
		this->setDrawMode(gMesh::DRAWMODE_LINESTRIP);
	} else {
		this->setDrawMode(gMesh::DRAWMODE_TRIANGLESTRIP);
	}
}
