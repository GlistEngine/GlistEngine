/*
 * gModel.cpp
 *
 *  Created on: May 10, 2020
 *      Author: noyan
 */

#include "gModel.h"
#include <memory>
#if defined(__i386__) || defined(__x86_64__)
#include <immintrin.h>
#elif defined(__ARM_NEON)
#include <arm_neon.h>
#endif
#include "gCamera.h"
#include "gTracy.h"

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
	isenablefrustumculling = true;
	needsboundingboxrecalculation = false;
}


gModel::~gModel() {
	for (const auto& item : textures_loaded) {
		delete item;
	}
}

void gModel::disassemble() {
	for (auto& mesh : meshes) {
		// 1. Get the world transform of this mesh
		glm::mat4 worldTransform = getTransformationMatrix();

		// 2. Transform all vertices into world space
		std::shared_ptr<std::vector<gVertex>> verticesptr = mesh->getVerticesPtr();
		std::vector<gVertex>& vertices = *verticesptr;
		glm::vec3 centroid(0.0f);

		for (auto& v : vertices) {
			glm::vec4 wp = worldTransform * glm::vec4(v.position, 1.0f);
			v.position = glm::vec3(wp);
			centroid += v.position;
		}

		// 3. Compute the centroid
		centroid /= (float)vertices.size();

		// 4. Rebase vertices so centroid is origin in mesh-local space
		for (auto& v : vertices) {
			v.position -= centroid;
		}

		// 5. Store updated vertices back
		mesh->setVertices(verticesptr, mesh->getIndicesPtr());

		// 6. Reset mesh transform: now its local origin is at centroid
		mesh->setPosition(centroid);

		// 7. Clear mesh scale/rotation if you want them baked into vertex positions
		mesh->setScale(1.0f);
		//mesh->setRotation(0, 0, 0);
	}

	setScale(1.0f);
	//setOrientation(glm::vec3(0, 0, 0));
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

	// Identify which meshes use node animations (no bones)
	meshHasNodeAnimation.resize(meshes.size(), false);
	for(unsigned int i = 0; i < scene->mNumMeshes; i++) {
		meshHasNodeAnimation[i] = (scene->mMeshes[i]->mNumBones == 0);
	}

	recalculateBoundingBox();
    initialboundingbox = boundingbox;
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
	recalculateBoundingBox();
	initialboundingbox = boundingbox;
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

void gModel::move(const glm::vec3& dv) {
	gNode::move(dv);
	for(unsigned int i = 0; i < meshes.size(); i++) meshes[i]->move(dv);
}

void gModel::rotate(const glm::quat& q) {
	gNode::rotate(q);
	for(unsigned int i = 0; i < meshes.size(); i++) {
		if(!meshHasNodeAnimation.empty() && i < meshHasNodeAnimation.size() && meshHasNodeAnimation[i]) {
			continue;
		}
		meshes[i]->rotate(q);
	}
}

void gModel::rotate(float radians, float ax, float ay, float az) {
	gNode::rotate(radians, ax, ay, az);
	for(unsigned int i = 0; i < meshes.size(); i++) {
		if(!meshHasNodeAnimation.empty() && i < meshHasNodeAnimation.size() && meshHasNodeAnimation[i]) {
			continue;
		}
		meshes[i]->rotate(radians, ax, ay, az);
	}
}

void gModel::rotateAround(float radians, const glm::vec3& axis, const glm::vec3& point) {
	gNode::rotateAround(radians, axis, point);
	for(unsigned int i = 0; i < meshes.size(); i++) meshes[i]->rotateAround(radians, axis, point);
}

void gModel::scale(float sx, float sy, float sz) {
	gNode::scale(sx, sy, sz);
	for(unsigned int i = 0; i < meshes.size(); i++) meshes[i]->scale(sx, sy, sz);
}

void gModel::scale(float s) {
	gNode::scale(s);
	for(unsigned int i = 0; i < meshes.size(); i++) meshes[i]->scale(s);
}

void gModel::setPosition(const glm::vec3& p) {
	gNode::setPosition(p);
	for(unsigned int i = 0; i < meshes.size(); i++) meshes[i]->setPosition(p);
}

void gModel::setOrientation(const glm::quat& o) {
	gNode::setOrientation(o);
	for(unsigned int i = 0; i < meshes.size(); i++) meshes[i]->setOrientation(o);
}

void gModel::setOrientation(const glm::vec3& angles) {
	gNode::setOrientation(angles);
	for(unsigned int i = 0; i < meshes.size(); i++) meshes[i]->setOrientation(angles);
}

void gModel::setScale(const glm::vec3& s) {
	gNode::setScale(s);
	for(unsigned int i = 0; i < meshes.size(); i++) meshes[i]->setScale(s);
}

void gModel::dolly(float distance) {
	gNode::dolly(distance);
	for(unsigned int i = 0; i < meshes.size(); i++) meshes[i]->dolly(distance);
}

void gModel::truck(float distance) {
	gNode::truck(distance);
	for(unsigned int i = 0; i < meshes.size(); i++) meshes[i]->truck(distance);
}

void gModel::boom(float distance) {
	gNode::boom(distance);
	for(unsigned int i = 0; i < meshes.size(); i++) meshes[i]->boom(distance);
}

void gModel::tilt(float radians) {
	gNode::tilt(radians);
	for(unsigned int i = 0; i < meshes.size(); i++) {
		if(!meshHasNodeAnimation.empty() && i < meshHasNodeAnimation.size() && meshHasNodeAnimation[i]) {
			continue;
		}
		meshes[i]->tilt(radians);
	}
}

void gModel::pan(float radians) {
	gNode::pan(radians);
	for(unsigned int i = 0; i < meshes.size(); i++) {
		if(!meshHasNodeAnimation.empty() && i < meshHasNodeAnimation.size() && meshHasNodeAnimation[i]) {
			continue;
		}
		meshes[i]->pan(radians);
	}
}

void gModel::roll(float radians) {
	gNode::roll(radians);
	for(unsigned int i = 0; i < meshes.size(); i++) {
		if(!meshHasNodeAnimation.empty() && i < meshHasNodeAnimation.size() && meshHasNodeAnimation[i]) {
			continue;
		}
		meshes[i]->roll(radians);
	}
}

void gModel::setTransformationMatrix(const glm::mat4& transformationMatrix) {
	gNode::setTransformationMatrix(transformationMatrix);
	for(unsigned int i = 0; i < meshes.size(); i++) {
		if(!meshHasNodeAnimation.empty() && i < meshHasNodeAnimation.size() && meshHasNodeAnimation[i]) {
			continue;
		}
		meshes[i]->setTransformationMatrix(transformationMatrix);
	}
}

void gModel::draw() {
	G_PROFILE_ZONE_SCOPED_N("gModel::draw()");
	for(int i = 0; i < meshes.size(); i++) {
		if (isenablefrustumculling && renderer->getCamera() &&
			!renderer->getCamera()->isInFrustum(meshes[i]->getBoundingBox())) {
			continue;
		}
		meshes[i]->draw();
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
	return *meshes[meshNo];
}

gSkinnedMesh* gModel::getMeshPtr(int meshNo) {
	return meshes[meshNo];
}

const std::string gModel::getMeshName(int meshNo) const {
	return scene->mMeshes[meshNo]->mName.C_Str();
}

void gModel::processNode(aiNode *node, const aiScene *scene) {
	// process each mesh located at the current node
	for(unsigned int i = 0; i < node->mNumMeshes; i++) {
		// the node object only contains indices to index the actual objects in the scene.
		// the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		gLogi("gModel") << "Loading mesh:" << mesh->mName.C_Str() << ", vertexnum:" << mesh->mNumVertices << ", tm:" << node->mTransformation[0];
		gSkinnedMesh* modelmesh = processMesh(mesh, scene, node->mTransformation);
//		if (isanimated) updateBones(&modelmesh, mesh, scene);
//		modelmesh.setParent(this);
		meshes.push_back(modelmesh);
//		gLogi("setting");
		meshes[meshes.size() - 1]->setBaseMesh(meshes[meshes.size() - 1]);
//		gLogi("setted");
	}
	// we cache nodes because FindNodes is costly to run
	// hashing is faster
	nodemap[node->mName.C_Str()] = node;
	// after we've processed all of the meshes (if any) we then recursively process each of the children nodes
	for(unsigned int i = 0; i < node->mNumChildren; i++) {
		processNode(node->mChildren[i], scene);
	}
}

gSkinnedMesh* gModel::processMesh(aiMesh *mesh, const aiScene *scene, aiMatrix4x4 matrix) {
	if(mesh2svimap.find(mesh) == mesh2svimap.end()) {
		auto verticesptr = std::make_shared<std::vector<gVertex>>();
		auto indicesptr = std::make_shared<std::vector<gIndex>>();
		mesh2svimap[mesh] = {verticesptr, indicesptr};
		std::vector<gVertex>& vertices = *mesh2svimap[mesh].vertices;
	    std::vector<gIndex>& indices = *mesh2svimap[mesh].indices;
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
	}
    //glm::mat4 mat = convertMatrix(matrix);

    // process materials
    aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
    // we assume a convention for sampler names in the shaders. Each diffuse texture should be named
    // as 'texture_diffuseN' where N is a sequential number ranging from 1 to MAX_SAMPLER_NUMBER.
    // Same applies to other texture as the following list summarizes:
    // diffuse: texture_diffuseN
    // specular: texture_specularN
    // normal: texture_normalN

    gSkinnedMesh* gmesh = new gSkinnedMesh();
    gmesh->setName(mesh->mName.C_Str());
	SharedVertexIndex& svi = mesh2svimap[mesh];
	gmesh->setVertices(svi.vertices, svi.indices);
	gmesh->setTransformationMatrix(convertMatrix(matrix));
	loadMaterialTextures(gmesh, material, aiTextureType_DIFFUSE, gTexture::TEXTURETYPE_DIFFUSE);
    loadMaterialTextures(gmesh, material, aiTextureType_SPECULAR, gTexture::TEXTURETYPE_SPECULAR);
    loadMaterialTextures(gmesh, material, aiTextureType_NORMALS, gTexture::TEXTURETYPE_NORMAL);
    loadMaterialTextures(gmesh, material, aiTextureType_HEIGHT, gTexture::TEXTURETYPE_HEIGHT);
    loadMaterialTextures(gmesh, material, aiTextureType_BASE_COLOR, gTexture::TEXTURETYPE_PBR_ALBEDO); // @suppress("Invalid arguments")
    loadMaterialTextures(gmesh, material, aiTextureType_DIFFUSE_ROUGHNESS, gTexture::TEXTURETYPE_PBR_ROUGHNESS);
    loadMaterialTextures(gmesh, material, aiTextureType_METALNESS, gTexture::TEXTURETYPE_PBR_METALNESS);
    loadMaterialTextures(gmesh, material, aiTextureType_NORMAL_CAMERA, gTexture::TEXTURETYPE_PBR_NORMAL);
    loadMaterialTextures(gmesh, material, aiTextureType_AMBIENT_OCCLUSION, gTexture::TEXTURETYPE_PBR_AO);
    return gmesh;
}

void gModel::loadMaterialTextures(gSkinnedMesh* mesh, aiMaterial *mat, aiTextureType type, gTexture::TextureType textureType) {
    aiString str;
    for(unsigned int i = 0; i < mat->GetTextureCount(type); i++) {
        size_t texno;
        mat->GetTexture(type, i, &str);
        // check if texture was loaded before and if so, continue to next iteration: skip loading a new texture
        bool skip = false;
        for(size_t j = 0; j < textures_loaded.size(); j++) {
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
            std::string texturepath = gFixPath(this->directory + "/" + str.C_Str());
            texture->load(texturepath);
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
		gMesh* targetmodelmesh = processMorphingMesh(mesh, scene, node->mTransformation);
//		if (isanimated) updateBones(&modelmesh, mesh, scene);
//		modelmesh.setParent(this);
		auto found = std::find_if(meshes.begin(), meshes.end(),
				[&] (const gSkinnedMesh* mesh) {
					return mesh->getName() == targetmodelmesh->getName();
				});
		if (found == meshes.end()) gLoge("gModel") << "Cannot find appropriate mesh to add the morphing target mesh.";
		(*found)->addTargetMesh(targetmodelmesh);
	}

	// after we've processed all of the meshes (if any) we then recursively process each of the children nodes
	for(unsigned int i = 0; i < node->mNumChildren; i++) {
		processMorphingNode(node->mChildren[i], scene);
	}
}

gMesh* gModel::processMorphingMesh(aiMesh *mesh, const aiScene *scene, aiMatrix4x4 matrix) {
	auto vertices = std::make_shared<std::vector<gVertex>>();
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
		vertices->push_back(vertex);
	}

	gMesh* gmesh = new gMesh();
	gmesh->setName(mesh->mName.C_Str());
	gmesh->setVertices(vertices);

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
	G_PROFILE_ZONE_SCOPED_N("gModel::animate()");
	if(!isanimated) return;

	animationpositionold = animationposition;
	animationposition = animationPosition;

	//The comment below this line is replaced with the !isvertexanimationstoredonvram check in the if staement, to perform the necessary operations if the mesh is animated morphing mesh.
	//(!isvertexanimationstoredonvram || (meshes.size() != 0 && std::find_if(meshes.begin(), meshes.end(), [] (const gSkinnedMesh& mesh) {return mesh.getTargetMeshCount() > 0;}) != meshes.end()))
	if(!isvertexanimationstoredonvram && animationposition != animationpositionold) {
		updateAnimationNodes();
		for (int i = 0; i < meshes.size(); i++) {
			//The if which is commented is a check to prevent unnecessary operation on a non-morphing mesh which is stored on vram. That if commented because it hasn't tested yet.
//			if (isvertexanimationstoredonvram && meshes[i]->getTargetMeshCount() == 0) return;
			//Below comment line is the third parameter of updateBones which is to perform animating operation on the target mesh of a mesh by taking the scene mesh as a reference if the animated mesh has a target mesh. Haven't tested yet.
			// (meshes[i]->getTargetMeshCount() > 0) ? (morphingtargetscenes[meshes[i]->getCurrentTargetMeshId()]->mMeshes[i]) : (nullptr)
			if(scene->mMeshes[i]->mNumBones <= 0) {
				aiNode* meshNode = findNodeFast(scene->mMeshes[i]->mName.C_Str());
				if(meshNode) {
					aiMatrix4x4 nodeTransform = aiMatrix4x4();
					for(aiNode* tempNode = meshNode; tempNode != nullptr; tempNode = tempNode->mParent) {
						nodeTransform = tempNode->mTransformation * nodeTransform;
					}
					glm::mat4 finalTransform = localtransformationmatrix * convertMatrix(nodeTransform);
					meshes[i]->setTransformationMatrix(finalTransform);
				}
				continue;
			}
			updateBones(meshes[i], scene->mMeshes[i]);
			updateVbo(meshes[i]);
		}
	}
}

void gModel::updateAnimationNodes() {
	G_PROFILE_ZONE_SCOPED_N("gModel::updateAnimationNodes()");
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
			aiNode* targetNode = findNodeFast(channel->mNodeName.C_Str());
			mat.a1 *= presentScaling.x; mat.b1 *= presentScaling.x; mat.c1 *= presentScaling.x;
			mat.a2 *= presentScaling.y; mat.b2 *= presentScaling.y; mat.c2 *= presentScaling.y;
			mat.a3 *= presentScaling.z; mat.b3 *= presentScaling.z; mat.c3 *= presentScaling.z;
			mat.a4 = presentPosition.x; mat.b4 = presentPosition.y; mat.c4 = presentPosition.z;

			targetNode->mTransformation = mat;
		}
	}
}

