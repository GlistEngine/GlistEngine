/*
* gPlane.cpp
 *
 *  Created on: Sep 1, 2020
 *      Author: noyan
 */

#include "gPlane.h"

gPlane::gPlane() {
    std::vector<gVertex> vertices(4);
    std::vector<gIndex> indices = {0, 3, 1, 1, 3, 2};

	// All vertices share the same normal, tangent, and bitangent for a flat plane
    const glm::vec3 normal(0.0f, 0.0f, 1.0f);
    const glm::vec3 tangent(1.0f, 0.0f, 0.0f);
    const glm::vec3 bitangent(0.0f, -1.0f, 0.0f);

    // Top right
    vertices[0].position = glm::vec3(1.0f, 1.0f, 0.0f);
    vertices[0].texcoords = glm::vec2(1.0f, 0.0f);

    // Bottom right
    vertices[1].position = glm::vec3(1.0f, -1.0f, 0.0f);
    vertices[1].texcoords = glm::vec2(1.0f, 1.0f);

    // Bottom left
    vertices[2].position = glm::vec3(-1.0f, -1.0f, 0.0f);
    vertices[2].texcoords = glm::vec2(0.0f, 1.0f);

    // Top left
    vertices[3].position = glm::vec3(-1.0f, 1.0f, 0.0f);
    vertices[3].texcoords = glm::vec2(0.0f, 0.0f);

    for (gVertex& vertex : vertices) {
        vertex.normal = normal;
        vertex.tangent = tangent;
        vertex.bitangent = bitangent;
    }

    setVertices(
        std::make_shared<std::vector<gVertex>>(std::move(vertices)),
        std::make_shared<std::vector<gIndex>>(std::move(indices))
    );
}

gPlane::~gPlane() {
}
