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
	needsboundingboxrecalculation = false;
	this->vertices = std::make_shared<std::vector<gVertex>>();
	this->indices = std::make_shared<std::vector<gIndex>>();
}

gMesh::gMesh(std::shared_ptr<std::vector<gVertex>> vertices,
			std::shared_ptr<std::vector<gIndex>> indices,
			std::vector<gTexture*> textures) {
	vbo = std::make_unique<gVbo>();
	name = "";
	drawmode = DRAWMODE_TRIANGLES;
	isprojection2d = false;

    setVertices(vertices, indices);
    setTextures(textures);
}

gMesh::gMesh(const gMesh& other) {
	vbo = std::make_unique<gVbo>();
	name = other.name;
	drawmode = DRAWMODE_TRIANGLES;
	isprojection2d = false;
	setVertices(other.vertices, other.indices);
	setTextures(other.textures);
}

gMesh::~gMesh() {
}

void gMesh::clear() {
	vbo->clear();
}

void gMesh::setName(const std::string& name) {
	this->name = name;
}

const std::string& gMesh::getName() const {
	return name;
}


void gMesh::setVertices(const std::vector<gVertex>& vertices, const std::vector<gIndex>& indices) {
	this->setVertices(std::make_shared<std::vector<gVertex>>(vertices), std::make_shared<std::vector<gIndex>>(indices));
}

void gMesh::setVertices(const std::vector<gVertex>& vertices) {
	this->setVertices(std::make_shared<std::vector<gVertex>>(vertices));
}

void gMesh::setVertices(std::shared_ptr<std::vector<gVertex>> vertices, std::shared_ptr<std::vector<gIndex>> indices) {
	G_PROFILE_ZONE_SCOPED_N("gModel::setVertices()");
	bool resetinitialboundingbox = this->vertices->size() != vertices->size() || this->indices->size() != indices->size();
	this->vertices = vertices;
	this->indices = indices;
	vbo->setVertexData(vertices->data(), sizeof(gVertex), vertices->size());
	vbo->setIndexData(indices->data(), indices->size());
	if (resetinitialboundingbox) {
		recalculateBoundingBox();
		initialboundingbox = boundingbox;
	} else {
		needsboundingboxrecalculation = true;
	}
}

void gMesh::setVertices(std::shared_ptr<std::vector<gVertex>> vertices) {
	G_PROFILE_ZONE_SCOPED_N("gModel::setVertices()");
	bool resetinitialboundingbox = this->vertices->size() != vertices->size();
	this->vertices = vertices;
	vbo->setVertexData(vertices->data(), sizeof(gVertex), vertices->size());
	if (resetinitialboundingbox) {
		recalculateBoundingBox();
		initialboundingbox = boundingbox;
	} else {
		needsboundingboxrecalculation = true;
	}
}

std::vector<gVertex>& gMesh::getVertices() {
	return *vertices;
}

std::vector<gIndex>& gMesh::getIndices() {
	return *indices;
}

std::shared_ptr<std::vector<gVertex>> gMesh::getVerticesPtr() {
	return vertices;
}

std::shared_ptr<std::vector<gIndex>> gMesh::getIndicesPtr() {
	return indices;
}

void gMesh::setTextures(const std::vector<gTexture*>& textures) {
	texturenames.clear();
	texturecounters.clear();
	this->textures.clear();
    for(loopindex = 0; loopindex < textures.size(); loopindex++) {
    	setTexture(textures[loopindex]);
    }
}

void gMesh::setTextures(const std::unordered_map<gTexture::TextureType, gTexture*>& textures) {
//	this->textures = textures;
	texturenames.clear();
	texturecounters.clear();
	this->textures.clear();
	for (auto pair : textures) {
		setTexture(pair.second);
	}
}

void gMesh::setTexture(gTexture* texture) {
	bool hasold = textures.find(texture->getType()) != textures.end();
	switch (texture->getType()) {
	case gTexture::TEXTURETYPE_DIFFUSE:
		material.setDiffuseMap(texture);
		break;
	case gTexture::TEXTURETYPE_SPECULAR:
		material.setSpecularMap(texture);
		break;
	case gTexture::TEXTURETYPE_NORMAL:
		material.setNormalMap(texture);
		break;
	case gTexture::TEXTURETYPE_HEIGHT:
		material.setHeightMap(texture);
		break;
	case gTexture::TEXTURETYPE_PBR_ALBEDO:
		material.setAlbedoMap(texture);
		break;
	case gTexture::TEXTURETYPE_PBR_ROUGHNESS:
		material.setRoughnessMap(texture);
		break;
	case gTexture::TEXTURETYPE_PBR_METALNESS:
		material.setMetalnessMap(texture);
		break;
	case gTexture::TEXTURETYPE_PBR_NORMAL:
		material.setPbrNormalMap(texture);
		break;
	case gTexture::TEXTURETYPE_PBR_AO:
		material.setAOMap(texture);
		break;
	}
	if (!hasold) {
		int count = ++texturecounters[texture->getType()];
		texturenames[texture->getType()] = texture->getTypeName() + gToStr(count);
	}
	textures[texture->getType()] = texture;
}

