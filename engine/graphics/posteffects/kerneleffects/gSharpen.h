/*
 * gSharpen.h
 *
 *  Created on: 2 Ara 2021
 *      Author: kayra
 */

#ifndef GRAPHICS_POSTEFFECTS_GSHARPEN_H_
#define GRAPHICS_POSTEFFECTS_GSHARPEN_H_

#include <gKernelEffect.h>

class gSharpen : public gKernelEffect {
public:
	gSharpen();
	virtual ~gSharpen();

	void use();
protected:
	const std::string getVertSrc();
	const std::string getFragSrc();
};

#endif /* GRAPHICS_POSTEFFECTS_GSHARPEN_H_ */
