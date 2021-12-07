/*
 * gEdgeDetection.h
 *
 *  Created on: 2 Ara 2021
 *      Author: kayra
 */

#ifndef GRAPHICS_POSTEFFECTS_GEDGEDETECTION_H_
#define GRAPHICS_POSTEFFECTS_GEDGEDETECTION_H_

#include <gKernelEffect.h>

class gEdgeDetection: public gKernelEffect {
public:
	gEdgeDetection();
	virtual ~gEdgeDetection();

	void use();
protected:
	const std::string getVertSrc();
	const std::string getFragSrc();
};

#endif /* GRAPHICS_POSTEFFECTS_GEDGEDETECTION_H_ */
