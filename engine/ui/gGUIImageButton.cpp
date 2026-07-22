/*
 * gGUIImageButton.cpp
 *
 *  Created on: 21 Tem 2022
 *      Author: Medine, Yasin
 */

#include "gGUIImageButton.h"

#include <algorithm>

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
	iscircularbackground = false;
	circularbackgroundcolor = gColor(0.129f, 0.588f, 0.953f);
}

gGUIImageButton::~gGUIImageButton() {

}

void gGUIImageButton::draw() {
	gColor oldcolor = *renderer->getColor();
	const int drawleft = getButtonDrawLeft();
	const int drawtop = getButtonDrawTop();
	const bool moderncontent = contentcentered || iscircularbackground;
	if(iscircularbackground) {
		renderer->setColor(&circularbackgroundcolor);
		gDrawCircle(drawleft + buttonw / 2, drawtop + buttonh / 2,
				std::max(0, std::min(buttonw, buttonh) / 2 - 3), true, 40);
		renderer->setColor(&oldcolor);
	}
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
    if(moderncontent) {
		const int iconinset = iscircularbackground ? 14 : 8;
		if(isPressed()) {
			if(pressediconid != gGUIResources::ICON_NONE) res.getIconImage(pressediconid, isiconbig)->draw(drawleft + iconinset, drawtop + ispressed + iconinset, buttonw - iconinset * 2, buttonh - iconinset * 2);
			else pressedbuttonimage.draw(drawleft, drawtop + ispressed, buttonw, buttonh);
		} else {
			if(iconid != gGUIResources::ICON_NONE) res.getIconImage(iconid, isiconbig)->draw(drawleft + iconinset, drawtop + ispressed + iconinset, buttonw - iconinset * 2, buttonh - iconinset * 2);
			else buttonimage.draw(drawleft, drawtop + ispressed, buttonw, buttonh);
		}
    } else if(isPressed()) {
		// Keep the historical image/icon state mapping and dimensions for existing
		// desktop applications. The corrected mapping is opt-in with modern content.
		if(iconid != gGUIResources::ICON_NONE) res.getIconImage(iconid, isiconbig)->draw(left + 8, top + ispressed + 8, buttonw - 20, buttonh - 20);
		else buttonimage.draw(left, top + ispressed, buttonw, buttonh);
    } else {
		if(pressediconid != gGUIResources::ICON_NONE) res.getIconImage(pressediconid, isiconbig)->draw(left + 8, top + ispressed + 8, buttonw - 20, buttonh - 20);
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

void gGUIImageButton::setCircularBackground(bool enabled, const gColor& color) {
	iscircularbackground = enabled;
	circularbackgroundcolor = color;
}
