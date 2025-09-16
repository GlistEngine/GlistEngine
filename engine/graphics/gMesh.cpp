/*
 * gMesh.cpp
 *
 *  Created on: May 7, 2020
 *      Author: noyan
 */

#include "gMesh.h"
#include <vector>
#include <glm/gtx/intersect.hpp>
#if defined(__i386__) || defined(__x86_64__)
#include <immintrin.h>
#elif defined(__ARM_NEON)
#include <arm_neon.h>
#endif

#include "gLight.h"
#include "gShader.h"
#include "gTracy.h"

gMesh::gMesh() {
	vbo = std::make_unique<gVbo>();
	name = "";
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
	needsboundingboxrecalculation = false;
}

gMesh::gMesh(const std::vector<gVertex>& vertices, const std::vector<gIndex>& indices, const std::vector<gTexture*>& textures) {
	vbo = std::make_unique<gVbo>();
	name = "";
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

gMesh::gMesh(const gMesh& other) {
	vbo = std::make_unique<gVbo>();
	name = other.name;
	drawmode = DRAWMODE_TRIANGLES;
	isprojection2d = false;
	diffuseNr  = 1;
	specularNr = 1;
	normalNr   = 1;
	heightNr   = 1;
	textype = 0;
	setVertices(other.vertices, other.indices);
	setTextures(other.textures);
}

gMesh::~gMesh() {
}

void gMesh::clear() {
	vbo->clear();
}

void gMesh::setName(std::string name) {
	this->name = name;
}

const std::string& gMesh::getName() const {
	return name;
}

void gMesh::setVertices(const std::vector<gVertex>& vertices, const std::vector<gIndex>& indices) {
	bool resetinitialboundingbox = this->vertices.size() != vertices.size() || this->indices.size() != indices.size();
	this->vertices = vertices;
	this->indices = indices;
	vbo->setVertexData(vertices.data(), sizeof(gVertex), vertices.size());
	if (!indices.empty()) {
		vbo->setIndexData(indices.data(), indices.size());
	} else {
		vbo->setIndexData(nullptr, 0);
	}
	if (resetinitialboundingbox) {
		recalculateBoundingBox();
		initialboundingbox = boundingbox;
	} else {
		needsboundingboxrecalculation = true;
	}
//	initialboundingbox.setTransformationMatrix(localtransformationmatrix);
}

void gMesh::setVertices(const std::vector<gVertex>& vertices) {
	G_PROFILE_ZONE_SCOPED_N("gModel::setVertices()");
	bool resetinitialboundingbox = this->vertices.size() != vertices.size();
	this->vertices = vertices;
	vbo->setVertexData(vertices.data(), sizeof(gVertex), vertices.size());
	if (resetinitialboundingbox) {
		recalculateBoundingBox();
		initialboundingbox = boundingbox;
	} else {
		needsboundingboxrecalculation = true;
	}
}

std::vector<gVertex>& gMesh::getVertices() {
	return vertices;
}

std::vector<gIndex>& gMesh::getIndices() {
	return indices;
}

void gMesh::setTextures(const std::vector<gTexture*>& textures) {
//	this->textures = textures;
    for(int i = 0; i < textures.size(); i++) {
        textype = textures[i]->getType();
        if(textype == gTexture::TEXTURETYPE_DIFFUSE) {
        	material.setDiffuseMap(textures[i]);
        } else if(textype == gTexture::TEXTURETYPE_SPECULAR) {
        	material.setSpecularMap(textures[i]);
        } else if(textype == gTexture::TEXTURETYPE_NORMAL) {
        	material.setNormalMap(textures[i]);
        } else if(textype == gTexture::TEXTURETYPE_HEIGHT) {
        	material.setHeightMap(textures[i]);
        } else if (textype == gTexture::TEXTURETYPE_PBR_ALBEDO) {
        	material.setAlbedoMap(textures[i]);
        } else if (textype == gTexture::TEXTURETYPE_PBR_ROUGHNESS) {
        	material.setRoughnessMap(textures[i]);
        } else if (textype == gTexture::TEXTURETYPE_PBR_METALNESS) {
        	material.setMetalnessMap(textures[i]);
        } else if (textype == gTexture::TEXTURETYPE_PBR_NORMAL) {
        	material.setPbrNormalMap(textures[i]);
        } else if (textype == gTexture::TEXTURETYPE_PBR_AO) {
        	material.setAOMap(textures[i]);
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

void gMesh::addTexture(gTexture* tex) {
	textures.push_back(tex);
}

gTexture* gMesh::getTexture(int textureNo) {
	return textures[textureNo];
}


void gMesh::setDrawMode(int drawMode) {
	drawmode = drawMode;
}

int gMesh::getDrawMode() const {
	return drawmode;
}

void gMesh::setMaterial(gMaterial* material) {
	this->material = *material;
}

gMaterial* gMesh::getMaterial() {
	return &material;
}


void gMesh::draw() {
	G_PROFILE_ZONE_SCOPED_N("gMesh::draw()");
	if (!isenabled) return;

	drawStart();
	drawVbo();
	drawEnd();
}

void gMesh::processTransformationMatrix() {
	G_PROFILE_ZONE_SCOPED_N("gMesh::processTransformationMatrix()");
	if (needsboundingboxrecalculation) {
		gNode::processTransformationMatrix();
		return;
	}

	bool positionchanged = (position != prevposition);
	bool orientationchanged = (orientation != prevorientation);
	bool scalechanged = (scalevec != prevscalevec);
	// Recalculate bounding box only if orientation or scale has changed
	if (orientationchanged || scalechanged) {
		// todo maybe impelement a way to rotate and scale the bb without fully recalculating?
		needsboundingboxrecalculation = true;
	}
	if (positionchanged && !needsboundingboxrecalculation) {
		glm::vec3 posdiff = position - prevposition;
		boundingbox.set(boundingbox.minX() + posdiff.x, boundingbox.minY() + posdiff.y, boundingbox.minZ() + posdiff.z,
						boundingbox.maxX() + posdiff.x, boundingbox.maxY() + posdiff.y, boundingbox.maxZ() + posdiff.z);
	}
	gNode::processTransformationMatrix();
}

void gMesh::drawStart() {
	G_PROFILE_ZONE_SCOPED_N("gMesh::drawStart()");
	if (isshadowmappingenabled && renderpassno == 0) {
		renderer->getShadowmapShader()->use();
		renderer->getShadowmapShader()->setMat4("model", localtransformationmatrix);
		return;
	}

    if (textures.empty() && !material.isPBR()) {
    	colorshader = renderer->getColorShader();
		colorshader->use();

	    // Set scene properties
	    colorshader->setVec3("viewPos", renderer->getCameraPosition());
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
	    	renderer->activateTexture(0);
		    material.bindDiffuseMap();
	    }

	    // Bind specular textures
	    colorshader->setInt("material.useSpecularMap", material.isDiffuseMapEnabled() && material.isSpecularMapEnabled());
	    if (material.isDiffuseMapEnabled() && material.isSpecularMapEnabled()) {
			colorshader->setInt("material.specularmap", 1); // Specular texture unit
	    	renderer->activateTexture(1);
		    material.bindSpecularMap();
	    }

	    // Bind normal textures
	    colorshader->setInt("aUseNormalMap", material.isDiffuseMapEnabled() && material.isNormalMapEnabled());
	    if (material.isDiffuseMapEnabled() && material.isNormalMapEnabled()) {
			colorshader->setInt("material.normalmap", 2); // Normal texture unit
	    	renderer->activateTexture(2);
		    material.bindNormalMap();
	    }

		// todo use uniform buffer objects for fog too
		if(renderer->isFogEnabled()) {
			colorshader->setBool("fog.enabled", true);
			colorshader->setVec3("fog.color", renderer->getFogColor().r, renderer->getFogColor().g, renderer->getFogColor().b);
			colorshader->setInt("fog.mode", renderer->getFogMode());
			colorshader->setFloat("fog.density", renderer->getFogDensity());
			colorshader->setFloat("fog.gradient", renderer->getFogGradient());
			colorshader->setFloat("fog.linearStart", renderer->getFogLinearStart());
			colorshader->setFloat("fog.linearEnd", renderer->getFogLinearEnd());
		} else {
			colorshader->setBool("fog.enabled", false);
		}

		if(renderer->isSSAOEnabled()) {
			colorshader->setBool("ssao_enabled", true);
			colorshader->setFloat("ssao_bias", renderer->getSSAOBias());
		} else {
			colorshader->setBool("ssao_enabled", false);
		}

	    // Set matrices
	    if(isprojection2d)colorshader->setMat4("projection", renderer->getProjectionMatrix2d());
	    else colorshader->setMat4("projection", renderer->getProjectionMatrix());
		colorshader->setMat4("view", renderer->getViewMatrix());
		colorshader->setMat4("model", localtransformationmatrix);
    } else if (textures.empty() && material.isPBR()) {
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
	} else {
		textureshader = renderer->getTextureShader();
        textureshader->use();
		// bind appropriate textures
	    diffuseNr  = 1;
	    specularNr = 1;
	    normalNr   = 1;
	    heightNr   = 1;
	    for(int i = 0; i < textures.size(); i++) {
	    	renderer->activateTexture(i); // active proper texture unit before binding

	        // retrieve texture number (the N in diffuse_textureN)
	        texnumber = "";
	        textype = textures[i]->getType();
	        if(textype == gTexture::TEXTURETYPE_DIFFUSE)
	            texnumber = gToStr(diffuseNr++);
	        else if(textype == gTexture::TEXTURETYPE_SPECULAR)
	            texnumber = gToStr(specularNr++); // transfer unsigned int to stream
	        else if(textype == gTexture::TEXTURETYPE_NORMAL)
	            texnumber = gToStr(normalNr++); // transfer unsigned int to stream
	        else if(textype == gTexture::TEXTURETYPE_HEIGHT)
	            texnumber = gToStr(heightNr++); // transfer unsigned int to stream

	        // Set the sampler to the correct texture unit
	        textureshader->setInt(gToStr(textype) + texnumber, i);

	        // Bind the texture
	        textures[i]->bind();
	    }

	    if (isprojection2d) textureshader->setMat4("projection", renderer->getProjectionMatrix2d());
	    else textureshader->setMat4("projection", renderer->getProjectionMatrix());
	    textureshader->setMat4("view", renderer->getViewMatrix());
	    textureshader->setMat4("model", localtransformationmatrix);
	}
}

void gMesh::drawVbo() {
	G_PROFILE_ZONE_SCOPED_N("gMesh::drawVbo()");
    // draw mesh
    vbo->bind();
    if (vbo->isIndexDataAllocated()) {
    	renderer->drawElements(drawmode, vbo->getIndicesNum());
    } else {
    	renderer->drawArrays(drawmode, vbo->getVerticesNum());
    }
    vbo->unbind();
//    vbo.clear();
}

void gMesh::drawEnd() {
	G_PROFILE_ZONE_SCOPED_N("gMesh::drawEnd()");
	// set everything back to defaults.
	renderer->activateTexture(0);
}

int gMesh::getVerticesNum() const {
	return vertices.size();
}

int gMesh::getIndicesNum() const {
	return indices.size();
}

gVbo& gMesh::getVbo() {
	return *vbo;
}

const gBoundingBox& gMesh::getBoundingBox() {
	G_PROFILE_ZONE_SCOPED_N("gMesh::getBoundingBox()");
	if (needsboundingboxrecalculation) {
		recalculateBoundingBox();
	}
	return boundingbox;
}

void gMesh::recalculateBoundingBox() {
	G_PROFILE_ZONE_SCOPED_N("gMesh::recalculateBoundingBox()");
	// Ensure the vertex list is not empty
	if (vertices.empty()) {
		// Handle empty vertices case appropriately
		boundingbox = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
		needsboundingboxrecalculation = false;
		return;
	}

	// Calculate the local bounding box
	glm::vec4 pos1 = localtransformationmatrix * glm::vec4(vertices[0].position, 1.0f);

	float minx = pos1.x, miny = pos1.y, minz = pos1.z;
	float maxx = pos1.x, maxy = pos1.y, maxz = pos1.z;

#if defined(__i386__) || defined(__x86_64__)
	__m128 minvals = _mm_set_ps(minz, miny, minx, 0);
	__m128 maxvals = _mm_set_ps(maxz, maxy, maxx, 0);

	for (size_t i = 1; i < vertices.size(); ++i) {
		glm::vec4 pos = localtransformationmatrix * glm::vec4(vertices[i].position, 1.0f);
		__m128 current = _mm_set_ps(pos.z, pos.y, pos.x, 0);

		minvals = _mm_min_ps(minvals, current);
		maxvals = _mm_max_ps(maxvals, current);
	}

	float minarray[4], maxarray[4];
	_mm_store_ps(minarray, minvals);
	_mm_store_ps(maxarray, maxvals);

	minx = minarray[1];
	miny = minarray[2];
	minz = minarray[3];
	maxx = maxarray[1];
	maxy = maxarray[2];
	maxz = maxarray[3];
#elif defined(__ARM_NEON)
	float32x4_t minvals = {minz, miny, minx, 0};
	float32x4_t maxvals = {maxz, maxy, maxx, 0};

	for (size_t i = 1; i < vertices.size(); ++i) {
		glm::vec4 pos = localtransformationmatrix * glm::vec4(vertices[i].position, 1.0f);
		float32x4_t current = {pos.z, pos.y, pos.x, 0};

		minvals = vminq_f32(minvals, current);
		maxvals = vmaxq_f32(maxvals, current);
	}
	float minarray[4], maxarray[4];
	vst1q_f32(minarray, minvals);
	vst1q_f32(maxarray, maxvals);

	minx = minarray[2];
	miny = minarray[1];
	minz = minarray[0];
	maxx = maxarray[2];
	maxy = maxarray[1];
	maxz = maxarray[0];
#else
	for (size_t i = 1; i < vertices.size(); ++i) {
		glm::vec4 pos = localtransformationmatrix * glm::vec4(vertices[i].position, 1.0f);

		minx = std::min(pos.x, minx);
		miny = std::min(pos.y, miny);
		minz = std::min(pos.z, minz);
		maxx = std::max(pos.x, maxx);
		maxy = std::max(pos.y, maxy);
		maxz = std::max(pos.z, maxz);
	}
#endif

	boundingbox = {minx, miny, minz, maxx, maxy, maxz};
	needsboundingboxrecalculation = false;
}

const gBoundingBox& gMesh::getInitialBoundingBox() const {
	return initialboundingbox;
}

bool gMesh::intersectsTriangles(gRay* ray) {
	float distance = distanceTriangles(ray);
	return distance > 0.0f && distance < ray->getLength();
}

float gMesh::distanceTriangles(gRay* ray) {
	glm::vec2 baryposition(0);
	float mindistance = std::numeric_limits<float>::max();
	float distance = 0.0f;
	for (int i = 0; i < indices.size(); i += 3) {
		//iterate through all faces of the mesh since each face has 3 vertices
		glm::vec3 a = vertices[indices[i]].position;
		glm::vec3 b = vertices[indices[i + 1]].position;
		glm::vec3 c = vertices[indices[i + 2]].position;
		if(glm::intersectRayTriangle(ray->getOrigin(), ray->getDirection(), a, b, c, baryposition, distance)) {
			if(distance > 0) {
				if(distance < mindistance) mindistance = distance;
			}
		}
	}
	return mindistance;
}
