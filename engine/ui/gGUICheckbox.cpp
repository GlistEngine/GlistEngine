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
	ischecked = false;
	istextvisible = true;
	buttonw = 12;
	buttonh = 12;
	bcolor = *backgroundcolor;
	tickcolor = *fontcolor;
	titlecolor = *fontcolor;
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

void gGUICheckbox::setBgColor(gColor color) {
	bcolor = color;
}

void gGUICheckbox::setTickColor(gColor color) {
	tickcolor = color;
}

gColor* gGUICheckbox::getBgColor() {
	return &bcolor;
}

gColor* gGUICheckbox::getTickColor() {
	return &tickcolor;
}

void gGUICheckbox::setChecked(bool isChecked) {
	ischecked = isChecked;
}

void gGUICheckbox::update() {
//	gLogi("gGUICheckbox") << "update";
}

void gGUICheckbox::draw() {
	if(!ischecked) {
		renderer->setColor(&bcolor);
		gDrawRectangle(left, top, buttonw, buttonh, true);
	}
	renderer->setColor(middlegroundcolor);
	gDrawRectangle(left, top, buttonw, buttonh, false);
	renderer->setColor(255, 255, 255);

	if (ischecked) {
		renderer->setColor(26, 115, 222);
		gDrawRectangle(left, top, buttonw, buttonh, true);
//		renderer->setColor(fontcolor);
		renderer->setColor(255, 255, 255);
		gDrawLine(left + 2, top + buttonh / 2, left + buttonw / 2, top + buttonh - 2);
		gDrawLine(left + buttonw / 2, top + buttonh - 2, left + buttonw - 2, top + 2);
		renderer->setColor(255, 255, 255);
	}

	if (istextvisible) {
		renderer->setColor(fontcolor);
		font->drawText(title, left + buttonw, top - 2 + (buttonh + titleh) / 2);
		renderer->setColor(255, 255, 255);
	}
}

void gGUICheckbox::mousePressed(int x, int y, int button) {
//	gLogi("gGUICheckbox") << "pressed, id:" << id;
	if(x >= left && x < left + buttonw + titlew && y >= top && y < top + buttonh) {

	}
}

void gGUICheckbox::mouseReleased(int x, int y, int button) {
//	gLogi("gGUICheckbox") << "released, id:" << id;
	if(x >= left && x < left + buttonw + titlew && y >= top && y < top + buttonh) {
		ischecked = !ischecked;
		if(ischecked) root->getCurrentCanvas()->onGuiEvent(id, G_GUIEVENT_CHECKBOXTICKED);
		else root->getCurrentCanvas()->onGuiEvent(id, G_GUIEVENT_CHECKBOXUNTICKED);
	}
}