gTexture* gMesh::getTexture(gTexture::TextureType textureType) {
	return textures[textureType];
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

	bool positionchanged = position != prevposition;
	bool orientationchanged = orientation != prevorientation;
	bool scalechanged = scalevec != prevscalevec;
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

    if (!material.isPBR()) {
    	gShader& colorshader = *renderer->getColorShader();
		colorshader.use();

	    // Set material colors
	    colorshader.setVec4("material.ambient", material.getAmbientColor()->r, material.getAmbientColor()->g, material.getAmbientColor()->b, material.getAmbientColor()->a);
	    colorshader.setVec4("material.diffuse", material.getDiffuseColor()->r, material.getDiffuseColor()->g, material.getDiffuseColor()->b, material.getDiffuseColor()->a);
	    colorshader.setVec4("material.specular", material.getSpecularColor()->r, material.getSpecularColor()->g, material.getSpecularColor()->b, material.getSpecularColor()->a);
	    colorshader.setFloat("material.shininess", material.getShininess());

	    // Bind diffuse textures
	    colorshader.setInt("material.useDiffuseMap", material.isDiffuseMapEnabled());
//		if(material.isDiffuseMapEnabled()) gLogi("gModel") << "mesh name:" << name;
//		if(material.isDiffuseMapEnabled()) gLogi("gModel") << "diffuse texture name:" << material.getDiffuseMap()->getFilename();
	    if (material.isDiffuseMapEnabled()) {
			colorshader.setInt("material.diffusemap", 0); // Diffuse texture unit
	    	renderer->activateTexture(0);
		    material.bindDiffuseMap();
	    }

	    // Bind specular textures
	    colorshader.setInt("material.useSpecularMap", material.isDiffuseMapEnabled() && material.isSpecularMapEnabled());
	    if (material.isDiffuseMapEnabled() && material.isSpecularMapEnabled()) {
			colorshader.setInt("material.specularmap", 1); // Specular texture unit
	    	renderer->activateTexture(1);
		    material.bindSpecularMap();
	    }

	    // Bind normal textures
	    colorshader.setInt("aUseNormalMap", material.isDiffuseMapEnabled() && material.isNormalMapEnabled());
	    if (material.isDiffuseMapEnabled() && material.isNormalMapEnabled()) {
			colorshader.setInt("material.normalmap", 2); // Normal texture unit
	    	renderer->activateTexture(2);
		    material.bindNormalMap();
	    }

	    // Set matrices
	    if(isprojection2d) {
		    colorshader.setMat4("projection", renderer->getProjectionMatrix2d());
	    } else {
		    colorshader.setMat4("projection", renderer->getProjectionMatrix());
	    }
		colorshader.setMat4("model", localtransformationmatrix);
    } else { // isPBR
    	gShader& pbrshader = *renderer->getPbrShader();
    	pbrshader.use();
    	pbrshader.setMat4("projection", renderer->getProjectionMatrix());
	    pbrshader.setMat4("view", renderer->getViewMatrix());
    	pbrshader.setMat4("model", localtransformationmatrix);
    	pbrshader.setInt("albedoMap", 3);
    	pbrshader.setInt("normalMap", 4);
    	pbrshader.setInt("metallicMap", 5);
    	pbrshader.setInt("roughnessMap", 6);
    	pbrshader.setInt("aoMap", 7);
    	material.bindAlbedoMap();
    	material.bindPbrNormalMap();
    	material.bindMetalnessMap();
    	material.bindRoughnessMap();
    	material.bindAOMap();
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
}

int gMesh::getVerticesNum() const {
	return vertices->size();
}

int gMesh::getIndicesNum() const {
	return indices->size();
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
	const std::vector<gVertex>& verts = *vertices;
	const std::vector<gIndex>& inds = *indices;
	if (verts.empty()) {
		// Handle empty vertices case appropriately
		boundingbox = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
		needsboundingboxrecalculation = false;
		return;
	}

	// Calculate the local bounding box
	glm::vec4 pos1 = localtransformationmatrix * glm::vec4(verts[0].position, 1.0f);

	float minx = pos1.x, miny = pos1.y, minz = pos1.z;
	float maxx = pos1.x, maxy = pos1.y, maxz = pos1.z;

#if defined(__i386__) || defined(__x86_64__)
	__m128 minvals = _mm_set_ps(minz, miny, minx, 0);
	__m128 maxvals = _mm_set_ps(maxz, maxy, maxx, 0);

	for (size_t i = 1; i < vertices->size(); ++i) {
		glm::vec4 pos = localtransformationmatrix * glm::vec4(verts[i].position, 1.0f);
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

	for (size_t i = 1; i < verts.size(); ++i) {
		glm::vec4 pos = localtransformationmatrix * glm::vec4(verts[i].position, 1.0f);
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
	for (size_t i = 1; i < verts.size(); ++i) {
		glm::vec4 pos = localtransformationmatrix * glm::vec4(verts[i].position, 1.0f);

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
	const std::vector<gVertex>& verts = *vertices;
	const std::vector<gIndex>& inds = *indices;
	for (loopindex = 0; loopindex < inds.size(); loopindex += 3) {
		//iterate through all faces of the mesh since each face has 3 vertices
		const glm::vec3& a = verts[inds[loopindex]].position;
		const glm::vec3& b = verts[inds[loopindex + 1]].position;
		const glm::vec3& c = verts[inds[loopindex + 2]].position;
		if(glm::intersectRayTriangle(ray->getOrigin(), ray->getDirection(), a, b, c, baryposition, distance)) {
			if(distance > 0 && distance < mindistance) {
				mindistance = distance;
			}
		}
	}
	return mindistance;
}
