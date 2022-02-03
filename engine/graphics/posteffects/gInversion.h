/*
 * gInversion.h
 *
 *  Created on: 29 Kas 2021
 *      Author: kayra
 */

#ifndef GRAPHICS_POSTEFFECTS_GINVERSION_H_
#define GRAPHICS_POSTEFFECTS_GINVERSION_H_

#include "gBasePostProcess.h"

class gInversion : public gBasePostProcess {
public:
	gInversion();
	virtual ~gInversion();

	void use();
protected:
	const std::string getVertSrc();
	const std::string getFragSrc();
};

#endif /* GRAPHICS_POSTEFFECTS_GINVERSION_H_ */
