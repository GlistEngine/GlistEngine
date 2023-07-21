/*
 * gMorphingMesh.cpp
 *
 *  Created on: 10 Tem 2023
 *      Author: Deniz Hatipoglu
 */

#include "gMorphingMesh.h"

gMorphingMesh::gMorphingMesh() {
	currenttargetmeshid = -1;
	currentframeid = -1;
	speed = 1;
	basemesh = nullptr;
}

gMorphingMesh::~gMorphingMesh() {
}

void gMorphingMesh::addTargetPositions(std::vector<gVertex> &targetvertices) {
	targetpositions.push_back(std::vector<glm::vec3>(targetvertices.size()));
	currenttargetmeshid = targetpositions.size() - 1;
	for (int i = 0; i < targetpositions[currenttargetmeshid].size(); i++) {
		targetpositions[currenttargetmeshid][i] = targetvertices[i].position;
	}
}

void gMorphingMesh::addTargetNormals(std::vector<gVertex> &targetvertices) {
	targetnormals.push_back(std::vector<glm::vec3>(targetvertices.size()));
	currenttargetmeshid = targetnormals.size() - 1;
	for (int i = 0; i < targetnormals[currenttargetmeshid].size(); i++) {
		targetnormals[currenttargetmeshid][i] = targetvertices[i].normal;
	}
}

int gMorphingMesh::addTargetMesh(gMesh *targetmesh) {
	addTargetPositions(targetmesh->getVertices());
	addTargetNormals(targetmesh->getVertices());
	framecounts.push_back(1);
	return targetpositions.size() - 1;
}

void gMorphingMesh::setBaseMesh(gMesh *basemesh) {
	if (basemesh == nullptr) {
		gLoge("gMorphingMesh::setBaseMesh::The given basemesh pointer's value is nullptr. Cannot set the base mesh.");
		return;
	}
	this->basemesh = basemesh;
	this->setDrawMode(basemesh->getDrawMode());
	this->setMaterial(basemesh->getMaterial());
	this->setVertices(basemesh->getVertices(), basemesh->getIndices());
}

void gMorphingMesh::setCurrentTargetMeshId(int currenttargetmeshid) {
	//Exception check
	if (currenttargetmeshid < 0 || currenttargetmeshid >= targetpositions.size()) {
		gLoge("gMorphingMesh::setCurrentTargetMeshId::Cannot assign the given current target mesh id.");
		return;
	}
	//Actual statement
	this->currenttargetmeshid = currenttargetmeshid;
}

void gMorphingMesh::interpolate() {
	//Exception check
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
}

void gMorphingMesh::setCurrentFrameId(int frameid) {
	//Exception check
	if (frameid < 0 || frameid >= framecounts[currenttargetmeshid]) {
		gLoge("gMorphingMesh::Cannot change the current frame with the desired value since it is beyond the bounds([0, framescount]).");
		return;
	}
	//Actual statement
	this->currentframeid = frameid;
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

void gMorphingMesh::setFrameCount(int targetmeshid, int framecount) {
	if (targetmeshid < 0 || targetmeshid >= targetpositions.size()) {
		gLoge("gMorphingMesh::setFrameCount::Cannot assign the frame count because the desired target mesh is out of beyonds.");
		return;
	}
	framecounts[targetmeshid] = framecount;
}

void gMorphingMesh::setTargetPosition(int targetid, int positionid, glm::vec3 newposition) {
	if (targetid < 0 || targetid >= targetpositions.size() || positionid < 0 || positionid >= targetpositions[targetid].size()) {
		gLoge("gMorphingMesh::setTargetPosition::Couldn't set the position to the desired position since either targetid is beyond bounds or positionid is beyond bounds.");
	}
	targetpositions[targetid][positionid] = newposition;
}

void gMorphingMesh::setTargetNormal(int targetid, int normalid, glm::vec3 newnormal) {
	if (targetid < 0 || targetid >= targetnormals.size() || normalid < 0 || normalid >= targetnormals[targetid].size()) {
		gLoge("gMorphingMesh::setTargetnormal::Couldn't set the normal to the desired normal since either targetid is beyond bounds or normalid is beyond bounds.");
	}
	targetnormals[targetid][normalid] = newnormal;
}

int gMorphingMesh::getCurrentTargetMeshId() {
	return currenttargetmeshid;
}

int gMorphingMesh::getCurrentFrameId() {
	return currentframeid;
}

int gMorphingMesh::getFrameCount(int targetmeshid) {
	//Exception check
	if (targetmeshid < 0 || targetmeshid >= framecounts.size()) {
		gLoge("gMorphingMesh::getFrameCount::Cannot return the frame count because your desired target mesh's id isn't found in target meshs' vector.");
		return -1;
	}
	//Actual statement
	return framecounts[targetmeshid];
}

int gMorphingMesh::getFrameCount() {
	return framecounts[currenttargetmeshid];
}

int gMorphingMesh::getTargetMeshCount() {
	return targetpositions.size();
}
