/*
 * gModel.h
 *
 *  Created on: May 10, 2020
 *      Author: noyan
 */

#ifndef ENGINE_GRAPHICS_GMODEL_H_
#define ENGINE_GRAPHICS_GMODEL_H_

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/cimport.h>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include "gSkinnedMesh.h"
#include <vector>
#include <deque>


class gModel : public gNode {
public:
    // model data
    std::deque<gTexture> textures_loaded;	// stores all the textures loaded so far, optimization to make sure textures aren't loaded more than once.
    std::vector<gSkinnedMesh>    meshes;
    std::string directory;

    // constructor, expects a filepath to a 3D model.
	gModel();
	virtual ~gModel();

	void loadModel(std::string modelPath);
	void load(std::string fullPath);
	void draw();

	std::string getFilename();
	std::string getFullpath();
	int getMeshNum();
	gSkinnedMesh getMesh(int meshNo);
	gSkinnedMesh* getMeshPtr(int meshNo);
	std::string getMeshName(int meshNo);
	gBoundingBox getBoundingBox();

	void move(float dx, float dy, float dz);
	void move(const glm::vec3 dv);
	void rotate(float angle, float ax, float ay, float az);
	void rotate(const glm::quat& q);
	void scale(float sx, float sy, float sz);
	void scale(float s);
	void dolly(float distance);
	void truck(float distance);
	void boom(float distance);
	void tilt(float radians);
	void pan(float radians);
	void roll(float radians);

	void setPosition(float px, float py, float pz);
	void setPosition(const glm::vec3& p);
	void setOrientation(const glm::quat& o);
	void setOrientation(const glm::vec3& angles);
	void setScale(const glm::vec3& s);
	void setScale(float sx, float sy, float sz);
	void setScale(float s);

	void setTransformationMatrix(glm::mat4 transformationMatrix);

	bool isAnimated();
	int getAnimationNum();
	float getAnimationDuration(int animationNo = 0);
	void animate(float animationPosition);
	float getAnimationPosition();
	void setAnimationFrameNo(int frameNo);
	void nextAnimationFrame();
	int getAnimationFrameNo();
	void setAnimationFrameNum(int animationKeyNum);
	int getAnimationFrameNum();
	void setAnimationFramerate(float animationFramerate);
	float getAnimationFramerate();

	bool isVertexAnimated();
	bool isVertexAnimationStoredOnVram();
	void makeVertexAnimated(bool storeOnVram = true);


private:
	const aiScene* scene;
	void loadModelFile(std::string fullPath);
	void processNode(aiNode *node, const aiScene *scene);
	gSkinnedMesh processMesh(aiMesh *mesh, const aiScene *scene);
	void loadMaterialTextures(gSkinnedMesh* mesh, aiMaterial *mat, aiTextureType type, int textureType);
	void updateBones(gSkinnedMesh* gmesh, aiMesh* aimesh);
	void updateVbo(gSkinnedMesh* gmesh);
	void updateAnimationNodes();
	void generateAnimationKeys();

	std::string filename;
	int animationnum;
	bool isanimated;
	float animationposition, animationpositionold;
	float animationframerate;
	int animationframenum;
	std::vector<float> animationkeys;
	int animationframeno;

	void prepareVertexAnimationData();
	bool isvertexanimated;
	bool isvertexanimationstoredonvram;

    float bbminx, bbminy, bbminz, bbmaxx, bbmaxy, bbmaxz;
};

#endif /* ENGINE_GRAPHICS_GMODEL_H_ */
