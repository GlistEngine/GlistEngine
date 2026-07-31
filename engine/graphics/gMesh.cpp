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
    texturetiling = other.texturetiling;
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
	bool resetinitialboundingbox = (!this->vertices || this->vertices->size() != vertices->size()) || (!this->indices || this->indices->size() != indices->size());
	this->vertices = vertices;
	fillMissingVertexColors(glm::vec3(1.0f, 1.0f, 1.0f));
	vbo->setVertexData(vertices->data(), sizeof(gVertex), vertices->size());
	this->indices = indices;
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
	bool resetinitialboundingbox = !this->vertices || this->vertices->size() != vertices->size();
	this->vertices = vertices;
	fillMissingVertexColors(glm::vec3(1.0f, 1.0f, 1.0f));
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
    for(size_t i = 0; i < textures.size(); i++) {
    	setTexture(textures[i]);
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
	material.setMap(texture->getType(), texture);
	if (!hasold) {
		int count = ++texturecounters[texture->getType()];
		texturenames[texture->getType()] = texture->getTypeName() + gToStr(count);
	}
	textures[texture->getType()] = texture;
}

gTexture* gMesh::getTexture(gTexture::TextureType textureType) {
	return textures[textureType];
}

void gMesh::setTextureTiling(float tilingX, float tilingY) {
    texturetiling = glm::vec2(tilingX, tilingY);
}

void gMesh::setTextureTiling(float tiling) {
    setTextureTiling(tiling, tiling);
}

const glm::vec2& gMesh::getTextureTiling() const {
    return texturetiling;
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

	if (renderer->isVulkan()) {
		drawVulkan2D();
		return;
	}

	drawStart();
	drawVbo();
	drawExtraShaders();
	drawEnd();
}

