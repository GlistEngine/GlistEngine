/*
 * gMesh.cpp
 *
 *  Created on: May 7, 2020
 *      Author: noyan
 */

#include <iosfwd>
#include <sstream>
#include <iostream>
#include <fstream>
#include <vector>
#include "gMesh.h"
#include "gLight.h"
#include <glm/gtx/intersect.hpp>


gMesh::gMesh() {
	name = "";
	sli = 0;
	ti = 0;
	drawmode = DRAWMODE_TRIANGLES;
	isprojection2d = false;
    diffuseNr  = 1;
    specularNr = 1;
    normalNr   = 1;
    heightNr   = 1;
    textype = 0;
    scenelight = nullptr;
    colorshader = nullptr;
    textureshader = nullptr;
    pbrshader = nullptr;
	bbminx = 0.0f, bbminy = 0.0f, bbminz = 0.0f;
	bbmaxx = 0.0f, bbmaxy = 0.0f, bbmaxz = 0.0f;
}

gMesh::gMesh(std::vector<gVertex> vertices, std::vector<unsigned int> indices, std::vector<gTexture> textures) {
	name = "";
	sli = 0;
	ti = 0;
	drawmode = DRAWMODE_TRIANGLES;
	isprojection2d = false;
    diffuseNr  = 1;
    specularNr = 1;
    normalNr   = 1;
    heightNr   = 1;
    textype = 0;

    setVertices(vertices, indices);
    setTextures(textures);
}

gMesh::~gMesh() {
}

void gMesh::setName(std::string name) {
	this->name = name;
}

std::string gMesh::getName() {
	return name;
}

void gMesh::setVertices(std::vector<gVertex> vertices, std::vector<unsigned int> indices) {
	this->vertices = vertices;
	this->indices = indices;
	vbo.setVertexData(&vertices[0], sizeof(gVertex), vertices.size());
	if (indices.size() != 0) vbo.setIndexData(&indices[0], indices.size());
    initialboundingbox = getBoundingBox();
	initialboundingbox.setTransformationMatrix(localtransformationmatrix);
}

std::vector<gVertex> gMesh::getVertices() {
	return vertices;
}

std::vector<unsigned int> gMesh::getIndices() {
	return indices;
}

void gMesh::setTextures(std::vector<gTexture>& textures) {
//	this->textures = textures;
    for(ti = 0; ti < textures.size(); ti++) {
        textype = textures[ti].getType();
        if(textype == gTexture::TEXTURETYPE_DIFFUSE) {
        	material.setDiffuseMap(&textures[ti]);
        } else if(textype == gTexture::TEXTURETYPE_SPECULAR) {
        	material.setSpecularMap(&textures[ti]);
        } else if(textype == gTexture::TEXTURETYPE_NORMAL) {
        	material.setNormalMap(&textures[ti]);
        } else if(textype == gTexture::TEXTURETYPE_HEIGHT) {
        	material.setHeightMap(&textures[ti]);
        } else if (textype == gTexture::TEXTURETYPE_PBR_ALBEDO) {
        	material.setAlbedoMap(&textures[ti]);
        } else if (textype == gTexture::TEXTURETYPE_PBR_ROUGHNESS) {
        	material.setRoughnessMap(&textures[ti]);
        } else if (textype == gTexture::TEXTURETYPE_PBR_METALNESS) {
        	material.setMetalnessMap(&textures[ti]);
        } else if (textype == gTexture::TEXTURETYPE_PBR_NORMAL) {
        	material.setPbrNormalMap(&textures[ti]);
        } else if (textype == gTexture::TEXTURETYPE_PBR_AO) {
        	material.setAOMap(&textures[ti]);
        }
    }
}

