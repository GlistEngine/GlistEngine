/*
 * gChromaticAberration.h
 *
 *  Created on: 17 Tem 2023
 *      Author: Batuhan Yigit
 */

#ifndef GRAPHICS_POSTEFFECTS_GCHROMATICABERRATION_H_
#define GRAPHICS_POSTEFFECTS_GCHROMATICABERRATION_H_

#include "gBasePostProcess.h"

class gChromaticAberration: public gBasePostProcess {
public:
	gChromaticAberration(float intensity = 1.0f);
	virtual ~gChromaticAberration();

	void use();

protected:
	const std::string getVertSrc();
	const std::string getFragSrc();
};

#endif /* GRAPHICS_POSTEFFECTS_GCHROMATICABERRATION_H_ */
