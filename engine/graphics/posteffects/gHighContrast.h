/*
 * gHighContrast.h
 *
 *  Created on: 17 Tem 2023
 *      Author: Batuhan Yigit
 */

#ifndef GRAPHICS_POSTEFFECTS_GHIGHCONTRAST_H_
#define GRAPHICS_POSTEFFECTS_GHIGHCONTRAST_H_

#include "gBasePostProcess.h"

class gHighContrast: public gBasePostProcess {
public:
	gHighContrast(float intensity = 5.0f, float midpoint = 0.5f, float threshold = 0.7f);
	virtual ~gHighContrast();

	void use();

protected:
	const std::string getVertSrc();
	const std::string getFragSrc();
};

#endif /* GRAPHICS_POSTEFFECTS_GHIGHCONTRAST_H_ */
