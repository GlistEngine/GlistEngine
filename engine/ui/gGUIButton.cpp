/*
 * gGUIButton.cpp
 *
 *  Created on: Aug 23, 2021
 *      Author: noyan
 */

#include "gGUIButton.h"
#include "gBaseApp.h"
#include "gBaseCanvas.h"
#include <algorithm>

gGUIButton::gGUIButton() {
	ispressed = false;
	ishover = false;
	buttonw = 96;
	buttonh = 32;
	istoggle = false;
	title = "Button";
	istextvisible = true;
	ispressednow = false;
	isdisabled = false;
	bcolor = *buttoncolor;
//	gLogi("Button") << "fgr:" << foregroundcolor->r << ", fgg:" << foregroundcolor->g << ", fgb:" << foregroundcolor->b;
//	gLogi("Button") << "br:" << buttoncolor->r << ", bg:" << buttoncolor->g << ", bb:" << buttoncolor->b;
//	gLogi("Button") << "r:" << bcolor.r << ", g:" << bcolor.g << ", b:" << bcolor.b;
	pressedbcolor = *pressedbuttoncolor;
	hcolor.set((pressedbcolor.r + bcolor.r) / 2, (pressedbcolor.g + bcolor.g) / 2, (pressedbcolor.b + bcolor.b) / 2);
	disabledbcolor = *disabledbuttoncolor;
	fcolor = *buttonfontcolor;
	pressedfcolor = *pressedbuttonfontcolor;
	disabledfcolor = *disabledbuttonfontcolor;
	fillbackground = true;
	contentcentered = false;
	resetTitlePosition();
}

gGUIButton::~gGUIButton() {
}

void gGUIButton::setTitle(std::string title) {
    gBaseGUIObject::setTitle(title);
    resetTitlePosition();
}

void gGUIButton::setSize(int width, int height) {
	buttonw = width;
	buttonh = height;
	resetTitlePosition();
}

void gGUIButton::setTextVisibility(bool isVisible) {
	istextvisible = isVisible;
}

void gGUIButton::setToggle(bool isToggle) {
	istoggle = isToggle;
}

void gGUIButton::setDisabled(bool isDisabled) {
	isdisabled = isDisabled;
}

bool gGUIButton::isDisabled() {
	return isdisabled;
}


bool gGUIButton::isToggle() {
	return istoggle;
}

bool gGUIButton::isTextVisible() {
	return istextvisible;
}

bool gGUIButton::isPressed() {
	return ispressed;
}

void gGUIButton::update() {
//	gLogi("gGUIButton") << "update";
}

void gGUIButton::draw() {
//	gLogi("gGUIButton") << "draw, w:" << width;
	gColor* oldcolor = renderer->getColor();
	const int drawleft = getButtonDrawLeft();
	const int drawtop = getButtonDrawTop();
	if(fillbackground) {
		if(isdisabled) renderer->setColor(&disabledbcolor);
		else {
			if(ispressed) renderer->setColor(&pressedbcolor);
			else if(ishover) renderer->setColor(&hcolor);
			else renderer->setColor(&bcolor);
		}
	//	renderer->setColor(gColor(0.1f, 0.45f, 0.87f));
		gDrawRectangle(drawleft, drawtop + ispressed, buttonw, buttonh, true);
	}

	if(istextvisible) {
	    if(isdisabled) renderer->setColor(&disabledfcolor);
	    else {
	        if(ispressed) renderer->setColor(&pressedfcolor);
	        else renderer->setColor(&fcolor);
	    }

	    resetTitlePosition();

	    font->drawText(title, drawleft + tx, drawtop + buttonh - ty + ispressed - 2);
	}
	renderer->setColor(oldcolor);
}

void gGUIButton::mousePressed(int x, int y, int button) {
//	gLogi("Button") << "pressed, id:" << id;
	if(isdisabled) return;
	const int drawleft = getButtonDrawLeft();
	const int drawtop = getButtonDrawTop();
	if(x >= drawleft && x < drawleft + buttonw && y >= drawtop && y < drawtop + buttonh) {
		if(!istoggle) ispressed = true;
		else {
			if(!ispressed) {
				ispressed = true;
				ispressednow = true;
			}
		}
		root->getCurrentCanvas()->onGuiEvent(id, G_GUIEVENT_BUTTONPRESSED);
	}
}

