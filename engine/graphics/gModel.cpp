/*
 * gModel.cpp
 *
 *  Created on: May 10, 2020
 *      Author: noyan
 */

#include "gModel.h"
#include <memory>


gModel::gModel() {
	scene = nullptr;
	animationnum = 0;
	isanimated = false;
	animationposition = -1.0f;
	animationpositionold = -1.0f;
	animationframerate = 25.0f;
	animationframenum = 0;
	animationframeno = 0;
	isvertexanimated = false;
	isvertexanimationstoredonvram = false;
	bbminx = 0.0f, bbminy = 0.0f, bbminz = 0.0f;
	bbmaxx = 0.0f, bbmaxy = 0.0f, bbmaxz = 0.0f;
}

// TODO Clean ptrs if any
gModel::~gModel() {
}

void gModel::load(std::string fullPath) {
	loadModelFile(fullPath);
}

void gModel::loadModel(std::string modelPath) {
	loadModelFile(gGetModelsDir() + modelPath);
}

void gModel::loadModelFile(std::string fullPath) {
    // read file via ASSIMP
    Assimp::Importer importer;
    importer.ReadFile(fullPath, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace | aiProcessPreset_TargetRealtime_Fast);
    scene = importer.GetOrphanedScene();
    // check for errors
    if(!scene || (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) || !scene->mRootNode) { // if is Not Zero
        std::cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
        return;
    }
    // retrieve the directory path of the filepath
    directory = fullPath.substr(0, fullPath.find_last_of('/'));
    filename = fullPath.substr(fullPath.find_last_of('/') + 1, fullPath.length());
    animationnum = scene->mNumAnimations;
    isanimated = animationnum > 0;

    // process ASSIMP's root node recursively
    processNode(scene->mRootNode, scene);
    if (isanimated) setAnimationFramerate(animationframerate);
    animate(0);
}

void gModel::move(float dx, float dy, float dz) {
	gNode::move(dx, dy, dz);
	for(unsigned int i = 0; i < meshes.size(); i++) meshes[i].move(dx, dy, dz);
}

void gModel::move(const glm::vec3 dv) {
	gNode::move(dv);
	for(unsigned int i = 0; i < meshes.size(); i++) meshes[i].move(dv);
}

void gModel::rotate(float angle, float ax, float ay, float az) {
	gNode::rotate(angle, ax, ay, az);
	for(unsigned int i = 0; i < meshes.size(); i++) meshes[i].rotate(angle, ax, ay, az);
}

void gModel::rotate(const glm::quat& q) {
	gNode::rotate(q);
	for(unsigned int i = 0; i < meshes.size(); i++) meshes[i].rotate(q);
}

void gModel::scale(float sx, float sy, float sz) {
	gNode::scale(sx, sy, sz);
	for(unsigned int i = 0; i < meshes.size(); i++) meshes[i].scale(sx, sy, sz);
}

void gModel::scale(float s) {
	gNode::scale(s);
	for(unsigned int i = 0; i < meshes.size(); i++) meshes[i].scale(s);
}

void gModel::setPosition(float px, float py, float pz) {
	gNode::setPosition(px, py, pz);
	for(unsigned int i = 0; i < meshes.size(); i++) meshes[i].setPosition(px, py, pz);
}

void gModel::setOrientation(const glm::quat& o) {
	gNode::setOrientation(o);
	for(unsigned int i = 0; i < meshes.size(); i++) meshes[i].setOrientation(o);
}

void gModel::setOrientation(const glm::vec3& angles) {
	gNode::setOrientation(angles);
	for(unsigned int i = 0; i < meshes.size(); i++) meshes[i].setOrientation(angles);
}

void gModel::setScale(const glm::vec3& s) {
	gNode::setScale(s);
	for(unsigned int i = 0; i < meshes.size(); i++) meshes[i].setScale(s);
}

void gModel::setScale(float sx, float sy, float sz) {
	gNode::setScale(sx, sy, sz);
	for(unsigned int i = 0; i < meshes.size(); i++) meshes[i].setScale(sx, sy, sz);
}

void gModel::setScale(float s) {
	gNode::setScale(s);
	for(unsigned int i = 0; i < meshes.size(); i++) meshes[i].setScale(s);
}

void gModel::dolly(float distance) {
	gNode::dolly(distance);
	for(unsigned int i = 0; i < meshes.size(); i++) meshes[i].dolly(distance);
}

