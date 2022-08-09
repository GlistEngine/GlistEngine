
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
	minlinenum = 5  ;
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
}

gGUITreelist::~gGUITreelist() {
}

void gGUITreelist::set(gBaseApp* root, gBaseGUIObject* topParentGUIObject, gBaseGUIObject* parentGUIObject, int parentSlotLineNo, int parentSlotColumnNo, int x, int y, int w, int h) {
	//gLogi("h") << h;
	totalh = h;
	gGUIScrollable::set(root, topParentGUIObject, parentGUIObject, parentSlotLineNo, parentSlotColumnNo, x, y, w, h);
	gGUIScrollable::setDimensions(width, listboxh);

}

void gGUITreelist::addElement(Element* element) {

	topelement.addElement(element);
	topelement.clearAllSubTitlesList();
	topelement.addSelfToList();

	linenum = element->nodenum;
	if(linenum > maxlinenum) linenum = maxlinenum;
	totalh = topelement.nodenum * lineh;;
	if(totalh < height) totalh = height;

}

void gGUITreelist::drawContent() {
	//topelement.logTitle();
	gColor* oldcolor = renderer->getColor();
	renderer->setColor(textbackgroundcolor);
	gDrawRectangle(0, 0, boxw, boxh , true);

	flno = firsty / lineh;
	fldy = firsty % lineh;
//	gLogi("Lineh") << lineh;
//	gLogi("Linenumber") << linenum;
//	gLogi("Totalh") << totalh;
//	gLogi("Nodenumber") << topelement.nodenum;
	if(selectedno >= flno && selectedno <= flno + linenum) {
		if(isfocused) renderer->setColor(255, 128, 0);
		else renderer->setColor(middlegroundcolor);
		gDrawRectangle(0, -fldy + (selectedno - flno) * lineh, boxw, lineh, true);
	}

	renderer->setColor(fontcolor);
	for(int i = 1; i < topelement.allsubtitles.size(); i++) font->drawText(topelement.allsubtitles[flno + i], 2, - fldy + (i * lineh) - datady);


	renderer->setColor(oldcolor);

}

void gGUITreelist::insertData(Element* element, std::string lineData) {
	element->title = lineData;
	topelement.clearAllSubTitlesList();
	topelement.addSelfToList();
}

void gGUITreelist::removeElement(Element* element) {
	topelement.removeElement(element);
	topelement.clearAllSubTitlesList();
	topelement.addSelfToList();

	linenum = topelement.nodenum;
	if(linenum > maxlinenum) linenum = maxlinenum;
	totalh = topelement.nodenum * lineh;
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


		tmp = topelement.allsubtitles[selectedno + 1];
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
			topelement.clearAllSubTitlesList();
			topelement.addSelfToList();
		}
		root->getCurrentCanvas()->onGuiEvent(id, G_GUIEVENT_LISTBOXSELECTED, gToStr(selectedno));
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

