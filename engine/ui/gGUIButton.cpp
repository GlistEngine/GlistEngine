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
	buttonw = 80;
	buttonh = 24;
	title = "Button";
	resetTitlePosition();
}

gGUIButton::~gGUIButton() {
}

void gGUIButton::setTitle(std::string title) {
	gBaseGUIObject::setTitle(title);
	resetTitlePosition();
}

void gGUIButton::update() {
//	gLogi("gGUIButton") << "update";
}

void gGUIButton::draw() {
//	gLogi("gGUIButton") << "draw, w:" << width;
	gColor oldcolor = *renderer->getColor();
	renderer->setColor(gColor(0.1f, 0.45f, 0.87f));
	gDrawRectangle(left, top + ispressed, buttonw, buttonh, true);

	renderer->setColor(*fontcolor);
	font->drawText(title, left + tx - 1, top + buttonh - ty + ispressed - 2);
	renderer->setColor(oldcolor);
}

void gGUIButton::mousePressed(int x, int y, int button) {
//	gLogi("Button") << "pressed, id:" << id;
	if(x >= left && x < left + buttonw && y >= top && y < top + buttonh) ispressed = true;
	root->getCurrentCanvas()->onGuiEvent(id, GUIEVENT_BUTTONPRESSED);
}

void gGUIButton::mouseReleased(int x, int y, int button) {
//	gLogi("Button") << "released, id:" << id;
	ispressed = false;
	root->getCurrentCanvas()->onGuiEvent(id, GUIEVENT_BUTTONRELEASED);
}

void gGUIButton::resetTitlePosition() {
	tx = (buttonw - font->getStringWidth(title)) / 2;
	ty = (buttonh - font->getStringHeight("a")) / 2;
}

