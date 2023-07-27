/*
 * gMorphingMesh.cpp
 *
 *  Created on: 10 Tem 2023
 *      Author: Deniz Hatipoglu
 */

#include "gMorphingMesh.h"

gMorphingMesh::gMorphingMesh() {
	oldtargetmeshid = currenttargetmeshid = -1;
	oldframeid = currentframeid = -1;
	speed = 1;
	basemesh = nullptr;
	ismorphinganimated = ismorphinganimationstoredonvram = false;
}

gMorphingMesh::~gMorphingMesh() {
}

void gMorphingMesh::addTargetPositions(std::vector<gVertex> &targetVertices) {
	targetpositions.push_back(std::vector<glm::vec3>(targetVertices.size()));
	currenttargetmeshid = targetpositions.size() - 1;
	for (int i = 0; i < targetpositions[currenttargetmeshid].size(); i++) {
		targetpositions[currenttargetmeshid][i] = targetVertices[i].position;
	}
}

void gMorphingMesh::addTargetNormals(std::vector<gVertex> &targetVertices) {
	targetnormals.push_back(std::vector<glm::vec3>(targetVertices.size()));
	currenttargetmeshid = targetnormals.size() - 1;
	for (int i = 0; i < targetnormals[currenttargetmeshid].size(); i++) {
		targetnormals[currenttargetmeshid][i] = targetVertices[i].normal;
	}
}

int gMorphingMesh::addTargetMesh(gMesh *targetMesh) {
	addTargetPositions(targetMesh->getVertices());
	addTargetNormals(targetMesh->getVertices());
	framecounts.push_back(1);
	vboframes.push_back(std::vector<gVbo>());
	framepositions.push_back(std::vector<std::vector<glm::vec3>>());
	framenormals.push_back(std::vector<std::vector<glm::vec3>>());
	return targetpositions.size() - 1;
}

void gMorphingMesh::setBaseMesh(gMesh *baseMesh) {
	if (baseMesh == nullptr) {
		gLoge("gMorphingMesh::setBaseMesh::The given basemesh pointer's value is nullptr. Cannot set the base mesh.");
		return;
	}
	this->basemesh = baseMesh;
	this->setDrawMode(baseMesh->getDrawMode());
	this->setMaterial(baseMesh->getMaterial());
	this->setVertices(baseMesh->getVertices(), baseMesh->getIndices());
}

void gMorphingMesh::setCurrentTargetMeshId(int targetMeshId) {
	//Exception check
	if (targetMeshId < 0 || targetMeshId >= targetpositions.size()) {
		gLoge("gMorphingMesh::setCurrentTargetMeshId::Cannot assign the given current target mesh id.");
		return;
	}
	//Actual statement
	this->currenttargetmeshid = targetMeshId;
}

