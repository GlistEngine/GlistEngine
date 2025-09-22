/*
 * gCone.h
 *
 *  Created on: 15 Mar 2023
 *      Author: Umutcan Turkmen
 */

#ifndef GRAPHICS_PRIMITIVES_GCONE_H_
#define GRAPHICS_PRIMITIVES_GCONE_H_

#include "gMesh.h"

class gCone: public gMesh {
public:
	gCone(int r, int h, glm::vec2 shiftdistance, int segmentnum = 32, bool isFilled = true);
	~gCone() override;
};



#endif /* GRAPHICS_PRIMITIVES_GCONE_H_ */
