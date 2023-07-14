/*
 * gColorAdjustments.h
 *
 *  Created on: 12 Tem 2023
 *      Author: Batuhan Yigit
 */

#include "gBasePostProcess.h"

#ifndef GRAPHICS_POSTEFFECTS_GCOLORADJUSTMENTS_H_
#define GRAPHICS_POSTEFFECTS_GCOLORADJUSTMENTS_H_

class gColorAdjustments: public gBasePostProcess {
public:
	gColorAdjustments(float brightness = 1.0f, float contrast = 1.0f, float saturation = 1.0f, float exposure = 1.0f);
	virtual ~gColorAdjustments();

	void use();

protected:
	const std::string getVertSrc();
	const std::string getFragSrc();
};

#endif /* GRAPHICS_POSTEFFECTS_GCOLORADJUSTMENTS_H_ */
