/*
 * gSoftFocus.h
 *
 *  Created on: 13 Tem 2023
 *      Author: Batuhan Yigit
 */

#ifndef GRAPHICS_POSTEFFECTS_GSOFTFOCUS_H_
#define GRAPHICS_POSTEFFECTS_GSOFTFOCUS_H_

#include "gBasePostProcess.h"

class gSoftFocus: public gBasePostProcess {
public:
	gSoftFocus(float radius, float softness, float focuscenterx, float focuscentery);
	virtual ~gSoftFocus();

	void use();

protected:
	const std::string getVertSrc();
	const std::string getFragSrc();
};

#endif /* GRAPHICS_POSTEFFECTS_GSOFTFOCUS_H_ */
