/*
 * gImage.cpp
 *
 *  Created on: Oct 25, 2020
 *      Author: noyan
 */

#include "gImage.h"
//#ifndef STB_IMAGE_IMPLEMENTATION
//#define STB_IMAGE_IMPLEMENTATION
//#endif
#include "stb/stb_image.h"


gImage::gImage() {

}

gImage::~gImage() {
}

unsigned int gImage::load(std::string fullPath) {
	fullpath = fullPath;
	directory = getDirName(fullpath);
	path = getFileName(fullpath);
	ishdr = false;
	if (gToLower(fullpath.substr(fullpath.length() - 3, 3)) == "hdr") ishdr = true;

    glGenTextures(1, &id);

    if (ishdr) {
    	stbi_set_flip_vertically_on_load(true);
    	datahdr = stbi_loadf(fullpath.c_str(), &width, &height, &componentnum, 0);
    	setDataHDR(datahdr, true);
    } else {
        data = stbi_load(fullpath.c_str(), &width, &height, &componentnum, 0);
        setData(data, true);
    }

	setupRenderData();
    return id;
}

unsigned int gImage::loadImage(std::string imagePath) {
	return load(gGetImagesDir() + imagePath);
}


void gImage::setImageData(unsigned char* imageData) {
	setData(imageData, true);
}

unsigned char* gImage::getImageData() {
	return getData();
}

void gImage::setImageDataHDR(float* imageData) {
	setDataHDR(imageData, true);
}

float* gImage::getImageDataHDR() {
	return getDataHDR();
}

void gImage::clearData() {
	if (ishdr) stbi_image_free(datahdr);
	else stbi_image_free(data);
}

