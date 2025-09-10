/*
 * gVbo.h
 *
 *  Created on: 25 Kas 2020
 *      Author: Acer
 */

#ifndef GRAPHICS_GVBO_H_
#define GRAPHICS_GVBO_H_

#include "gRenderObject.h"


struct gVertex {
    // position
    glm::vec3 position{0};
    // normal
    glm::vec3 normal{0};
    // texCoords
    glm::vec2 texcoords{0};
    // tangent
    glm::vec3 tangent{0};
    // bitangent
    glm::vec3 bitangent{0};
};


class gVbo : public gRenderObject {
public:
	gVbo();
	virtual ~gVbo();

	gVbo(const gVbo&) = delete;
	gVbo& operator=(const gVbo& other) = delete;

	void setVertexData(const gVertex* vertices, int coordNum, int total);
	void setIndexData(const gIndex* indices, int total);
	void clear();

	void bind() const;
	void unbind() const;

	void enable();
	void disable();
	bool isEnabled() const;
	bool isVertexDataAllocated() const;
	bool isIndexDataAllocated() const;

//	void setMesh(const gMesh* mesh, int usage);
	void setVertexData(const glm::vec3* vertices, int total, int usage = GL_STATIC_DRAW);
	void setVertexData(const glm::vec2* vertices, int total, int usage = GL_STATIC_DRAW);
	void setVertexData(const float* verticesptr, int coordNum, int total, int usage, int stride = 0);
	void setColorData(const gColor* colors, int total, int usage);
	void setColorData(const float* color0r, int total, int usage, int stride = 0);
	void setTexCoordData(const glm::vec2* texCoords, int total, int usage);
	void setTexCoordData(const float* texCoord0x, int total, int usage, int stride = 0);
	void setNormalData(const glm::vec3* normals, int total, int usage = GL_STATIC_DRAW);
	void setNormalData(const float* normal0x, int total, int usage, int stride = 0);
	void setIndexData(const int* indices, int total, int usage);

	int getVAOid() const;
	int getVerticesNum() const;
	int getIndicesNum() const;


private:
    GLuint vao;
    GLuint vbo, ebo;
    bool isenabled;

    bool isvertexdataallocated;
    int vertexdatacoordnum, totalvertexnum;
    int vertexusage, vertexstride;

    bool isindexdataallocated;
    int totalindexnum;

};

#endif /* GRAPHICS_GVBO_H_ */