void gMorphingMesh::interpolate(bool forceInterpolation) {
	//Checking if the current frame is as the old frame for optimization. However only if the datas are not animated and not stored on vram.
	if ((!ismorphinganimated && !ismorphinganimationstoredonvram) && (!forceInterpolation || (oldframeid == currentframeid && oldtargetmeshid == currenttargetmeshid))) return;
	//Exception check
	if (currenttargetmeshid < 0 || basemesh == nullptr) {
		gLoge("gMorphingMesh::interpolate::Cannot interpolate the morphing mesh since either there aren't any target mesh added for morphig mesh or there aren't any base mesh setted.");
		return;
	}
	//Setting vertices of the frame/frames depending on the options.
	if (!ismorphinganimationstoredonvram) {
		if (!ismorphinganimated) {
			std::vector<gVertex> framevertices;
			//Filling the framevertices corresponding to the current frame.
			for (int i = 0; i < targetpositions[currenttargetmeshid].size(); i++) {
				framevertices.push_back(basemesh->getVertices()[i]);
				framevertices[i].position += ((targetpositions[currenttargetmeshid][i] - framevertices[i].position) * ((float)(currentframeid + 1) / (float)framecounts[currenttargetmeshid]));
				framevertices[i].normal += ((targetnormals[currenttargetmeshid][i] - framevertices[i].normal) * ((float)(currentframeid + 1) / (float)framecounts[currenttargetmeshid]));
			}
			//Setting the vertices with framevertices.
			this->setVertices(framevertices, basemesh->getIndices());
			//Setting the olds as currents. Since they are going to be checked in other interpolation to avoid unnecessary frame calculations.
			oldframeid = currentframeid;
			oldtargetmeshid = currenttargetmeshid;
		}
		else {
			std::vector<gVertex> &basevertices = basemesh->getVertices();
			//Clearing the old frames' datas and resizing them according to new ones.
			framepositions[currenttargetmeshid].clear();
			framepositions[currenttargetmeshid].resize(framecounts[currenttargetmeshid]);
			framenormals[currenttargetmeshid].clear();
			framenormals[currenttargetmeshid].resize(framecounts[currenttargetmeshid]);
			for (int i = 0; i < framecounts[currenttargetmeshid]; i++) {
				//Resizing the positions and normals.
				framepositions[currenttargetmeshid][i].resize(basemesh->getVerticesNum());
				framenormals[currenttargetmeshid][i].resize(basemesh->getVerticesNum());
				//Filling the positions with their corresponding frame's interolation factor.
				for (int j = 0; j < targetpositions[currenttargetmeshid].size(); j++) {
					framepositions[currenttargetmeshid][i][j] = basevertices[j].position + ((targetpositions[currenttargetmeshid][j] - basevertices[j].position) * ((float)(i + 1) / (float)framecounts[currenttargetmeshid]));
					framenormals[currenttargetmeshid][i][j] = basevertices[j].normal + ((targetnormals[currenttargetmeshid][j] - basevertices[j].position) * ((float)(i + 1) / (float)framecounts[currenttargetmeshid]));
				}
			}
		}
	}
	else {
		std::vector<gVertex> &basevertices = basemesh->getVertices();
		//Clearing the old frames' datas and resizing them according to new ones.
		vboframes[currenttargetmeshid].clear();
		vboframes[currenttargetmeshid].resize(framecounts[currenttargetmeshid]);
		for (int i = 0; i < framecounts[currenttargetmeshid]; i++) {
			std::vector<gVertex> framevertices(targetpositions[currenttargetmeshid].size());
			//Filling the framevertices vector with the corresponding frame's interpolation factor.
			for (int j = 0; j < targetpositions[currenttargetmeshid].size(); j++) {
				framevertices[j] = basevertices[j];
				framevertices[j].position = basevertices[j].position + ((targetpositions[currenttargetmeshid][j] - basevertices[j].position) * ((float)(i + 1) / (float)framecounts[currenttargetmeshid]));
				framevertices[j].normal = basevertices[j].normal + ((targetnormals[currenttargetmeshid][j] - basevertices[j].position) * ((float)(i + 1) / (float)framecounts[currenttargetmeshid]));
			}
			//Setting the datas of frame's vbo as the datas filled above.
			vboframes[currenttargetmeshid][i].setVertexData(framevertices.data(), sizeof(gVertex), framevertices.size());
			vboframes[currenttargetmeshid][i].setIndexData(basemesh->getIndices().data(), basemesh->getIndicesNum());
		}
	}
}

void gMorphingMesh::setCurrentFrameId(int frameId) {
	//Exception check
	if (frameId < 0 || frameId >= framecounts[currenttargetmeshid]) {
		gLoge("gMorphingMesh::Cannot change the current frame with the desired value since it is beyond the bounds([0, framescount]).");
		return;
	}
	//Actual statement
	this->currentframeid = frameId;
}

void gMorphingMesh::nextFrameId() {
	currentframeid = (currentframeid + speed) % getFrameCount();
}

void gMorphingMesh::setSpeed(int speed) {
	if (speed < 0) {
		gLoge("gMorphingMesh::setSpeed::Cannot assign speed since it is a negative value.");
		return;
	}
	this->speed = speed;
}

void gMorphingMesh::setFrameCount(int targetMeshId, int frameCount) {
	if (targetMeshId < 0 || targetMeshId >= targetpositions.size()) {
		gLoge("gMorphingMesh::setFrameCount::Cannot assign the frame count because the desired target mesh is out of beyonds.");
		return;
	}
	framecounts[targetMeshId] = frameCount;
}

