/*
 * gCone.h
 *
 *  Created on: 15 Mar 2023
 *      Author: Umutcan Türkmen
 */

#ifndef GRAPHICS_PRIMITIVES_GCONE_H_
#define GRAPHICS_PRIMITIVES_GCONE_H_

#include "gMesh.h"

class gCone: public gMesh {
public:
	gCone(int r, int h, int segments, glm::vec2 shiftdistance, bool isFilled);
	virtual ~gCone();
};



#endif /* GRAPHICS_PRIMITIVES_GCONE_H_ */
