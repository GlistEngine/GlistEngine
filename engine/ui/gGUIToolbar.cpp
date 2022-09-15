/*
 * gGUIToolbar.cpp
 *
 *  Created on: Sep 25, 2021
 *      Author: noyan
 */

#include "gGUIToolbar.h"


gGUIToolbar::gGUIToolbar() {
	sizerrescaling = false;
	toolbartype = TOOLBAR_HORIZONTAL;
}

gGUIToolbar::~gGUIToolbar() {
}

void gGUIToolbar::setToolbarType(int toolbarType) {
	toolbartype = toolbarType;
}

int gGUIToolbar::getToolbarType() {
	return toolbartype;
}

void gGUIToolbar::draw() {
//	gLogi("gGUIToolbar") << "draw";
//	gLogi("gGUIToolbar") << "l:" << left << ", t:" << top << ", w:" << width << ", h:" << height;
	gColor oldcolor = *renderer->getColor();
	if(toolbartype == TOOLBAR_HORIZONTAL) {
		renderer->setColor(foregroundcolor);
		gDrawRectangle(left, top, width, height, true);
		renderer->setColor(backgroundcolor);
		gDrawLine(left, bottom, right, bottom);
	//	gDrawRectangle(left, top, width, height, false);
	} else {
		renderer->setColor(foregroundcolor);
		gDrawRectangle(left, top, width, height, true);
		renderer->setColor(backgroundcolor);
		gDrawLine(right, top, right, bottom);
	}
	renderer->setColor(&oldcolor);
	if(guisizer) guisizer->draw();
}

