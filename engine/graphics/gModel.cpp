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


gModel::~gModel() {
	for (const auto& item : textures_loaded) {
		delete item;
	}
}

void gModel::load(const std::string& fullPath) {
	loadModelFile(fullPath);
}

void gModel::loadModel(const std::string& modelPath) {
	loadModelFile(gGetModelsDir() + modelPath);
}

void gModel::loadModelFile(const std::string& fullPath) {
    // read file via ASSIMP
#ifdef LINUX
	std::shared_ptr<aiPropertyStore> store;
    store.reset(aiCreatePropertyStore(), aiReleasePropertyStore);
    // only ever give us triangles.
    aiSetImportPropertyInteger(store.get(), AI_CONFIG_PP_SBP_REMOVE, aiPrimitiveType_LINE | aiPrimitiveType_POINT );
    aiSetImportPropertyInteger(store.get(), AI_CONFIG_PP_PTV_NORMALIZE, true);

    scene = aiImportFileExWithProperties(fullPath.c_str(), aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace | aiProcessPreset_TargetRealtime_Fast, NULL, store.get());
    // check for errors
    if(!scene || (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) || !scene->mRootNode) { // if is Not Zero
    	std::cout << "ERROR::ASSIMP:: " << aiGetErrorString() << std::endl;
        return;
    }
#else
    Assimp::Importer importer;
    importer.SetPropertyInteger(AI_CONFIG_PP_SBP_REMOVE, aiPrimitiveType_LINE | aiPrimitiveType_POINT);
    importer.SetPropertyBool(AI_CONFIG_PP_PTV_NORMALIZE, true);
    importer.ReadFile(fullPath, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace | aiProcessPreset_TargetRealtime_Fast);
    scene = importer.GetOrphanedScene();
    // check for errors
    if(!scene || (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) || !scene->mRootNode) { // if is Not Zero
        std::cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
        return;
    }
#endif

    // retrieve the directory path of the filepath
    directory = fullPath.substr(0, fullPath.find_last_of('/'));
    filename = fullPath.substr(fullPath.find_last_of('/') + 1, fullPath.length());
    animationnum = scene->mNumAnimations;
    isanimated = animationnum > 0;

    // process ASSIMP's root node recursively
    processNode(scene->mRootNode, scene);
    initialboundingbox = getBoundingBox();
    if (isanimated) setAnimationFramerate(animationframerate);
    animate(0);
}

void gModel::loadModelWithOriginalVertices(const std::string& modelPath) {
	loadModelFileWithOriginalVertices(gGetModelsDir() + modelPath);
}

void gModel::loadModelFileWithOriginalVertices(const std::string &fullPath) {
#ifdef LINUX
	std::shared_ptr<aiPropertyStore> store;
    store.reset(aiCreatePropertyStore(), aiReleasePropertyStore);
    // only ever give us triangles.
    aiSetImportPropertyInteger(store.get(), AI_CONFIG_PP_SBP_REMOVE, aiPrimitiveType_LINE | aiPrimitiveType_POINT );
    aiSetImportPropertyInteger(store.get(), AI_CONFIG_PP_PTV_NORMALIZE, true);

    scene = aiImportFileExWithProperties(fullPath.c_str(), aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace | aiProcess_GenNormals | aiProcess_GenUVCoords | aiProcess_SplitLargeMeshes | aiProcess_SortByPType, NULL, store.get());
    // check for errors
    if(!scene || (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) || !scene->mRootNode) { // if is Not Zero
    	std::cout << "ERROR::ASSIMP:: " << aiGetErrorString() << std::endl;
        return;
    }
#else
    Assimp::Importer importer;
    importer.SetPropertyInteger(AI_CONFIG_PP_SBP_REMOVE, aiPrimitiveType_LINE | aiPrimitiveType_POINT);
    importer.SetPropertyBool(AI_CONFIG_PP_PTV_NORMALIZE, true);
    importer.ReadFile(fullPath, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace | aiProcess_GenNormals | aiProcess_GenUVCoords | aiProcess_SplitLargeMeshes | aiProcess_SortByPType);
    scene = importer.GetOrphanedScene();
    // check for errors
    //aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace | aiProcess_GenNormals | aiProcess_GenUVCoords | aiProcess_SplitLargeMeshes | aiProcess_SortByPType
    if(!scene || (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) || !scene->mRootNode) { // if is Not Zero
        std::cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
        return;
    }
#endif

    // retrieve the directory path of the filepath
    directory = fullPath.substr(0, fullPath.find_last_of('/'));
    filename = fullPath.substr(fullPath.find_last_of('/') + 1, fullPath.length());
    animationnum = scene->mNumAnimations;
    isanimated = animationnum > 0;

    // process ASSIMP's root node recursively
    processNode(scene->mRootNode, scene);
    initialboundingbox = getBoundingBox();
    if (isanimated) setAnimationFramerate(animationframerate);
    animate(0);
}