void gModel::truck(float distance) {
	gNode::truck(distance);
	for(unsigned int i = 0; i < meshes.size(); i++) meshes[i].truck(distance);
}

void gModel::boom(float distance) {
	gNode::boom(distance);
	for(unsigned int i = 0; i < meshes.size(); i++) meshes[i].boom(distance);
}

void gModel::tilt(float radians) {
	gNode::tilt(radians);
	for(unsigned int i = 0; i < meshes.size(); i++) meshes[i].tilt(radians);
}

void gModel::pan(float radians) {
	gNode::pan(radians);
	for(unsigned int i = 0; i < meshes.size(); i++) meshes[i].pan(radians);
}

void gModel::roll(float radians) {
	gNode::roll(radians);
	for(unsigned int i = 0; i < meshes.size(); i++) meshes[i].roll(radians);
}

void gModel::setTransformationMatrix(glm::mat4 transformationMatrix) {
	gNode::setTransformationMatrix(transformationMatrix);
	for(unsigned int i = 0; i < meshes.size(); i++) meshes[i].setTransformationMatrix(transformationMatrix);
}

void gModel::draw() {
	for(unsigned int i = 0; i < meshes.size(); i++) meshes[i].draw();
}

std::string gModel::getFilename() {
	return filename;
}

std::string gModel::getFullpath() {
	return directory + "/" + filename;
}

int gModel::getMeshNum() {
	return meshes.size();
}

gSkinnedMesh gModel::getMesh(int meshNo) {
	return meshes[meshNo];
}

std::string gModel::getMeshName(int meshNo) {
	return scene->mMeshes[meshNo]->mName.C_Str();
}

void gModel::processNode(aiNode *node, const aiScene *scene) {
	// process each mesh located at the current node
	for(unsigned int i = 0; i < node->mNumMeshes; i++) {
		// the node object only contains indices to index the actual objects in the scene.
		// the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		logi("Loading mesh:" + std::string(mesh->mName.C_Str()));
		gSkinnedMesh modelmesh = processMesh(mesh, scene);
//		if (isanimated) updateBones(&modelmesh, mesh, scene);
//		modelmesh.setParent(this);
		meshes.push_back(modelmesh);
	}

	// after we've processed all of the meshes (if any) we then recursively process each of the children nodes
	for(unsigned int i = 0; i < node->mNumChildren; i++) {
		processNode(node->mChildren[i], scene);
	}

}

gSkinnedMesh gModel::processMesh(aiMesh *mesh, const aiScene *scene) {
    // data to fill
    std::vector<gVertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<gTexture> textures;

    // walk through each of the mesh's vertices
    for(unsigned int i = 0; i < mesh->mNumVertices; i++) {
        gVertex vertex;
        glm::vec3 vector; // we declare a placeholder vector since assimp uses its own vector class that doesn't directly convert to glm's vec3 class so we transfer the data to this placeholder glm::vec3 first.
        // positions
        vector.x = mesh->mVertices[i].x;
        vector.y = mesh->mVertices[i].y;
        vector.z = mesh->mVertices[i].z;
        vertex.position = vector;
        // normals
        vector.x = mesh->mNormals[i].x;
        vector.y = mesh->mNormals[i].y;
        vector.z = mesh->mNormals[i].z;
        vertex.normal = vector;

        // texture coordinates
        if(mesh->mTextureCoords[0]) {
            glm::vec2 vec;
            // a vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't
            // use models where a vertex can have multiple texture coordinates so we always take the first set (0).
            vec.x = mesh->mTextureCoords[0][i].x;
            vec.y = mesh->mTextureCoords[0][i].y;
            vertex.texcoords = vec;
        } else vertex.texcoords = glm::vec2(0.0f, 0.0f);

        // tangent
        vector.x = mesh->mTangents[i].x;
        vector.y = mesh->mTangents[i].y;
        vector.z = mesh->mTangents[i].z;
        vertex.tangent = vector;

        // bitangent
        vector.x = mesh->mBitangents[i].x;
        vector.y = mesh->mBitangents[i].y;
        vector.z = mesh->mBitangents[i].z;
        vertex.bitangent = vector;
        vertices.push_back(vertex);
    }

    // now walk through each of the mesh's faces (a face is a mesh its triangle) and retrieve the corresponding vertex indices.
    for(unsigned int i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];

        // retrieve all indices of the face and store them in the indices vector
        for(unsigned int j = 0; j < face.mNumIndices; j++)
            indices.push_back(face.mIndices[j]);
    }

    // process materials
    aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
    // we assume a convention for sampler names in the shaders. Each diffuse texture should be named
    // as 'texture_diffuseN' where N is a sequential number ranging from 1 to MAX_SAMPLER_NUMBER.
    // Same applies to other texture as the following list summarizes:
    // diffuse: texture_diffuseN
    // specular: texture_specularN
    // normal: texture_normalN

    // return a mesh object created from the extracted mesh data
    gSkinnedMesh gmesh;
    gmesh.setVertices(vertices,  indices);

    // 1. diffuse maps
    std::vector<gTexture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, gTexture::TEXTURETYPE_DIFFUSE);
    for (int i = 0; i < diffuseMaps.size(); i++) diffuseMaps[i].setType(gTexture::TEXTURETYPE_DIFFUSE);
    textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
    // 2. specular maps
    std::vector<gTexture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, gTexture::TEXTURETYPE_SPECULAR);
    for (int i = 0; i < specularMaps.size(); i++) specularMaps[i].setType(gTexture::TEXTURETYPE_SPECULAR);
    textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
    // 3. normal maps
    std::vector<gTexture> normalMaps = loadMaterialTextures(material, aiTextureType_NORMALS, gTexture::TEXTURETYPE_NORMAL);
    for (int i = 0; i < normalMaps.size(); i++) normalMaps[i].setType(gTexture::TEXTURETYPE_NORMAL);
    textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
    // 4. height maps
    std::vector<gTexture> heightMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, gTexture::TEXTURETYPE_HEIGHT);
    for (int i = 0; i < heightMaps.size(); i++) heightMaps[i].setType(gTexture::TEXTURETYPE_HEIGHT);
    textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());
