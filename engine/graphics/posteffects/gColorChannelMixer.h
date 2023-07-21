/*
 * gColorChannelMixer.h
 *
 *  Created on: 13 Tem 2023
 *      Author: Batuhan Yigit
 */

#ifndef GRAPHICS_POSTEFFECTS_GCOLORCHANNELMIXER_H_
#define GRAPHICS_POSTEFFECTS_GCOLORCHANNELMIXER_H_

#include "gBasePostProcess.h"

class gColorChannelMixer: public gBasePostProcess {
public:
	gColorChannelMixer(float rgbmatrix[3][3]);
	virtual ~gColorChannelMixer();

	void use();

protected:
	const std::string getVertSrc();
	const std::string getFragSrc();
};

#endif /* GRAPHICS_POSTEFFECTS_GCOLORCHANNELMIXER_H_ */
