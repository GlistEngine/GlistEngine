/*
 * gGUIPicturebox.cpp
 *
 *  Created on: 20 Tem 2022
 *      Author: TheGo
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
	if(!stretch) {

			imagew = image.getWidth();
			imageh = image.getHeight();
			if(imageh > height) {
				proportion = imagew / imageh;
				imageh = height;
				imagew = proportion * imageh;
			}
			if(imagew > width) {
				proportion = imageh / imagew;
				imagew = width;
				imageh = proportion * imagew;
			}
		}
		else {
			imagew = width;
			imageh = height;
		}
	image.draw(left + setsizex, top + setsizey, imagew * setsizew, imageh * setsizeh);



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

void gGUIBitmap::setImage(gImage setImage, bool isProportional) {
	image=setImage;
	stretch = isProportional;

}

int gGUIBitmap::getImageWidth() {
	return imagew ;
}
int gGUIBitmap::getImageHeight() {
	return imageh ;
}

std::string gGUIBitmap::getImagePath() {
	return imagepath;
}

