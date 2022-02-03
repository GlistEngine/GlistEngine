/*
 * gGUIToolbar.cpp
 *
 *  Created on: Sep 25, 2021
 *      Author: noyan
 */

#include "gGUIToolbar.h"


gGUIToolbar::gGUIToolbar() {
	sizerrescaling = false;
}

gGUIToolbar::~gGUIToolbar() {
}

void gGUIToolbar::draw() {
//	gLogi("gGUIToolbar") << "draw";
//	gLogi("gGUIToolbar") << "l:" << left << ", t:" << top << ", w:" << width << ", h:" << height;
	gColor oldcolor = *renderer->getColor();
	renderer->setColor(*foregroundcolor);
	gDrawRectangle(left, top, width, height, true);
	renderer->setColor(*backgroundcolor);
	gDrawLine(left, bottom, right, bottom);
//	gDrawRectangle(left, top, width, height, false);
	renderer->setColor(oldcolor);
	if(guisizer) guisizer->draw();
}