#ifdef LINUX
    // 5. pbr albedo maps
    std::vector<gTexture> albedoMaps = loadMaterialTextures(material, aiTextureType_BASE_COLOR, gTexture::TEXTURETYPE_PBR_ALBEDO);
    for (int i = 0; i < albedoMaps.size(); i++) albedoMaps[i].setType(gTexture::TEXTURETYPE_PBR_ALBEDO);
    textures.insert(textures.end(), albedoMaps.begin(), albedoMaps.end());
    // 6. pbr roughness maps
    std::vector<gTexture> roughnessMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE_ROUGHNESS, gTexture::TEXTURETYPE_PBR_ROUGHNESS);
    for (int i = 0; i < roughnessMaps.size(); i++) roughnessMaps[i].setType(gTexture::TEXTURETYPE_PBR_ROUGHNESS);
    textures.insert(textures.end(), roughnessMaps.begin(), roughnessMaps.end());
    // 7. pbr metalness maps
    std::vector<gTexture> metalnessMaps = loadMaterialTextures(material, aiTextureType_METALNESS, gTexture::TEXTURETYPE_PBR_METALNESS);
    for (int i = 0; i < metalnessMaps.size(); i++) metalnessMaps[i].setType(gTexture::TEXTURETYPE_PBR_METALNESS);
    textures.insert(textures.end(), metalnessMaps.begin(), metalnessMaps.end());
    // 8. pbr normal maps
    std::vector<gTexture> pbrnormalMaps = loadMaterialTextures(material, aiTextureType_NORMAL_CAMERA, gTexture::TEXTURETYPE_PBR_NORMAL);
    for (int i = 0; i < pbrnormalMaps.size(); i++) pbrnormalMaps[i].setType(gTexture::TEXTURETYPE_PBR_NORMAL);
    textures.insert(textures.end(), pbrnormalMaps.begin(), pbrnormalMaps.end());
    // 5. pbr ao maps
    std::vector<gTexture> aoMaps = loadMaterialTextures(material, aiTextureType_AMBIENT_OCCLUSION, gTexture::TEXTURETYPE_PBR_AO);
    for (int i = 0; i < aoMaps.size(); i++) aoMaps[i].setType(gTexture::TEXTURETYPE_PBR_AO);
    textures.insert(textures.end(), aoMaps.begin(), aoMaps.end());
#endif

    gmesh.setTextures(textures);

    return gmesh;
}