void gMesh::setTexture(gTexture* texture) {
    textype = texture->getType();
    if(textype == gTexture::TEXTURETYPE_DIFFUSE) {
    	material.setDiffuseMap(texture);
    } else if(textype == gTexture::TEXTURETYPE_SPECULAR) {
    	material.setSpecularMap(texture);
    } else if(textype == gTexture::TEXTURETYPE_NORMAL) {
    	material.setNormalMap(texture);
    } else if(textype == gTexture::TEXTURETYPE_HEIGHT) {
    	material.setHeightMap(texture);
    } else if (textype == gTexture::TEXTURETYPE_PBR_ALBEDO) {
    	material.setAlbedoMap(texture);
    } else if (textype == gTexture::TEXTURETYPE_PBR_ROUGHNESS) {
    	material.setRoughnessMap(texture);
    } else if (textype == gTexture::TEXTURETYPE_PBR_METALNESS) {
    	material.setMetalnessMap(texture);
    } else if (textype == gTexture::TEXTURETYPE_PBR_NORMAL) {
    	material.setPbrNormalMap(texture);
    } else if (textype == gTexture::TEXTURETYPE_PBR_AO) {
    	material.setAOMap(texture);
    }
}

void gMesh::addTexture(gTexture tex) {
	textures.push_back(tex);
}

gTexture* gMesh::getTexture(int textureNo) {
	return &textures[textureNo];
}


void gMesh::setDrawMode(int drawMode) {
	drawmode = drawMode;
}

int gMesh::getDrawMode() {
	return drawmode;
}

void gMesh::setMaterial(gMaterial* material) {
	this->material = *material;
}

gMaterial* gMesh::getMaterial() {
	return &material;
}


void gMesh::draw() {
	if (!isenabled) return;

	drawStart();
	drawVbo();
	drawEnd();
}

