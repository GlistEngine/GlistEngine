/*
 * gColorCurves.h
 *
 *  Created on: 18 Tem 2023
 *      Author: Batuhan Yigit
 */

#ifndef GRAPHICS_POSTEFFECTS_GCOLORCURVES_H_
#define GRAPHICS_POSTEFFECTS_GCOLORCURVES_H_

#include "gBasePostProcess.h"

class gColorCurves: public gBasePostProcess {
public:
	gColorCurves(float rgbcurvepoints[3][2]);
	virtual ~gColorCurves();

	void use();

protected:
	const std::string getVertSrc();
	const std::string getFragSrc();
};

#endif /* GRAPHICS_POSTEFFECTS_GCOLORCURVES_H_ */