std::vector<gTexture> gModel::loadMaterialTextures(aiMaterial *mat, aiTextureType type, int textureType) {
	std::vector<gTexture> textures;
    for(unsigned int i = 0; i < mat->GetTextureCount(type); i++) {
        aiString str;
        mat->GetTexture(type, i, &str);
        // check if texture was loaded before and if so, continue to next iteration: skip loading a new texture
        bool skip = false;
        for(unsigned int j = 0; j < textures_loaded.size(); j++) {
            if(std::strcmp(textures_loaded[j].getFilename().data(), str.C_Str()) == 0) {
                textures.push_back(textures_loaded[j]);
                skip = true; // a texture with the same filepath has already been loaded, continue to next one. (optimization)
                break;
            }
        }

        if(!skip) {   // if texture hasn't been loaded already, load it
            gTexture texture;
            std::string tpath = this->directory + "/" + str.C_Str();
            texture.load(tpath);
            texture.setType(textureType);
            textures.push_back(texture);
            textures_loaded.push_back(texture);  // store it as texture loaded for entire model, to ensure we won't unnecesery load duplicate textures.
        }
    }
    return textures;
}

gTexture gModel::loadMaterialTexture(aiMaterial *mat, aiTextureType type, int textureType) {
	gTexture texture;
    for(unsigned int i = 0; i < mat->GetTextureCount(type); i++) {
        aiString str;
        mat->GetTexture(type, i, &str);

		std::string tpath = this->directory + "/" + str.C_Str();
		texture.load(tpath);
		texture.setType(textureType);
    }
    return texture;
}

bool gModel::isAnimated() {
	return isanimated;
}

int gModel::getAnimationNum() {
	return animationnum;
}

/**
 * Set animation position to given position. Position should be a normalized value;
 */
void gModel::animate(float animationPosition) {
	if(!isanimated) return;

	animationpositionold = animationposition;
	animationposition = animationPosition;

	if(!isvertexanimationstoredonvram && animationposition != animationpositionold) {
		updateAnimationNodes();
		for (int i=0; i<meshes.size(); i++) {
			updateBones(&meshes[i], scene->mMeshes[i]);
			updateVbo(&meshes[i]);
		}
	}
}

void gModel::updateAnimationNodes() {
	int numOfAnimations = scene->mNumAnimations;
	if (numOfAnimations == 0) return;

	for (int ani = 0; ani<numOfAnimations; ani++) {
		aiAnimation* animation = scene->mAnimations[ani];
		float durationInSeconds = getAnimationDuration(ani);
//		logi("animno:" + str(ani) + ", duration:" + str(durationInSeconds));
		float progressInSeconds = animationposition * durationInSeconds;

		unsigned int animnumchan = animation->mNumChannels;
		for(unsigned int i=0; i<animnumchan; i++) {
			const aiNodeAnim * channel = animation->mChannels[i];

			aiVector3D presentPosition(0, 0, 0);
			if(channel->mNumPositionKeys) {
				unsigned int frame = 0;
				while(frame < channel->mNumPositionKeys - 1) {
					if(progressInSeconds < channel->mPositionKeys[frame+1].mTime) {
						break;
					}
					frame++;
				}

				unsigned int nextFrame = (frame + 1) % channel->mNumPositionKeys;
				const aiVectorKey & key = channel->mPositionKeys[frame];
				const aiVectorKey & nextKey = channel->mPositionKeys[nextFrame];
				double diffTime = nextKey.mTime - key.mTime;
				if(diffTime < 0.0) {
					diffTime += durationInSeconds;
				}
				if(diffTime > 0) {
					float factor = float((progressInSeconds - key.mTime) / diffTime);
					presentPosition = key.mValue + (nextKey.mValue - key.mValue) * factor;
				} else {
					presentPosition = key.mValue;
				}
			}

			aiQuaternion presentRotation(1, 0, 0, 0);
			if(channel->mNumRotationKeys > 0) {
				unsigned int frame = 0;
				while(frame < channel->mNumRotationKeys - 1) {
					if(progressInSeconds < channel->mRotationKeys[frame+1].mTime) {
						break;
					}
					frame++;
				}

				unsigned int nextFrame = (frame + 1) % channel->mNumRotationKeys;
				const aiQuatKey& key = channel->mRotationKeys[frame];
				const aiQuatKey& nextKey = channel->mRotationKeys[nextFrame];
				double diffTime = nextKey.mTime - key.mTime;
				if(diffTime < 0.0) {
					diffTime += durationInSeconds;
				}
				if(diffTime > 0) {
					float factor = float((progressInSeconds - key.mTime) / diffTime);
					aiQuaternion::Interpolate(presentRotation, key.mValue, nextKey.mValue, factor);
				} else {
					presentRotation = key.mValue;
				}
			}

			aiVector3D presentScaling(1, 1, 1);
			if(channel->mNumScalingKeys > 0) {
				unsigned int frame = 0;
				while(frame < channel->mNumScalingKeys - 1){
					if(progressInSeconds < channel->mScalingKeys[frame+1].mTime) {
						break;
					}
					frame++;
				}

				presentScaling = channel->mScalingKeys[frame].mValue;
			}

			aiMatrix4x4 mat = aiMatrix4x4(presentRotation.GetMatrix());
			aiNode * targetNode = scene->mRootNode->FindNode(channel->mNodeName);
			mat.a1 *= presentScaling.x; mat.b1 *= presentScaling.x; mat.c1 *= presentScaling.x;
			mat.a2 *= presentScaling.y; mat.b2 *= presentScaling.y; mat.c2 *= presentScaling.y;
			mat.a3 *= presentScaling.z; mat.b3 *= presentScaling.z; mat.c3 *= presentScaling.z;
			mat.a4 = presentPosition.x; mat.b4 = presentPosition.y; mat.c4 = presentPosition.z;

			targetNode->mTransformation = mat;
		}
	}
}