void gModel::loadMorphingTargetModel(const std::string& modelPath) {
	loadMorphingTargetModelFile(gGetModelsDir() + modelPath);
}

void gModel::loadMorphingTargetModelFile(const std::string& fullPath) {
	int targetid = morphingtargetscenes.size();
#ifdef LINUX
	std::shared_ptr<aiPropertyStore> store;
    store.reset(aiCreatePropertyStore(), aiReleasePropertyStore);
    // only ever give us triangles.
    aiSetImportPropertyInteger(store.get(), AI_CONFIG_PP_SBP_REMOVE, aiPrimitiveType_LINE | aiPrimitiveType_POINT );
    aiSetImportPropertyInteger(store.get(), AI_CONFIG_PP_PTV_NORMALIZE, true);

    morphingtargetscenes.push_back(aiImportFileExWithProperties(fullPath.c_str(), aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace | aiProcess_GenNormals | aiProcess_GenUVCoords | aiProcess_SplitLargeMeshes | aiProcess_SortByPType, NULL, store.get()));
    // check for errors
    if(!morphingtargetscenes[targetid] || (morphingtargetscenes[targetid]->mFlags & AI_SCENE_FLAGS_INCOMPLETE) || !morphingtargetscenes[targetid]->mRootNode) { // if is Not Zero
    	std::cout << "ERROR::ASSIMP:: " << aiGetErrorString() << std::endl;
        return;
    }
#else
    Assimp::Importer importer;
    importer.SetPropertyInteger(AI_CONFIG_PP_SBP_REMOVE, aiPrimitiveType_LINE | aiPrimitiveType_POINT);
    importer.SetPropertyBool(AI_CONFIG_PP_PTV_NORMALIZE, true);
    importer.ReadFile(fullPath, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace | aiProcess_GenNormals | aiProcess_GenUVCoords | aiProcess_SplitLargeMeshes | aiProcess_SortByPType);
    morphingtargetscenes.push_back(importer.GetOrphanedScene());
    // check for errors
    //aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace | aiProcess_GenNormals | aiProcess_GenUVCoords | aiProcess_SplitLargeMeshes | aiProcess_SortByPType
    if(!morphingtargetscenes[targetid] || (morphingtargetscenes[targetid]->mFlags & AI_SCENE_FLAGS_INCOMPLETE) || !morphingtargetscenes[targetid]->mRootNode) { // if is Not Zero
        std::cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
        return;
    }
#endif

    // retrieve the directory path of the filepath
    directory = fullPath.substr(0, fullPath.find_last_of('/'));
    filename = fullPath.substr(fullPath.find_last_of('/') + 1, fullPath.length());
    processMorphingNode(morphingtargetscenes[targetid]->mRootNode, morphingtargetscenes[targetid]);
}

void gModel::move(float dx, float dy, float dz) {
	gNode::move(dx, dy, dz);
	for(unsigned int i = 0; i < meshes.size(); i++) meshes[i].move(dx, dy, dz);
}

void gModel::move(const glm::vec3& dv) {
	gNode::move(dv);
	for(unsigned int i = 0; i < meshes.size(); i++) meshes[i].move(dv);
}

void gModel::rotate(const glm::quat& q) {
	gNode::rotate(q);
	for(unsigned int i = 0; i < meshes.size(); i++) meshes[i].rotate(q);
}

void gModel::rotate(float radians, float ax, float ay, float az) {
	gNode::rotate(radians, ax, ay, az);
	for(unsigned int i = 0; i < meshes.size(); i++) meshes[i].rotate(radians, ax, ay, az);
}

