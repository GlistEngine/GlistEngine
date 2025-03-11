/*
 * gGUIListbox.cpp
 *
 *  Created on: Feb 5, 2022
 *      Author: noyan
 */

#include "gGUIListbox.h"

#include <algorithm>
#include "gBaseApp.h"


gGUIListbox::gGUIListbox() {
	lineh = 2 * font->getSize() + 2;
	firstlineno = 0;
	flno = firstlineno;
	selectedno = 0;
	mousepressedonlist = false;
	textoffset = (lineh - font->getStringHeight("ae")) / 2 + 1;
	fldy = 0;
	chosencolor = gColor(1.0f, 0.5f, 0.0f);
	iconcolor = textbackgroundcolor;
	isicon = false;
	updateTotalHeight();
	setVisibleLineNumber(5);
	isdisabled = false;
}

gGUIListbox::~gGUIListbox() {

}

void gGUIListbox::set(gBaseApp* root, gBaseGUIObject* topParentGUIObject, gBaseGUIObject* parentGUIObject, int parentSlotLineNo, int parentSlotColumnNo, int x, int y, int w, int h) {
	gGUIScrollable::set(root, topParentGUIObject, parentGUIObject, parentSlotLineNo, parentSlotColumnNo, x, y, w, h);
	gGUIScrollable::setDimensions(width, listboxh + textoffset);
	updateTotalHeight();
}

void gGUIListbox::drawContent() {
	gColor* oldcolor = renderer->getColor();

	renderer->setColor(textbackgroundcolor);
	gDrawRectangle(0, 0, boxw, boxh, true);

	flno = verticalscroll / lineh;
	fldy = verticalscroll % lineh;

	int linenum = data.size();
	if(selectedno >= flno && selectedno <= flno + linenum) {
		if(isfocused && !isdisabled) {
			renderer->setColor(chosencolor);
		} else {
			renderer->setColor(middlegroundcolor);
		}
		gDrawRectangle(0, -fldy + (selectedno - flno) * lineh, boxw, lineh, true);
	}

	int startindex = flno;
	int endindex = flno + visibilelinenum + 1;
	endindex = std::min(endindex, linenum);
	startindex = std::max(startindex, 0);

	for(int i = startindex; i < endindex; i++) {
		renderer->setColor(fontcolor);
		font->drawText(data[i], 2, (i * lineh) + lineh - textoffset - verticalscroll);
	}

	renderer->setColor(oldcolor);
}

void gGUIListbox::addData(std::string lineData) {
	data.push_back(lineData);
	updateTotalHeight();
}

void gGUIListbox::setData(int lineNo, std::string lineData) {
	if(lineNo < 0 || lineNo >= data.size()) return;
	data[lineNo] = lineData;
}

void gGUIListbox::insertData(int lineNo, std::string lineData) {
	data.insert(data.begin() + lineNo, lineData);
	updateTotalHeight();
}

void gGUIListbox::removeData(int lineNo) {
	if(lineNo>=0 && lineNo < data.size()) {
		data.erase(data.begin() + lineNo);
	}
	if(lineNo>=0 && lineNo < icons.size()) {
		icons.erase(icons.begin() + lineNo);
	}
	if(selectedno > data.size() - 1) {
		selectedno = data.size() - 1;
		flno -= 1;
		if(flno < 0) flno = 0;
		verticalscroll -= 3 * scrollamount;
		if(verticalscroll < 0) {
			verticalscroll = 0;
		}
	}
	updateTotalHeight();
}

void gGUIListbox::removeSelected() {
	removeData(selectedno);
}

void gGUIListbox::clear() {
	data.clear();
	icons.clear();
	verticalscroll = 0;
	updateTotalHeight();
	selectedno = 0;
}


void gGUIListbox::mousePressed(int x, int y, int button) {
	if(isdisabled) return;
	gGUIScrollable::mousePressed(x, y, button);
	if(x >= left && x < left + boxw && y >= top + titleheight && y < top + titleheight + boxh) {
		mousepressedonlist = true;
	}
}

void gGUIListbox::mouseReleased(int x, int y, int button) {
	if(isdisabled) return;
	gGUIScrollable::mouseReleased(x, y, button);
	if(mousepressedonlist) mousepressedonlist = false;
	if(x >= left && x < left + boxw && y >= top + titleheight && y < top + titleheight + boxh) {
		int newselectedno = (y - top - titleheight + verticalscroll) / lineh;
		if(newselectedno < data.size()) selectedno = newselectedno;
		isfocused = true;
		root->getCurrentCanvas()->onGuiEvent(id, G_GUIEVENT_LISTBOXSELECTED, gToStr(selectedno));
	}
}

void gGUIListbox::setSelected(int lineNo) {
	if(lineNo < 0 || lineNo > data.size() - 1) return;

	selectedno = lineNo;
	root->getCurrentCanvas()->onGuiEvent(id, G_GUIEVENT_LISTBOXSELECTED, gToStr(selectedno));
}

void gGUIListbox::setChosenColor(float r, float g, float b) {
	chosencolor = gColor(r, g, b);
}

void gGUIListbox::setVisibleLineNumber(int linenumber) {
	if(linenumber <= 0) {
		return;
	}
	visibilelinenum = linenumber;
	minboxh = visibilelinenum * lineh;
	listboxh = minboxh + textoffset;
}

void gGUIListbox::setIconType(bool isicon) {
	this->isicon = isicon;
	if(isicon) {
		setIcons();
	}
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
			if(title != i) {
				index++;
			} else {
				break;
			}
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

std::vector<std::string> gGUIListbox::getData() {
	return data;
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
	return visibilelinenum;
}

int gGUIListbox::getTotalHeight() {
	return totalh;
}

void gGUIListbox::updateTotalHeight() {
	totalh = data.size() * lineh;
	if(totalh < minboxh) {
		totalh = minboxh;
	}
}

void gGUIListbox::setDisabled(bool isDisabled) {
	isdisabled = isDisabled;
}

bool gGUIListbox::isDisabled() {
	return isdisabled;
}
