/*
 * gGUIRadioButton.cpp
 *
 *  Created on: 19 Tem 2022
 *      Author: Utku Sarýalan
 */

#include "gGUIRadioButton.h"
#include "gBaseApp.h"
#include "gBaseCanvas.h"

gGUIRadioButton::gGUIRadioButton() {
	istextvisible = true;
	istitlevisible = true;
	isdisabled = false;
	buttonradius = 6.5f;
	buttoncount = 1;
	titlecolor = *fontcolor;
	selectedcolor = gColor(26 / 255.0f, 115 / 255.0f, 222 / 255.0f);
	title = "Radio Buttons";
	selectedbutton = 0;
	titles = new std::string;
	*titles = {"Radio button"};
	titlew = font->getStringWidth(title);
	titleh = font->getStringHeight(title);
	horizontaldistance = 8;
	verticaldistance = 5;
	columncount = 1;
	maxradiowidth = titlew + buttonradius * 2 + horizontaldistance;

	linemidpoint = buttonradius;
	textmargin = buttonradius - titleh / 2;
	buttonmargin = 0;
	if(buttonradius < titleh / 2) {
		linemidpoint = titleh / 2;
		buttonmargin = textmargin;
		textmargin = 0;
	}
	lineheightlimit = linemidpoint * 2 + verticaldistance;
}

gGUIRadioButton::~gGUIRadioButton() {
	delete(titles);
}

void gGUIRadioButton::setTitle(std::string title) {
	gBaseGUIObject::setTitle(title);
	titlew = font->getStringWidth(title);
	titleh = font->getStringHeight(title);
}

void gGUIRadioButton::setRadioTitle(int index, std::string title) {
	if(index >= buttoncount || index < 0) return;
	titles[index] = title;

	int temp = font->getStringWidth(title);
	if(temp > titlew) {
		titlew = temp;
		maxradiowidth = titlew + buttonradius * 2 + 2 + horizontaldistance;
	}
}

void gGUIRadioButton::setButtonRadius(float radius) {
	buttonradius = radius;
	updateHeightLimit();
	maxradiowidth = titlew + buttonradius * 2 + horizontaldistance;
}

void gGUIRadioButton::setButtonCount(int buttoncount) {
	if(buttoncount <= 0) buttoncount = 1;
	delete(titles);
	titles = new std::string[buttoncount];
	for(int i = 0; i < buttoncount; i++) {
		titles[i] = "Radio Button";
	}

	this->buttoncount = buttoncount;
}

void gGUIRadioButton::setHorizontalDistance(int distance) {
	horizontaldistance = distance;
}

void gGUIRadioButton::setVerticalDistance(int distance) {
	verticaldistance = distance;
	lineheightlimit = linemidpoint * 2 + verticaldistance;
}

void gGUIRadioButton::setSelectedButtonColor(gColor selectedcolor) {
	this->selectedcolor = selectedcolor;
}

void gGUIRadioButton::setColumnCount(int columncount) {
	this->columncount = columncount;
	if(columncount <= 0) this->columncount = 1;
}

void gGUIRadioButton::setSelectedButton(int index) {
	selectedbutton = index;
	if(selectedbutton >= buttoncount || selectedbutton < 0) selectedbutton = 0;
}

void gGUIRadioButton::setDisabled(bool command) {
	isdisabled = command;
}

bool gGUIRadioButton::isDisabled() {
	return isdisabled;
}

std::string gGUIRadioButton::getTitle() {
	return title;
}

int gGUIRadioButton::getButtonRadius() {
	return buttonradius;
}

int gGUIRadioButton::getButtonCount() {
	return buttoncount;
}

std::string gGUIRadioButton::getRadioTitle(int index) {
	if(index >= buttoncount) return "Error: Index out of range\n";
	return titles[index];
}

int gGUIRadioButton::getHorizontalDistance() {
	return horizontaldistance;
}

int gGUIRadioButton::getVerticalDistance() {
	return verticaldistance;
}