void gMesh::drawInstanced(const std::vector<glm::mat4>& instanceTransformations) {
    G_PROFILE_ZONE_SCOPED_N("gMesh::drawInstanced()");

    if (!isenabled || instanceTransformations.empty()) {
        return;
    }

    // Instancing is an OpenGL-only path for now: it needs the colour shader's
    // useInstancing branch and a per-instance vertex buffer, neither of which the
    // Vulkan 2D pipelines have. Without this the call would reach a null shader.
    if (renderer->isVulkan()) return;

    drawStart(true);
    drawVboInstanced(instanceTransformations);
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

void gMesh::bindMaterialUniforms(gShader& shader) {
	if(!material.isPBR()) {
		shader.setVec4("material.ambient", material.getAmbientColor()->r, material.getAmbientColor()->g, material.getAmbientColor()->b, material.getAmbientColor()->a);
		shader.setVec4("material.diffuse", material.getDiffuseColor()->r, material.getDiffuseColor()->g, material.getDiffuseColor()->b, material.getDiffuseColor()->a);
		shader.setVec4("material.specular", material.getSpecularColor()->r, material.getSpecularColor()->g, material.getSpecularColor()->b, material.getSpecularColor()->a);
		shader.setFloat("material.shininess", material.getShininess());

		bool hasDiffuse = material.isMapEnabled(gTexture::TEXTURETYPE_DIFFUSE);
		bool hasSpecular = hasDiffuse && material.isMapEnabled(gTexture::TEXTURETYPE_SPECULAR);
		bool hasNormal = hasDiffuse && material.isMapEnabled(gTexture::TEXTURETYPE_NORMAL);

		shader.setInt("material.useDiffuseMap", hasDiffuse);
		shader.setInt("material.useSpecularMap", hasSpecular);
		shader.setInt("material.useNormalMap", hasNormal);
	} else { // isPBR
		bool hasAlbedo = material.isMapEnabled(gTexture::TEXTURETYPE_PBR_ALBEDO);
		bool hasDiffuseFallback = !hasAlbedo && material.isMapEnabled(gTexture::TEXTURETYPE_DIFFUSE);

		shader.setInt("hasAlbedoMap", (hasAlbedo || hasDiffuseFallback) ? 1 : 0);
		shader.setInt("hasNormalMap", material.isMapEnabled(gTexture::TEXTURETYPE_PBR_NORMAL) ? 1 : 0);
		shader.setInt("hasMetallicMap", material.isMapEnabled(gTexture::TEXTURETYPE_PBR_METALNESS) ? 1 : 0);
		shader.setInt("hasRoughnessMap", material.isMapEnabled(gTexture::TEXTURETYPE_PBR_ROUGHNESS) ? 1 : 0);
		shader.setInt("hasAOMap", material.isMapEnabled(gTexture::TEXTURETYPE_PBR_AO) ? 1 : 0);
	}
}

void gMesh::bindMaterialTextures(gShader& shader) {
	if(!material.isPBR()) {
		bool hasDiffuse = material.isMapEnabled(gTexture::TEXTURETYPE_DIFFUSE);
		if(hasDiffuse) {
			shader.setInt("material.diffusemap", 0);
			renderer->activateTexture(0);
			material.bindMap(gTexture::TEXTURETYPE_DIFFUSE);
		}
		bool hasSpecular = hasDiffuse && material.isMapEnabled(gTexture::TEXTURETYPE_SPECULAR);
		if(hasSpecular) {
			shader.setInt("material.specularmap", 1);
			renderer->activateTexture(1);
			material.bindMap(gTexture::TEXTURETYPE_SPECULAR);
		}
		bool hasNormal = hasDiffuse && material.isMapEnabled(gTexture::TEXTURETYPE_NORMAL);
		if(hasNormal) {
			shader.setInt("material.normalMap", 2);
			renderer->activateTexture(2);
			material.bindMap(gTexture::TEXTURETYPE_NORMAL);
		}
	} else { // isPBR
		shader.setInt("albedoMap", 3);
		shader.setInt("normalMap", 4);
		shader.setInt("metallicMap", 5);
		shader.setInt("roughnessMap", 6);
		shader.setInt("aoMap", 7);

		bool hasAlbedo = material.isMapEnabled(gTexture::TEXTURETYPE_PBR_ALBEDO);
		bool hasDiffuseFallback = !hasAlbedo && material.isMapEnabled(gTexture::TEXTURETYPE_DIFFUSE);

		if(hasAlbedo) {
			material.bindMap(gTexture::TEXTURETYPE_PBR_ALBEDO, 3);
		} else if(hasDiffuseFallback) {
			material.getMap(gTexture::TEXTURETYPE_DIFFUSE)->bind(3);
		}
		material.bindMap(gTexture::TEXTURETYPE_PBR_NORMAL, 4);
		material.bindMap(gTexture::TEXTURETYPE_PBR_METALNESS, 5);
		material.bindMap(gTexture::TEXTURETYPE_PBR_ROUGHNESS, 6);
		material.bindMap(gTexture::TEXTURETYPE_PBR_AO, 7);
	}
}

void gMesh::drawStart(bool isInstanced) {
	G_PROFILE_ZONE_SCOPED_N("gMesh::drawStart()");
		if(isshadowmappingenabled && renderpassno == 0) {
			renderer->getShadowmapShader()->use();
			renderer->getShadowmapShader()->setMat4("model", localtransformationmatrix.back());
			renderer->getShadowmapShader()->setInt("useInstancing", isInstanced ? 1 : 0);
			return;
		}

		if(!material.isPBR()) {
			gShader& colorshader = *renderer->getColorShader();
			colorshader.use();
			colorshader.setVec2("textureTiling", texturetiling);
			bindMaterialUniforms(colorshader);
			bindMaterialTextures(colorshader);

			if(isprojection2d) {
				colorshader.setMat4("projection", renderer->getProjectionMatrix2d());
			} else {
				colorshader.setMat4("projection", renderer->getProjectionMatrix());
			}
			colorshader.setMat4("model", localtransformationmatrix.back());
			colorshader.setInt("useInstancing", isInstanced ? 1 : 0);
		} else { // isPBR
			gShader& pbrshader = *renderer->getPbrShader();
			pbrshader.use();
			pbrshader.setVec2("textureTiling", texturetiling);
			pbrshader.setMat4("projection", renderer->getProjectionMatrix());
			pbrshader.setMat4("view", renderer->getViewMatrix());
			pbrshader.setMat4("model", localtransformationmatrix.back());
			pbrshader.setInt("useInstancing", isInstanced ? 1 : 0);
			pbrshader.setVec3("camPos", renderer->getCameraPosition());
			bindMaterialUniforms(pbrshader);
			bindMaterialTextures(pbrshader);
		}
}

void gMesh::drawVbo() {
	G_PROFILE_ZONE_SCOPED_N("gMesh::drawVbo()");
    // draw mesh
    vbo->bind();
    if(vbo->isIndexDataAllocated()) {
    	renderer->drawElements(drawmode, vbo->getIndicesNum());
    } else {
    	renderer->drawArrays(drawmode, vbo->getVerticesNum());
    }
    vbo->unbind();
//    vbo.clear();
}

void gMesh::drawVulkan2D() {
	G_PROFILE_ZONE_SCOPED_N("gMesh::drawVulkan2D()");
	// Only 2D meshes are recorded so far. A 3D mesh needs lighting, materials and a
	// view matrix that the backend's two colour pipelines do not carry, so drawing
	// it here would be wrong rather than merely unfinished - it is skipped instead.
	if (!isprojection2d) return;

	const std::vector<gVertex>& verts = *vertices;
	if (verts.empty()) return;
	const std::vector<gIndex>& inds = *indices;

	// The colour pipeline takes plain 2D positions, so the mesh is flattened here:
	// indices are resolved and z is dropped (a 2D mesh keeps it at 0). The draw mode
	// travels with the points, because a fan, a strip and a line loop each need a
	// different expansion on the Vulkan side. The scratch buffer is reused between
	// draws so a per-frame primitive allocates nothing.
	static thread_local std::vector<glm::vec2> points;
	points.clear();
	if (inds.empty()) {
		points.reserve(verts.size());
		for (const gVertex& vertex : verts) {
			points.emplace_back(vertex.position.x, vertex.position.y);
		}
	} else {
		points.reserve(inds.size());
		for (gIndex index : inds) {
			if (index >= verts.size()) return;
			points.emplace_back(verts[index].position.x, verts[index].position.y);
		}
	}

	// The model matrix rides in the mvp exactly like the OpenGL colour shader gets
	// it, and the colour is the renderer's current one - the 2D primitives are drawn
	// flat, without the material the 3D path applies.
	glm::mat4 mvp = renderer->getProjectionMatrix2d() * localtransformationmatrix.back();
	gColor* color = renderer->getColor();
	renderer->drawColored2D(points.data(), static_cast<int>(points.size()),
			glm::vec4(color->r, color->g, color->b, color->a), mvp, drawmode);
}

void gMesh::drawVboInstanced(const std::vector<glm::mat4>& instanceTransformations) {
    G_PROFILE_ZONE_SCOPED_N("gMesh::drawVboInstanced()");

    vbo->setInstanceData(instanceTransformations.data(), static_cast<int>(instanceTransformations.size()));
    vbo->bind();
    if (vbo->isIndexDataAllocated()) {
        renderer->drawElementsInstanced(drawmode, vbo->getIndicesNum(), static_cast<int>(instanceTransformations.size()));
    } else {
        renderer->drawArraysInstanced(drawmode,vbo->getVerticesNum(), static_cast<int>(instanceTransformations.size()));
    }
    vbo->unbind();
}

void gMesh::drawExtraShaders() {
	G_PROFILE_ZONE_SCOPED_N("gMesh::drawExtraShaders()");
	if(isshadowmappingenabled && renderpassno == 0) return;
	const std::vector<gShader*>& extraShaders = material.getShaders();
	if(extraShaders.empty()) return;
	for(gShader* shader : extraShaders) {
		if(!shader) continue;
		shader->use();
		shader->setMat4("model", localtransformationmatrix.back());
		if(isprojection2d) {
			shader->setMat4("projection", renderer->getProjectionMatrix2d());
		} else {
			shader->setMat4("projection", renderer->getProjectionMatrix());
			shader->setMat4("view", renderer->getViewMatrix());
			shader->setVec3("camPos", renderer->getCameraPosition());
		}
		bindMaterialUniforms(*shader);
		bindMaterialTextures(*shader);
		drawVbo();
	}
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
	glm::vec4 pos1 = localtransformationmatrix.back() * glm::vec4(verts[0].position, 1.0f);

	float minx = pos1.x, miny = pos1.y, minz = pos1.z;
	float maxx = pos1.x, maxy = pos1.y, maxz = pos1.z;

#if defined(__i386__) || defined(__x86_64__)
	__m128 minvals = _mm_set_ps(minz, miny, minx, 0);
	__m128 maxvals = _mm_set_ps(maxz, maxy, maxx, 0);

	for (size_t i = 1; i < vertices->size(); ++i) {
		glm::vec4 pos = localtransformationmatrix.back() * glm::vec4(verts[i].position, 1.0f);
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
		glm::vec4 pos = localtransformationmatrix.back() * glm::vec4(verts[i].position, 1.0f);
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
		glm::vec4 pos = localtransformationmatrix.back() * glm::vec4(verts[i].position, 1.0f);

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
	for (size_t i = 0; i < inds.size(); i += 3) {
		//iterate through all faces of the mesh since each face has 3 vertices
		const glm::vec3& a = verts[inds[i]].position;
		const glm::vec3& b = verts[inds[i + 1]].position;
		const glm::vec3& c = verts[inds[i + 2]].position;
		if(glm::intersectRayTriangle(ray->getOrigin(), ray->getDirection(), a, b, c, baryposition, distance)) {
			if(distance > 0 && distance < mindistance) {
				mindistance = distance;
			}
		}
	}
	return mindistance;
}

/*
 * Vertex color feature implemented by: Engin Kutlu
 * */
void gMesh::setAllVertexColor(const glm::vec3& color) {
    if (!vertices) return;
    auto& verts = *vertices;
    for (size_t i = 0; i < verts.size(); ++i) {
        verts[i].color = color;
    }
    vbo->setVertexData(verts.data(), sizeof(gVertex), static_cast<int>(verts.size()));
}

void gMesh::fillMissingVertexColors(const glm::vec3& defColor) {
    if (!vertices) return;
    auto& verts = *vertices;
    bool changed = false;
    for (size_t i = 0; i < verts.size(); ++i) {
        if (verts[i].color == glm::vec3(0.0f)) {
            verts[i].color = defColor;
            changed = true;
        }
    }
    if (changed) {
        vbo->setVertexData(verts.data(), sizeof(gVertex), static_cast<int>(verts.size()));
    }
}

void gMesh::applyVertexGradient() {
    if (!vertices || vertices->empty()) return;

    auto& verts = *vertices;

    for (size_t i = 0; i < verts.size(); ++i) {
        float r = ((i * 97) % 255) / 255.0f;
        float g = ((i * 57) % 255) / 255.0f;
        float b = ((i * 23) % 255) / 255.0f;
        verts[i].color = glm::vec3(r, g, b);
    }

    vbo->setVertexData(
        verts.data(),
        sizeof(gVertex),
        static_cast<int>(verts.size())
    );
}
