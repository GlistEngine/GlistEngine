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
	minlinenum = 4;
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
	chosencolor = gColor(1.0f, 0.5f, 0.0f);
	iconcolor = textbackgroundcolor;
	isicon = false;
	iconw = lineh / 2;
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

	renderer->setColor(textbackgroundcolor);
	gDrawRectangle(0, 0, boxw, boxh, true);

	flno = firsty / lineh;
	fldy = firsty % lineh;

	if(selectedno >= flno && selectedno < flno + linenum) {
		if(isfocused) renderer->setColor(chosencolor);
		else renderer->setColor(middlegroundcolor);
		gDrawRectangle(0, -fldy + (selectedno - flno) * lineh, boxw, lineh, true);
	}

	for(int i = 0; i < linenum; i++) {
		if(flno + i < data.size()) {
			if(isicon) {
				renderer->setColor(iconcolor);
				icons[i + flno]->draw(2 , - fldy + (i * lineh) - datady / 2 + lineh / 2, iconw, iconw);
				renderer->setColor(fontcolor);
				font->drawText(data[flno + i], 12, -fldy + lineh + (i * lineh) - datady);
			}
			else {
				renderer->setColor(fontcolor);
				font->drawText(data[flno + i], 2, -fldy + lineh + (i * lineh) - datady);
			}
		}
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
	if(lineNo>=0 && lineNo < data.size()) data.erase(data.begin() + lineNo);
	if(lineNo>=0 && lineNo < icons.size()) icons.erase(icons.begin() + lineNo);
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
	icons.clear();
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
	if(mousepressedonlist) mousepressedonlist = false;
	if(x >= left && x < left + vsbx && y >= top + titledy && y < top + titledy + hsby) {
		int newselectedno = (y - top - titledy + firsty) / lineh;
		if(newselectedno < data.size() + 1) selectedno = newselectedno;
		isfocused = true;
		root->getCurrentCanvas()->onGuiEvent(id, G_GUIEVENT_LISTBOXSELECTED, gToStr(selectedno));

	}
}

void gGUIListbox::setSelected(int lineNo) {
	if(lineNo < 0 || lineNo > data.size() - 1) return;

	selectedno = lineNo;
}

void gGUIListbox::setChosenColor(float r, float g, float b) {
	chosencolor = gColor(r, g, b);
}

void gGUIListbox::setVisibleLineNumber(int linenumber) {
	if(linenumber > 0) {
		minlinenum = linenumber;
		minboxh = minlinenum * lineh;
		listboxh = minboxh;
		maxlinenum = listboxh / lineh;
	}
}

void gGUIListbox::setIconType(bool isicon) {
	this->isicon = isicon;
	if(this->isicon) setIcons();
}

void gGUIListbox::setIcons() {
	for(int i = 0; i < data.size(); i++) icons.push_back(res.getIconImage(gGUIResources::ICON_FILE));
}

void  gGUIListbox::setIconsColor(float r, float g, float b) {
	iconcolor = gColor(r, g, b);
}

void gGUIListbox::setIcon(gImage* icon, std::string title) {
	if(isicon) {
		int index = 0;
		for(auto i : data) {
			if(title != i) index++;
			else break;
		}
		if(index <= data.size()) {
			icons.erase(icons.begin() + index);
			icons.insert(icons.begin() + index, icon);
		}
	}
}

void  gGUIListbox::setIcon(int iconid, std::string title) {
	if(isicon) {
		int index = 0;
		for(auto i : data) {
			if(title != i) index++;
			else break;
		}
		if(index <= data.size()) {
			icons.erase(icons.begin() + index);
			icons.insert(icons.begin() + index, res.getIconImage(iconid));
		}
	}

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

bool gGUIListbox::getIconType() {
	return isicon;
}

gTexture* gGUIListbox::getIcon(std::string title) {
	int index = 0;
	for(auto i : data) {
		if(title != i) index++;
		else break;
	}
	return icons[index];
}

gColor gGUIListbox::getChosenColor() {
	return chosencolor;
}

gColor gGUIListbox::getIconsColor() {
	return iconcolor;
}

int gGUIListbox::getVisibleLineNumber() {
	return minlinenum;
}
