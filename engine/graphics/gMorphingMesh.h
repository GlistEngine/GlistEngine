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

	void draw();

	void drawVboFrame();

	/*
	 * @brief loads the model's vertex positions and normals from the given path.
	 * @param path is the location of model file in the model directory.
	 */
	void loadMorphingModel(std::string path);

	/*
	 * @brief sets the base mesh which is going to be used during interpolation.
	 * @param basemesh is the base mesh's pointer.
	 */
	void loadModel(gModel *basemodel);

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
	 * @brief sets the current base mesh id to the desired basemeshid if possible.
	 * @param basemeshid is the desired base mesh id.
	 */
	void setCurrentBaseMeshId(int basemeshid);

	/*
	 * @brief sets the current target mesh's vbo's vertices and indices.
	 * @param vertices is the vertices.
	 * @param indices is the indices.
	 */
	void setVerticesData(int frameno, std::vector<gVertex>& vertices, std::vector<unsigned int>& indices);

	/*
	 * @brief sets the frame jump speed.
	 * @param speed is the desired speed.
	 */
	void setSpeed(int speed);

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
	 * @brief returns the base meshes' count.
	 */
	int getBaseMeshCount();

	/*
	 * returns the current base mesh's id.
	 */
	int getCurrentBaseMeshId();

	/*
	 * @brief interpolates the vertices in base and target meshes' from base's to target's meshes.
	 * @param framescount indicates how many frames the function will create.
	 */
	void interpolate(int framescount);

private:
	//The loaded base meshes' positions which are the end point of interpolation.
	std::vector<std::vector<std::vector<glm::vec3>>> targetpositions, framepositions;
	//The frames vertex buffers.
	std::vector<std::vector<std::vector<gVbo>>> vboframes;
	//The current base mesh(that is going to be used in interpolation)'s index on the vector.
	int currenttargetmeshid, currentframeid, currentbasemeshid;
	//The target mesh which is the begin point of interpolation.
	std::vector<gMesh*> basemeshes;
	std::string directory, filename;
	const aiScene *scene;
	/*
	 * @variable loadorder is order of load.
	 * @variable speed is how many frame jumps are in an iteration of nextFrameId function.
	 */
	int loadorder, speed;

	void loadModelFile(const std::string& fullpath);
	void processNode(aiNode *node, const aiScene *scene);
	void processMesh(aiMesh *mesh, const aiScene *scene, aiMatrix4x4 matrix, int basemeshid);

};

#endif /* GRAPHICS_GMORPHINGMESH_H_ */
