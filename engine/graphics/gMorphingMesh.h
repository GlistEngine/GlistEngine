/*
 * gMorphingMesh.h
 *
 *  Created on: 10 Tem 2023
 *      Author: Deniz Hatipoglu
 */

#ifndef GRAPHICS_GMORPHINGMESH_H_
#define GRAPHICS_GMORPHINGMESH_H_

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
#include <vector>
#include "gMesh.h"


class gMorphingMesh : public gMesh {
public:
	gMorphingMesh();
	virtual ~gMorphingMesh();

	void draw();

	void drawVboFrames();

	/*
	 * @brief adds the given targetmesh's specific attributes to the vectors.
	 * @param targetMesh is a pointer to the desired target mesh.
	 */
	int addTargetMesh(gMesh *targetMesh);

	/*
	 * @brief sets the base mesh which is going to be used during interpolation.
	 * @param baseMesh is the base mesh's pointer.
	 */
	void setBaseMesh(gMesh *baseMesh);

	/*
	 * @brief sets the current target mesh id to the given id if it is valid. Else it doesn't set.
	 * @param targetMeshId is the id of the desired target mesh.
	 */
	void setCurrentTargetMeshId(int targetMeshId);

	/*
	 * @brief sets the current frame id to the desired id.
	 * @param frameId is the desired frame id.
	 */
	void setCurrentFrameId(int frameId);

	/*
	 * @brief sets the desired target mesh's frame count which is the beyond of interpolation.
	 * @param targetMeshId is the target mesh which will be affected by this function.
	 * @param frameCount is the desired framecount.
	 */
	void setFrameCount(int targetMeshId, int frameCount);

	/*
	 * @brief sets the frame jump speed.
	 * @param speed is the desired speed.
	 */
	void setSpeed(int speed);

	/*
	 * @brief sets position at the given indices to the new one.
	 * @param targetId is the target mesh's index.
	 * @param positionId is one of the target mesh's position's index.
	 * @param newPosition is the desired position.
	 */
	void setTargetPosition(int targetId, int positionId, glm::vec3 newPosition);

	/*
	 * @brief sets normal at the given indices to the new one.
	 * @param targetId is the target mesh's index.
	 * @param normalId is one of the target mesh's normal's index.
	 * @param newNormal is the desired normal.
	 */
	void setTargetNormal(int targetId, int normalId, glm::vec3 newNormal);

	/*
	 * @brief sets the member ismorphinganimated as the desired value.
	 * @param isMorphingAnimated is the desired value.
	 */
	void setMorphingAnimated(bool isMorphingAnimated);

	/*
	 * @brief sets the member ismorphinganimationstoredonvram as the desired value.
	 * @param isMorphingAnimationStoredOnVram is the desired value.
	 */
	void setMorphingAnimationStoredOnVram(bool isMorphingAnimationStoredOnVram);

	/*
	 * @brief sets all the glm vectors of normals and positions to zero vector.
	 * @param targetId is the target mesh's index.
	 */
	void resetTargetData(int targetId);

	/*
	 * @brief jumps frames as many as the given value of the speed variable.
	 */
	void nextFrameId();

	/*
	 * @brief returns the current target mesh's id.
	 */
	int getCurrentTargetMeshId() const;

	/*
	 * @brief returns the current frame's id.
	 */
	int getCurrentFrameId() const;

	/*
	 * @brief returns the desired target mesh's frame count.
	 * @param targetMeshId is the desired target mesh id.
	 */
	int getFrameCount(int targetMeshId) const;

	/*
	 * @brief returns the current target mesh's frame count.
	 */
	int getFrameCount() const;

	/*
	 * @brief returns the target mesh's count.
	 */
	int getTargetMeshCount() const;

	/*
	 * @brief returns the current frame jump speed.
	 */
	int getSpeed() const;

	/*
	 * @brief returns the desired target position.
	 * @param targetId is the desired target mesh's id.
	 * @param positionId is the desired position's id.
	 * @return returns the constant reference of the desired position as a glm::vec3.
	 */
	const glm::vec3& getTargetPosition(int targetId, int positionId) const;

	/*
	 * @brief returns the desired target normal.
	 * @param targetId is the desired target mesh's id.
	 * @param positionId is the desired normal's id.
	 * @return returns the constant reference of the desired normal as a glm::vec3.
	 */
	const glm::vec3& getTargetNormal(int targetId, int normalId) const;

	/*
	 * @brief interpolates the vertices in base and target meshes' from base's to target's meshes.
	 * @param forceInterpolation is an indicator if it checks the current target mesh and current frame are as last interpolate.
	 */
	void interpolate(bool forceInterpolation = true);

private:
	//The loaded target meshes' positions which are the end points of interpolation.
	std::vector<std::vector<glm::vec3>> targetpositions, targetnormals;
	//The base mesh's vertices' spare.
	std::vector<gVertex> basevertices;
	//The animated frames' data.
	std::vector<std::vector<std::vector<glm::vec3>>> framepositions, framenormals;
	//The frames data on vram.
	std::vector<std::vector<gVbo>> vboframes;
	//The current target mesh(that is going to be end point of interpolation)'s index on the targetpositions vector.
	//The current frame id indicates how much should the interpolation progress.
	//The speed is used in function nextFrame which adjusts the frame automatically as a jump indicator.
	int currenttargetmeshid, currentframeid, speed, oldtargetmeshid, oldframeid;
	//ismorphinganimated is an indicator of the holding state of frames. If it is true then the whole frames' positions and normals will be held in vectors those occupies memory in RAM.
	//ismorphinganimationstoredonvram is an indicator of the holding state of frames. If it is true all the frames will be held in vbos which are stored in VRAM.
	//They initialized as false.
	bool ismorphinganimated, ismorphinganimationstoredonvram;
	std::vector<int> framecounts;
	//The base mesh which is the begin point of interpolation.
	gMesh *basemesh;

	//Private Functions
	/*
	 * @brief extracts the positions from the given vertices to the targetpositions vector.
	 * @param targetvertices is the vertex vector of target.
	 */
	void addTargetPositions(std::vector<gVertex>& targetvertices);

	/*
	 * @brief extracts the normals from the given vertices to the targetnormals vector.
	 * @param targetvertices is the vertex vector of target.
	 */
	void addTargetNormals(std::vector<gVertex>& targetvertices);
};

#endif /* GRAPHICS_GMORPHINGMESH_H_ */
