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
	setSizer(&panelsizer);
	title = "Panel";
	istitle = true;
}

gGUIPanel::~gGUIPanel() {
}

void gGUIPanel::showTitle(bool istitle) {
	this->istitle = istitle;
	if(istitle) {
		topbarh = 30;
		toplineh = topbarh * 7 / 10;
		guisizer->set(root, topparent, parent, parentslotlineno, parentslotcolumnno, left, top + toplineh, width, height - toplineh);
	}
	else {
		guisizer->set(root, topparent, parent, parentslotlineno, parentslotcolumnno, left, top - toplineh, width, height + toplineh);
		topbarh = 0;
		toplineh = 0;
	}
}

void gGUIPanel::draw() {
//	gLogi("gGUIPanel") << "draw";
	gColor oldcolor = *renderer->getColor();
	renderer->setColor(foregroundcolor);
	gDrawRectangle(left, top, width, height, true);
	if(istitle) {
		renderer->setColor(middlegroundcolor->r, middlegroundcolor->g, middlegroundcolor->b);
		gDrawLine(left, top + toplineh, right, top + toplineh);
		renderer->setColor(fontcolor);
		font->drawText(title, left + 2, top + 12);
	}
	renderer->setColor(&oldcolor);
	if(guisizer) guisizer->draw();
}