void gModel::rotateDeg(float degrees, float ax, float ay, float az) {
	gNode::rotateDeg(degrees, ax, ay, az);
	for(unsigned int i = 0; i < meshes.size(); i++) meshes[i].rotateDeg(degrees, ax, ay, az);
}

void gModel::rotateAround(float radians, const glm::vec3& axis, const glm::vec3& point) {
	gNode::rotateAround(radians, axis, point);
	for(unsigned int i = 0; i < meshes.size(); i++) meshes[i].rotateAround(radians, axis, point);
}

void gModel::rotateAroundDeg(float degrees, const glm::vec3& axis, const glm::vec3& point) {
	gNode::rotateAroundDeg(degrees, axis, point);
	for(unsigned int i = 0; i < meshes.size(); i++) meshes[i].rotateAroundDeg(degrees, axis, point);
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

void gModel::setPosition(const glm::vec3& p) {
	gNode::setPosition(p);
	for(unsigned int i = 0; i < meshes.size(); i++) meshes[i].setPosition(p);
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

void gModel::tiltDeg(float degrees) {
	gNode::tiltDeg(degrees);
	for(unsigned int i = 0; i < meshes.size(); i++) meshes[i].tiltDeg(degrees);
}

void gModel::pan(float radians) {
	gNode::pan(radians);
	for(unsigned int i = 0; i < meshes.size(); i++) meshes[i].pan(radians);
}

void gModel::panDeg(float degrees) {
	gNode::panDeg(degrees);
	for(unsigned int i = 0; i < meshes.size(); i++) meshes[i].panDeg(degrees);
}

void gModel::roll(float radians) {
	gNode::roll(radians);
	for(unsigned int i = 0; i < meshes.size(); i++) meshes[i].roll(radians);
}

void gModel::rollDeg(float degrees) {
	gNode::rollDeg(degrees);
	for(unsigned int i = 0; i < meshes.size(); i++) meshes[i].rollDeg(degrees);
}

void gModel::setTransformationMatrix(const glm::mat4& transformationMatrix) {
	gNode::setTransformationMatrix(transformationMatrix);
	for(unsigned int i = 0; i < meshes.size(); i++) meshes[i].setTransformationMatrix(transformationMatrix);
}

void gModel::draw() {
	for(dri = 0; dri < meshes.size(); dri++) {
		meshes[dri].draw();
	}
}

const std::string& gModel::getFilename() const {
	return filename;
}

const std::string gModel::getFullpath() const {
	return directory + "/" + filename;
}

int gModel::getMeshNum() const {
	return meshes.size();
}

int gModel::getMeshNo(const std::string& meshName) const {
	for(unsigned int i = 0; i < meshes.size(); i++) {
		if (meshName == scene->mMeshes[i]->mName.C_Str()) return i;
	}
	return -1;
}

gSkinnedMesh& gModel::getMesh(int meshNo) {
	return meshes[meshNo];
}

gSkinnedMesh* gModel::getMeshPtr(int meshNo) {
	return &meshes[meshNo];
}

const std::string gModel::getMeshName(int meshNo) const {
	return scene->mMeshes[meshNo]->mName.C_Str();
}

void gModel::processNode(aiNode *node, const aiScene *scene) {
	static int x = 0;
	// process each mesh located at the current node
	for(unsigned int i = 0; i < node->mNumMeshes; i++) {
		// the node object only contains indices to index the actual objects in the scene.
		// the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		gLogi("gModel") << "Loading mesh:" << mesh->mName.C_Str() << ", tm:" << node->mTransformation[0];
		gSkinnedMesh modelmesh = processMesh(mesh, scene, node->mTransformation);
//		if (isanimated) updateBones(&modelmesh, mesh, scene);
//		modelmesh.setParent(this);
		meshes.push_back(modelmesh);
//		gLogi("setting");
		meshes[meshes.size() - 1].setBaseMesh(&meshes[meshes.size() - 1]);
//		gLogi("setted");
	}

	// after we've processed all of the meshes (if any) we then recursively process each of the children nodes
	for(unsigned int i = 0; i < node->mNumChildren; i++) {
		processNode(node->mChildren[i], scene);
	}

}

gSkinnedMesh gModel::processMesh(aiMesh *mesh, const aiScene *scene, aiMatrix4x4 matrix) {
    // data to fill
    std::vector<gVertex> vertices;
    std::vector<gIndex> indices;
    glm::mat4 mat = convertMatrix(matrix);

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
    gmesh.setName(mesh->mName.C_Str());
    gmesh.setVertices(vertices, indices);
//    gmesh.setTransformationMatrix(convertMatrix(matrix));

    loadMaterialTextures(&gmesh, material, aiTextureType_DIFFUSE, gTexture::TEXTURETYPE_DIFFUSE);
    loadMaterialTextures(&gmesh, material, aiTextureType_SPECULAR, gTexture::TEXTURETYPE_SPECULAR);
    loadMaterialTextures(&gmesh, material, aiTextureType_NORMALS, gTexture::TEXTURETYPE_NORMAL);
    loadMaterialTextures(&gmesh, material, aiTextureType_HEIGHT, gTexture::TEXTURETYPE_HEIGHT);
    loadMaterialTextures(&gmesh, material, aiTextureType_BASE_COLOR, gTexture::TEXTURETYPE_PBR_ALBEDO); // @suppress("Invalid arguments")
    loadMaterialTextures(&gmesh, material, aiTextureType_DIFFUSE_ROUGHNESS, gTexture::TEXTURETYPE_PBR_ROUGHNESS);
    loadMaterialTextures(&gmesh, material, aiTextureType_METALNESS, gTexture::TEXTURETYPE_PBR_METALNESS);
    loadMaterialTextures(&gmesh, material, aiTextureType_NORMAL_CAMERA, gTexture::TEXTURETYPE_PBR_NORMAL);
    loadMaterialTextures(&gmesh, material, aiTextureType_AMBIENT_OCCLUSION, gTexture::TEXTURETYPE_PBR_AO);

    return gmesh;
}

void gModel::loadMaterialTextures(gSkinnedMesh* mesh, aiMaterial *mat, aiTextureType type, int textureType) {
    for(unsigned int i = 0; i < mat->GetTextureCount(type); i++) {
    	int texno = -1;
        aiString str;
        mat->GetTexture(type, i, &str);
        // check if texture was loaded before and if so, continue to next iteration: skip loading a new texture
        bool skip = false;
        for(unsigned int j = 0; j < textures_loaded.size(); j++) {
        	std::string aip = str.C_Str();
        	int aipspos = aip.find_last_of('/');
        	aip = aip.substr(aipspos + 1, aip.length() - aipspos - 1);
            if(aip == textures_loaded[j]->getFilename()) {
                skip = true; // a texture with the same filepath has already been loaded, continue to next one. (optimization)
                texno = j;
                break;
            }
        }

        if(!skip) {   // if texture hasn't been loaded already, load it
            gTexture* texture = new gTexture();
            std::string tpath = this->directory + "/" + str.C_Str();
            texture->load(tpath);
            texture->setType(textureType);
            textures_loaded.push_back(texture);  // store it as texture loaded for entire model, to ensure we won't unnecesery load duplicate textures.
            texno = textures_loaded.size() - 1;
        }

        mesh->setTexture(textures_loaded[texno]);
    }
}

void gModel::processMorphingNode(aiNode *node, const aiScene *scene) {
	// process each mesh located at the current node
	for(unsigned int i = 0; i < node->mNumMeshes; i++) {
		// the node object only contains indices to index the actual objects in the scene.
		// the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		gLogi("gModel") << "Loading morphing mesh:" << mesh->mName.C_Str() << ", tm:" << node->mTransformation[0];
		gMesh targetmodelmesh = processMorphingMesh(mesh, scene, node->mTransformation);
//		if (isanimated) updateBones(&modelmesh, mesh, scene);
//		modelmesh.setParent(this);
		auto found = std::find_if(meshes.begin(), meshes.end(),
				[&] (const gSkinnedMesh& mesh) {
					return mesh.getName() == targetmodelmesh.getName();
				});
		if (found == meshes.end()) gLoge("gModel") << "Cannot find appropriate mesh to add the morphing target mesh.";
		found->addTargetMesh(&targetmodelmesh);
	}

	// after we've processed all of the meshes (if any) we then recursively process each of the children nodes
	for(unsigned int i = 0; i < node->mNumChildren; i++) {
		processMorphingNode(node->mChildren[i], scene);
	}
}

gMesh gModel::processMorphingMesh(aiMesh *mesh, const aiScene *scene, aiMatrix4x4 matrix) {
	 std::vector<gVertex> vertices;
	glm::mat4 mat = convertMatrix(matrix);

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
		vertices.push_back(vertex);
	}

	gMesh gmesh;
	gmesh.setName(mesh->mName.C_Str());
	gmesh.setVertices(vertices);

	return gmesh;
}

bool gModel::isAnimated() const {
	return isanimated;
}

int gModel::getAnimationNum() const {
	return animationnum;
}

/**
 * Set animation position to given position. Position should be a normalized value;
 */
void gModel::animate(float animationPosition) {
	if(!isanimated) return;

	animationpositionold = animationposition;
	animationposition = animationPosition;

	//The comment below this line is replaced with the !isvertexanimationstoredonvram check in the if staement, to perform the necessary operations if the mesh is animated morphing mesh.
	//(!isvertexanimationstoredonvram || (meshes.size() != 0 && std::find_if(meshes.begin(), meshes.end(), [] (const gSkinnedMesh& mesh) {return mesh.getTargetMeshCount() > 0;}) != meshes.end()))
	if(!isvertexanimationstoredonvram && animationposition != animationpositionold) {
		updateAnimationNodes();
		for (int i=0; i<meshes.size(); i++) {
			//The if which is commented is a check to prevent unnecessary operation on a non-morphing mesh which is stored on vram. That if commented because it hasn't tested yet.
//			if (isvertexanimationstoredonvram && meshes[i].getTargetMeshCount() == 0) return;
			//Below comment line is the third parameter of updateBones which is to perform animating operation on the target mesh of a mesh by taking the scene mesh as a reference if the animated mesh has a target mesh. Haven't tested yet.
			// (meshes[i].getTargetMeshCount() > 0) ? (morphingtargetscenes[meshes[i].getCurrentTargetMeshId()]->mMeshes[i]) : (nullptr)
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
	//Below line is to reset all the data belong to the target mesh. Haven't tested yet.
	//if (aiTargetMesh != nullptr) gmesh->resetTargetData(gmesh->getCurrentTargetMeshId());

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

			//The commented lines below are to perform an animation to the morphing target's vertices' position. They haven't tested yet.
			/*if (aiTargetMesh != nullptr) {
				const aiVector3D& srcPos = aiTargetMesh->mVertices[vertexId];
				oldweightpos = gmesh->getTargetPosition(gmesh->getCurrentTargetMeshId(), vertexId);
				aiaddweight = weight.mWeight * (posTrafo * srcPos);
				glm::vec3 vPos(oldweightpos.x + aiaddweight.x, oldweightpos.y + aiaddweight.y, oldweightpos.z + aiaddweight.z);
				gmesh->setTargetPosition(gmesh->getCurrentTargetMeshId(), vertexId, vPos);
			}*/
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

				//The commented lines below are to perform an animation to the morphing target's vertices' normal. They haven't tested yet.
				/*if (aiTargetMesh != nullptr) {
					const aiVector3D& srcNorm = aiTargetMesh->mNormals[vertexId];
					oldweightnorm = gmesh->getTargetNormal(gmesh->getCurrentTargetMeshId(), vertexId);
					aiaddweight = weight.mWeight * (posTrafo * srcNorm);
					gmesh->setTargetNormal(gmesh->getCurrentTargetMeshId(), vertexId, glm::vec3(oldweightnorm.x + aiaddweight.x, oldweightnorm.y + aiaddweight.y, oldweightnorm.z + aiaddweight.z));
				}*/
			}
		}
	}
}

