/*
 * gGUITreelist.h
 *
 *  Created on: 08 Aug 2022
 *      Author: Sevval Bulburu, Aynur Dogan
 */

#include "gGUITreelist.h"
#include "gBaseApp.h"
#include "gBaseCanvas.h"
#include <deque>
#include <iostream>
#include <string.h>


std::vector<std::string> gGUITreelist::Element::allsubtitles;
int::gGUITreelist::Element::nodenum;

gGUITreelist::gGUITreelist() {
	topelement.title = "Top";
	topelement.isexpanded = true;
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
	arrowsize = font->getStringWidth(">");
	chosencolor = gColor(1.0f, 0.5f, 0.0f);
}

gGUITreelist::~gGUITreelist() {
}

void gGUITreelist::set(gBaseApp* root, gBaseGUIObject* topParentGUIObject, gBaseGUIObject* parentGUIObject, int parentSlotLineNo, int parentSlotColumnNo, int x, int y, int w, int h) {
	totalh = h;
	gGUIControl::set(root, topParentGUIObject, parentGUIObject, parentSlotLineNo, parentSlotColumnNo, x, y, w, h);
	gGUIScrollable::setDimensions(width, listboxh);
}

void  gGUITreelist::addElement(Element* element, Element* parent) {
	parent->sub.push_back(element);
	element->parent = parent;
	element->orderno = parent->orderno + 1;
	element->nodenum = parent->nodenum + 1;

	refreshList();
}

void gGUITreelist::addElement(Element* element) {
	topelement.sub.push_back(element);
	element->parent = &topelement;
	element->orderno = topelement.orderno;
	element->nodenum = topelement.nodenum + 1;

	refreshList();

}

void gGUITreelist::drawContent() {
	//topelement.logTitle();
	gColor* oldcolor = renderer->getColor();
	renderer->setColor(textbackgroundcolor);
	gDrawRectangle(0, 0, boxw, boxh , true);

	flno = firsty / lineh;
	fldy = firsty % lineh;

	if(selectedno >= flno && selectedno <= flno + linenum) {
		if(isfocused) renderer->setColor(chosencolor);
		else renderer->setColor(middlegroundcolor);
		gDrawRectangle(0, -fldy + (selectedno - flno) * lineh, boxw, lineh, true);
	}

	renderer->setColor(fontcolor);

	for(int i = 0; i < linenum; i++) {
		//if(flno + i <= topelement.allsubtitles.size())
		font->drawText(topelement.allsubtitles[flno + i], 2, - fldy + (i * lineh) + lineh - datady);
	}

	renderer->setColor(oldcolor);

}

void gGUITreelist::insertData(Element* element, std::string lineData) {
	element->title = lineData;
	topelement.clearAllSubTitlesList();
	topelement.addSelfToList();
}

void gGUITreelist::removeElement(Element* element) {
	topelement.removeElement(element);
	refreshList();
}

void gGUITreelist::refreshList() {
    topelement.clearAllSubTitlesList();
    topelement.addSelfToList();

    linenum = topelement.nodenum;
    if(linenum > maxlinenum) linenum = maxlinenum;
    totalh = topelement.allsubtitles.size() * lineh;
    if(totalh < height) totalh = height;
}

void gGUITreelist::mousePressed(int x, int y, int button) {
	gGUIScrollable::mousePressed(x, y, button);
	if(x >= left && x < left + vsbx && y >= top + titledy && y < top + titledy + hsby) {
		mousepressedonlist = true;
	}
}

void gGUITreelist::mouseReleased(int x, int y, int button) {
	std::string tmp = "";
	std::string tmptitle = "";
	Element* element;
	bool arrow = false;
	int arrowposx = 0;

	gGUIScrollable::mouseReleased(x, y, button);
	if(mousepressedonlist) mousepressedonlist = false;
	if(x >= left && x < left + vsbx && y >= top + titledy && y < top + titledy + hsby) {
		int newselectedno = (y - top - titledy + firsty) / lineh;
		if(newselectedno <= topelement.allsubtitles.size() - 1) selectedno = newselectedno;


		tmp = topelement.allsubtitles[selectedno];
		int i = 0;
		while(i < tmp.size() && arrow == false) {
			if(tmp[i] == '>') {
				arrow = true;
				tmptitle = tmp.substr(i + 2, tmp.size() - i);
				arrowposx = font->getStringWidth(tmp.substr(0, i + 1));
			}
			i++;
		}

		if(x < left + arrowposx  - arrowsize || x > left + arrowposx) isfocused = true;
		else if(x > left + arrowposx  - arrowsize && x < left + arrowposx) {
			element = topelement.findElement(tmptitle);
			element->isexpanded = !element->isexpanded;
			refreshList();
		}
		//root->getCurrentCanvas()->onGuiEvent(id, G_GUIEVENT_TREELISTSELECTED, gToStr(selectedno));
	}
}

std::string gGUITreelist::getTitle(Element* element) {
	return element->title;
}

int gGUITreelist::getNodenum(Element* element) {
	return element->nodenum;
}

int gGUITreelist::getOrderno(Element* element) {
	return element->orderno;
}

bool gGUITreelist::isExpanded(Element* element) {
	return element->isexpanded;
}

bool gGUITreelist::isParent(Element* element) {
	return element->isparent;
}

void gGUITreelist::setChosenColor(float r, float g, float b) {
	chosencolor = gColor(r, g, b);
}

void gGUITreelist::setVisibleLineNumber(int linenumber) {
	if(linenumber > 0) {
		minlinenum = linenumber;
		minboxh = minlinenum * lineh;
		listboxh = minboxh;
		maxlinenum = listboxh / lineh;
	}
}

