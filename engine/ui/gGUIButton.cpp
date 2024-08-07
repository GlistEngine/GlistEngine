/*
 * gGUIButton.cpp
 *
 *  Created on: Aug 23, 2021
 *      Author: noyan
 */

#include "gGUIButton.h"
#include "gBaseApp.h"
#include "gBaseCanvas.h"


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
	if(fillbackground) {
		if(isdisabled) renderer->setColor(&disabledbcolor);
		else {
			if(ispressed) renderer->setColor(&pressedbcolor);
			else if(ishover) renderer->setColor(&hcolor);
			else renderer->setColor(&bcolor);
		}
	//	renderer->setColor(gColor(0.1f, 0.45f, 0.87f));
		gDrawRectangle(left, top + ispressed, buttonw, buttonh, true);
	}

	if(istextvisible) {
		if(isdisabled) renderer->setColor(&disabledfcolor);
		else {
			if(ispressed) renderer->setColor(&pressedfcolor);
			else renderer->setColor(&fcolor);
		}
		font->drawText(title, left + tx - 1, top + buttonh - ty + ispressed - 2);
	}
	renderer->setColor(oldcolor);
}

void gGUIButton::mousePressed(int x, int y, int button) {
//	gLogi("Button") << "pressed, id:" << id;
	if(isdisabled) return;
	if(x >= left && x < left + buttonw && y >= top && y < top + buttonh) {
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
	if(ispressed && x >= left && x < left + buttonw && y >= top && y < top + buttonh) {
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
	if(iscursoron && x >= left && y >= top && x < left + buttonw && y < top + buttonh) ishover = true;
	else ishover = false;
}

void gGUIButton::mouseDragged(int x, int y, int button) {
	if(iscursoron && x >= left && y >= top && x < left + buttonw && y < top + buttonh) ishover = true;
	else ishover = false;
}

void gGUIButton::mouseEntered() {

}

void gGUIButton::mouseExited() {
	ishover = false;
}

void gGUIButton::resetTitlePosition() {
	tx = (buttonw - font->getStringWidth(title)) / 2 - 1;
	ty = (buttonh - font->getStringHeight("a")) / 2;
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

bool& gGUIButton::isPressedRef() {
		return ispressed;
}
