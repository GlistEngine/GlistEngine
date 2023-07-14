/*
 * gDynamicGrain.h
 *
 *  Created on: 11 Tem 2023
 *      Author: Batuhan Yigit
 */

#ifndef GRAPHICS_POSTEFFECTS_GDYNAMICGRAIN_H_
#define GRAPHICS_POSTEFFECTS_GDYNAMICGRAIN_H_

#include "gBasePostProcess.h"

class gDynamicGrain: public gBasePostProcess {
public:
	gDynamicGrain(float intensity = 0.35f, float speed = 0.02f);
	virtual ~gDynamicGrain();

	void use();

protected:
	const std::string getVertSrc();
	const std::string getFragSrc();

private:
	float speed;
	float counter;
};

#endif /* GRAPHICS_POSTEFFECTS_GDYNAMICGRAIN_H_ */
