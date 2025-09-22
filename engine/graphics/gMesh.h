/*
 * gMesh.h
 *
 *  Created on: May 7, 2020
 *      Author: noyan
 */

#ifndef ENGINE_GRAPHICS_GMESH_H_
#define ENGINE_GRAPHICS_GMESH_H_
/*
#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <glm/glm.hpp>
#include "glm/gtc/matrix_transform.hpp"
*/
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
	gMesh(const std::vector<gVertex>& vertices, const std::vector<gIndex>& indices, const std::vector<gTexture*>& textures);
	gMesh(std::shared_ptr<std::vector<gVertex>> vertices,
			std::shared_ptr<std::vector<gIndex>> indices,
			std::vector<gTexture*> textures);
	gMesh(const gMesh&);
	virtual ~gMesh();

	void setVertices(const std::vector<gVertex>& vertices, const std::vector<gIndex>& indices);
	void setVertices(const std::vector<gVertex>& vertices);
	void setVertices(std::shared_ptr<std::vector<gVertex>> vertices);
	void setVertices(std::shared_ptr<std::vector<gVertex>> vertices, std::shared_ptr<std::vector<gIndex>> indices);
	void setTextures(const std::vector<gTexture*>& textures);
	void setTexture(gTexture* texture);
	void addTexture(gTexture* tex);
	gTexture* getTexture(int textureNo);

	std::vector<gVertex>& getVertices();
	std::vector<gIndex>& getIndices();
	std::shared_ptr<std::vector<gVertex>> getVerticesPtr();
	std::shared_ptr<std::vector<gIndex>> getIndicesPtr();
	int getVerticesNum() const;
	int getIndicesNum() const;
	const gBoundingBox& getBoundingBox();
	gVbo& getVbo();
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

	void recalculateBoundingBox();

protected:
	void processTransformationMatrix() override;

    void drawStart();
    void drawVbo();
    void drawEnd();
	std::unique_ptr<gVbo> vbo;
	std::shared_ptr<std::vector<gVertex> >vertices;
    bool isprojection2d;

private:
    std::string name;
	std::shared_ptr<std::vector<gIndex>> indices;
	std::vector<gTexture*> textures;
	int drawmode;
    gMaterial material;

    unsigned int diffuseNr, specularNr, normalNr, heightNr;
    std::string texnumber;
    int textype;

    gLight* scenelight;
    gShader* colorshader;
    gShader* textureshader;
    gShader *pbrshader;

    gBoundingBox initialboundingbox;
    gBoundingBox boundingbox;
	bool needsboundingboxrecalculation;
};

#endif /* ENGINE_GRAPHICS_GMESH_H_ */
