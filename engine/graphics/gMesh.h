/*
 * gMesh.h
 *
 *  Created on: May 7, 2020
 *      Author: noyan
 */

#ifndef ENGINE_GRAPHICS_GMESH_H_
#define ENGINE_GRAPHICS_GMESH_H_

#include <vector>

#include "gNode.h"
#include "gTexture.h"
#include "gMaterial.h"
#include "gVbo.h"
#include "gBoundingBox.h"

class gMesh : public gNode {
public:
	static const int DRAWMODE_POINTS = GL_POINTS, DRAWMODE_LINES = GL_LINES, DRAWMODE_LINELOOP = GL_LINE_LOOP, DRAWMODE_LINESTRIP = GL_LINE_STRIP,
	DRAWMODE_TRIANGLES = GL_TRIANGLES, DRAWMODE_TRIANGLESTRIP = GL_TRIANGLE_STRIP, DRAWMODE_TRIANGLEFAN = GL_TRIANGLE_FAN;

	gMesh();
	gMesh(std::vector<gVertex> vertices, std::vector<gIndex> indices, std::vector<gTexture*> textures);
	virtual ~gMesh();

	void setVertices(std::vector<gVertex> vertices, std::vector<gIndex> indices = std::vector<gIndex>());
	void setTextures(std::vector<gTexture*> textures);
	void setTexture(gTexture* texture);
	void addTexture(gTexture* tex);
	gTexture* getTexture(int textureNo);

	std::vector<gVertex>& getVertices();
	std::vector<gIndex>& getIndices();
	int getVerticesNum() const;
	int getIndicesNum() const;
	gBoundingBox getBoundingBox();
	gVbo* getVbo();
	void clear();

	void setName(std::string name);
	const std::string& getName() const;

	void setDrawMode(int drawMode);
	int getDrawMode() const;

	void setMaterial(gMaterial* material);
	gMaterial* getMaterial();

	void draw();

    const gBoundingBox& getInitialBoundingBox() const;
    bool intersectsTriangles(gRay* ray);
    float distanceTriangles(gRay* ray);

protected:
    void drawStart();
    void drawVbo();
    void drawEnd();
	gVbo vbo;
	std::vector<gVertex> vertices;
    bool isprojection2d;

private:
    std::string name;
	std::vector<gIndex> indices;
	std::vector<gTexture*> textures;
	int drawmode;
    gMaterial material;
    int sli;
    unsigned int ti;

    unsigned int diffuseNr, specularNr, normalNr, heightNr;
    std::string texnumber;
    int textype;

    gLight* scenelight;
    gShader* colorshader;
    gShader* textureshader;
    gShader *pbrshader;

    float bbminx, bbminy, bbminz, bbmaxx, bbmaxy, bbmaxz;
    glm::vec3 bbvpos;
    int bbi;

    gBoundingBox initialboundingbox;
};

#endif /* ENGINE_GRAPHICS_GMESH_H_ */