void gModel::updateBones(gSkinnedMesh* gmesh, aiMesh* aimesh) {
    G_PROFILE_ZONE_SCOPED_N("gModel::updateBones()");
    gmesh->resizeAnimation(aimesh->mNumVertices);

    if (aimesh->mNumBones == 0) {
        return;
    }

    std::vector<aiMatrix4x4> boneMatrices;
    boneMatrices.reserve(aimesh->mNumBones);

    for(unsigned int a = 0; a < aimesh->mNumBones; a++) {
       const aiBone* bone = aimesh->mBones[a];
       aiNode* node = findNodeFast(bone->mName.C_Str());

       // Start with the mesh-to-bone matrix
       aiMatrix4x4 boneMatrix = bone->mOffsetMatrix;

       // Traverse parent chain once and accumulate transformations
       for(aiNode* tempNode = node; tempNode != nullptr; tempNode = tempNode->mParent) {
          boneMatrix = tempNode->mTransformation * boneMatrix;
       }

       boneMatrices.emplace_back(boneMatrix);
    }

    gmesh->resetAnimation();

    // Cache hasNormals check
    const bool hasNormals = aimesh->HasNormals();

    // Loop through all vertex weights of all bones
    for(unsigned int a = 0; a < aimesh->mNumBones; ++a) {
       const aiBone* bone = aimesh->mBones[a];
       const aiMatrix4x4& posTrafo = boneMatrices[a];

       for(unsigned int b = 0; b < bone->mNumWeights; ++b) {
          const aiVertexWeight& weight = bone->mWeights[b];
          const size_t vertexId = weight.mVertexId;
          const float weightValue = weight.mWeight;

          // Transform position
          const aiVector3D transformedPos = posTrafo * aimesh->mVertices[vertexId];
          const glm::vec3 oldPos = gmesh->getVertexPos(vertexId);
          gmesh->setVertexPos(vertexId, {
              oldPos.x + weightValue * transformedPos.x,
              oldPos.y + weightValue * transformedPos.y,
              oldPos.z + weightValue * transformedPos.z
          });

          // Transform normals if available
          if(hasNormals) {
             const aiVector3D transformedNorm = posTrafo * aimesh->mNormals[vertexId];
             const glm::vec3 oldNorm = gmesh->getVertexNorm(vertexId);
             gmesh->setVertexNorm(vertexId, {
                 oldNorm.x + weightValue * transformedNorm.x,
                 oldNorm.y + weightValue * transformedNorm.y,
                 oldNorm.z + weightValue * transformedNorm.z
             });
          }
       }
    }
}

