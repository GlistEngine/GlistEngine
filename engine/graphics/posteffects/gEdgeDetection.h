/*
 * gEdgeDetection.h
 *
 *  Created on: 2 Ara 2021
 *      Author: kayra
 */

#ifndef GRAPHICS_POSTEFFECTS_GEDGEDETECTION_H_
#define GRAPHICS_POSTEFFECTS_GEDGEDETECTION_H_

#include <gBasePostProcess.h>

class gEdgeDetection: public gBasePostProcess {
public:
	gEdgeDetection();
	virtual ~gEdgeDetection();
	void use();
};

#endif /* GRAPHICS_POSTEFFECTS_GEDGEDETECTION_H_ */