void gModel::updateBones(gSkinnedMesh* gmesh, aiMesh* aimesh) {
	gmesh->resizeAnimation(aimesh->mNumVertices);

	std::vector<aiMatrix4x4> boneMatrices(aimesh->mNumBones);
	for(unsigned int a = 0; a < aimesh->mNumBones; a++) {
		const aiBone* bone = aimesh->mBones[a];

		// find the corresponding node by again looking recursively through the node hierarchy for the same name
		aiNode* node = scene->mRootNode->FindNode(bone->mName);

		// start with the mesh-to-bone matrix
		boneMatrices[a] = bone->mOffsetMatrix;
		// and now append all node transformations down the parent chain until we're back at mesh coordinates again
		const aiNode* tempNode = node;
		while(tempNode) {
			// check your matrix multiplication order here!!!
			boneMatrices[a] = tempNode->mTransformation * boneMatrices[a];
			// boneMatrices[a] = boneMatrices[a] * tempNode->mTransformation;
			tempNode = tempNode->mParent;
		}
	}

	gmesh->resetAnimation();

	// loop through all vertex weights of all bones
	for(unsigned int a = 0; a < aimesh->mNumBones; ++a) {
		const aiBone* bone = aimesh->mBones[a];
		const aiMatrix4x4& posTrafo = boneMatrices[a];

		for(unsigned int b = 0; b < bone->mNumWeights; ++b) {
			const aiVertexWeight& weight = bone->mWeights[b];
			size_t vertexId = weight.mVertexId;
			const aiVector3D& srcPos = aimesh->mVertices[vertexId];

			glm::vec3 oldweightpos = gmesh->getVertexPos(vertexId);
			aiVector3D aiaddweight = weight.mWeight * (posTrafo * srcPos);
			glm::vec3 vPos(oldweightpos.x + aiaddweight.x, oldweightpos.y + aiaddweight.y, oldweightpos.z + aiaddweight.z);
			gmesh->setVertexPos(vertexId, vPos);
		}
		if(aimesh->HasNormals()){
			// 3x3 matrix, contains the bone matrix without the translation, only with rotation and possibly scaling
			aiMatrix3x3 normTrafo = aiMatrix3x3( posTrafo);
			for(unsigned int b = 0; b < bone->mNumWeights; ++b) {
				const aiVertexWeight& weight = bone->mWeights[b];
				size_t vertexId = weight.mVertexId;
				const aiVector3D& srcNorm = aimesh->mNormals[vertexId];

				glm::vec3 oldweightnorm = gmesh->getVertexNorm(vertexId);
				aiVector3D aiaddweight = weight.mWeight * (posTrafo * srcNorm);
				gmesh->setVertexNorm(vertexId, glm::vec3(oldweightnorm.x + aiaddweight.x, oldweightnorm.y + aiaddweight.y, oldweightnorm.z + aiaddweight.z));
			}
		}
	}
}

