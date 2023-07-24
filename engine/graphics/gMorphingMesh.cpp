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
	//Exception check
	if (!forceInterpolation || oldframeid == currentframeid && oldtargetmeshid == currenttargetmeshid) return;
	if (currenttargetmeshid < 0 || basemesh == nullptr) {
		gLoge("gMorphingMesh::interpolate::Cannot interpolate the morphing mesh since either there aren't any target mesh added for morphig mesh or there aren't any base mesh setted.");
		return;
	}
	//Setting vertices of the current frame.
	std::vector<gVertex> framevertices;
	for (int i = 0; i < targetpositions[currenttargetmeshid].size(); i++) {
		framevertices.push_back(basemesh->getVertices()[i]);
		framevertices[i].position += ((targetpositions[currenttargetmeshid][i] - framevertices[i].position) * ((float)(currentframeid + 1) / (float)framecounts[currenttargetmeshid]));
		framevertices[i].normal += ((targetnormals[currenttargetmeshid][i] - framevertices[i].normal) * ((float)(currentframeid + 1) / (float)framecounts[currenttargetmeshid]));
	}
	this->setVertices(framevertices);
	oldframeid = currentframeid;
	oldtargetmeshid = currenttargetmeshid;
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
		gLoge("gMorphingMesh::setTargetPosition::Couldn't set the position to the desired position since either targetid is beyond bounds or positionid is beyond bounds.");
	}
	targetpositions[targetId][positionId] = newPosition;
}

void gMorphingMesh::setTargetNormal(int targetId, int normalId, glm::vec3 newNormal) {
	if (targetId < 0 || targetId >= targetnormals.size() || normalId < 0 || normalId >= targetnormals[targetId].size()) {
		gLoge("gMorphingMesh::setTargetnormal::Couldn't set the normal to the desired normal since either targetid is beyond bounds or normalid is beyond bounds.");
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
