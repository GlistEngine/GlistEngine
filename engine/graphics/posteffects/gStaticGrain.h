/*
 * gStaticGrain.h
 *
 *  Created on: 11 Tem 2023
 *      Author: Batuhan Yigit
 */

#ifndef GRAPHICS_POSTEFFECTS_GSTATICGRAIN_H_
#define GRAPHICS_POSTEFFECTS_GSTATICGRAIN_H_

#include "gBasePostProcess.h"

class gStaticGrain: public gBasePostProcess {
public:
	gStaticGrain(float intensity = 0.35f);
	virtual ~gStaticGrain();

	void use();

protected:
	const std::string getVertSrc();
	const std::string getFragSrc();
};

#endif /* GRAPHICS_POSTEFFECTS_GSTATICGRAIN_H_ */
