/*
 * gTerrain.h
 *
 *  Created on: 24 Þub 2026
 *      Author: Engin Kutlu
 */

#ifndef GRAPHICS_PRIMITIVES_GTERRAIN_H_
#define GRAPHICS_PRIMITIVES_GTERRAIN_H_

#include "gMesh.h"

class gTerrain: public gMesh {
public:
	gTerrain();
	virtual ~gTerrain();

	void generateTerrain(const std::string& heightMapPath, int widthSegments, int depthSegments, float cellSize, float maxHeight);
};

#endif /* GRAPHICS_PRIMITIVES_GTERRAIN_H_ */
