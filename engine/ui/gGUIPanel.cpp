/*
 * gGUIPanel.cpp
 *
 *  Created on: Aug 20, 2021
 *      Author: noyan
 */

#include "gGUIPanel.h"


gGUIPanel::gGUIPanel() {
	topbarh = 30;
	toplineh = topbarh * 7 / 10;
}

gGUIPanel::~gGUIPanel() {
}

void gGUIPanel::draw() {
//	gLogi("gGUIPanel") << "draw";
	gColor oldcolor = *renderer->getColor();
	renderer->setColor(foregroundcolor);
	gDrawRectangle(left, top, width, height, true);
	renderer->setColor(middlegroundcolor->r, middlegroundcolor->g, middlegroundcolor->b);
	gDrawLine(left, top + toplineh, right, top + toplineh);
	renderer->setColor(fontcolor);
	font->drawText(title, left + 2, top + 12);
	renderer->setColor(&oldcolor);
	if(guisizer) guisizer->draw();
}
