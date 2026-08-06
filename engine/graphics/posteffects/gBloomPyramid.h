/*
 * gBloomPyramid.h
 *
 *  Created on: 1 Agu 2026
 *      Author: Numan Affan Kahya
 */

#ifndef GRAPHICS_POSTEFFECTS_GBLOOMPYRAMID_H_
#define GRAPHICS_POSTEFFECTS_GBLOOMPYRAMID_H_

#include "gBasePostProcess.h"
#include "gFbo.h"
#include <vector>
#include <string>

/**
 * @class gBloomPyramid
 *
 * Unlike traditional single-pass Gaussian blur blooms which are highly dependent on
 * kernel size and can bottleneck GPU texture fetching, this implementation uses a
 * downsample/upsample mip-chain.
 * This allows for extremely wide, physically accurate glowing effects with minimal performance cost.
 *
 * 	  **Filtering Modes (Highly recommended for LDR engines):**
 * 1. **Luminance Threshold (Default):** Blooms pixels that are brighter than the `threshold`.
 * 2. **Alpha Mask (Emissive Masking):** By calling `setAlphaMask(true)`, the engine will use
 *    the FBO's Alpha channel as an emissive multiplier (`RGB * Alpha`). Draw your glowing objects
 *    (e.g., car taillights, neon signs) with an Alpha of 1.0, and clear your background with an Alpha of 0.0.
 * 3. **RGB Mask:** Isolates and blooms only specific colors (e.g., only red pixels) regardless of global threshold.
 */


class gBloomPyramid: public gBasePostProcess {
public:

	/**
	 * @brief Constructs a Luminance/Alpha based Bloom Pyramid (Auto-sized).
	 *
	 * @param miplevels Number of downsample/upsample passes. Higher values create a wider, softer glow. (Recommended: 5-7)
	 * @param intensity Global multiplier for the final bloom output added to the scene.
	 * @param threshold The brightness cutoff point. Pixels with luminance below this value will not bloom. (Ignored if Alpha Mask is enabled).
	 * @param knee The soft-knee transition value. Smooths out harsh cutoffs around the threshold (0.0 = hard cutoff, 0.5 = very smooth transition).
	 * @param bloomsize Radius multiplier for the tent filter during the upsampling pass.
	 */


	gBloomPyramid(int miplevels = 6, float intensity = 1.0f,
		              float threshold = 1.0f, float knee = 0.2f, float bloomsize = 1.0f);

	/**
	 * @brief Constructs a Luminance/Alpha based Bloom Pyramid with specific FBO dimensions.
	 *
	 * @param screenwidth Width of the internal FBO mip-chain.
	 * @param screenheight Height of the internal FBO mip-chain.
	 */

	gBloomPyramid(int screenwidth, int screenheight, int miplevels = 6,
				  float intensity = 1.0f, float threshold = 1.0f,
				  float knee = 0.2f, float bloomsize = 1.0f);

	/**
	 * @brief Constructs an RGB Mask based Bloom Pyramid (Auto-sized).
	 *
	 * Isolates a specific color in the scene and applies bloom only to pixels matching that color.
	 *
	 * @param rgb The target RGB color to isolate and bloom (values 0.0f - 1.0f).
	 * @param miplevels Number of downsample/upsample passes.
	 * @param intensity Global multiplier for the final bloom output.
	 * @param bloomsize Radius multiplier for the tent filter.
	 * @param colorSoftness Tolerance distance for the RGB mask. Higher values allow slightly different shades of the target color to bloom.
	 * @param minGlowBrightness The minimum luminance a pixel must have to be considered for blooming, even if it matches the RGB mask.
	 */

	gBloomPyramid(float rgb[3], int miplevels = 6, float intensity = 1.0f,
				  float bloomsize = 1.0f, float colorSoftness = 0.0f, float minGlowBrightness = 0.0f);

	gBloomPyramid(int screenwidth, int screenheight, int miplevels,
				  float intensity, float bloomsize, float rgb[3],
				  float colorSoftness = 0.0f, float minGlowBrightness = 0.0f);

	virtual ~gBloomPyramid();
	void use() override;

	/**
	 * @brief Executes the multi-pass bloom rendering pipeline.
	 *
	 * 1. Prefilter (Threshold/Alpha/RGB Masking) -> Mip 0
	 * 2. Downsample Chain (13-tap box filter)
	 * 3. Upsample Chain (3x3 tent filter)
	 * 4. Composite (Additive blend with original scene)
	 */

	void render(gFbo& src, gFbo& dst) override;

	void setIntensity(float newintensity);
	void setThreshold(float newthreshold);
	void setKnee(float newknee);
	void setBloomSize(float newbloomsize);
	void setRgbMask(float r, float g, float b);
	void setColorSoftness(float newcolorsoftness);
	void setMinGlowBrightness(float newminglowbrightness);
	void setAlphaMask(bool enabled);

	void disableRgbMask();
	void disableAlphaMask();
	void resize(int screenwidth, int screenheight);

protected:
	const std::string getVertSrc() override;

	const std::string getFragSrc() override;

private:
	const std::string getPrefilterFragSrc();
	const std::string getDownsampleFragSrc();
	const std::string getUpsampleFragSrc();
	const std::string getCompositeFragSrc();

	void allocateMips(int screenwidth, int screenheight);
	void freeMips();

	void mipSize(int level, int& outw, int& outh) const;

	int miplevels;
	int screenwidth, screenheight;

	float intensity;
	float threshold;
	float knee;
	float bloomsize;

	bool isrgbmask;
	bool isalphamask;
	float rgbmask[3];
	float colorsoftness;
	float minglowbrightness;

	gShader* prefiltershader;
	gShader* downsampleshader;
	gShader* upsampleshader;


	gFbo* downfbo;
	gFbo* upfbo;
};

#endif /* GRAPHICS_POSTEFFECTS_GBLOOMPYRAMID_H_ */
