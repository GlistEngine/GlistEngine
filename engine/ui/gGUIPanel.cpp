/*
 * gGUIPanel.cpp
 *
 *  Created on: Aug 20, 2021
 *      Author: noyan
 */

#include "gGUIPanel.h"


gGUIPanel::gGUIPanel() {
	topbarh = 30;
}

gGUIPanel::~gGUIPanel() {
}

void gGUIPanel::update() {
//	gLogi("gGUIPanel") << "update";
	if(guisizer) guisizer->update();
}

void gGUIPanel::draw() {
//	gLogi("gGUIPanel") << "draw";
	gColor oldcolor = *renderer->getColor();
	renderer->setColor(*foregroundcolor);
	gDrawRectangle(left, top, width, height, true);
	renderer->setColor(foregroundcolor->r - 0.05f, foregroundcolor->g - 0.05f, foregroundcolor->b - 0.05f);
	gDrawLine(left, top + 21, right, top + 21);
	renderer->setColor(*fontcolor);
	font->drawText(title, left + 2, top + 12);
	renderer->setColor(oldcolor);
	if(guisizer) guisizer->draw();
}
