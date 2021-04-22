/*
 * gImage.h
 *
 *  Created on: Oct 25, 2020
 *      Author: noyan
 */

#ifndef ENGINE_GRAPHICS_GIMAGE_H_
#define ENGINE_GRAPHICS_GIMAGE_H_

#include "gTexture.h"


class gImage : public gTexture {
public:
	gImage();
	virtual ~gImage();

	unsigned int load(std::string fullPath, bool isHDR = false);
	unsigned int loadImage(std::string imagePath, bool isHDR = false);

    void setImageData(unsigned char* imageData);
    unsigned char* getImageData();

    void setImageDataHDR(float* imageData);
    float* getImageDataHDR();

    void clearData();
};

#endif /* ENGINE_GRAPHICS_GIMAGE_H_ */
