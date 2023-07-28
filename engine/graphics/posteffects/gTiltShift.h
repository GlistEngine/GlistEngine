/*
 * gTiltShift.h
 *
 *  Created on: 19 Tem 2023
 *      Author: Batuhan Yigit
 */

#ifndef GRAPHICS_POSTEFFECTS_GTILTSHIFT_H_
#define GRAPHICS_POSTEFFECTS_GTILTSHIFT_H_

#include "gBasePostProcess.h"

class gTiltShift: public gBasePostProcess {
public:
	gTiltShift(float upperheight, float lowerheight, float softness = 0.5f);
	virtual ~gTiltShift();

	void use();

protected:
	const std::string getVertSrc();
	const std::string getFragSrc();
};

#endif /* GRAPHICS_POSTEFFECTS_GTILTSHIFT_H_ */
