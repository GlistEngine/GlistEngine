/*
 * gImage.cpp
 *
 *  Created on: Oct 25, 2020
 *      Author: noyan
 */

#include "gImage.h"
#ifndef STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#endif
#include "stb/stb_image_write.h"
#include "gHttpFile.h"
#ifdef ANDROID
#include "gAndroidUtil.h"
#endif

int gImage::downloadno = 1;

gImage::gImage() : gImage(0, 0, GL_RGBA) {
}

gImage::gImage(int w, int h, int format) : gTexture(w, h, format, false) {
    ishdr = false;
    loadedfromurl = false;
    imageurl = "";
}

gImage::~gImage() {
#ifdef ANDROID
	if (androidasset) gAndroidUtil::closeAsset(androidasset);
#endif
}

unsigned int gImage::load(const std::string& fullPath) {
    return gTexture::load(fullPath);
}

unsigned int gImage::loadImage(const std::string& imagePath) {
	return load(gGetImagesDir() + imagePath);
}

unsigned int gImage::loadImageFromURL(const std::string& imageUrl) {
	return loadImageFromURL(imageUrl, true);
}

unsigned int gImage::loadImageFromURL(const std::string& imageUrl, bool cutUrlParameters) {
#if(ANDROID)
	return 0; // todo
#else
	imageurl = imageUrl;
	std::string imageurledited = imageurl;
	if(cutUrlParameters) {
		int firstmark = imageurledited.find_first_of('?');
		imageurledited = imageurledited.substr(0, firstmark);
	}
	int lastslash = imageurledited.find_last_of('/');
	std::string imagename = imageurledited.substr(lastslash + 1, imageurledited.size() - lastslash - 1);
	int lastdot = imagename.find_last_of('.');
	std::string imagetype = gToLower(imagename.substr(lastdot + 1, imagename.size() - lastdot - 1));

	std::string imagepath = generateDownloadedImagePath(imagetype);
	gHttpFile urlfile;
	urlfile.load(imageurl);
	urlfile.save(imagepath, true);
	loadedfromurl = true;

	return load(imagepath);
#endif
}

void gImage::loadImageData(const std::string& imagePath) {
	loadData(gGetImagesDir() + imagePath);
}

unsigned int gImage::useData() {
	gTexture::allocate();
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
	int lastdot = fileName.find_last_of('.');
	std::string imagetype = gToLower(fileName.substr(lastdot + 1, fileName.size() - lastdot - 1));

	if(imagetype == "png") {
	    stbi_write_png(path.c_str(), width, height, componentnum, data, width * componentnum * sizeof(unsigned char));
	} else if(imagetype == "jpg" || imagetype == "jpeg") {
	    stbi_write_jpg(path.c_str(), width, height, componentnum, data, 100);
	} else if(imagetype == "bmp") {
	    stbi_write_bmp(path.c_str(), width, height, componentnum, data);
	} else if(imagetype == "tga") {
	    stbi_write_tga(path.c_str(), width, height, componentnum, data);
	} else if(imagetype == "hdr") {
	    stbi_write_hdr(path.c_str(), width, height, componentnum, datahdr);
	} else {
	    stbi_write_png(path.c_str(), width, height, componentnum, data, width * componentnum * sizeof(unsigned char));
	}
}

std::string gImage::getImageUrl() {
	return imageurl;
}

std::string gImage::generateDownloadedImagePath(std::string imageType) {
#if(ANDROID)
	return ""; // todo
#else
	std::string imagepath = "";
	do {
		imagepath = gGetImagesDir() + "downloadedimage_" + gToStr(downloadno) + "." + imageType;
		downloadno++;
	} while(gFile::doesFileExist(imagepath));
	return imagepath;
#endif
}

unsigned int gImage::loadMaskImage(const std::string& maskImagePath) {
	return loadMask(gGetImagesDir() + maskImagePath);
}


