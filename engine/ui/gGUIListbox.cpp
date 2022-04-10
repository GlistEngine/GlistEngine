/*
 * gGUIListbox.cpp
 *
 *  Created on: Feb 5, 2022
 *      Author: noyan
 */

#include "gGUIListbox.h"
#include "gBaseApp.h"
#include "gBaseCanvas.h"


gGUIListbox::gGUIListbox() {
	lineh = 24;
	minlinenum = 5;
	linenum = 0;
	totalh = linenum * lineh;
	minboxh = minlinenum * lineh;
	listboxh = minboxh;
	maxlinenum = listboxh / lineh + 1;
	firstlineno = 0;
	flno = firstlineno;
	selectedno = 0;
	mousepressedonlist = false;
}

gGUIListbox::~gGUIListbox() {

}

void gGUIListbox::set(gBaseApp* root, gBaseGUIObject* parentGUIObject, int parentSlotLineNo, int parentSlotColumnNo, int x, int y, int w, int h) {
	gGUIScrollable::set(root, parentGUIObject, parentSlotLineNo, parentSlotColumnNo, x, y, w, h);
	gGUIScrollable::setDimensions(width, listboxh);
}

void gGUIListbox::drawContent() {
	gColor* oldcolor = renderer->getColor();
//	gGUIScrollable::drawContent();

	renderer->setColor(textbackgroundcolor);
	gDrawRectangle(0, 0, boxw, boxh, true);

	flno = firsty / lineh;
	fldy = firsty % lineh;

	if(selectedno >= flno && selectedno < flno + linenum) {
		renderer->setColor(middlegroundcolor);
		gDrawRectangle(0, -fldy + (selectedno - flno) * lineh, boxw, lineh, true);
	}

	renderer->setColor(fontcolor);
	for(int i = 0; i < linenum; i++) {
		font->drawText(data[flno + i], 2, -fldy + lineh + (i * lineh) - datady[flno + i]);
	}

	renderer->setColor(oldcolor);
}

void gGUIListbox::addData(std::string lineData) {
	data.push_back(lineData);
	datady.push_back((lineh - font->getStringHeight("ae")) / 2 + 1);
	linenum = data.size();
	if(linenum > maxlinenum) linenum = maxlinenum;
	totalh = data.size() * lineh;
}

void gGUIListbox::mousePressed(int x, int y, int button) {
	gGUIScrollable::mousePressed(x, y, button);
	if(x >= left && x < left + vsbx && y >= top && y < top + hsby) mousepressedonlist = true;
}

void gGUIListbox::mouseReleased(int x, int y, int button) {
	gGUIScrollable::mouseReleased(x, y, button);
	if(mousepressedonlist && x >= left && x < left + vsbx && y >= top && y < top + hsby) {
		int newselectedno = (y - top + firsty) / lineh;
		if(newselectedno < data.size()) selectedno = newselectedno;
		root->getCurrentCanvas()->onGuiEvent(id, G_GUIEVENT_LISTBOXSELECTED, gToStr(selectedno));
	}
	mousepressedonlist = false;
}

void gGUIListbox::setSelected(int lineNo) {
	if(lineNo < 0 || lineNo > data.size() - 1) return;

	selectedno = lineNo;
}

int gGUIListbox::getSelected() {
	return selectedno;
}

std::string gGUIListbox::getData(int lineNo) {
	return data[lineNo];
}

