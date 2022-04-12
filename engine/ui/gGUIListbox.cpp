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
	lineh = 2 * font->getSize() + 2;
	minlinenum = 5;
	linenum = 0;
	totalh = linenum * lineh;
	minboxh = minlinenum * lineh;
	listboxh = minboxh;
	maxlinenum = listboxh / lineh;
	firstlineno = 0;
	flno = firstlineno;
	selectedno = 0;
	mousepressedonlist = false;
	datady = (lineh - font->getStringHeight("ae")) / 2 + 1;
	fldy = 0;
}

gGUIListbox::~gGUIListbox() {

}

void gGUIListbox::set(gBaseApp* root, gBaseGUIObject* topParentGUIObject, gBaseGUIObject* parentGUIObject, int parentSlotLineNo, int parentSlotColumnNo, int x, int y, int w, int h) {
	totalh = h;
	gGUIScrollable::set(root, topParentGUIObject, parentGUIObject, parentSlotLineNo, parentSlotColumnNo, x, y, w, h);
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
		if(isfocused) renderer->setColor(255, 128, 0);
		else renderer->setColor(middlegroundcolor);
		gDrawRectangle(0, -fldy + (selectedno - flno) * lineh, boxw, lineh, true);
	}

	renderer->setColor(fontcolor);
	for(int i = 0; i < linenum; i++) {
		font->drawText(data[flno + i], 2, -fldy + lineh + (i * lineh) - datady);
	}

	renderer->setColor(oldcolor);
}

void gGUIListbox::addData(std::string lineData) {
	data.push_back(lineData);
	linenum = data.size();
	if(linenum > maxlinenum) linenum = maxlinenum;
	totalh = data.size() * lineh;
	if(totalh < height) totalh = height;
}

void gGUIListbox::insertData(int lineNo, std::string lineData) {
	data.insert(data.begin() + lineNo, lineData);
	linenum = data.size();
	if(linenum > maxlinenum) linenum = maxlinenum;
	totalh = data.size() * lineh;
	if(totalh < height) totalh = height;
}

void gGUIListbox::removeData(int lineNo) {
	data.erase(data.begin() + lineNo);
	linenum = data.size();
	if(linenum > maxlinenum) linenum = maxlinenum;
	totalh = data.size() * lineh;
	if(totalh < height) totalh = height;
	if(selectedno > data.size() - 1) {
		selectedno = data.size() - 1;
		flno -= 1;
		if(flno < 0) flno = 0;
		firsty -= 3 * scrolldiff;
		if(firsty < 0) firsty = 0;
	}
}

void gGUIListbox::removeSelected() {
	removeData(selectedno);
}

void gGUIListbox::clear() {
	data.clear();
	linenum = 0;
	totalh = height;
	firsty = 0;
}


void gGUIListbox::mousePressed(int x, int y, int button) {
	gGUIScrollable::mousePressed(x, y, button);
	if(x >= left && x < left + vsbx && y >= top + titledy && y < top + titledy + hsby) {
		mousepressedonlist = true;
	}
}

void gGUIListbox::mouseReleased(int x, int y, int button) {
	gGUIScrollable::mouseReleased(x, y, button);
	if(mousepressedonlist && x >= left && x < left + vsbx && y >= top + titledy && y < top + titledy + hsby) {
		int newselectedno = (y - top - titledy + firsty) / lineh;
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

std::string gGUIListbox::getSelectedData() {
	return data[selectedno];
}

std::string gGUIListbox::getData(int lineNo) {
	return data[lineNo];
}

int gGUIListbox::getDataNum() {
	return data.size();
}

