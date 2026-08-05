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
		drawVulkanMesh();
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

    if (renderer->isVulkan()) {
        for(const glm::mat4& transformation : instanceTransformations)
            drawVulkanMesh(&transformation);
        return;
    }

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

void gMesh::drawVulkanMesh(const glm::mat4* instanceTransformation) {
	G_PROFILE_ZONE_SCOPED_N("gMesh::drawVulkanMesh()");
	const std::vector<gVertex>& verts = *vertices;
	if (verts.empty()) return;
	const std::vector<gIndex>& inds = *indices;

	// The colour pipeline takes plain 2D positions, so the mesh is flattened here:
	// indices are resolved and z is dropped (a 2D mesh keeps it at 0). The draw mode
	// travels with the points, because a fan, a strip and a line loop each need a
	// different expansion on the Vulkan side. The scratch buffer is reused between
	// draws so a per-frame primitive allocates nothing.
	static thread_local std::vector<glm::vec2> points2d;
	static thread_local std::vector<gRenderer::MeshVertex3D> points3d;
	points2d.clear();
	points3d.clear();
	if(!isprojection2d && drawmode != DRAWMODE_TRIANGLES &&
			drawmode != DRAWMODE_TRIANGLESTRIP && drawmode != DRAWMODE_TRIANGLEFAN) return;
	const glm::mat4 drawmodel = instanceTransformation != nullptr
			? localtransformationmatrix.back() * *instanceTransformation
			: localtransformationmatrix.back();
	const glm::mat3 modelmatrix(drawmodel);
	const float modeldeterminant = glm::determinant(modelmatrix);
	const glm::mat3 normalmatrix = std::abs(modeldeterminant) > 0.000001f
			? glm::transpose(glm::inverse(modelmatrix)) : glm::mat3(1.0f);
	const auto makeVertex3D = [&](const gVertex& vertex) {
		return gRenderer::MeshVertex3D{glm::vec3(drawmodel * glm::vec4(vertex.position, 1.0f)),
				glm::normalize(normalmatrix * vertex.normal), vertex.texcoords * texturetiling, vertex.color};
	};
	if (inds.empty()) {
		points2d.reserve(verts.size());
		points3d.reserve(verts.size());
		for (const gVertex& vertex : verts) {
			if(isprojection2d) points2d.emplace_back(vertex.position.x, vertex.position.y);
			else points3d.push_back(makeVertex3D(vertex));
		}
	} else {
		points2d.reserve(inds.size());
		points3d.reserve(inds.size());
		for (gIndex index : inds) {
			if (index >= verts.size()) return;
			if(isprojection2d) points2d.emplace_back(verts[index].position.x, verts[index].position.y);
			else {
				const gVertex& vertex = verts[index];
				points3d.push_back(makeVertex3D(vertex));
			}
		}
	}

	// The model matrix rides in the mvp exactly like the OpenGL colour shader gets
	// it, and the colour is the renderer's current one - the 2D primitives are drawn
	// flat, without the material the 3D path applies.
	gColor* color = renderer->getColor();
	const glm::vec4 rgba(color->r, color->g, color->b, color->a);
	if(isprojection2d) {
		glm::mat4 mvp = renderer->getProjectionMatrix2d() * localtransformationmatrix.back();
		renderer->drawColored2D(points2d.data(), static_cast<int>(points2d.size()), rgba, mvp, drawmode);
	} else {
		glm::mat4 mvp = renderer->getProjectionMatrix() * renderer->getViewMatrix();
		// Vulkan's material pipeline is a triangle list. Expand the strip/fan modes
		// used by primitives such as gSphere while preserving winding. This mirrors
		// the existing 2D expansion and also removes the degenerate connector
		// triangles sphere rows use.
		static thread_local std::vector<gRenderer::MeshVertex3D> triangles;
		const std::vector<gRenderer::MeshVertex3D>* drawvertices = &points3d;
		if(drawmode == DRAWMODE_TRIANGLESTRIP || drawmode == DRAWMODE_TRIANGLEFAN) {
			triangles.clear();
			triangles.reserve(points3d.size() > 2 ? (points3d.size() - 2) * 3 : 0);
			for(size_t i = 2; i < points3d.size(); i++) {
				const size_t a = drawmode == DRAWMODE_TRIANGLEFAN ? 0 : i - 2;
				const size_t b = i - 1;
				if(glm::all(glm::equal(points3d[a].position, points3d[b].position)) ||
						glm::all(glm::equal(points3d[b].position, points3d[i].position)) ||
						glm::all(glm::equal(points3d[a].position, points3d[i].position))) continue;
				if(drawmode == DRAWMODE_TRIANGLESTRIP && (i & 1u)) {
					triangles.push_back(points3d[b]);
					triangles.push_back(points3d[a]);
				} else {
					triangles.push_back(points3d[a]);
					triangles.push_back(points3d[b]);
				}
				triangles.push_back(points3d[i]);
			}
			drawvertices = &triangles;
		}
		if(renderer->isShadowPassActive()) {
			renderer->drawShadowMesh3D(drawvertices->data(), static_cast<int>(drawvertices->size()));
			return;
		}
		gColor* ambient = material.getAmbientColor();
		gColor* diffuse = material.getDiffuseColor();
		gColor* specular = material.getSpecularColor();
		gRenderer::MaterialTextures3D materialtextures;
		gTexture* diffusemap = material.isMapEnabled(gTexture::TEXTURETYPE_DIFFUSE)
				? material.getMap(gTexture::TEXTURETYPE_DIFFUSE) : nullptr;
		if(diffusemap == nullptr && material.isMapEnabled(gTexture::TEXTURETYPE_PBR_ALBEDO)) {
			diffusemap = material.getMap(gTexture::TEXTURETYPE_PBR_ALBEDO);
		}
		if(diffusemap != nullptr) materialtextures.albedo = diffusemap->getId();
		const auto mapid = [this](gTexture::TextureType type) -> GLuint {
			gTexture* map = material.isMapEnabled(type) ? material.getMap(type) : nullptr;
			return map != nullptr ? map->getId() : 0;
		};
		materialtextures.normal = mapid(material.isPBR() ? gTexture::TEXTURETYPE_PBR_NORMAL : gTexture::TEXTURETYPE_NORMAL);
		materialtextures.metallic = mapid(gTexture::TEXTURETYPE_PBR_METALNESS);
		materialtextures.roughness = mapid(gTexture::TEXTURETYPE_PBR_ROUGHNESS);
		materialtextures.ao = mapid(gTexture::TEXTURETYPE_PBR_AO);
		gRenderer::MaterialLighting3D lighting;
		lighting.globalAmbient = renderer->getGlobalAmbientColor()->asVec4();
		lighting.lightCount = std::min(renderer->getSceneLightNum(), GLIST_MAX_LIGHTS);
		for(int i = 0; i < lighting.lightCount; i++) {
			gLight* light = renderer->getSceneLight(i);
			if(light == nullptr) continue;
			auto& dst = lighting.lights[i];
			dst.type = light->getType();
			dst.position = light->getPosition();
			dst.direction = light->getDirection();
			dst.ambient = light->getAmbientColor()->asVec4();
			dst.diffuse = light->getDiffuseColor()->asVec4();
			dst.specular = light->getSpecularColor()->asVec4();
			dst.attenuation = light->getAttenuation();
			dst.spotCutoff = glm::vec2(light->getSpotCutOffAngle(), light->getSpotOuterCutOffAngle());
			if(renderer->isLightingEnabled() && light->isEnabled()) lighting.enabledMask |= 1u << i;
		}
		renderer->drawMaterialMesh3D(drawvertices->data(), static_cast<int>(drawvertices->size()),
				glm::vec4(ambient->r, ambient->g, ambient->b, ambient->a),
				glm::vec4(diffuse->r, diffuse->g, diffuse->b, diffuse->a),
				glm::vec4(specular->r, specular->g, specular->b, specular->a), material.getShininess(),
				material.isPBR(),
				materialtextures, lighting, mvp, DRAWMODE_TRIANGLES);
	}
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
