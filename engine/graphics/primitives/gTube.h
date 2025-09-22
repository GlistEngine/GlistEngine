/*
 * gTube.h
 *
 *  Created on: 14 Tem 2023
 *      Author: Aak-4
 */

#ifndef GRAPHICS_PRIMITIVES_GTUBE_H_
#define GRAPHICS_PRIMITIVES_GTUBE_H_

#include "gMesh.h"

class gTube : public gMesh {
public:
	gTube(int topOuterRadius, int topInnerRadius, int bottomOuterRadius, int bottomInnerRadius, int h, glm::vec2 shiftdistance, int segmentnum, bool isFilled);
	~gTube() override;
};

#endif /* GRAPHICS_PRIMITIVES_GTUBE_H_ */
