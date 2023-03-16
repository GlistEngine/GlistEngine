/*
 * gCylinder.h
 *
 *  Created on: 13 Mar 2023
 *      Author: Umutcan Turkmen
 */

#ifndef GRAPHICS_PRIMITIVES_GCYLINDER_H_
#define GRAPHICS_PRIMITIVES_GCYLINDER_H_

#include "gMesh.h"

class gCylinder: public gMesh {
public:
	gCylinder(int r1, int r2, int h, glm::vec2 shiftdistance, int segmentnum = 64, bool isFilled = true);
	virtual ~gCylinder();
};


#endif /* GRAPHICS_PRIMITIVES_GCYLINDER_H_ */
