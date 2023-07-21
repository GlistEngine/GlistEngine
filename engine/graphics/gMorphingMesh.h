/*
 * gMorphingMesh.h
 *
 *  Created on: 10 Tem 2023
 *      Author: Deniz Hatipoglu
 */

#ifndef GRAPHICS_GMORPHINGMESH_H_
#define GRAPHICS_GMORPHINGMESH_H_

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
#include "gModel.h"


class gMorphingMesh : public gMesh {
public:
	gMorphingMesh();
	virtual ~gMorphingMesh();

	/*
	 * @brief adds the given targetmesh's specific attributes to the vectors.
	 * @param targetmesh is a pointer to the desired target mesh.
	 */
	int addTargetMesh(gMesh *targetmesh);

	/*
	 * @brief sets the base mesh which is going to be used during interpolation.
	 * @param basemesh is the base mesh's pointer.
	 */
	void setBaseMesh(gMesh *basemesh);

	/*
	 * @brief sets the current target mesh id to the given id if it is valid. Else it doesn't set.
	 * @param currenttargetmeshid is the id of target mesh which is going to be used during interpolation.
	 */
	void setCurrentTargetMeshId(int currenttargetmeshid);

	/*
	 * @brief sets the current frame id.
	 * @param frameid is the desired frame id to change with original.
	 */
	void setCurrentFrameId(int frameid);

	/*
	 * @brief sets the desired target mesh's frame count which is the beyond of interpolation.
	 * @param targetmeshid is the target mesh which will be affected by this function.
	 * @param framecount is the desired framecount.
	 */
	void setFrameCount(int targetmeshid, int framecount);

	/*
	 * @brief sets the frame jump speed.
	 * @param speed is the desired speed.
	 */
	void setSpeed(int speed);

	/*
	 * @brief sets position at the given indices to the new one.
	 * @param targetid is the target mesh's index.
	 * @param vertxid is one of the target mesh's position's index.
	 * @param newposition is the desired position.
	 */
	void setTargetPosition(int targetid, int positionid, glm::vec3 newposition);

	/*
	 * @brief sets normal at the given indices to the new one.
	 * @param targetid is the target mesh's index.
	 * @param vertxid is one of the target mesh's normal's index.
	 * @param newnormal is the desired normal.
	 */
	void setTargetNormal(int targetid, int normalid, glm::vec3 newnormal);

	/*
	 * @brief jumps frames as many as the given value of the speed variable.
	 */
	void nextFrameId();

	/*
	 * @brief returns the current target mesh's id.
	 */
	int getCurrentTargetMeshId();

	/*
	 * @brief returns the current frame's id.
	 */
	int getCurrentFrameId();

	/*
	 * @brief returns the given target mesh's frame count.
	 */
	int getFrameCount(int targetmeshid);

	/*
	 * @brief returns the current target mesh's frame count.
	 */
	int getFrameCount();

	/*
	 * @brief returns the target mesh's count.
	 */
	int getTargetMeshCount();

	/*
	 * @brief returns the current frame jump speed.
	 */
	int getSpeed();

	/*
	 * @brief interpolates the vertices in base and target meshes' from base's to target's meshes.
	 */
	void interpolate();

private:
	//The loaded target meshes' positions which are the end points of interpolation.
	std::vector<std::vector<glm::vec3>> targetpositions, targetnormals;
	//The current target mesh(that is going to be end point of interpolation)'s index on the targetpositions vector.
	//The current frame id indicates how much should the interpolation progress.
	//The speed is used in function nextFrame which adjusts the frame automatically as a jump indicator.
	int currenttargetmeshid, currentframeid, speed;
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
