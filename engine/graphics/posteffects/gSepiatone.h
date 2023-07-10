/*
 * gSepiatone.h
 *
 *  Created on: 10 Tem 2023
 *      Author: BATUHAN
 */

#ifndef GRAPHICS_POSTEFFECTS_GSEPIATONE_H_
#define GRAPHICS_POSTEFFECTS_GSEPIATONE_H_

#include <gBasePostProcess.h>

class gSepiatone: public gBasePostProcess {
public:
	gSepiatone();
	virtual ~gSepiatone();

	void use();

protected:
	const std::string getVertSrc();
	const std::string getFragSrc();
};

#endif /* GRAPHICS_POSTEFFECTS_GSEPIATONE_H_ */
