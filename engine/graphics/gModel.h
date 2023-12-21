/*
 * gModel.h
 *
 *  Created on: May 10, 2020
 *      Author: noyan
 */

#ifndef ENGINE_GRAPHICS_GMODEL_H_
#define ENGINE_GRAPHICS_GMODEL_H_

#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
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
    std::deque<gTexture*> textures_loaded;	// stores all the textures loaded so far, optimization to make sure textures aren't loaded more than once.
    std::deque<gSkinnedMesh>    meshes;
    std::string directory;

    // constructor, expects a filepath to a 3D model.
	gModel();
	virtual ~gModel();

	void loadModel(const std::string& modelPath);
	void loadModelWithOriginalVertices(const std::string& modelPath);
	void loadMorphingTargetModel(const std::string& modelPath);
	void load(const std::string& fullPath);
	void draw();

	const std::string& getFilename() const;
	const std::string getFullpath() const;
	int getMeshNum() const;
	int getMeshNo(const std::string& meshName) const;
	gSkinnedMesh& getMesh(int meshNo);
	gSkinnedMesh* getMeshPtr(int meshNo);
	const std::string getMeshName(int meshNo) const;
	gBoundingBox getBoundingBox();

	void move(float dx, float dy, float dz);
	void move(const glm::vec3& dv);
	void rotate(const glm::quat& q);
	void rotate(float radians, float ax, float ay, float az); //first change
	void rotateDeg(float degrees, float ax, float ay, float az);
	void rotateAround(float radians, const glm::vec3& axis, const glm::vec3& point);
	void rotateAroundDeg(float degrees, const glm::vec3& axis, const glm::vec3& point);
	void scale(float sx, float sy, float sz);
	void scale(float s);
	void dolly(float distance);
	void truck(float distance);
	void boom(float distance);
	void tilt(float radians);
	void tiltDeg(float degrees);
	void pan(float radians);
	void panDeg(float degrees);
	void roll(float radians);
	void rollDeg(float degrees);

	void setPosition(float px, float py, float pz);
	void setPosition(const glm::vec3& p);
	void setOrientation(const glm::quat& o);
	void setOrientation(const glm::vec3& angles);
	void setScale(const glm::vec3& s);
	void setScale(float sx, float sy, float sz);
	void setScale(float s);

	void setTransformationMatrix(const glm::mat4& transformationMatrix);

	bool isAnimated() const;
	int getAnimationNum() const;
	float getAnimationDuration(int animationNo = 0) const;
	void animate(float animationPosition);
	float getAnimationPosition() const;
	void setAnimationFrameNo(int frameNo);
	void nextAnimationFrame();
	int getAnimationFrameNo() const;
	void setAnimationFrameNum(int animationKeyNum);
	int getAnimationFrameNum() const;
	void setAnimationFramerate(float animationFramerate);
	float getAnimationFramerate() const;
	void setMorphingFrameNo(int morphingAnimationNo);
	int getMorphingFrameNo() const;
	void nextMorphingFrame();
	void setMorphingSpeed(int speed);
	int getMorphingSpeed() const;
	void setMorphingFrameNum(int morphingFrameNum);
	int getMorphingFrameNum() const;
	void setMorphingTarget(int morphingTargetId);
	int getMorphingTarget() const;

	bool isVertexAnimated() const;
	bool isVertexAnimationStoredOnVram() const;
	void makeVertexAnimated(bool storeOnVram = true);

    gBoundingBox& getInitialBoundingBox();

private:
	const aiScene* scene;
	std::vector<const aiScene*> morphingtargetscenes;
	void loadModelFile(const std::string& fullPath);
	void loadModelFileWithOriginalVertices(const std::string& fullPath);
	void loadMorphingTargetModelFile(const std::string& fullPath);
	void processNode(aiNode *node, const aiScene *scene);
	gSkinnedMesh processMesh(aiMesh *mesh, const aiScene *scene, aiMatrix4x4 matrix);
	void loadMaterialTextures(gSkinnedMesh* mesh, aiMaterial *mat, aiTextureType type, int textureType);
	void processMorphingNode(aiNode *node, const aiScene *scene);
	gMesh processMorphingMesh(aiMesh *mesh, const aiScene *scene, aiMatrix4x4 matrix);
	//The below line's third parameter is to perform the animation on the target mesh by taking the aiTargetMesh as a reference. Haven't tested yet.
	//void updateBones(gSkinnedMesh* gmesh, aiMesh* aimesh, aiMesh* aiTargetMesh);
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
    int bbi, bbj;
    std::vector<gVertex> bbvertices;
    glm::vec3 bbvpos;
    gVertex bbv;

    glm::mat4 convertMatrix(const aiMatrix4x4 &aiMat);
    gBoundingBox initialboundingbox;

    bool oldalpha;
    unsigned int dri;
};

#endif /* ENGINE_GRAPHICS_GMODEL_H_ */
