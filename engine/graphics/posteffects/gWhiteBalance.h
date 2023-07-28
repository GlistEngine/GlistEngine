/*
 * gWhiteBalance.h
 *
 *  Created on: 18 Tem 2023
 *      Author: Batuhan Yigit
 */

#ifndef GRAPHICS_POSTEFFECTS_GWHITEBALANCE_H_
#define GRAPHICS_POSTEFFECTS_GWHITEBALANCE_H_

#include "gBasePostProcess.h"

class gWhiteBalance: public gBasePostProcess {
public:
	gWhiteBalance(float temperature = 0.0f, float tint = 0.0f);
	virtual ~gWhiteBalance();

	void use();

protected:
	const std::string getVertSrc();
	const std::string getFragSrc();
};

#endif /* GRAPHICS_POSTEFFECTS_GWHITEBALANCE_H_ */
