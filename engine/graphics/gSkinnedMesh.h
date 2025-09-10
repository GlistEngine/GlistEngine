/*
 * gSkinnedMesh.h
 *
 * If a mesh contains any morphing target, then the frame processed during runtime. Which means you cannot use the mesh in VRAM if it has morphing target(s).
 *
 *  Created on: 27 Kas 2020
 *      Author: Acer
 */

#ifndef GRAPHICS_GSKINNEDMESH_H_
#define GRAPHICS_GSKINNEDMESH_H_

#include "gMorphingMesh.h"

class gSkinnedMesh : public gMorphingMesh {
public:
	gSkinnedMesh();
	~gSkinnedMesh() override;

	void draw() override;

	void resizeAnimation(int verticesNum);
	void resetAnimation();
	void setVertexPos(int vertexNo, const glm::vec3& newWeight);
	void setVertexNorm(int vertexNo, const glm::vec3& newWeight);
	const glm::vec3& getVertexPos(int vertexNo) const;
	const glm::vec3& getVertexNorm(int vertexNo) const;
	void clearAnimation();

	void resizeVertexAnimationData(int animationNum, int frameNum, int verticesNum, bool isOnVram);
	void resetVertexAnimationData(int animationNo, int frameNo);
	void setVertexPosData(int animationNo, int frameNo, int vertexNo, const glm::vec3& newWeight);
	void setVertexNormData(int animationNo, int frameNo, int vertexNo, const glm::vec3& newWeight);
	const glm::vec3& getVertexPosData(int animationNo, int frameNo, int vertexNo) const;
	const glm::vec3& getVertexNormData(int animationNo, int frameNo, int vertexNo) const;
	void setVerticesData(int animationNo, int frameNo, const std::vector<gVertex>& vertices, const std::vector<gIndex>& indices);

	void setVertexAnimated(bool isVertexAnimated);
	void setVertexAnimationStoredOnVram(bool isVertexAnimationStoredOnVram);
	void setFrameNo(int frameNo);
	int getFrameNo() const;

	const std::vector<glm::vec3>& getAnimatedPos() const;
	const std::vector<glm::vec3>& getAnimatedNorm() const;

private:
	std::vector<glm::vec3> animatedPos;
	std::vector<glm::vec3> animatedNorm;

	std::vector<std::vector<std::vector<glm::vec3>>> animatedPosData;
	std::vector<std::vector<std::vector<glm::vec3>>> animatedNormData;
	std::vector<std::vector<std::unique_ptr<gVbo>>> vboframe;

	void drawVboFrame();
	bool isvertexanimated, isvertexanimationstoredonvram;
	int frameno, framenoold;
};

#endif /* GRAPHICS_GSKINNEDMESH_H_ */
