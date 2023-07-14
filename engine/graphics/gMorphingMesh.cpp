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
	currentbasemeshid = -1;
	speed = 1;
}

gMorphingMesh::~gMorphingMesh() {
}

void gMorphingMesh::loadMorphingModel(std::string path) {
	loadModelFile(gGetModelsDir() + path);
}

void gMorphingMesh::setCurrentTargetMeshId(int currenttargetmeshid) {
	//Exception check
	if (currenttargetmeshid < 0 || currenttargetmeshid >= targetpositions[currentbasemeshid].size()) {
		std::cout << "ERROR::MORPHINGMESH::Cannot assign the given current target mesh id." << std::endl;
		return;
	}
	//Actual statement
	this->currenttargetmeshid = currenttargetmeshid;
}

void gMorphingMesh::loadModel(gModel *basemodel) {
	//Copying the necessary mesh attributes to this morphing mesh.
	this->basemeshes.clear();
	this->basemeshes.resize(basemodel->getMeshNum());
	for (int i = 0; i < basemeshes.size(); i++) {
		basemeshes[i] = basemodel->getMeshPtr(i);
		targetpositions.push_back(std::vector<std::vector<glm::vec3>>());
		vboframes.push_back(std::vector<std::vector<gVbo>>());
		framepositions.push_back(std::vector<std::vector<glm::vec3>>());
	}
	currentbasemeshid = this->basemeshes.size() - 1;
}

void gMorphingMesh::interpolate(int framescount) {
	//Exception check
	if (currenttargetmeshid < 0 || framescount < 0) {
		std::cout << "ERROR::MORPHINGMESH::Cannot interpolate the morphing mesh since either there aren't any model loaded for morphig mesh or the argument is invalid." << std::endl;
		return;
	}
	//Resetting the previous frame datas.
	framepositions[currentbasemeshid].clear();
	framepositions[currentbasemeshid].resize(framescount, std::vector<glm::vec3>(basemeshes[currentbasemeshid]->getVertices().size()));
	vboframes[currentbasemeshid][currenttargetmeshid].clear();
	vboframes[currentbasemeshid][currenttargetmeshid].resize(framescount);
	std::vector<gVertex> vertices;
	//Creating frames
	for (int i = 0; i < framescount; i++) {
		//Setting vertices of the current frame.
		vertices.clear();
		for (int j = 0; j < basemeshes[currentbasemeshid]->getVertices().size(); j++) {
			vertices.push_back(basemeshes[currentbasemeshid]->getVertices()[j]);
			vertices[j].position = basemeshes[currentbasemeshid]->getVertices()[j].position + ((targetpositions[currentbasemeshid][currenttargetmeshid][j] - basemeshes[currentbasemeshid]->getVertices()[j].position) * ((float)(i + 1) / (float)framescount));
		}
		//Assigning the filled vertices to the current frame's vbo.
		setVerticesData(i, vertices, basemeshes[currentbasemeshid]->getIndices());
	}
	currentframeid = framescount - 1;
}

void gMorphingMesh::loadModelFile(const std::string &fullpath) {
#ifdef LINUX
	std::shared_ptr<aiPropertyStore> store;
    store.reset(aiCreatePropertyStore(), aiReleasePropertyStore);
    // only ever give us triangles.
    aiSetImportPropertyInteger(store.get(), AI_CONFIG_PP_SBP_REMOVE, aiPrimitiveType_LINE | aiPrimitiveType_POINT );
    aiSetImportPropertyInteger(store.get(), AI_CONFIG_PP_PTV_NORMALIZE, true);

    scene = aiImportFileExWithProperties(fullpath.c_str(), aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace | aiProcessPreset_TargetRealtime_Fast, NULL, store.get());
    // check for errors
    if(!scene || (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) || !scene->mRootNode) { // if is Not Zero
    	std::cout << "ERROR::ASSIMP:: " << aiGetErrorString() << std::endl;
        return;
    }
#else
    Assimp::Importer importer;
    importer.SetPropertyInteger(AI_CONFIG_PP_SBP_REMOVE, aiPrimitiveType_LINE | aiPrimitiveType_POINT);
    importer.SetPropertyBool(AI_CONFIG_PP_PTV_NORMALIZE, true);
    importer.ReadFile(fullpath, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace | aiProcess_GenNormals | aiProcess_GenUVCoords | aiProcess_SplitLargeMeshes | aiProcess_SortByPType);
    scene = importer.GetOrphanedScene();
    // check for errors
    if(!scene || (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) || !scene->mRootNode) { // if is Not Zero
        std::cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
        return;
    }
#endif

    // retrieve the directory path of the filepath
    directory = fullpath.substr(0, fullpath.find_last_of('/'));
    filename = fullpath.substr(fullpath.find_last_of('/') + 1, fullpath.length());

    // process ASSIMP's root node recursively
    loadorder = 0;
    processNode(scene->mRootNode, scene);
}

