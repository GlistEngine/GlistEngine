/*
* gPlane.cpp
 *
 *  Created on: Sep 1, 2020
 *      Author: noyan
 */

#include "gPlane.h"

gPlane::gPlane() {
	tilingx = 1.0f;
	tilingy = 1.0f;
	initializePlane();
}

gPlane::~gPlane() {
}

void gPlane::setTextureTiling(float tilingX, float tilingY) {
	tilingx = tilingX;
	tilingy = tilingY;
	updateTextureCoordinates();
}

void gPlane::setTextureTiling(float tiling) {
	setTextureTiling(tiling, tiling);
}

void gPlane::initializePlane() {
	std::vector<gVertex> vertices(4);
	std::vector<gIndex> indices = {0, 1, 3, 1, 2, 3};

	// All vertices share the same normal, tangent, and bitangent for a flat plane
	glm::vec3 normal(0.0f, 0.0f, 1.0f);
	glm::vec3 tangent(1.0f, 0.0f, 0.0f);
	glm::vec3 bitangent(0.0f, -1.0f, 0.0f);

	// Top right
	vertices[0].position = glm::vec3(1.0f, 1.0f, 0.0f);
	vertices[0].texcoords = glm::vec2(tilingx, 0.0f);
	vertices[0].normal = normal;
	vertices[0].tangent = tangent;
	vertices[0].bitangent = bitangent;

	// Bottom right
	vertices[1].position = glm::vec3(1.0f, -1.0f, 0.0f);
	vertices[1].texcoords = glm::vec2(tilingx, tilingy);
	vertices[1].normal = normal;
	vertices[1].tangent = tangent;
	vertices[1].bitangent = bitangent;

	// Bottom left
	vertices[2].position = glm::vec3(-1.0f, -1.0f, 0.0f);
	vertices[2].texcoords = glm::vec2(0.0f, tilingy);
	vertices[2].normal = normal;
	vertices[2].tangent = tangent;
	vertices[2].bitangent = bitangent;

	// Top left
	vertices[3].position = glm::vec3(-1.0f, 1.0f, 0.0f);
	vertices[3].texcoords = glm::vec2(0.0f, 0.0f);
	vertices[3].normal = normal;
	vertices[3].tangent = tangent;
	vertices[3].bitangent = bitangent;

	setVertices(std::make_shared<std::vector<gVertex>>(std::move(vertices)),
				std::make_shared<std::vector<gIndex>>(std::move(indices)));
}

void gPlane::updateTextureCoordinates() {
	initializePlane();
}