void gModel::updateVbo(gSkinnedMesh* gmesh) {
	std::vector<gVertex> vertexarray = gmesh->getVertices();
	std::vector<unsigned int> indexarray = gmesh->getIndices();
	for (int i=0; i<gmesh->getVbo()->getVerticesNum(); i++) {
		vertexarray[i].position = gmesh->getVertexPos(i);
		vertexarray[i].normal = gmesh->getVertexNorm(i);
	}
	gmesh->setVertices(vertexarray, indexarray);
}

/**
 * Returns duration in seconds.
 */
float gModel::getAnimationDuration(int animationNo) {
	return scene->mAnimations[animationNo]->mDuration;
}

float gModel::getAnimationPosition() {
	return animationposition;
}

void gModel::setAnimationFramerate(float animationFramerate) {
	animationframerate = animationFramerate;
	setAnimationFrameNum(getAnimationDuration() * animationframerate);
}

float gModel::getAnimationFramerate() {
	return animationframerate;
}

void gModel::setAnimationFrameNum(int animationKeyNum) {
	if (animationKeyNum == animationframenum) return;
	animationframenum = animationKeyNum;
	generateAnimationKeys();
	animationframeno = 0;
}

int gModel::getAnimationFrameNum() {
	return animationframenum;
}

void gModel::setAnimationFrameNo(int frameNo) {
	animationframeno = frameNo;
	for(int i=0; i<meshes.size(); i++) meshes[i].setFrameNo(frameNo);
	animate(animationkeys[frameNo]);
}

void gModel::nextAnimationFrame() {
	animationframeno++;
	if (animationframeno >= animationframenum) animationframeno = 0;
	setAnimationFrameNo(animationframeno);
}

int gModel::getAnimationFrameNo() {
	return animationframeno;
}

void gModel::generateAnimationKeys() {
	animationkeys.clear();

//	logi("animationkeynum:" + str(animationframenum));
	float keylength = 1.0f / (animationframenum - 1);
	for (int i=0; i<animationframenum-1; i++) {
		animationkeys.push_back(i * keylength);
	}
	animationkeys.push_back(1.0f);
}

void gModel::makeVertexAnimated(bool storeOnVram) {
	if (!isvertexanimated) {
		isvertexanimationstoredonvram = storeOnVram;
		prepareVertexAnimationData();
		isvertexanimated = true;
		for(int i=0; i<meshes.size(); i++) {
			meshes[i].setVertexAnimated(true);
			meshes[i].setVertexAnimationStoredOnVram(storeOnVram);
		}
	} else {
		logi("The model is already vertex animated!");
	}
}

