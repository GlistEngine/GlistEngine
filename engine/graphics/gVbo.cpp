/*
 * gVbo.cpp
 *
 *  Created on: 25 Kas 2020
 *      Author: Acer
 */

#include "gVbo.h"
#include "gLight.h"
#include "gShader.h"
#include "gTracy.h"

gVbo::gVbo() {
	vao = renderer->createVAO();
	vbo = 0;
	ebo = 0;
	isenabled = true;
	isvertexdataallocated = false;
	vertexdatacoordnum = 0;
	totalvertexnum = 0;
	isindexdataallocated = false;
	totalindexnum = 0;
}

gVbo::~gVbo() {
	clear();
}

void gVbo::setVertexData(const gVertex* vertices, int coordNum, int total) {
	G_PROFILE_ZONE_SCOPED_N("gVbo::setVertexData()");
	if (vao == GL_NONE) {
		vao = renderer->createVAO();
	}
	bind();
	if (!isvertexdataallocated) {
		vbo = renderer->genBuffers();
		isvertexdataallocated = true;
	}
	vertexdatacoordnum = coordNum;
	totalvertexnum = total;

	renderer->bindBuffer(GL_ARRAY_BUFFER, vbo);
	renderer->setVertexBufferData(vbo, totalvertexnum * sizeof(gVertex), vertices, GL_STATIC_DRAW);
	renderer->enableVertexAttrib(0);
	renderer->setVertexAttribPointer(0, 3, GL_FLOAT, false, sizeof(gVertex), (void*)0);
    // vertex normals
	renderer->enableVertexAttrib(1);
	renderer->setVertexAttribPointer(1, 3, GL_FLOAT, false, sizeof(gVertex), (void*)offsetof(gVertex, normal));
    // vertex texture coords
	renderer->enableVertexAttrib(2);
	renderer->setVertexAttribPointer(2, 2, GL_FLOAT, false, sizeof(gVertex), (void*)offsetof(gVertex, texcoords));
    // vertex tangent
	renderer->enableVertexAttrib(3);
	renderer->setVertexAttribPointer(3, 3, GL_FLOAT, false, sizeof(gVertex), (void*)offsetof(gVertex, tangent));
    // vertex bitangent
	renderer->enableVertexAttrib(4);
	renderer->setVertexAttribPointer(4, 3, GL_FLOAT, false, sizeof(gVertex), (void*)offsetof(gVertex, bitangent));
	unbind();
}

void gVbo::setVertexData(const float* verticesptr, int coordNum, int total, int usage, int stride) {
	if (vao == GL_NONE) {
		vao = renderer->createVAO();
	}
	bind();
	if (!isvertexdataallocated) {
		vbo = renderer->genBuffers();
		isvertexdataallocated = true;
    }
	vertexdatacoordnum = coordNum;
	totalvertexnum = total;
	vertexusage = usage;
	vertexstride = stride;


	GLsizeiptr size = stride == 0 ? vertexdatacoordnum * sizeof(float) : stride;
	renderer->setVertexBufferData(vbo, totalvertexnum * size, verticesptr, usage);
	renderer->enableVertexAttrib(0);
	renderer->setVertexAttribPointer(0, vertexdatacoordnum, GL_FLOAT, GL_FALSE, vertexdatacoordnum * sizeof(float), nullptr);
	unbind();
}

void gVbo::setIndexData(const gIndex* indices, int total) {
	if (indices == nullptr) {
		glDeleteBuffers(1, &ebo);
		isindexdataallocated = false;
		return;
	}
	if (vao == GL_NONE) {
		vao = renderer->createVAO();
	}
	bind();
	if (!isindexdataallocated) {
		ebo = renderer->genBuffers();
		isindexdataallocated = true;
    }
	totalindexnum = total;

	renderer->setIndexBufferData(ebo, totalindexnum * sizeof(gIndex), indices, GL_STATIC_DRAW);
	unbind();
}

void gVbo::clear() {
	if(isvertexdataallocated) {
		renderer->deleteBuffer(vbo);
		isvertexdataallocated = false;
	}
	if(isindexdataallocated) {
		renderer->deleteBuffer(ebo);
		isvertexdataallocated = false;
    }
	if (vao != GL_NONE) {
		renderer->deleteVAO(vao);
		vao = GL_NONE;
	}
}

void gVbo::bind() const {
#ifdef DEBUG
	assert(vao != GL_NONE);
#endif
	renderer->bindVAO(vao);
}

void gVbo::unbind() const {
	renderer->unbindVAO();
}

void gVbo::enable() {
	isenabled = true;
}

void gVbo::disable() {
	isenabled = false;
}

bool gVbo::isVertexDataAllocated() const {
	return isvertexdataallocated;
}

bool gVbo::isIndexDataAllocated() const {
	return isindexdataallocated;
}

void gVbo::setVertexData(const glm::vec3* vertices, int total, int usage) {
	setVertexData(&vertices[0].x, 3, total, usage);
}

void gVbo::setVertexData(const glm::vec2* vertices, int total, int usage) {
	setVertexData(&vertices[0].x, 2, total, usage);
}

void gVbo::setColorData(const gColor* colors, int total, int usage) {
	setColorData(&colors[0].r, total, usage);
}

void gVbo::setColorData(const float* color0r, int total, int usage, int stride) {

}

void gVbo::setTexCoordData(const glm::vec2* texCoords, int total, int usage) {
	setTexCoordData(&texCoords[0].x, total, usage);
}

void gVbo::setTexCoordData(const float* texCoord0x, int total, int usage, int stride) {

}

void gVbo::setNormalData(const glm::vec3* normals, int total, int usage) {
	setNormalData(&normals[0].x, total, usage);
}

void gVbo::setNormalData(const float* normal0x, int total, int usage, int stride) {

}

void gVbo::setIndexData(const int* indices, int total, int usage) {

}

int gVbo::getVAOid() const {
	return vao;
}

int gVbo::getVerticesNum() const {
	return totalvertexnum;
}

int gVbo::getIndicesNum() const {
	return totalindexnum;
}