void gMorphingMesh::setTargetPosition(int targetId, int positionId, glm::vec3 newPosition) {
	if (targetId < 0 || targetId >= targetpositions.size() || positionId < 0 || positionId >= targetpositions[targetId].size()) {
		gLoge("gMorphingMesh::setTargetPosition::Couldn't set the position as the desired position since either targetid is beyond bounds or positionid is beyond bounds.");
	}
	targetpositions[targetId][positionId] = newPosition;
}

void gMorphingMesh::setTargetNormal(int targetId, int normalId, glm::vec3 newNormal) {
	if (targetId < 0 || targetId >= targetnormals.size() || normalId < 0 || normalId >= targetnormals[targetId].size()) {
		gLoge("gMorphingMesh::setTargetnormal::Couldn't set the normal as the desired normal since either targetid is beyond bounds or normalid is beyond bounds.");
	}
	targetnormals[targetId][normalId] = newNormal;
}

int gMorphingMesh::getCurrentTargetMeshId() {
	return currenttargetmeshid;
}

int gMorphingMesh::getCurrentFrameId() {
	oldframeid = currentframeid;
	return currentframeid;
}

int gMorphingMesh::getFrameCount(int targetMeshId) {
	//Exception check
	if (targetMeshId < 0 || targetMeshId >= framecounts.size()) {
		gLoge("gMorphingMesh::getFrameCount::Cannot return the frame count because your desired target mesh's id isn't found in target meshs' vector.");
		return -1;
	}
	//Actual statement
	return framecounts[targetMeshId];
}

int gMorphingMesh::getFrameCount() {
	return framecounts[currenttargetmeshid];
}

int gMorphingMesh::getTargetMeshCount() {
	return targetpositions.size();
}

void gMorphingMesh::resetTargetData(int targetId) {
	targetpositions[targetId].assign(targetpositions[targetId].size(), glm::vec3(0));
	targetnormals[targetId].assign(targetnormals[targetId].size(), glm::vec3(0));
}

const glm::vec3& gMorphingMesh::getTargetPosition(int targetId, int positionId) {
	return targetpositions[targetId][positionId];
}

const glm::vec3& gMorphingMesh::getTargetNormal(int targetId, int normalId) {
	return targetnormals[targetId][normalId];
}

void gMorphingMesh::setMorphingAnimated(bool isMorphingAnimated) {
	this->ismorphinganimated = isMorphingAnimated;
}

void gMorphingMesh::setMorphingAnimationStoredOnVram(bool isMorphingAnimationStoredOnVram) {
	this->ismorphinganimationstoredonvram = isMorphingAnimationStoredOnVram;
}

void gMorphingMesh::draw() {
	if (!ismorphinganimationstoredonvram) {
		if (ismorphinganimated && (currentframeid != oldframeid || currenttargetmeshid != oldtargetmeshid)) {
			//Setting the vertices' datas as the stored datas.
			for (int i = 0; i < framepositions[currenttargetmeshid][currentframeid].size(); i++) {
				vertices[i].position = framepositions[currenttargetmeshid][currentframeid][i];
				vertices[i].normal = framenormals[currenttargetmeshid][currentframeid][i];
			}
			this->setVertices(vertices, this->getIndices());
			//Settin olds as currents to avoid unnecessary frame data changes.
			oldtargetmeshid = currenttargetmeshid;
			oldframeid = currentframeid;
		}
		gMesh::draw();
	}
	else {
		if (!isenabled) return;

		drawStart();
		drawVboFrames();
		drawEnd();
	}
}

void gMorphingMesh::drawVboFrames() {
	vboframes[currenttargetmeshid][currentframeid].bind();
	if (vboframes[currenttargetmeshid][currentframeid].isIndexDataAllocated()) {
		glDrawElements(GL_TRIANGLES, vboframes[currenttargetmeshid][currentframeid].getIndicesNum(), GL_UNSIGNED_INT, 0);
	} else {
		glDrawArrays(GL_TRIANGLES, 0, vboframes[currenttargetmeshid][currentframeid].getVerticesNum());
	}
	vboframes[currenttargetmeshid][currentframeid].unbind();
}
