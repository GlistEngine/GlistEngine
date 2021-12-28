/*
 * gBlur.h
 *
 *  Created on: 2 Ara 2021
 *      Author: kayra
 */

#ifndef GRAPHICS_POSTEFFECTS_GBLUR_H_
#define GRAPHICS_POSTEFFECTS_GBLUR_H_

#include <gKernelEffect.h>

class gBlur: public gKernelEffect {
public:
	gBlur();
	virtual ~gBlur();

	void use();
protected:
	const std::string getVertSrc();
	const std::string getFragSrc();
};

#endif /* GRAPHICS_POSTEFFECTS_GBLUR_H_ */
