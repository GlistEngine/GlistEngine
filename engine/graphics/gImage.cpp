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


gImage::gImage() {
	ishdr = false;
	loadedfromurl = false;
	imageurl = "";
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
#ifdef ANDROID
		AAsset* asset = gAndroidUtil::loadAsset(fullpath, 0);
		auto* buf = (unsigned char*) AAsset_getBuffer(asset);
		int length = AAsset_getLength(asset);
		data = stbi_load_from_memory(buf, length, &width, &height, &componentnum, 0);
		gAndroidUtil::closeAsset(asset);
#else
		data = stbi_load(fullpath.c_str(), &width, &height, &componentnum, 0);
#endif
		setData(data, true);
	}

	//	setupRenderData();
	return id;
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

	//	setupRenderData();
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
	masktexture = new gTexture();
	ismaskloaded = true;
	return masktexture->load(gGetImagesDir() + maskImagePath);
}


bool gImage::gImage::checkPixelPerfectCollision(gImage *otherImage,
		float imgposX, float imgposY, float otherimgposX, float otherimgposY,
		float imgrot, float otherimgrot) {
unsigned char* data2 = otherImage->data;
	if (checkCollision(imgposX, imgposY, (imgposX + getWidth()), (imgposY + getHeight()), otherimgposX, otherimgposY, (otherimgposX + otherImage->getWidth()), (otherimgposY + otherImage->getHeight()))) {
		int overlapleft = std::max(imgposX, otherimgposX);
		int overlapright = std::min(imgposX + getWidth(), otherimgposX + otherImage->getWidth());
		int overlaptop = std::max(imgposY, otherimgposY);
		int overlapbot = std::min(imgposY + getHeight(), otherimgposY + otherImage->getHeight());

		for(int r = overlapleft; r < overlapright; ++r) {

			for (int c = overlaptop; c < overlapbot; ++c) {
				//findout which row, then add collumn finally multipy with number of channel
				int pixelIndex1 =  ((c - imgposY) * getWidth() + (r - imgposX)) * componentnum;
				int pixelIndex2 = ((c - otherimgposY) * otherImage->getWidth() + (r - otherimgposX)) * otherImage->getComponentNum();
				// get to the A channel and check if it is tranparent, only works for PNGs
				if(data[pixelIndex1 + 3] != 0 && data2[pixelIndex2 + 3] != 0 )
				{
					return true;
				}

			}

		}

	}
	return false;
}
