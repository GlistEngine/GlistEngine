/*
 * gImage.cpp
 *
 *  Created on: Oct 25, 2020
 *      Author: noyan
 */

#include "gImage.h"
#include "stb/stb_image.h"


gImage::gImage() {

}

gImage::~gImage() {
}

unsigned int gImage::load(std::string fullPath) {
	fullpath = fullPath;
	directory = getDirName(fullpath);
	path = getFileName(fullpath);

    glGenTextures(1, &id);

    data = stbi_load(fullpath.c_str(), &width, &height, &componentnum, 0);
    setData(data, true);

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

void gImage::clearData() {
	stbi_image_free(data);
	ismutable = false;
}