void gGUIButton::mouseReleased(int x, int y, int button) {
//	gLogi("Button") << "released, id:" << id;
	if(isdisabled) return;
	const int drawleft = getButtonDrawLeft();
	const int drawtop = getButtonDrawTop();
	if(ispressed && x >= drawleft && x < drawleft + buttonw && y >= drawtop && y < drawtop + buttonh) {
		if(!istoggle) ispressed = false;
		else {
			if(!ispressednow) ispressed = false;
		}
		root->getCurrentCanvas()->onGuiEvent(id, G_GUIEVENT_BUTTONRELEASED);
		actionmanager.onGUIEvent(id, G_GUIEVENT_BUTTONRELEASED);
	} else {
		if(!istoggle) ispressed = false;
	}
	ispressednow = false;
}

void gGUIButton::mouseMoved(int x, int y) {
	const int drawleft = getButtonDrawLeft();
	const int drawtop = getButtonDrawTop();
	if(iscursoron && x >= drawleft && y >= drawtop && x < drawleft + buttonw && y < drawtop + buttonh) ishover = true;
	else ishover = false;
}

void gGUIButton::mouseDragged(int x, int y, int button) {
	const int drawleft = getButtonDrawLeft();
	const int drawtop = getButtonDrawTop();
	if(iscursoron && x >= drawleft && y >= drawtop && x < drawleft + buttonw && y < drawtop + buttonh) ishover = true;
	else ishover = false;
}

void gGUIButton::mouseEntered() {

}

void gGUIButton::mouseExited() {
	ishover = false;
}

void gGUIButton::resetTitlePosition() {
	float xmin = 0.0f, xmax = 0.0f;

	font->getVisualBoundsX(title, xmin, xmax);
	float visualw = xmax - xmin;

	tx = (int)std::round((buttonw - visualw) * 0.5f - xmin);
	ty = (buttonh - font->getStringHeight("a")) / 2;

	// clamp
	if(tx < 4) tx = 4;
	if(ty < 0) ty = 0;
}

void gGUIButton::setButtonColor(gColor color) {
	bcolor = color;
	hcolor.set((pressedbcolor.r + bcolor.r) / 2, (pressedbcolor.g + bcolor.g) / 2, (pressedbcolor.b + bcolor.b) / 2);
}

void gGUIButton::setPressedButtonColor(gColor color) {
	pressedbcolor = color;
	hcolor.set((pressedbcolor.r + bcolor.r) / 2, (pressedbcolor.g + bcolor.g) / 2, (pressedbcolor.b + bcolor.b) / 2);
}

void gGUIButton::setDisabledButtonColor(gColor color) {
	disabledbcolor = color;
}

void gGUIButton::setButtonFontColor(gColor color) {
	fcolor = color;
}

void gGUIButton::setPressedButtonFontColor(gColor color) {
	pressedfcolor = color;
}

void gGUIButton::setDisabledButtonFontColor(gColor color) {
	disabledfcolor = color;
}

gColor* gGUIButton::getButtonColor() {
	return &bcolor;
}

gColor* gGUIButton::getPressedButtonColor() {
	return &pressedbcolor;
}

gColor* gGUIButton::getDisabledButtonColor() {
	return &disabledbcolor;
}

gColor* gGUIButton::getButtonFontColor() {
	return &fcolor;
}

gColor* gGUIButton::getPressedButtonFontColor() {
	return &pressedfcolor;
}

gColor* gGUIButton::getDisabledButtonFontColor() {
	return &disabledfcolor;
}

void gGUIButton::enableBackgroundFill(bool isEnabled) {
	fillbackground = isEnabled;
}

void gGUIButton::setContentCentered(bool centered) {
	contentcentered = centered;
}

int gGUIButton::getButtonDrawLeft() const {
	return contentcentered ? left + std::max(0, (width - buttonw) / 2) : left;
}

int gGUIButton::getButtonDrawTop() const {
	return contentcentered ? top + std::max(0, (height - buttonh) / 2) : top;
}

int gGUIButton::getButtonWidth() {
	return buttonw;
}

int gGUIButton::getButtonHeight() {
	return buttonh;
}

void gGUIButton::setButtonh(int buttonh) {
	this->buttonh = buttonh;
}

void gGUIButton::setButtonw(int buttonw) {
	this->buttonw = buttonw;
}
