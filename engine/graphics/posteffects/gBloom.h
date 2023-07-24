/*
 * gBloom.h
 *
 *  Created on: 19 Tem 2023
 *      Author: Batuhan Yigit
 */

#ifndef GRAPHICS_POSTEFFECTS_GBLOOM_H_
#define GRAPHICS_POSTEFFECTS_GBLOOM_H_

#include "gBasePostProcess.h"
#include <vector>

class gBloom: public gBasePostProcess {
public:
	gBloom(float intensity = 1.0f, float bloomsize = 2.0f, float threshold = 0.8f);
	gBloom(float intensity, float bloomsize, float threshold, std::vector<std::vector<float>> rects, float screenwidth, float screenheight);
	gBloom(float intensity, float bloomsize, float rgb[]);
	virtual ~gBloom();

	void use();
	void setRect(int rectnum, float x, float y, float width, float height);
	std::vector<std::vector<float>> rects;

protected:
	const std::string getVertSrc();
	const std::string getFragSrc();

private:
	bool isrectsset = false;
	int rectssize;
	int deneme;
};

#endif /* GRAPHICS_POSTEFFECTS_GBLOOM_H_ */
