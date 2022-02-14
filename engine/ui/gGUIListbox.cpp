/*
 * gGUIListbox.cpp
 *
 *  Created on: Feb 5, 2022
 *      Author: noyan
 */

#include "gGUIListbox.h"


gGUIListbox::gGUIListbox() {
	lineh = 24;
	minlinenum = 5;
	linenum = minlinenum;
	totalh = linenum * lineh;
	minboxh = minlinenum * lineh;
	listboxh = minboxh;
}

gGUIListbox::~gGUIListbox() {

}

void gGUIListbox::set(gBaseApp* root, gBaseGUIObject* parentGUIObject, int parentSlotLineNo, int parentSlotColumnNo, int x, int y, int w, int h) {
	gGUIScrollable::set(root, parentGUIObject, parentSlotLineNo, parentSlotColumnNo, x, y, w, h);
	gGUIScrollable::setDimensions(width, listboxh);
}

void gGUIListbox::drawContent() {
	gColor* oldcolor = renderer->getColor();

	renderer->setColor(backgroundcolor);
//	gDrawRectangle(left, top, width, boxh, true);
	renderer->setColor(fontcolor);
	for(int i = 0; i < data.size(); i++) {
		font->drawText(data[i], left + 2, top + 13 + i * lineh);
	}

	renderer->setColor(oldcolor);
}

void gGUIListbox::addData(std::string lineData) {
	data.push_back(lineData);
	linenum = data.size();
	totalh = linenum * lineh;
}

