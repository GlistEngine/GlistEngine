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
	isstatic = true;
	issetupped = false;
	buttonw = 96;
	buttonh = 32;
	buttonx = left;
	buttony = top;
	buttonmargin = 0;
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
	freesans12.loadFont("FreeSans.ttf", 12);
	freesans18.loadFont("FreeSans.ttf", 18);
	freesans24.loadFont("FreeSans.ttf", 24);
	freesans36.loadFont("FreeSans.ttf", 36);
	freesans48.loadFont("FreeSans.ttf", 48);
	freesans60.loadFont("FreeSans.ttf", 60);
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
	if(issetupped == false) {
		if(!isstatic) {
			buttonx = left + buttonmargin;
			buttony = top + buttonmargin;
			buttonw = width - buttonmargin * 2;
			buttonh = height - buttonmargin * 2;
			resetTitlePosition();
		}else {
			buttonx = left;
			buttony = top;
		}
		issetupped = true;
	}
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
		if(buttonmargin > 0 && isstatic == false) {
			gDrawRectangle(buttonx, buttony + ispressed, buttonw, buttonh, true);
		}else {
			gDrawRectangle(left, top + ispressed, buttonw, buttonh, true);
		}
	}

	if(istextvisible) {
		if(isdisabled) renderer->setColor(&disabledfcolor);
		else {
			if(ispressed) renderer->setColor(&pressedfcolor);
			else renderer->setColor(&fcolor);
		}

		if((buttonw > 0) && (buttonh > 0))  fontsize = 12;
		if((buttonw > 100) && (buttonh > 50)) fontsize = 18;
		if((buttonw > 200) && (buttonh > 80)) fontsize = 24;
		if((buttonw > 300) && (buttonh > 110)) fontsize = 36;
		if((buttonw > 400) && (buttonh > 140)) fontsize = 48;
		if((buttonw > 500) && (buttonh > 170)) fontsize = 60;

		switch(fontsize) {
		case 12:
			freesans12.drawText(title, buttonx + tx - 1 - 12, buttony + buttonh - ty + ispressed - 2 );
		break;
		case 18:
			freesans18.drawText(title, buttonx + tx - 1 - 18, buttony + buttonh - ty + ispressed - 2 );
		break;
		case 24:
			freesans24.drawText(title, buttonx + tx - 1 - 24, buttony + buttonh - ty + ispressed - 2 );
		break;
		case 36:
			freesans36.drawText(title, buttonx + tx - 1 - 36, buttony + buttonh - ty + ispressed - 2 );
		break;
		case 48:
			freesans48.drawText(title, buttonx + tx - 1 - 48, buttony + buttonh - ty + ispressed - 2 );
		break;
		case 60:
			freesans60.drawText(title, buttonx + tx - 1 - 60, buttony + buttonh - ty + ispressed - 2 );
		break;
		}

	}
	renderer->setColor(oldcolor);
}

void gGUIButton::mousePressed(int x, int y, int button) {
//	gLogi("Button") << "pressed, id:" << id;
	if(isdisabled) return;
	if(x >= buttonx && x < buttonx + buttonw && y >= buttony && y < buttony + buttonh) {
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
	if(ispressed && x >= buttonx && x < buttonx + buttonw && y >= buttony && y < buttony + buttonh) {
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
	if(iscursoron && x >= buttonx && y >= buttony && x < buttonx + buttonw && y < buttony + buttonh) ishover = true;
	else ishover = false;
}

void gGUIButton::mouseDragged(int x, int y, int button) {
	if(iscursoron && x >= buttonx && y >= buttony && x < buttonx + buttonw && y < buttony + buttonh) ishover = true;
	else ishover = false;
}

void gGUIButton::mouseEntered() {

}

void gGUIButton::mouseExited() {
	ishover = false;
}

void gGUIButton::resetTitlePosition() {
	tx = (buttonw + buttonmargin - font->getStringWidth(title)) / 2 - 1;
	ty = (buttonh - buttonmargin - font->getStringHeight("a")) / 2;
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

void gGUIButton::setStaticness(bool isStatic) {
	isstatic = isStatic;
}

void gGUIButton::setButtonMargin(int margin) {
	buttonmargin = margin;
}
