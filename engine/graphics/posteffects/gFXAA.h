/*
 * gFXAA.cpp
 *
 *  Created on: 6 Agu 2026
 *      Author: Numan Affan Kahya
 *
 *  Post-process class implementing FXAA (Fast Approximate Anti-Aliasing).
 *  Follows the same single-pass gBasePostProcess pattern as gVignette.
 */

#ifndef GRAPHICS_POSTEFFECTS_GFXAA_H_
#define GRAPHICS_POSTEFFECTS_GFXAA_H_

#include "gBasePostProcess.h"

class gFXAA: public gBasePostProcess {
public:

	/**
	 * @note **Pipeline Order Recommendation:**
	 * For optimal results, FXAA should always be the **first** effect added to the
	 * gPostProcessManager. FXAA relies on the raw, unaltered luminance contrast of the
	 * scene to accurately detect jagged edges. If placed after effects that blur or scatter
	 * light (such as Bloom, Depth of Field, or Motion Blur), the original geometry edges
	 * will be lost, rendering the anti-aliasing ineffective. Always apply FXAA on the
	 * pristine frame before adding any lens or atmospheric effects!
	 *
	 *
	 * @param subpixelblend  Amount of sub-pixel aliasing removal (0.0 - 1.0).
	 *                       Higher values = softer image, more blur.
	 * @param edgethresholdmin Minimum local contrast to be considered an edge
	 *                          (removes shader work in flat areas).
	 * @param edgethresholdmax Maximum contrast threshold trades speed for quality.
	 *
	 * Example Quality Presets:
	 * - Extreme Quality: (1.0f, 0.02f, 0.0312f)
	 * - High Quality: (0.75f, 0.0312f, 0.0625f)
	 * - Fast/Low Quality: (0.25f, 0.0833f, 0.25f)
	 *
	 */

	gFXAA(float subpixelblend = 0.75f, float edgethresholdmin = 0.0312f, float edgethresholdmax = 0.125f);
	virtual ~gFXAA();

	void use();

	void setSubpixelBlend(float value);
	void setEdgeThresholdMin(float value);
	void setEdgeThresholdMax(float value);

protected:
	const std::string getVertSrc();
	const std::string getFragSrc();
	void render(gFbo& src, gFbo& dst) override;

private:
	float subpixelblend;
	float edgethresholdmin;
	float edgethresholdmax;
	unsigned int linearsampler;
};

#endif /* GRAPHICS_POSTEFFECTS_GFXAA_H_ */