void gMorphingMesh::processNode(aiNode *node, const aiScene *scene) {
	// process each mesh located at the current node
	for(unsigned int i = 0; i < node->mNumMeshes; i++) {
		// the node object only contains indices to index the actual objects in the scene.
		// the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		gLogi("gMorphingMesh") << "Loading mesh:" << mesh->mName.C_Str() << ", tm:" << node->mTransformation[0];
//		if (isanimated) updateBones(&modelmesh, mesh, scene);
//		modelmesh.setParent(this);
		processMesh(mesh, scene, node->mTransformation, loadorder);
	}

	// after we've processed all of the meshes (if any) we then recursively process each of the children nodes
	for(unsigned int i = 0; i < node->mNumChildren; i++) {
		processNode(node->mChildren[i], scene);
		loadorder++;
	}
}

void gMorphingMesh::processMesh(aiMesh *mesh, const aiScene *scene, aiMatrix4x4 matrix, int basemeshid) {
	targetpositions[basemeshid].push_back(std::vector<glm::vec3>(mesh->mNumVertices));
	vboframes[basemeshid].push_back(std::vector<gVbo>());
	currenttargetmeshid = targetpositions[basemeshid].size() - 1;
	for (int i = 0; i < mesh->mNumVertices; i++) {
		glm::vec3 vector;
		//positions
		vector.x = mesh->mVertices[i].x;
		vector.y = mesh->mVertices[i].y;
		vector.z = mesh->mVertices[i].z;
		targetpositions[basemeshid][currenttargetmeshid][i] = vector;
	}
}

void gMorphingMesh::setVerticesData(int frameno, std::vector<gVertex>& vertices, std::vector<unsigned int>& indices) {
	vboframes[currentbasemeshid][currenttargetmeshid][frameno].setVertexData(vertices.data(), sizeof(gVertex), vertices.size());
	if (indices.size() != 0) vboframes[currentbasemeshid][currenttargetmeshid][frameno].setIndexData(indices.data(), indices.size());
}

void gMorphingMesh::draw() {
	if (!isenabled) return;

	this->setTransformationMatrix(basemeshes[currentbasemeshid]->getTransformationMatrix());
	this->setVertices(basemeshes[currentbasemeshid]->getVertices(), basemeshes[currentbasemeshid]->getIndices());
	this->setDrawMode(basemeshes[currentbasemeshid]->getDrawMode());
	this->setMaterial(basemeshes[currentbasemeshid]->getMaterial());

	drawStart();
	drawVboFrame();
	drawEnd();
}

void gMorphingMesh::drawVboFrame() {
	if (!isenabled) return;

    vboframes[currentbasemeshid][currenttargetmeshid][currentframeid].bind();
    if (vboframes[currentbasemeshid][currenttargetmeshid][currentframeid].isIndexDataAllocated()) {
        glDrawElements(this->getDrawMode(), vboframes[currentbasemeshid][currenttargetmeshid][currentframeid].getIndicesNum(), GL_UNSIGNED_INT, 0);
    } else {
    	glDrawArrays(this->getDrawMode(), 0, vboframes[currentbasemeshid][currenttargetmeshid][currentframeid].getVerticesNum());
    }
    vboframes[currentbasemeshid][currenttargetmeshid][currentframeid].unbind();
}

void gMorphingMesh::setCurrentFrameId(int frameid) {
	//Exception check
	if (frameid < 0 || frameid >= vboframes[currentbasemeshid][currenttargetmeshid].size()) {
		std::cout << "ERROR::MORPHINGMESH::Cannot change the current frame with the desired value since it is beyond the bounds([0, framescount])." << std::endl;
		return;
	}
	//Actual statement
	this->currentframeid = frameid;
}

int gMorphingMesh::getCurrentTargetMeshId() {
	return currenttargetmeshid;
}

int gMorphingMesh::getCurrentFrameId() {
	return currentframeid;
}

int gMorphingMesh::getFrameCount(int targetmeshid) {
	//Exception check
	if (targetmeshid < 0 || targetmeshid >= vboframes[currentbasemeshid].size()) {
		gLogi("ERROR::MORPHINGMESH::Cannot return the frame count because your desired target mesh's id isn't found in target meshs' vector.");
	}
	//Actual statement
	return vboframes[currentbasemeshid][targetmeshid].size();
}

int gMorphingMesh::getFrameCount() {
	return vboframes[currentbasemeshid][currenttargetmeshid].size();
}

int gMorphingMesh::getTargetMeshCount() {
	return targetpositions[currentbasemeshid].size();
}

void gMorphingMesh::setCurrentBaseMeshId(int basemeshid) {
	if (basemeshid < 0 || basemeshid >= basemeshes.size()) {
		std::cout << "ERROR::GMORPHINGMESH::Cannot assign the current base mesh id to the desired value since it is beyond the bounds." << std::endl;
		return;
	}
	currentbasemeshid = basemeshid;
}

int gMorphingMesh::getBaseMeshCount() {
	return basemeshes.size();
}

int gMorphingMesh::getCurrentBaseMeshId() {
	return currentbasemeshid;
}

void gMorphingMesh::nextFrameId() {
	currentframeid = (currentframeid + speed) % getFrameCount();
}

void gMorphingMesh::setSpeed(int speed) {
	this->speed = speed;
}
