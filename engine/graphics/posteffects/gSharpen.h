/*
 * gSharpen.h
 *
 *  Created on: 2 Ara 2021
 *      Author: kayra
 */

#ifndef GRAPHICS_POSTEFFECTS_GSHARPEN_H_
#define GRAPHICS_POSTEFFECTS_GSHARPEN_H_

#include <gBasePostProcess.h>

class gSharpen : public gBasePostProcess {
public:
	gSharpen();
	virtual ~gSharpen();
	void use();
};

#endif /* GRAPHICS_POSTEFFECTS_GSHARPEN_H_ */