void gModel::updateVbo(gSkinnedMesh* gmesh) {
	G_PROFILE_ZONE_SCOPED_N("gModel::updateVbo()");

	std::vector<gVertex>& vertexarray = gmesh->getVertices();
	const std::vector<glm::vec3>& animatedpos = gmesh->getAnimatedPos();
	const std::vector<glm::vec3>& animatednorm = gmesh->getAnimatedNorm();
	const int verticesnum = gmesh->getVerticesNum();

	for (int i = 0; i < verticesnum; i++) {
		vertexarray[i].position = animatedpos[i];
		vertexarray[i].normal = animatednorm[i];
	}
	gmesh->setVertices(vertexarray);
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
	G_PROFILE_ZONE_SCOPED_N("gModel::setAnimationFramerate()");
	animationframerate = animationFramerate;

	float ticksPerSecond = (scene->mAnimations[0]->mTicksPerSecond != 0)
								   ? scene->mAnimations[0]->mTicksPerSecond
								   : 25.0f;

	float durationTicks = getAnimationDuration(); // returns mDuration
	float durationSeconds = durationTicks / ticksPerSecond;

	setAnimationFrameNum(static_cast<int>(durationSeconds * animationframerate));
}


float gModel::getAnimationFramerate() const {
	return animationframerate;
}

