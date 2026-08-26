/*
 * gMagnifier.h
 *
 *  Created on: 26 Aug 2026
 *      Author: Bahar Kucukozer, Mehmet Sefa Ciftci
 */

#ifndef GRAPHICS_POSTEFFECTS_GMAGNIFIER_H_
#define GRAPHICS_POSTEFFECTS_GMAGNIFIER_H_

#include "gBasePostProcess.h"

class gMagnifier: public gBasePostProcess {
public:
	gMagnifier(float radius = 120.0f, float zoom = 2.0f,
			float softness = 2.0f);
	virtual ~gMagnifier();

	void use();

	// Mouse coordinates use a top-left origin and pixel units.
	void setMousePosition(float mousex, float mousey);

	// Width and height must use the same coordinate scale as the mouse.
	void setScreenSize(float width, float height);

	void setRadius(float radius);
	void setZoom(float zoom);
	void setSoftness(float softness);

protected:
	const std::string getVertSrc();
	const std::string getFragSrc();

private:
	float mousex;
	float mousey;
	float screenwidth;
	float screenheight;
	float radius;
	float zoom;
	float softness;
};

#endif /* GRAPHICS_POSTEFFECTS_GMAGNIFIER_H_ */
