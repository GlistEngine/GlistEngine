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


gMesh::gMesh() {
	sli = 0;
	ti = 0;
    diffuseNr  = 1;
    specularNr = 1;
    normalNr   = 1;
    heightNr   = 1;
    textype = 0;
    scenelight = nullptr;
    colorshader = nullptr;
    textureshader = nullptr;
	bbminx = 0.0f, bbminy = 0.0f, bbminz = 0.0f;
	bbmaxx = 0.0f, bbmaxy = 0.0f, bbmaxz = 0.0f;
}

gMesh::gMesh(std::vector<gVertex> vertices, std::vector<unsigned int> indices, std::vector<gTexture> textures) {
	sli = 0;
	ti = 0;
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


void gMesh::setVertices(std::vector<gVertex> vertices, std::vector<unsigned int> indices) {
	this->vertices = vertices;
	this->indices = indices;
	vbo.setVertexData(&vertices[0], sizeof(gVertex), vertices.size());
	if (indices.size() != 0) vbo.setIndexData(&indices[0], indices.size());
    initialboundingbox = getBoundingBox();
}

std::vector<gVertex> gMesh::getVertices() {
	return vertices;
}

std::vector<unsigned int> gMesh::getIndices() {
	return indices;
}

void gMesh::setTextures(std::vector<gTexture> textures) {
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
        }
    }

}

void gMesh::addTexture(gTexture tex) {
	textures.push_back(tex);
}

gTexture* gMesh::getTexture(int textureNo) {
	return &textures[textureNo];
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
    if (textures.size() == 0) {
    	colorshader = renderer->getColorShader();
		colorshader->use();

	    // Set scene properties
	    colorshader->setVec3("viewPos", 0.0f, 0.0f, 0.0f);
	    colorshader->setVec4("renderColor", renderer->getColor()->r, renderer->getColor()->g, renderer->getColor()->b, renderer->getColor()->a);

	    // Set material colors
	    colorshader->setVec4("material.ambient", material.getAmbientColor()->r, material.getAmbientColor()->g, material.getAmbientColor()->b, material.getAmbientColor()->a);
	    colorshader->setVec4("material.diffuse", material.getDiffuseColor()->r, material.getDiffuseColor()->g, material.getDiffuseColor()->b, material.getDiffuseColor()->a);
	    colorshader->setVec4("material.specular", material.getSpecularColor()->r, material.getSpecularColor()->g, material.getSpecularColor()->b, material.getSpecularColor()->a);
	    colorshader->setFloat("material.shininess", material.getShininess());

	    // Bind diffuse textures
	    colorshader->setInt("material.useDiffuseMap", material.isDiffuseMapEnabled());
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
	    colorshader->setMat4("projection", renderer->getProjectionMatrix());
		colorshader->setMat4("view", renderer->getViewMatrix());
		colorshader->setMat4("model", localtransformationmatrix);
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

	    textureshader->setMat4("projection", renderer->getProjectionMatrix());
	    textureshader->setMat4("view", renderer->getViewMatrix());
	    textureshader->setMat4("model", localtransformationmatrix);
	}
}

void gMesh::drawVbo() {
    // draw mesh
    vbo.bind();
    if (vbo.isIndexDataAllocated()) {
        glDrawElements(GL_TRIANGLES, vbo.getIndicesNum(), GL_UNSIGNED_INT, 0);
    } else {
    	glDrawArrays(GL_TRIANGLES, 0, vbo.getVerticesNum());
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
	bbminx = 0.0f, bbminy = 0.0f, bbminz = 0.0f;
	bbmaxx = 0.0f, bbmaxy = 0.0f, bbmaxz = 0.0f;

	for (int i = 0; i< vertices.size(); i++) {
		gVertex v = vertices[i];
		glm::vec3 vpos = glm::vec3(localtransformationmatrix * glm::vec4(v.position, 1.0));

		bbminx = std::min(bbminx, vpos.x);
		bbminy = std::min(bbminy, vpos.y);
		bbminz = std::min(bbminz, vpos.z);
		bbmaxx = std::max(bbmaxx, vpos.x);
		bbmaxy = std::max(bbmaxy, vpos.y);
		bbmaxz = std::max(bbmaxz, vpos.z);
	}

	return gBoundingBox(bbminx, bbminy, bbminz, bbmaxx, bbmaxy, bbmaxz);
}

gBoundingBox gMesh::getInitialBoundingBox() {
	return initialboundingbox;
}

