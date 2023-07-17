/*
 * gVignette.h
 *
 *  Created on: 14 Tem 2023
 *      Author: Batuhan Yigit
 */

#ifndef GRAPHICS_POSTEFFECTS_GVIGNETTE_H_
#define GRAPHICS_POSTEFFECTS_GVIGNETTE_H_

#include "gBasePostProcess.h"

class gVignette: public gBasePostProcess {
public:
	gVignette(float outerradius = 0.75f, float innerradius = 0.25f, float intensity = 1.0f);
	virtual ~gVignette();

	void use();

protected:
	const std::string getVertSrc();
	const std::string getFragSrc();
};

#endif /* GRAPHICS_POSTEFFECTS_GVIGNETTE_H_ */