void gModel::setAnimationFrameNum(int animationKeyNum) {
	G_PROFILE_ZONE_SCOPED_N("gModel::setAnimationFrameNum()");
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
	for(int i=0; i<meshes.size(); i++) meshes[i]->setFrameNo(frameNo);
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
		meshes[i]->setCurrentFrameId(morphingFrameNo);
	}
}

int gModel::getMorphingFrameNo() const {
	return meshes[0]->getCurrentFrameId();
}

void gModel::nextMorphingFrame() {
	for (int i = 0; i < meshes.size(); i++) {
		meshes[i]->nextFrameId();
	}
}

void gModel::setMorphingSpeed(int speed) {
	for (int i = 0; i < meshes.size(); i++) {
		meshes[i]->setSpeed(speed);
	}
}

int gModel::getMorphingSpeed() const {
	return meshes[0]->getSpeed();
}

void gModel::setMorphingTarget(int morphingTargetId) {
	for (int i = 0; i < meshes.size(); i++) {
		meshes[i]->setCurrentTargetMeshId(morphingTargetId);
	}
}

int gModel::getMorphingTarget() const {
	return meshes[0]->getCurrentTargetMeshId();
}

void gModel::setMorphingFrameNum(int morphingFrameNum) {
	for (int i = 0; i < meshes.size(); i++) {
		meshes[i]->setFrameCount(meshes[i]->getCurrentTargetMeshId(), morphingFrameNum);
	}
}

