/*
 * gSkinnedMesh.cpp
 *
 *  Created on: 27 Kas 2020
 *      Author: Acer
 */

#include "gSkinnedMesh.h"

gSkinnedMesh::gSkinnedMesh() {
	isvertexanimated = false;
	isvertexanimationstoredonvram = false;
	frameno = 0;
	framenoold = 0;
	this->setBaseMesh(static_cast<gMesh*>(this));
}

gSkinnedMesh::~gSkinnedMesh() {
}

void gSkinnedMesh::draw() {
	if (this->getTargetMeshCount() > 0) {
		if (frameno != framenoold) {
			for(int i = 0; i < vbo.getVerticesNum(); i++) {
				vertices[i].position = animatedPosData[0][frameno][i];
				vertices[i].normal = animatedNormData[0][frameno][i];
			}
			setBaseMesh(static_cast<gMesh*>(this));
			interpolate(false);
		}
		else interpolate();
		gMesh::draw();
	}
	else if (!isvertexanimationstoredonvram) {
		if (isvertexanimated && frameno != framenoold) {
			// TODO Below lines of vertex animation stored on CPU needs to be optimized
			for(int i = 0; i < vbo.getVerticesNum(); i++) {
				vertices[i].position = animatedPosData[0][frameno][i];
				vertices[i].normal = animatedNormData[0][frameno][i];
			}
			vbo.setVertexData(&vertices[0], sizeof(gVertex), vbo.getVerticesNum());
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
    vboframe[0][frameno].bind();
    if (vboframe[0][frameno].isIndexDataAllocated()) {
        glDrawElements(GL_TRIANGLES, vboframe[0][frameno].getIndicesNum(), GL_UNSIGNED_INT, 0);
    } else {
    	glDrawArrays(GL_TRIANGLES, 0, vboframe[0][frameno].getVerticesNum());
    }
    vboframe[0][frameno].unbind();
    framenoold = frameno;
}

void gSkinnedMesh::resizeAnimation(int verticesNum) {
	animatedPos.resize(verticesNum);
	animatedNorm.resize(verticesNum);
}

void gSkinnedMesh::resetAnimation() {
	animatedPos.assign(animatedPos.size(), glm::vec3(0.0f));
	animatedNorm.assign(animatedNorm.size(), glm::vec3(0.0f));
}

void gSkinnedMesh::setVertexPos(int vertexNo, glm::vec3 newWeight) {
	animatedPos[vertexNo] = newWeight;
}

void gSkinnedMesh::setVertexNorm(int vertexNo, glm::vec3 newWeight) {
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
	animatedPosData = std::vector< std::vector< std::vector< glm::vec3 > > >(animationNum, std::vector< std::vector< glm::vec3 > >(frameNum, std::vector< glm::vec3 >(verticesNum)));
	animatedNormData = std::vector< std::vector< std::vector< glm::vec3 > > >(animationNum, std::vector< std::vector< glm::vec3 > >(frameNum, std::vector< glm::vec3 >(verticesNum)));
    if (isOnVram) {
		vboframe = std::vector<std::vector<gVbo>>(animationNum, std::vector<gVbo>(frameNum));
	}
//	animatedPosData[animationNo][frameNo].resize(verticesNum);
//	animatedNormData[animationNo][frameNo].resize(verticesNum);
}

void gSkinnedMesh::resetVertexAnimationData(int animationNo, int frameNo) {
	animatedPosData[animationNo][frameNo].assign(animatedPosData[animationNo][frameNo].size(), glm::vec3(0.0f));
	animatedNormData[animationNo][frameNo].assign(animatedNormData[animationNo][frameNo].size(), glm::vec3(0.0f));
}

void gSkinnedMesh::setVertexPosData(int animationNo, int frameNo, int vertexNo, glm::vec3 newWeight) {
	animatedPosData[animationNo][frameNo][vertexNo] = newWeight;
}

void gSkinnedMesh::setVertexNormData(int animationNo, int frameNo, int vertexNo, glm::vec3 newWeight) {
	animatedNormData[animationNo][frameNo][vertexNo] = newWeight;
}

const glm::vec3& gSkinnedMesh::getVertexPosData(int animationNo, int frameNo, int vertexNo) const {
	return animatedPosData[animationNo][frameNo][vertexNo];
}

const glm::vec3& gSkinnedMesh::getVertexNormData(int animationNo, int frameNo, int vertexNo) const {
	return animatedNormData[animationNo][frameNo][vertexNo];
}

void gSkinnedMesh::setVerticesData(int animationNo, int frameNo, std::vector<gVertex> vertices, std::vector<gIndex> indices) {
//	this->vertices = vertices;
//	this->indices = indices;
	vboframe[animationNo][frameNo].setVertexData(&vertices[0], sizeof(gVertex), vertices.size());
	if (indices.size() != 0) vboframe[animationNo][frameNo].setIndexData(&indices[0], indices.size());
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

