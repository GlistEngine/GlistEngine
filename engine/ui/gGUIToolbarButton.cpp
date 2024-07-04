/*
 * gGUIToolbarButton.cpp
 *
 *  Created on: Sep 25, 2021
 *      Author: noyan
 */

#include "gGUIToolbarButton.h"
#include "gImage.h"


//gGUIResources gGUIToolbarButton::res;


gGUIToolbarButton::gGUIToolbarButton() {
	iconid = gGUIResources::ICON_NONE;
	setSize(32, 32);
	//setTitle("");
	setToggle(false);
	setDisabled(false);
	setTextVisibility(false);
	bcolor = *foregroundcolor;
	bcoloraddress = &foregroundcolor;
	pressedbcolor = *pressedbuttoncolor;
	pressedbcoloraddress = &pressedbuttoncolor;
//	disabledbuttoncolor = &disabledbcolor;
	fcolor = *fontcolor;
	fcoloraddress = &fontcolor;
	pressedfcolor = *pressedbuttonfontcolor;
	pressedfcoloraddress = &pressedbuttonfontcolor;
//	disabledbuttonfontcolor = &disabledfcolor;
	iw = buttonw * 2 / 4;
	ih = buttonh * 2 / 4;
	ix = (buttonw - iw) / 2;
	iy = (buttonh - ih) / 2;
}

gGUIToolbarButton::~gGUIToolbarButton() {
}

void gGUIToolbarButton::set(gBaseApp* root, gBaseGUIObject* topParentGUIObject, gBaseGUIObject* parentGUIObject, int parentSlotLineNo, int parentSlotColumnNo, int x, int y, int w, int h) {
	this->root = root;
	topparent = topParentGUIObject;
	parent = parentGUIObject;
	parentslotlineno = parentSlotLineNo;
	parentslotcolumnno = parentSlotColumnNo;
	left = x;
	top = y;
	right = x + h;
	bottom = y + h;
	width = h;
	height = h;
}

void gGUIToolbarButton::setImage(int iconId) {
	iconid = iconId;
}

void gGUIToolbarButton::draw() {
//	gLogi("gGUIToolbarButton") << "draw";
//	gLogi("gGUIToolbarButton") << "l:" << left << ", t:" << top << ", w:" << width << ", h:" << height << ", bw:" << buttonw << ", bh:" << buttonh;
	gColor* oldcolor = renderer->getColor();
	if(fillbackground) {
		if(isdisabled) {
			renderer->setColor(disabledbcolor);
		} else if(ispressed) {
			renderer->setColor(*pressedbcoloraddress);
		} else {
			renderer->setColor(*bcoloraddress);
		}
	//	renderer->setColor(gColor(0.1f, 0.45f, 0.87f));
		gDrawRectangle(left, top + ispressed, buttonw, buttonh - 2, true);
	}

	renderer->setColor(255, 255, 255);
	// icon image
	if(iconid != gGUIResources::ICON_NONE) {
		res.getIconImage(iconid)->draw(left + ix, top + iy, iw, ih);
	}
//	if(iconid != gGUIResources::ICON_NONE) res.getIconImage(iconid)->draw(left, top, buttonw, buttonh);

	if(istextvisible) {
		if(isdisabled) {
			renderer->setColor(disabledfcolor);
		} else if(ispressed) {
			renderer->setColor(*pressedfcoloraddress);
		} else {
			renderer->setColor(*fcoloraddress);
		}
		font->drawText(title, left + tx - 1, top + buttonh - ty + ispressed - 2);
	}
	renderer->setColor(oldcolor);
}
/*
void gGUIToolbarButton::mousePressed(int x, int y, int button) {
	gLogi("ToolbarButton") << "mousePressed";
}

void gGUIToolbarButton::mouseReleased(int x, int y, int button) {
	gLogi("ToolbarButton") << "mouseReleased";
}
*/