void gMesh::drawStart() {
	if (isshadowmappingenabled && renderpassno == 0) {
		renderer->getShadowmapShader()->use();
		renderer->getShadowmapShader()->setMat4("model", localtransformationmatrix);
		return;
	}

    if (textures.size() == 0 && !material.isPBR()) {
    	colorshader = renderer->getColorShader();
		colorshader->use();

	    // Set scene properties
//	    colorshader->setVec3("viewPos", 0.0f, 0.0f, 0.0f); //bunu shadowmap testi için kapattim
	    colorshader->setVec4("renderColor", renderer->getColor()->r, renderer->getColor()->g, renderer->getColor()->b, renderer->getColor()->a);

	    // Set material colors
	    colorshader->setVec4("material.ambient", material.getAmbientColor()->r, material.getAmbientColor()->g, material.getAmbientColor()->b, material.getAmbientColor()->a);
	    colorshader->setVec4("material.diffuse", material.getDiffuseColor()->r, material.getDiffuseColor()->g, material.getDiffuseColor()->b, material.getDiffuseColor()->a);
	    colorshader->setVec4("material.specular", material.getSpecularColor()->r, material.getSpecularColor()->g, material.getSpecularColor()->b, material.getSpecularColor()->a);
	    colorshader->setFloat("material.shininess", material.getShininess());

	    // Bind diffuse textures
	    colorshader->setInt("material.useDiffuseMap", material.isDiffuseMapEnabled());
//		if(material.isDiffuseMapEnabled()) gLogi("gModel") << "mesh name:" << name;
//		if(material.isDiffuseMapEnabled()) gLogi("gModel") << "diffuse texture name:" << material.getDiffuseMap()->getFilename();
	    if (material.isDiffuseMapEnabled()) {
		    colorshader->setInt("material.diffusemap", 0); // Diffuse texture unit
		    glActiveTexture(GL_TEXTURE0);
		    material.bindDiffuseMap();
	    }

	    // Bind specular textures
	    colorshader->setInt("material.useSpecularMap", material.isDiffuseMapEnabled() && material.isSpecularMapEnabled());
	    if (material.isDiffuseMapEnabled() && material.isSpecularMapEnabled()) {
		    colorshader->setInt("material.specularmap", 1); // Specular texture unit
		    glActiveTexture(GL_TEXTURE1);
		    material.bindSpecularMap();
	    }

	    // Bind normal textures
	    colorshader->setInt("aUseNormalMap", material.isDiffuseMapEnabled() && material.isNormalMapEnabled());
	    if (material.isDiffuseMapEnabled() && material.isNormalMapEnabled()) {
		    colorshader->setInt("material.normalmap", 2); // Normal texture unit
		    glActiveTexture(GL_TEXTURE2);
		    material.bindNormalMap();
	    }

	    // Set lights
	    if (renderer->isLightingEnabled()) {
	    	for (sli = 0; sli < renderer->getSceneLightNum(); sli++) {
	    		scenelight = renderer->getSceneLight(sli);
	    	    colorshader->setInt("light.type", scenelight->getType());
	    	    colorshader->setVec3("light.position", scenelight->getPosition());
	    	    colorshader->setVec3("light.direction", scenelight->getDirection());
	    	    colorshader->setFloat("light.cutOff", scenelight->getSpotCutOffAngle());
	    	    colorshader->setFloat("light.outerCutOff", scenelight->getSpotOuterCutOffAngle());
	    	    colorshader->setVec4("light.ambient", scenelight->getAmbientColorRed(), scenelight->getAmbientColorGreen(), scenelight->getAmbientColorBlue(), scenelight->getAmbientColorAlpha());
	    	    colorshader->setVec4("light.diffuse",  scenelight->getDiffuseColorRed(), scenelight->getDiffuseColorGreen(), scenelight->getDiffuseColorBlue(), scenelight->getDiffuseColorAlpha());
	    	    colorshader->setVec4("light.specular", scenelight->getSpecularColorRed(), scenelight->getSpecularColorGreen(), scenelight->getSpecularColorBlue(), scenelight->getSpecularColorAlpha());
	    	    colorshader->setFloat("light.constant", scenelight->getAttenuationConstant());
	    	    colorshader->setFloat("light.linear", scenelight->getAttenuationLinear());
	    	    colorshader->setFloat("light.quadratic", scenelight->getAttenuationQuadratic());
	    	}
	    } else {
    	    colorshader->setInt("light.type", gLight::LIGHTTYPE_AMBIENT);
    	    colorshader->setVec4("light.ambient", renderer->getLightingColor()->r, renderer->getLightingColor()->g, renderer->getLightingColor()->b, renderer->getLightingColor()->a);
	    }

	    // Set matrices
	    if(isprojection2d)colorshader->setMat4("projection", renderer->getProjectionMatrix2d());
	    else colorshader->setMat4("projection", renderer->getProjectionMatrix());
		colorshader->setMat4("view", renderer->getViewMatrix());
		colorshader->setMat4("model", localtransformationmatrix);
    } else if (textures.size() == 0 && material.isPBR()) {
    	pbrshader = renderer->getPbrShader();
    	pbrshader->use();
    	pbrshader->setMat4("projection", renderer->getProjectionMatrix());
	    pbrshader->setMat4("view", renderer->getViewMatrix());
    	pbrshader->setMat4("model", localtransformationmatrix);
    	pbrshader->setInt("albedoMap", 3);
    	pbrshader->setInt("normalMap", 4);
    	pbrshader->setInt("metallicMap", 5);
    	pbrshader->setInt("roughnessMap", 6);
    	pbrshader->setInt("aoMap", 7);
    	material.bindAlbedoMap();
    	material.bindPbrNormalMap();
    	material.bindMetalnessMap();
    	material.bindRoughnessMap();
    	material.bindAOMap();
	    if (renderer->isLightingEnabled()) {
    		pbrshader->setInt("lightNum", renderer->getSceneLightNum());
	    	for (sli = 0; sli < renderer->getSceneLightNum(); sli++) {
	    		pbrshader->setVec3("lightPositions[" + gToStr(sli) + "]", renderer->getSceneLight(sli)->getPosition());
	    		pbrshader->setVec3("lightColors[" + gToStr(sli) + "]", glm::vec3(renderer->getSceneLight(sli)->getDiffuseColor()->r, renderer->getSceneLight(sli)->getDiffuseColor()->g, renderer->getSceneLight(sli)->getDiffuseColor()->b));
	    	}
	    }


	} else {
		textureshader = renderer->getTextureShader();
        textureshader->use();
		// bind appropriate textures
	    diffuseNr  = 1;
	    specularNr = 1;
	    normalNr   = 1;
	    heightNr   = 1;
	    for(ti = 0; ti < textures.size(); ti++) {
	        glActiveTexture(GL_TEXTURE0 + ti); // active proper texture unit before binding

	        // retrieve texture number (the N in diffuse_textureN)
	        texnumber = "";
	        textype = textures[ti].getType();
	        if(textype == gTexture::TEXTURETYPE_DIFFUSE)
	            texnumber = gToStr(diffuseNr++);
	        else if(textype == gTexture::TEXTURETYPE_SPECULAR)
	            texnumber = gToStr(specularNr++); // transfer unsigned int to stream
	        else if(textype == gTexture::TEXTURETYPE_NORMAL)
	            texnumber = gToStr(normalNr++); // transfer unsigned int to stream
	         else if(textype == gTexture::TEXTURETYPE_HEIGHT)
	            texnumber = gToStr(heightNr++); // transfer unsigned int to stream

	        // Set the sampler to the correct texture unit
	        textureshader->setInt(gToStr(textype) + texnumber, ti);

	        // Bind the texture
	        textures[ti].bind();
	    }

	    if (isprojection2d) textureshader->setMat4("projection", renderer->getProjectionMatrix2d());
	    else textureshader->setMat4("projection", renderer->getProjectionMatrix());
	    textureshader->setMat4("view", renderer->getViewMatrix());
	    textureshader->setMat4("model", localtransformationmatrix);
	}
}