void gModel::updateVbo(gSkinnedMesh* gmesh) {
	std::vector<gVertex> vertexarray = gmesh->getVertices();
	std::vector<gIndex> indexarray = gmesh->getIndices();
	for (int i=0; i<gmesh->getVbo()->getVerticesNum(); i++) {
		vertexarray[i].position = gmesh->getVertexPos(i);
		vertexarray[i].normal = gmesh->getVertexNorm(i);
	}
	gmesh->setVertices(vertexarray, indexarray);
}

/**
 * Returns duration in seconds.
 */
float gModel::getAnimationDuration(int animationNo) const {
	return scene->mAnimations[animationNo]->mDuration;
}

float gModel::getAnimationPosition() const {
	return animationposition;
}

void gModel::setAnimationFramerate(float animationFramerate) {
	animationframerate = animationFramerate;
#ifdef WIN32
	setAnimationFrameNum(getAnimationDuration() * animationframerate / scene->mAnimations[0]->mTicksPerSecond);
#else
	setAnimationFrameNum(getAnimationDuration() * animationframerate);
#endif
}

float gModel::getAnimationFramerate() const {
	return animationframerate;
}

void gModel::setAnimationFrameNum(int animationKeyNum) {
	if (animationKeyNum == animationframenum) return;
	animationframenum = animationKeyNum;
	generateAnimationKeys();
	animationframeno = 0;
}