void gModel::prepareVertexAnimationData() {
    int mnum = meshes.size();
    int anum = scene->mNumAnimations;
    int fnum = getAnimationFrameNum();
    int vnum = scene->mMeshes[0]->mNumVertices;
    for (int i=0; i<scene->mNumMeshes; i++) if (scene->mMeshes[i]->mNumVertices > vnum) scene->mMeshes[i]->mNumVertices;

    // update mesh position for the animation
    for(unsigned int i=0; i<meshes.size(); ++i) {
    	meshes[i].resizeVertexAnimationData(anum, fnum, scene->mMeshes[i]->mNumVertices, isvertexanimationstoredonvram);

        // current mesh we are introspecting
        const aiMesh* mesh = scene->mMeshes[i];
        for (unsigned int j=0; j<scene->mNumAnimations; j++) {
            for (unsigned int k=0; k<fnum; k++) {
//            	meshes[i].resizeVertexAnimation(j, k, mesh->mNumVertices);
            	setAnimationFrameNo(k);
                updateAnimationNodes();
                updateBones(&meshes[i], scene->mMeshes[i]);

                // calculate bone matrices
                std::vector<aiMatrix4x4> boneMatrices(mesh->mNumBones);
                for(unsigned int a=0; a<mesh->mNumBones; ++a) {
                    const aiBone* bone = mesh->mBones[a];

                    // find the corresponding node by again looking recursively through the node hierarchy for the same name
                    aiNode* node = scene->mRootNode->FindNode(bone->mName);

                    // start with the mesh-to-bone matrix
                    boneMatrices[a] = bone->mOffsetMatrix;
                    // and now append all node transformations down the parent chain until we're back at mesh coordinates again
                    const aiNode* tempNode = node;
                    while(tempNode) {
                        // check your matrix multiplication order here!!!
                        boneMatrices[a] = tempNode->mTransformation * boneMatrices[a];
                        // boneMatrices[a] = boneMatrices[a] * tempNode->mTransformation;
                        tempNode = tempNode->mParent;
                    }
                }

                meshes[i].resetVertexAnimationData(j, k);

                // loop through all vertex weights of all bones
                for(unsigned int a=0; a<mesh->mNumBones; ++a) {
                    const aiBone* bone = mesh->mBones[a];
                    const aiMatrix4x4& posTrafo = boneMatrices[a];

                    for(unsigned int b=0; b<bone->mNumWeights; ++b) {
                        const aiVertexWeight& weight = bone->mWeights[b];
                        size_t vertexId = weight.mVertexId;
                        const aiVector3D& srcPos = mesh->mVertices[vertexId];

            			glm::vec3 oldweightpos = meshes[i].getVertexPosData(j, k, vertexId);
            			aiVector3D aiaddweight = weight.mWeight * (posTrafo * srcPos);
            			glm::vec3 vPos(oldweightpos.x + aiaddweight.x, oldweightpos.y + aiaddweight.y, oldweightpos.z + aiaddweight.z);
            			meshes[i].setVertexPosData(j, k, vertexId, vPos);
                    }
                    if(mesh->HasNormals()){
                        // 3x3 matrix, contains the bone matrix without the translation, only with rotation and possibly scaling
                        aiMatrix3x3 normTrafo = aiMatrix3x3( posTrafo);
                        for(unsigned int b=0; b<bone->mNumWeights; ++b) {
                            const aiVertexWeight& weight = bone->mWeights[b];
                            size_t vertexId = weight.mVertexId;

                            const aiVector3D& srcNorm = mesh->mNormals[vertexId];

            				glm::vec3 oldweightnorm = meshes[i].getVertexNormData(j, k, vertexId);
            				aiVector3D aiaddweight = weight.mWeight * (posTrafo * srcNorm);
            				meshes[i].setVertexNormData(j, k, vertexId, glm::vec3(oldweightnorm.x + aiaddweight.x, oldweightnorm.y + aiaddweight.y, oldweightnorm.z + aiaddweight.z));
                        }
                    }
                }

                if (isvertexanimationstoredonvram) {
                	std::vector<gVertex> vertexarray = meshes[i].getVertices();
                	std::vector<unsigned int> indexarray = meshes[i].getIndices();
                	for (int l=0; l<meshes[i].getVbo()->getVerticesNum(); l++) {
                		vertexarray[l].position = meshes[i].getVertexPosData(j, k, l);
                		vertexarray[l].normal = meshes[i].getVertexNormData(j, k, l);
                	}
                	// TODO Set vertices into vboframes!!!
                	meshes[i].setVerticesData(j, k, vertexarray, indexarray);
//                	meshes[i].animatedPosData[j][k].clear();
//                	meshes[i].animatedNormData[j][k].clear();
//                	meshes[i].getVbo()->clear(); // clear vbo
                }
            }
        }
        if (isvertexanimationstoredonvram) {
        	meshes[i].clearAnimation();
        }
    }
    if (!isvertexanimationstoredonvram) {
        setAnimationFrameNo(0);
    }
}

bool gModel::isVertexAnimated() {
	return isvertexanimated;
}

bool gModel::isVertexAnimationStoredOnVram() {
	return isvertexanimationstoredonvram;
}

gBoundingBox gModel::getBoundingBox() {
	bbminx = 0.0f, bbminy = 0.0f, bbminz = 0.0f;
	bbmaxx = 0.0f, bbmaxy = 0.0f, bbmaxz = 0.0f;

	for (int i = 0; i< meshes.size(); i++) {
		std::vector<gVertex> vertices = meshes[i].getVertices();
		for (int j = 0; j < vertices.size(); j++) {
			gVertex v = vertices[j];
			glm::vec3 vpos = glm::vec3(localtransformationmatrix * glm::vec4(v.position, 1.0));

			bbminx = std::min(bbminx, vpos.x);
			bbminy = std::min(bbminy, vpos.y);
			bbminz = std::min(bbminz, vpos.z);
			bbmaxx = std::max(bbmaxx, vpos.x);
			bbmaxy = std::max(bbmaxy, vpos.y);
			bbmaxz = std::max(bbmaxz, vpos.z);
		}
	}

	return gBoundingBox(bbminx, bbminy, bbminz, bbmaxx, bbmaxy, bbmaxz);
}


