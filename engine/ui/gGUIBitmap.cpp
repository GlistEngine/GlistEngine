/*
 * gGUIPicturebox.cpp
 *
 *  Created on: 20 Tem 2022
 *      Author: Yasin, Medine
 */

#include "gGUIBitmap.h"

gGUIBitmap::gGUIBitmap() {
	imagew = 1;
	imageh = 1;
	proportion = 0;
	stretch = true;
	setsizex = 0;
	setsizey = 0;
	setsizew = 1;
	setsizeh = 1;

}

gGUIBitmap::~gGUIBitmap() {

}

void gGUIBitmap::draw() {
	gColor* oldcolor = renderer->getColor();
	gColor white(1.0f, 1.0f, 1.0f, 1.0f);
	renderer->setColor(&white);

	int cx = 0;
	int cy = 0;

	if(!stretch) {
		float scaleW = (float)width / image.getWidth();
		float scaleH = (float)height / image.getHeight();
		float minScale = scaleW < scaleH ? scaleW : scaleH;

		imagew = image.getWidth() * minScale;
		imageh = image.getHeight() * minScale;

		cx = (width - imagew) / 2;
		cy = (height - imageh) / 2;
	} else {
		imagew = width;
		imageh = height;
	}

	image.draw(left + cx + setsizex, top + cy + setsizey, imagew * setsizew, imageh * setsizeh);

	renderer->setColor(oldcolor);
}

void gGUIBitmap::setImageSize(int x, int y, float scalex, float scaley) {
	setsizex = x;
	setsizey = y;
	setsizew = scalex;
	setsizeh = scaley;
}

void gGUIBitmap::loadImage(const std::string& imagePath , bool isProportional) {
	imagepath = imagePath;
	image.loadImage(imagepath);
	stretch = isProportional;
}

void gGUIBitmap::load(const std::string &fullPath, bool isProportional) {
	imagepath = fullPath;
	image.load(imagepath);
	stretch = isProportional;
}

void gGUIBitmap::setImage(gImage setImage, bool isProportional) {
	image = setImage;
	stretch = isProportional;
}

int gGUIBitmap::getImageWidth() {
	return imagew;
}
int gGUIBitmap::getImageHeight() {
	return imageh;
}

std::string gGUIBitmap::getImagePath() {
	return imagepath;
}