int gGUIRadioButton::getSelectedButton() {
	return selectedbutton;
}

gColor* gGUIRadioButton::getSelectedButtonColor() {
	return &selectedcolor;
}

int gGUIRadioButton::getColumnCount() {
	return columncount;
}

void gGUIRadioButton::showTitle(bool command) {
	istitlevisible = command;
}

void gGUIRadioButton::updateHeightLimit() {
	linemidpoint = buttonradius;
	textmargin = (buttonradius - titleh) / 2;
	buttonmargin = 0;
	if(buttonradius < titleh / 2) {
		linemidpoint = titleh / 2;
		buttonmargin = textmargin;
		textmargin = 0;
	}
	lineheightlimit = linemidpoint * 2 + verticaldistance;
}

bool gGUIRadioButton::isTextVisible() {
	return istextvisible;
}

void gGUIRadioButton::update() {
//	gLogi("gGUICheckbox") << "update";
}

void gGUIRadioButton::draw() {
	gColor oldcolor = renderer->getColor();
	float index = 0.0f;

	renderer->setColor(fontcolor);
	font->drawText(title, left, top);

	for(int i = 0; i < buttoncount; i++) {

		renderer->setColor(fontcolor);

		if(selectedbutton != i) {
			gDrawCircle(left + buttonradius + 1 + (i % columncount) * maxradiowidth,
					top + titleh + buttonradius + i / columncount * lineheightlimit, buttonradius, true);

			renderer->setColor(foregroundcolor);
			gDrawCircle(left + buttonradius + 1 + (i % columncount) * maxradiowidth,
					top + titleh + buttonradius + i / columncount * lineheightlimit, buttonradius * 0.7f, true);
		} else {
			gDrawCircle(left + buttonradius + 1 + (i % columncount) * maxradiowidth,
					top + titleh + buttonradius + i / columncount * lineheightlimit, buttonradius, true);

			renderer->setColor(foregroundcolor);
			gDrawCircle(left + buttonradius + 1 + (i % columncount) * maxradiowidth,
					top + titleh + buttonradius + i / columncount * lineheightlimit, buttonradius * 0.75f, true);

			renderer->setColor(selectedcolor);
			gDrawCircle(left + buttonradius + 1 + (i % columncount) * maxradiowidth,
					top + titleh + buttonradius + i / columncount * lineheightlimit, buttonradius * 0.5f, true);
		}

		index++;

	}

	index = 0.0f;
	if(istextvisible) {
		for(int i = 0; i < buttoncount; i++) {
			renderer->setColor(fontcolor);
//			font->drawText(title, left + buttonradius * 2 + 2, top - 2 + (buttonradius * 2 + titleh) / 2 - textmargin + i * lineheightlimit);
			font->drawText(titles[i], left + buttonradius * 2 + 2 + (i % columncount) * maxradiowidth,
					top - 2 + titleh + buttonradius + titleh / 2 + i / columncount * lineheightlimit);

			index++;
		}
	}

	renderer->setColor(oldcolor);
}

void gGUIRadioButton::mousePressed(int x, int y, int button) {
//	gLogi("gGUICheckbox") << "pressed, id:" << id;

}

void gGUIRadioButton::mouseReleased(int x, int y, int button) {
//	gLogi("gGUICheckbox") << "released, id:" << id;
	int sidebysidemargin =  titlew + buttonradius * 2;
	float index = 0.0f;

	for(int i = 0; i < buttoncount; i++){
		if(x >= left + (i % columncount)  * sidebysidemargin && x < left + sidebysidemargin + (i % columncount)  * maxradiowidth &&
				y >= top + titleh + i / columncount * lineheightlimit && y < top + titleh + i / columncount * lineheightlimit + buttonradius * 2) {
			selectedbutton = i;
			// TODO Create a new GUI event for radio buttons
			root->getCurrentCanvas()->onGuiEvent(id, G_GUIEVENT_CHECKBOXTICKED);
		}

		index++;
	}
}
