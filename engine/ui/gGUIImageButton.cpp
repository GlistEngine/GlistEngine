/*
 * gGUIImageButton.cpp
 *
 *  Created on: 21 Tem 2022
 *      Author: Medine, Yasin
 */

#include "gGUIImageButton.h"

gGUIImageButton::gGUIImageButton() {

	imagew = 0;
	imageh = 0;
	proportion = 0;
	stretch = true;
	isiconbig = false;
	buttonimagepath = "";
	pressedbuttonimagepath = "";

	iconid = gGUIResources::ICON_NONE;
	pressediconid = gGUIResources::ICON_NONE;
}

gGUIImageButton::~gGUIImageButton() {

}

void gGUIImageButton::draw() {
    if(!stretch) {
    	 	imagew = pressedbuttonimage.getWidth();
            imageh = pressedbuttonimage.getHeight();
            if(imageh > buttonh) {
                proportion = imagew / imageh;
                imageh = buttonh;
                imagew = proportion *imageh;
            }
            if(imagew > buttonw) {
                proportion = imageh / imagew;
                imagew = buttonw;
                imageh = proportion * imagew;
            }
        }
        else {
            imagew = buttonw;
            imageh = buttonh;
        }
    if(isPressed()) {
    	if (iconid != gGUIResources::ICON_NONE) res.getIconImage(iconid, isiconbig)->draw(left + 8, top + ispressed + 8, buttonw - 20, buttonh - 20);
    	else buttonimage.draw(left, top + ispressed, buttonw, buttonh);
    }
    else {
    	if (pressediconid != gGUIResources::ICON_NONE) res.getIconImage(pressediconid, isiconbig)->draw(left + 8, top + ispressed + 8, buttonw - 20, buttonh - 20);
    	else pressedbuttonimage.draw(left, top + ispressed, buttonw, buttonh);
    }
    setSize(imagew, imageh);
}


void gGUIImageButton::setImageStretched(bool stretchMod) {
    stretch = stretchMod;
}

   void gGUIImageButton::loadButtonImages(const std::string& imagePath){
	   buttonimage.loadImage(imagePath);
	   pressedbuttonimage.loadImage(imagePath);
	   buttonimagepath = imagePath;
   }

   void gGUIImageButton::loadPressedButtonImages(const std::string& imagePath){
	   pressedbuttonimage.loadImage(imagePath);
	   pressedbuttonimagepath = imagePath;
   }

   void gGUIImageButton::setButtonImage(gImage setImage){
	   buttonimage = setImage;
   }

   void gGUIImageButton::setPressedButtonImage(gImage setImage){
	   pressedbuttonimage = setImage;
   }

   int gGUIImageButton::getButtonWidth() {
   	return imagew ;
   }
   int gGUIImageButton::getButtonHeight() {
   	return imageh ;
   }
   std::string gGUIImageButton::getButtonImagePath() {
   	return buttonimagepath;
   }
   std::string gGUIImageButton::getPressedButtonImagePath() {
    return pressedbuttonimagepath;
   }

   void gGUIImageButton::setButtonImageFromIcon(int iconId, bool isIconBig){
   	iconid = iconId;
   	isiconbig = isIconBig;
   }
   void gGUIImageButton::setPressedButtonImageFromIcon(int pressedIconId, bool isIconBig){
   	pressediconid = pressedIconId;
   	isiconbig = isIconBig;
   }
