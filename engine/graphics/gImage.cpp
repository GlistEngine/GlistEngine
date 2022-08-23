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
#ifndef STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#endif
#include "stb/stb_image.h"
#include "stb/stb_image_write.h"


gImage::gImage() {
	ishdr = false;
}

gImage::gImage(int w, int h, int format) : gTexture(w, h, format, false) {
}

gImage::~gImage() {
}

unsigned int gImage::load(const std::string& fullPath) {
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

unsigned int gImage::loadImage(const std::string& imagePath) {
	return load(gGetImagesDir() + imagePath);
}

void gImage::loadData(const std::string& fullPath) {
	fullpath = fullPath;
	directory = getDirName(fullpath);
	path = getFileName(fullpath);
	ishdr = false;
	if (gToLower(fullpath.substr(fullpath.length() - 3, 3)) == "hdr") ishdr = true;

    if (ishdr) {
    	stbi_set_flip_vertically_on_load(true);
    	datahdr = stbi_loadf(fullpath.c_str(), &width, &height, &componentnum, 0);
    } else {
        data = stbi_load(fullpath.c_str(), &width, &height, &componentnum, 0);
    }
}

void gImage::loadImageData(const std::string& imagePath) {
	loadData(gGetImagesDir() + imagePath);
}

unsigned int gImage::useData() {
    glGenTextures(1, &id);

    if (ishdr) {
    	setDataHDR(datahdr, true);
    } else {
        setData(data, true);
    }

	setupRenderData();
    return id;
}

void gImage::setImageData(unsigned char* imageData) {
	setData(imageData, true);
}

void gImage::setImageData(unsigned char* imageData, int width, int height, int componentNum) {
	gTexture::loadData(imageData, width, height, componentNum);
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

void gImage::saveImage(std::string fileName) {
    std::string path = gGetImagesDir() + fileName;
    stbi_write_png(path.c_str(), width, height, componentnum, data, width * componentnum * sizeof(unsigned char));
}




