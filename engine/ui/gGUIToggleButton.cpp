/*
 * gGUIToggleButton.cpp
 *
 *  Created on: 18 Tem 2022
 *      Author: utkus
 */

#include "gGUIToggleButton.h"
#include "gBaseApp.h"
#include "gBaseCanvas.h"

gGUIToggleButton::gGUIToggleButton() {
	togglew = 64;
	toggleh = 24;
	ison = false;
	isdisabled = false;
}

gGUIToggleButton::~gGUIToggleButton() {

}

void gGUIToggleButton::draw() {
	gColor oldcolor = renderer->getColor();
	renderer->setColor(middlegroundcolor);
	gDrawRectangle(left, top, togglew, toggleh);
	renderer->setColor(buttoncolor);
	gDrawRectangle(left + 1 + ison * (togglew / 2 - 2), top + 1, togglew / 2 - 1, toggleh - 3, true);
	renderer->setColor(oldcolor);
}

void gGUIToggleButton::mousePressed(int x, int y, int button) {
//	gLogi("Button") << "pressed, id:" << id;
	if(isdisabled) return;
	if(x >= left && x < left + togglew && y >= top && y < top + toggleh) {
		ison = !ison;
		root->getCurrentCanvas()->onGuiEvent(id, G_GUIEVENT_TOGGLE_OFF - ison);
	}
}

bool gGUIToggleButton::isOn() {
	return ison;
}
