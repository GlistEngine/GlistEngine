/*
 * gVbo.cpp
 *
 *  Created on: 25 Kas 2020
 *      Author: Acer
 */

#include "gVbo.h"
#include "gLight.h"
#include "gShader.h"

gVbo::gVbo() {
	glGenVertexArrays(1, &vao);
	vbo = 0;
	ebo = 0;
	isenabled = true;
	isvertexdataallocated = false;
	verticesptr = nullptr;
	vertexarrayptr = nullptr;
	vertexdatacoordnum = 0;
	totalvertexnum = 0;
	isindexdataallocated = false;
	indexarrayptr = nullptr;
	totalindexnum = 0;
}

gVbo::~gVbo() {
	clear();
}

gVbo::gVbo(const gVbo& other) {
	glGenVertexArrays(1, &vao);
	vbo = 0;
	ebo = 0;
	isenabled = true;
	isvertexdataallocated = false;
	verticesptr = nullptr;
	vertexarrayptr = nullptr;
	vertexdatacoordnum = 0;
	totalvertexnum = 0;
	isindexdataallocated = false;
	indexarrayptr = nullptr;
	totalindexnum = 0;

	isenabled = other.isenabled;
	if (other.verticesptr) {
		setVertexData(other.verticesptr, other.vertexdatacoordnum, other.totalvertexnum);
	} else if (other.vertexarrayptr) {
		setVertexData(other.vertexarrayptr, other.vertexdatacoordnum, other.totalvertexnum, other.vertexusage, other.vertexstride);
	}
	if (other.indexarrayptr) {
		setIndexData(other.indexarrayptr, other.totalindexnum);
	}
}

gVbo& gVbo::operator=(const gVbo& other) {
	if (this == &other) {
		return *this;
	}
	// clear current
	clear();
	verticesptr = nullptr;
	vertexarrayptr = nullptr;
	vertexdatacoordnum = 0;
	totalvertexnum = 0;
	isindexdataallocated = false;
	indexarrayptr = nullptr;
	totalindexnum = 0;
	// generate vao
	glGenVertexArrays(1, &vao);
	// copy from the other gVbo
	isenabled = other.isenabled;
	if (other.verticesptr) {
		setVertexData(other.verticesptr, other.vertexdatacoordnum, other.totalvertexnum);
	} else if (other.vertexarrayptr) {
		setVertexData(other.vertexarrayptr, other.vertexdatacoordnum, other.totalvertexnum, other.vertexusage, other.vertexstride);
	}
	if (other.indexarrayptr) {
		setIndexData(other.indexarrayptr, other.totalindexnum);
	}
	return *this;
}

void gVbo::setVertexData(gVertex* vertices, int coordNum, int total) {
	if (vao == GL_NONE) {
		glGenVertexArrays(1, &vao);
	}
	bind();
	if (!isvertexdataallocated) {
		glGenBuffers(1, &vbo);
		isvertexdataallocated = true;
	}
	verticesptr = &vertices[0];
	vertexarrayptr = &vertices[0].position.x;
	vertexdatacoordnum = coordNum;
	totalvertexnum = total;

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, totalvertexnum * sizeof(gVertex), vertexarrayptr, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(gVertex), (void*)0);
    // vertex normals
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(gVertex), (void*)offsetof(gVertex, normal));
    // vertex texture coords
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(gVertex), (void*)offsetof(gVertex, texcoords));
    // vertex tangent
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(gVertex), (void*)offsetof(gVertex, tangent));
    // vertex bitangent
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(gVertex), (void*)offsetof(gVertex, bitangent));
	unbind();
}

void gVbo::setVertexData(const float* vertexData, int coordNum, int total, int usage, int stride) {
	if (vao == GL_NONE) {
		glGenVertexArrays(1, &vao);
	}
	bind();
	if (!isvertexdataallocated) {
      glGenBuffers(1, &vbo);
      isvertexdataallocated = true;
    }
	vertexarrayptr = vertexData;
	vertexdatacoordnum = coordNum;
	totalvertexnum = total;
	vertexusage = usage;
	vertexstride = stride;

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	GLsizeiptr size = (stride == 0) ? vertexdatacoordnum * sizeof(float) : stride;
	glBufferData(GL_ARRAY_BUFFER, totalvertexnum * size, vertexarrayptr, usage);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, vertexdatacoordnum, GL_FLOAT, GL_FALSE, vertexdatacoordnum * sizeof(float),nullptr);
	unbind();
}

void gVbo::setIndexData(gIndex* indices, int total) {
	if (vao == GL_NONE) {
		glGenVertexArrays(1, &vao);
	}
	bind();
	if (!isindexdataallocated) {
      glGenBuffers(1, &ebo);
      isindexdataallocated = true;
    }
	indexarrayptr = indices;
	totalindexnum = total;

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, totalindexnum * sizeof(gIndex), indexarrayptr, GL_STATIC_DRAW);
	unbind();
}

void gVbo::clear() {
	if(isvertexdataallocated) {
	  glDeleteBuffers(1, &vbo);
	  isvertexdataallocated = false;
	}
	if(isindexdataallocated) {
      glDeleteBuffers(1, &ebo);
      isvertexdataallocated = false;
    }
	if (vao != GL_NONE) {
	  glDeleteVertexArrays(1, &vao);
	  vao = GL_NONE;
	}
}

gVertex* gVbo::getVertices() const {
	return verticesptr;
}

gIndex* gVbo::getIndices() const {
	return indexarrayptr;
}

void gVbo::bind() const {
#ifdef DEBUG
	assert(vao != GL_NONE);
#endif
	G_CHECK_GL(glBindVertexArray(vao));
}

void gVbo::unbind() const {
	G_CHECK_GL(glBindVertexArray(0));
}

void gVbo::draw() {
	draw(GL_TRIANGLES);
}

/**
 * drawMode: GL_TRIANGLES
 */
void gVbo::draw(int drawMode) {
	if (!isenabled) return;
	if(!isvertexdataallocated) {
		logw("Vertex data is not allocated!");
		return;
	}

	gShader* colorshader = renderer->getColorShader();
	colorshader->use();

    // Set scene properties
    colorshader->setVec4("renderColor", renderer->getColor()->r, renderer->getColor()->g, renderer->getColor()->b, renderer->getColor()->a);

    // Set matrices
    colorshader->setMat4("projection", glm::mat4(1.0f));
	colorshader->setMat4("view", glm::mat4(1.0f));
	colorshader->setMat4("model", glm::mat4(1.0f));

    bind();
	if (isindexdataallocated) {
	    glDrawElements(drawMode, totalindexnum, GL_UNSIGNED_INT, 0);
	} else {
		glDrawArrays(drawMode, 0, totalvertexnum);
	}
    unbind();
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
