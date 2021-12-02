/*
 * gBlur.h
 *
 *  Created on: 2 Ara 2021
 *      Author: kayra
 */

#ifndef GRAPHICS_POSTEFFECTS_GBLUR_H_
#define GRAPHICS_POSTEFFECTS_GBLUR_H_

#include <gBasePostProcess.h>

class gBlur: public gBasePostProcess {
public:
	gBlur();
	virtual ~gBlur();
	void use();

private:
	float kernel[9];
};

#endif /* GRAPHICS_POSTEFFECTS_GBLUR_H_ */
