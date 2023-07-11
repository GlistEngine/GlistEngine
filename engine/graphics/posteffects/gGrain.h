/*
 * gGrain.h
 *
 *  Created on: 11 Tem 2023
 *      Author: Batuhan Yigit
 */

#ifndef GRAPHICS_POSTEFFECTS_GGRAIN_H_
#define GRAPHICS_POSTEFFECTS_GGRAIN_H_

#include "gBasePostProcess.h"

class gGrain: public gBasePostProcess {
public:
	gGrain(float intensity = 0.35f);
	virtual ~gGrain();

	void use();

protected:
	const std::string getVertSrc();
	const std::string getFragSrc();
};

#endif /* GRAPHICS_POSTEFFECTS_GGRAIN_H_ */
