/*
 * gShadowsMidtonesHighlights.h
 *
 *  Created on: 18 Tem 2023
 *      Author: Batuhan Yigit
 */

#ifndef GRAPHICS_POSTEFFECTS_GSHADOWSMIDTONESHIGHLIGHTS_H_
#define GRAPHICS_POSTEFFECTS_GSHADOWSMIDTONESHIGHLIGHTS_H_

#include "gBasePostProcess.h"

class gShadowsMidtonesHighlights: public gBasePostProcess {
public:
	gShadowsMidtonesHighlights(float shadowsrgb[3], float midtonesrgb[3], float highlightsrgb[3], float shadowscontrast = 1.0f, float midtonescontrast = 1.0f, float highlightscontrast = 1.0f);
	virtual ~gShadowsMidtonesHighlights();

	void use();

protected:
	const std::string getVertSrc();
	const std::string getFragSrc();
};

#endif /* GRAPHICS_POSTEFFECTS_GSHADOWSMIDTONESHIGHLIGHTS_H_ */
