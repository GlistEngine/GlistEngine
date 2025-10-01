/*
 * gSkinnedMesh.cpp
 *
 *  Created on: 27 Kas 2020
 *      Author: Acer
 */

#include "gSkinnedMesh.h"

#include "gTracy.h"

gSkinnedMesh::gSkinnedMesh() {
	isvertexanimated = false;
	isvertexanimationstoredonvram = false;
	frameno = 0;
	framenoold = 0;
	this->setBaseMesh(this);
}

gSkinnedMesh::~gSkinnedMesh() {
}

void gSkinnedMesh::draw() {
	G_PROFILE_ZONE_SCOPED_N("gSkinnedMesh::draw()");
	std::vector<gVertex>& verts = *vertices;
	if (getTargetMeshCount() > 0) {
		if (frameno != framenoold) {
			for(int i = 0; i < vbo->getVerticesNum(); i++) {
				verts[i].position = animatedPosData[0][frameno][i];
				verts[i].normal = animatedNormData[0][frameno][i];
			}
			setBaseMesh(this);
			interpolate(false);
		} else {
			interpolate();
		}
		gMesh::draw();
	}
	else if (!isvertexanimationstoredonvram) {
		if (isvertexanimated && frameno != framenoold) {
			// TODO Below lines of vertex animation stored on CPU needs to be optimized
			for(int i = 0; i < vbo->getVerticesNum(); i++) {
				verts[i].position = animatedPosData[0][frameno][i];
				verts[i].normal = animatedNormData[0][frameno][i];
			}
			vbo->setVertexData(&verts[0], sizeof(gVertex), vbo->getVerticesNum());
//			vbo.setVertexData(&animatedPosData[0][frameno][0].x, 3, vbo.getVerticesNum(), GL_STREAM_DRAW);
//			vbo.setNormalData(&animatedNormData[0][frameno][0].x, 3,  vbo.getVerticesNum(), GL_STREAM_DRAW);
			framenoold = frameno;
		}
		gMesh::draw();
	} else {
		if (!isenabled) return;

		drawStart();
		drawVboFrame();
		drawEnd();
	}
}

void gSkinnedMesh::drawVboFrame() {
	// draw mesh
	gVbo& vbo = *vboframe[0][frameno];
	vbo.bind();
	if (vbo.isIndexDataAllocated()) {
		renderer->drawElements(GL_TRIANGLES, vbo.getIndicesNum());
	} else {
		renderer->drawArrays(GL_TRIANGLES, vbo.getVerticesNum());
	}
	vbo.unbind();
	framenoold = frameno;
}

void gSkinnedMesh::resizeAnimation(int verticesNum) {
	G_PROFILE_ZONE_SCOPED_N("gModel::resizeAnimation()");
	G_PROFILE_ZONE_VALUE(verticesNum);
	animatedPos.resize(verticesNum);
	animatedNorm.resize(verticesNum);
}

void gSkinnedMesh::resetAnimation() {
	G_PROFILE_ZONE_SCOPED_N("gModel::resetAnimation()");
	animatedPos.assign(animatedPos.size(), glm::vec3(0.0f));
	animatedNorm.assign(animatedNorm.size(), glm::vec3(0.0f));
}

void gSkinnedMesh::setVertexPos(int vertexNo, const glm::vec3& newWeight) {
	animatedPos[vertexNo] = newWeight;
}

void gSkinnedMesh::setVertexNorm(int vertexNo, const glm::vec3& newWeight) {
	animatedNorm[vertexNo] = newWeight;
}

const glm::vec3& gSkinnedMesh::getVertexPos(int vertexNo) const {
	return animatedPos[vertexNo];
}

const glm::vec3& gSkinnedMesh::getVertexNorm(int vertexNo) const {
	return animatedNorm[vertexNo];
}

void gSkinnedMesh::clearAnimation() {
	animatedPos.clear();
	animatedNorm.clear();
}

void gSkinnedMesh::resizeVertexAnimationData(int animationNum, int frameNum, int verticesNum, bool isOnVram) {
	animatedPosData.resize(animationNum);
	animatedNormData.resize(animationNum);

	for (int i = 0; i < animationNum; ++i) {
		animatedPosData[i].resize(frameNum);
		animatedNormData[i].resize(frameNum);

		for (int j = 0; j < frameNum; ++j) {
			animatedPosData[i][j].resize(verticesNum);
			animatedNormData[i][j].resize(verticesNum);
		}
	}

	if (isOnVram) {
		vboframe.resize(animationNum);
		for (auto& animVbos : vboframe) {
			animVbos.resize(frameNum);
			for (size_t i = 0; i < animVbos.size(); i++) {
				animVbos[i] = std::make_unique<gVbo>();
			}
		}
	}
}

void gSkinnedMesh::resetVertexAnimationData(int animationNo, int frameNo) {
	animatedPosData[animationNo][frameNo].assign(animatedPosData[animationNo][frameNo].size(), glm::vec3(0.0f));
	animatedNormData[animationNo][frameNo].assign(animatedNormData[animationNo][frameNo].size(), glm::vec3(0.0f));
}

void gSkinnedMesh::setVertexPosData(int animationNo, int frameNo, int vertexNo, const glm::vec3& newWeight) {
	animatedPosData[animationNo][frameNo][vertexNo] = newWeight;
}

void gSkinnedMesh::setVertexNormData(int animationNo, int frameNo, int vertexNo, const glm::vec3& newWeight) {
	animatedNormData[animationNo][frameNo][vertexNo] = newWeight;
}

const glm::vec3& gSkinnedMesh::getVertexPosData(int animationNo, int frameNo, int vertexNo) const {
	return animatedPosData[animationNo][frameNo][vertexNo];
}

const glm::vec3& gSkinnedMesh::getVertexNormData(int animationNo, int frameNo, int vertexNo) const {
	return animatedNormData[animationNo][frameNo][vertexNo];
}

void gSkinnedMesh::setVerticesData(int animationNo, int frameNo, const std::vector<gVertex>& vertices, const std::vector<gIndex>& indices) {
//	this->vertices = vertices;
//	this->indices = indices;
	gVbo& vbo = *vboframe[animationNo][frameNo];
	vbo.setVertexData(&vertices[0], sizeof(gVertex), vertices.size());
	if (indices.size() != 0) {
		vbo.setIndexData(&indices[0], indices.size());
	}
}

void gSkinnedMesh::setVertexAnimated(bool isVertexAnimated) {
	isvertexanimated = isVertexAnimated;
}

void gSkinnedMesh::setVertexAnimationStoredOnVram(bool isOnVram) {
	isvertexanimationstoredonvram = isOnVram;
}

void gSkinnedMesh::setFrameNo(int frameNo) {
	framenoold = frameno;
	frameno = frameNo;
}

int gSkinnedMesh::getFrameNo() const {
	return frameno;
}


const std::vector<glm::vec3>& gSkinnedMesh::getAnimatedPos() const {
	return animatedPos;
}

const std::vector<glm::vec3>& gSkinnedMesh::getAnimatedNorm() const {
	return animatedNorm;
}