/*
 * gGUIPane.cpp
 *
 *  Created on: Apr 3, 2022
 *      Author: noyan
 */

#include "gGUIPane.h"


gGUIPane::gGUIPane() {
	setSizer(&panesizer);
	totalh = 0;
	title = "Pane";
	titlefontsize = 24;
	titlefont.loadFont("FreeSansBold.ttf", titlefontsize);
	titlecolor = gColor(128, 128, 128);
	topbarh = titlefontsize * 4 + font->getSize();
	panesizer.setSlotPadding(titlefontsize * 2);
}

gGUIPane::~gGUIPane() {
}

void gGUIPane::set(gBaseApp* root, gBaseGUIObject* parentGUIObject, int parentSlotLineNo, int parentSlotColumnNo, int x, int y, int w, int h) {
	totalh = h;
	gGUIContainer::set(root, parentGUIObject, parentSlotLineNo, parentSlotColumnNo, x, y, w, h);
	gGUIContainer::set(x, y, w, h);
//	gGUIScrollable::set(root, parentGUIObject, parentSlotLineNo, parentSlotColumnNo, x, y, w, h);
//	gGUIScrollable::setDimensions(w, h);
}

void gGUIPane::draw() {
	gColor oldcolor = *renderer->getColor();

	renderer->setColor(255, 255, 255);
	gDrawRectangle(left, top, width, height, true);

	renderer->setColor(212, 212, 212);
	titlefont.drawText(title, left + titlefontsize * 2, top + titlefontsize * 3);

	renderer->setColor(&oldcolor);
	if(guisizer) guisizer->draw();
}

