/*
 * gGrayscale.h
 *
 *  Created on: 2 Ara 2021
 *      Author: kayra
 */

#ifndef GRAPHICS_POSTEFFECTS_GGRAYSCALE_H_
#define GRAPHICS_POSTEFFECTS_GGRAYSCALE_H_

#include <gBasePostProcess.h>

class gGrayscale: public gBasePostProcess {
public:
	gGrayscale();
	virtual ~gGrayscale();

	void use();

protected:
	const std::string getVertSrc();
	const std::string getFragSrc();
};

#endif /* GRAPHICS_POSTEFFECTS_GGRAYSCALE_H_ */