void gMesh::drawVbo() {
    // draw mesh
    vbo.bind();
    if (vbo.isIndexDataAllocated()) {
        glDrawElements(drawmode, vbo.getIndicesNum(), GL_UNSIGNED_INT, 0);
    } else {
    	glDrawArrays(drawmode, 0, vbo.getVerticesNum());
    }
    vbo.unbind();
}

void gMesh::drawEnd() {
    // set everything back to defaults.
    glActiveTexture(GL_TEXTURE0);
}

int gMesh::getVerticesNum() {
	return vbo.getVerticesNum();
}

int gMesh::getIndicesNum() {
	return vbo.getIndicesNum();
}

gVbo* gMesh::getVbo() {
	return &vbo;
}

gBoundingBox gMesh::getBoundingBox() {
	for (bbi = 0; bbi< vertices.size(); bbi++) {
//		bbvpos = vertices[bbi].position;
		bbvpos = glm::vec3(localtransformationmatrix * glm::vec4(vertices[bbi].position, 1.0));

		if (bbi == 0) {
			bbminx = bbvpos.x, bbminy = bbvpos.y, bbminz = bbvpos.z;
			bbmaxx = bbvpos.x, bbmaxy = bbvpos.y, bbmaxz = bbvpos.z;
			continue;
		}

		bbminx = std::min(bbminx, bbvpos.x);
		bbminy = std::min(bbminy, bbvpos.y);
		bbminz = std::min(bbminz, bbvpos.z);
		bbmaxx = std::max(bbmaxx, bbvpos.x);
		bbmaxy = std::max(bbmaxy, bbvpos.y);
		bbmaxz = std::max(bbmaxz, bbvpos.z);
	}

	return gBoundingBox(bbminx, bbminy, bbminz, bbmaxx, bbmaxy, bbmaxz, localtransformationmatrix);
}


gBoundingBox gMesh::getInitialBoundingBox() {
	return initialboundingbox;
}

bool gMesh::intersectsTriangles(gRay* ray) {
	glm::vec2 baryposition(0);
	float mindistance = std::numeric_limits<float>::max();
	float distance = 0.0f;
	bool intersecting = false;
	for (int i = 0; i < indices.size(); i += 3) {
		//iterate through all faces of the mesh since each face has 3 vertices
		glm::vec3 a = vertices[indices[i]].position;
		glm::vec3 b = vertices[indices[i + 1]].position;
		glm::vec3 c = vertices[indices[i + 2]].position;
		if(glm::intersectRayTriangle(ray->getOrigin(), ray->getDirection(), a, b, c, baryposition, distance)) {
			if(distance > 0) {
				intersecting = true;
				if(distance < mindistance) mindistance = distance;
			}
		}
	}
	return intersecting && mindistance > 0.0f && mindistance < ray->getLength();
}
