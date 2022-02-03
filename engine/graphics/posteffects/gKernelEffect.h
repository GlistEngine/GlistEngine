/*
 * gKernelEffect.h
 *
 *  Created on: 7 Ara 2021
 *      Author: kayra
 */

#ifndef GRAPHICS_POSTEFFECTS_GKERNELEFFECT_H_
#define GRAPHICS_POSTEFFECTS_GKERNELEFFECT_H_

#include <gBasePostProcess.h>
#include <gShader.h>

class gKernelEffect: public gBasePostProcess {
public:
	gKernelEffect();
	virtual ~gKernelEffect();

protected:
	const std::string getVertSrc();
	const std::string getFragSrc();
};

#endif /* GRAPHICS_POSTEFFECTS_GKERNELEFFECT_H_ */