int gModel::getAnimationFrameNum() const {
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

int gModel::getAnimationFrameNo() const {
	return animationframeno;
}

void gModel::setMorphingFrameNo(int morphingFrameNo) {
	for (int i = 0; i < meshes.size(); i++) {
		meshes[i].setCurrentFrameId(morphingFrameNo);
	}
}

int gModel::getMorphingFrameNo() const {
	return meshes[0].getCurrentFrameId();
}

void gModel::nextMorphingFrame() {
	for (int i = 0; i < meshes.size(); i++) {
		meshes[i].nextFrameId();
	}
}

void gModel::setMorphingSpeed(int speed) {
	for (int i = 0; i < meshes.size(); i++) {
		meshes[i].setSpeed(speed);
	}
}

int gModel::getMorphingSpeed() const {
	return meshes[0].getSpeed();
}

void gModel::setMorphingTarget(int morphingTargetId) {
	for (int i = 0; i < meshes.size(); i++) {
		meshes[i].setCurrentTargetMeshId(morphingTargetId);
	}
}

int gModel::getMorphingTarget() const {
	return meshes[0].getCurrentTargetMeshId();
}

void gModel::setMorphingFrameNum(int morphingFrameNum) {
	for (int i = 0; i < meshes.size(); i++) {
		meshes[i].setFrameCount(meshes[i].getCurrentTargetMeshId(), morphingFrameNum);
	}
}

int gModel::getMorphingFrameNum() const {
	return meshes[0].getFrameCount();
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
                //The below line's third parameter is given as a nullptr since the meshes aren't morphing. Haven't tested yet.
                //updateBones(&meshes[i], scene->mMeshes[i], nullptr);
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
                	std::vector<gIndex> indexarray = meshes[i].getIndices();
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

bool gModel::isVertexAnimated() const {
	return isvertexanimated;
}

bool gModel::isVertexAnimationStoredOnVram() const {
	return isvertexanimationstoredonvram;
}

gBoundingBox gModel::getBoundingBox() {
	for (bbi = 0; bbi< meshes.size(); bbi++) {
		bbvertices = meshes[bbi].getVertices();
		for (bbj = 0; bbj < bbvertices.size(); bbj++) {
			bbv = bbvertices[bbj];
			bbvpos = glm::vec3(localtransformationmatrix * glm::vec4(bbv.position, 1.0));

			if (bbi == 0 && bbj == 0) {
				bbminx = bbvpos.x, bbminy = bbvpos.y, bbminz = bbvpos.z;
				bbmaxx = bbvpos.x, bbmaxy = bbvpos.y, bbmaxz = bbvpos.z;
				continue;
			}

			bbminx = std::min(bbminx, bbvpos.x);
			bbminy = std::min(bbminy, bbvpos.y);
			bbminz = std::min(bbminz, bbvpos.z);
			bbmaxx = std::max(bbmaxx, bbvpos.x);
			bbmaxy = std::max(bbmaxy, bbvpos.y);
			bbmaxz = std::max(bbmaxz, bbvpos.z);
		}
	}

	return gBoundingBox(bbminx, bbminy, bbminz, bbmaxx, bbmaxy, bbmaxz, localtransformationmatrix);
}

glm::mat4 gModel::convertMatrix(const aiMatrix4x4 &aiMat) {
	return {
	aiMat.a1, aiMat.b1, aiMat.c1, aiMat.d1,
	aiMat.a2, aiMat.b2, aiMat.c2, aiMat.d2,
	aiMat.a3, aiMat.b3, aiMat.c3, aiMat.d3,
	aiMat.a4, aiMat.b4, aiMat.c4, aiMat.d4
	};
}

gBoundingBox& gModel::getInitialBoundingBox() {
	initialboundingbox.setTransformationMatrix(localtransformationmatrix);
	return initialboundingbox;
}
