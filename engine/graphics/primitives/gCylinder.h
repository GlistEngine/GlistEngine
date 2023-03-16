/*
 * gCylinder.h
 *
 *  Created on: 13 Mar 2023
 *      Author: Umutcan Türkmen
 */

#ifndef GRAPHICS_PRIMITIVES_GCYLINDER_H_
#define GRAPHICS_PRIMITIVES_GCYLINDER_H_

#include "gMesh.h"

class gCylinder: public gMesh {
public:
	gCylinder(int r1, int r2, int h, glm::vec2 shiftdistance, bool isFilled);
	virtual ~gCylinder();
};


#endif /* GRAPHICS_PRIMITIVES_GCYLINDER_H_ */