int gModel::getMorphingFrameNum() const {
	return meshes[0]->getFrameCount();
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
	G_PROFILE_ZONE_SCOPED_N("gModel::makeVertexAnimated()");
	G_PROFILE_ZONE_VALUE(storeOnVram);
	if (!isvertexanimated) {
		isvertexanimationstoredonvram = storeOnVram;
		prepareVertexAnimationData();
		isvertexanimated = true;
		for(int i=0; i<meshes.size(); i++) {
			meshes[i]->setVertexAnimated(true);
			meshes[i]->setVertexAnimationStoredOnVram(storeOnVram);
		}
	} else {
		logi("The model is already vertex animated!");
	}
}

void gModel::prepareVertexAnimationData() {
	G_PROFILE_ZONE_SCOPED_N("gModel::prepareVertexAnimationData()");
    int mnum = meshes.size();
    int anum = scene->mNumAnimations;
    int fnum = getAnimationFrameNum();

    // update mesh position for the animation
    for(unsigned int i=0; i<meshes.size(); ++i) {
    	meshes[i]->resizeVertexAnimationData(anum, fnum, scene->mMeshes[i]->mNumVertices, isvertexanimationstoredonvram);

        // current mesh we are introspecting
        const aiMesh* mesh = scene->mMeshes[i];
        for (unsigned int j=0; j<scene->mNumAnimations; j++) {
            for (unsigned int k=0; k<fnum; k++) {
//            	meshes[i]->resizeVertexAnimation(j, k, mesh->mNumVertices);
            	setAnimationFrameNo(k);
                updateAnimationNodes();
                //The below line's third parameter is given as a nullptr since the meshes aren't morphing. Haven't tested yet.
                //updateBones(&meshes[i], scene->mMeshes[i], nullptr);
                updateBones(meshes[i], scene->mMeshes[i]);

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

                meshes[i]->resetVertexAnimationData(j, k);

                // loop through all vertex weights of all bones
                for(unsigned int a=0; a<mesh->mNumBones; ++a) {
                    const aiBone* bone = mesh->mBones[a];
                    const aiMatrix4x4& posTrafo = boneMatrices[a];

                    for(unsigned int b=0; b<bone->mNumWeights; ++b) {
                        const aiVertexWeight& weight = bone->mWeights[b];
                        size_t vertexId = weight.mVertexId;
                        const aiVector3D& srcPos = mesh->mVertices[vertexId];

            			glm::vec3 oldweightpos = meshes[i]->getVertexPosData(j, k, vertexId);
            			aiVector3D aiaddweight = weight.mWeight * (posTrafo * srcPos);
            			glm::vec3 vPos(oldweightpos.x + aiaddweight.x, oldweightpos.y + aiaddweight.y, oldweightpos.z + aiaddweight.z);
            			meshes[i]->setVertexPosData(j, k, vertexId, vPos);
                    }
                    if(mesh->HasNormals()){
                        // 3x3 matrix, contains the bone matrix without the translation, only with rotation and possibly scaling
                        aiMatrix3x3 normTrafo = aiMatrix3x3( posTrafo);
                        for(unsigned int b=0; b<bone->mNumWeights; ++b) {
                            const aiVertexWeight& weight = bone->mWeights[b];
                            size_t vertexId = weight.mVertexId;

                            const aiVector3D& srcNorm = mesh->mNormals[vertexId];

            				glm::vec3 oldweightnorm = meshes[i]->getVertexNormData(j, k, vertexId);
            				aiVector3D aiaddweight = weight.mWeight * (posTrafo * srcNorm);
            				meshes[i]->setVertexNormData(j, k, vertexId, glm::vec3(oldweightnorm.x + aiaddweight.x, oldweightnorm.y + aiaddweight.y, oldweightnorm.z + aiaddweight.z));
                        }
                    }
                }

                if (isvertexanimationstoredonvram) {
                	std::vector<gVertex> vertexarray = meshes[i]->getVertices();
                	std::vector<gIndex> indexarray = meshes[i]->getIndices();
                	for (int l=0; l<meshes[i]->getVerticesNum(); l++) {
                		vertexarray[l].position = meshes[i]->getVertexPosData(j, k, l);
                		vertexarray[l].normal = meshes[i]->getVertexNormData(j, k, l);
                	}
                	// TODO Set vertices into vboframes!!!
                	meshes[i]->setVerticesData(j, k, vertexarray, indexarray);
//                	meshes[i]->animatedPosData[j][k].clear();
//                	meshes[i]->animatedNormData[j][k].clear();
//                	meshes[i]->getVbo()->clear(); // clear vbo
                }
            }
        }
        if (isvertexanimationstoredonvram) {
        	meshes[i]->clearAnimation();
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

const gBoundingBox& gModel::getBoundingBox() {
	if (needsboundingboxrecalculation) {
		recalculateBoundingBox();
	}
	return boundingbox;
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

void gModel::recalculateBoundingBox() {
	G_PROFILE_ZONE_SCOPED_N("gModel::recalculateBoundingBox()");
	// Ensure the mesh list is not empty
	if (meshes.empty()) {
		// Handle empty meshes case appropriately
		boundingbox = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
		needsboundingboxrecalculation = false;
		return;
	}
	const gBoundingBox& bb1 = meshes[0]->getBoundingBox();

	float minx = bb1.minX(), miny = bb1.minY(), minz = bb1.minZ();
	float maxx = bb1.maxX(), maxy = bb1.maxY(), maxz = bb1.maxZ();

#if defined(__i386__) || defined(__x86_64__)
	__m128 minvals = _mm_set_ps(minz, miny, minx, 0);
	__m128 maxvals = _mm_set_ps(maxz, maxy, maxx, 0);

	for (size_t i = 1; i < meshes.size(); ++i) {
		gSkinnedMesh*& mesh = meshes[i];
		const gBoundingBox& bb = mesh->getBoundingBox();

		__m128 mincurrent = _mm_set_ps(bb.minZ(), bb.minY(), bb.minX(), 0);
		__m128 maxcurrent = _mm_set_ps(bb.maxZ(), bb.maxY(), bb.maxX(), 0);

		minvals = _mm_min_ps(minvals, mincurrent);
		maxvals = _mm_max_ps(maxvals, maxcurrent);
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

	for (size_t i = 1; i < meshes.size(); ++i) {
		gSkinnedMesh*& mesh = meshes[i];
		const gBoundingBox& bb = mesh->getBoundingBox();

		float32x4_t mincurrent = {bb.minZ(), bb.minY(), bb.minX(), 0};
		float32x4_t maxcurrent = {bb.maxZ(), bb.maxY(), bb.maxX(), 0};

		minvals = vminq_f32(minvals, mincurrent);
		maxvals = vmaxq_f32(maxvals, maxcurrent);
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
	for (int i = 1; i < meshes.size(); ++i) {
		gSkinnedMesh*& mesh = meshes[i];
		const gBoundingBox& bb = mesh->getBoundingBox();
		minx = std::min(bb.minX(), minx);
		miny = std::min(bb.minY(), miny);
		minz = std::min(bb.minZ(), minz);
		maxx = std::max(bb.maxX(), maxx);
		maxy = std::max(bb.maxY(), maxy);
		maxz = std::max(bb.maxZ(), maxz);
	}
#endif

	boundingbox = {minx, miny, minz, maxx, maxy, maxz};
	needsboundingboxrecalculation = false;
}

void gModel::setEnableFrustumCulling(bool enable) {
	isenablefrustumculling = enable;
}

void gModel::processTransformationMatrix() {
	if (needsboundingboxrecalculation) {
		gNode::processTransformationMatrix();
		return;
	}
	bool positionchanged = (position != prevposition);
	bool orientationchanged = (orientation != prevorientation);
	bool scalechanged = (scalevec != prevscalevec);
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
