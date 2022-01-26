/*
 * gGUICheckbox.cpp
 *
 *  Created on: Nov 5, 2021
 *      Author: Ahmet Melih
 */

#include "gGUICheckbox.h"
#include "gBaseCanvas.h"

gGUICheckbox::gGUICheckbox() {
//	gLogi("id") << id;
	ischecked = true;
	istextvisible = true;
	buttonw = 32;
	buttonh = 32;
	bcolor = gColor(0.1f, 0.45f, 0.87f);
	tickcolor = gColor::WHITE;
	titlecolor = gColor::WHITE;
	title = "Checkbox";
	titlew = font->getStringWidth(title);
	titleh = font->getStringHeight(title);
}

gGUICheckbox::~gGUICheckbox() {

}

void gGUICheckbox::setTitle(std::string title) {
	gBaseGUIObject::setTitle(title);
	titlew = font->getStringWidth(title);
	titleh = font->getStringHeight(title);
}

void gGUICheckbox::setSize(int width, int height) {
	buttonw = width;
	buttonh = height;
}

void gGUICheckbox::setPosition(int left, int top) {
	this->left = left;
	this->top = top;
}

bool gGUICheckbox::isChecked() {
	return ischecked;
}

bool gGUICheckbox::isTextVisible() {
	return istextvisible;
}

void gGUICheckbox::setButtonColor(gColor color) {
	bcolor = color;
}

void gGUICheckbox::setTickColor(gColor color) {
	tickcolor = color;
}

gColor* gGUICheckbox::getButtonColor() {
	return &bcolor;
}

gColor* gGUICheckbox::getTickColor() {
	return &tickcolor;
}

void gGUICheckbox::update() {
//	gLogi("gGUICheckbox") << "update";
}

void gGUICheckbox::draw() {
//	gLogi("gGUICheckbox") << "draw";
	renderer->setColor(bcolor);
	gDrawRectangle(left, top, buttonw, buttonh, true);
	renderer->setColor(255, 255, 255);

	if (ischecked) {
		renderer->setColor(tickcolor);
		gDrawLine(left, top + buttonh / 2, left + buttonw / 2, top + buttonh);
		gDrawLine(left + buttonw / 2, top + buttonh, left + buttonw, top);
		renderer->setColor(255, 255, 255);
	}

	if (istextvisible) {
		renderer->setColor(titlecolor);
		font->drawText(title, left + buttonw, top + titleh);
		renderer->setColor(255, 255, 255);
	}
}

void gGUICheckbox::mousePressed(int x, int y, int button) {
//	gLogi("gGUICheckbox") << "pressed, id:" << id;
	if(x >= left && x < left + buttonw && y >= top && y < top + buttonh) {
		ischecked = !ischecked;
	}
	root->getCurrentCanvas()->onGuiEvent(id, GUIEVENT_BUTTONPRESSED);
}

void gGUICheckbox::mouseReleased(int x, int y, int button) {
//	gLogi("gGUICheckbox") << "released, id:" << id;
	if(x >= left && x < left + buttonw && y >= top && y < top + buttonh) {

	}
	root->getCurrentCanvas()->onGuiEvent(id, GUIEVENT_BUTTONRELEASED);
}

