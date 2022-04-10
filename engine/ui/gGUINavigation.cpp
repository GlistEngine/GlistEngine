/*
 * gGUINavigation.cpp
 *
 *  Created on: Apr 3, 2022
 *      Author: noyan
 */

#include "gGUINavigation.h"


gGUINavigation::gGUINavigation() {
	panetoph = 100;
	panelineh = 40;
	panelinepad = 20;
}

gGUINavigation::~gGUINavigation() {
}

void gGUINavigation::set(gBaseApp* root, gBaseGUIObject* parentGUIObject, int parentSlotLineNo, int parentSlotColumnNo, int x, int y, int w, int h) {
	totalh = h;
	gGUIScrollable::set(root, parentGUIObject, parentSlotLineNo, parentSlotColumnNo, x, y, w, h);
	gGUIScrollable::setDimensions(w, h);
}

void gGUINavigation::update() {

}

void gGUINavigation::draw() {
	gColor* oldcolor = renderer->getColor();
	gGUIScrollable::drawContent();
	renderer->setColor(navigationbackgroundcolor);
	gDrawRectangle(0, 0, boxw, boxh, true);

	renderer->setColor(255, 255, 255);
	for(int i = 0; i < panes.size(); i++) {
		font->drawText(gToStr(i + 1) + ". " + panes[i]->getTitle(), panelinepad, panetoph + i * panelineh);
	}

	renderer->setColor(oldcolor);
}

void gGUINavigation::addPane(gGUIPane* newPane) {
	panes.push_back(newPane);
}

void gGUINavigation::setPane(int paneNo, gGUIPane* newPane) {
	panes.insert(panes.begin() + paneNo, newPane);
}

void gGUINavigation::removePane(int paneNo) {
	panes.erase(panes.begin() + paneNo);
}

gGUIPane* gGUINavigation::getPane(int paneNo) {
	return panes[paneNo];
}

int gGUINavigation::getPaneNum() {
	return panes.size();
}

void gGUINavigation::clear() {
	panes.clear();
}

void gGUINavigation::mousePressed(int x, int y, int button) {
	gGUIScrollable::mousePressed(x, y, button);
}

void gGUINavigation::mouseReleased(int x, int y, int button) {
	gGUIScrollable::mouseReleased(x, y, button);
	int selected = -1;
	for(int i = 0; i < panes.size(); i++) {
		if(x >= panelinepad && x < width - panelinepad && y >= panetoph + i * panelineh - font->getSize() && y < panetoph + i * panelineh + font->getSize() / 2) {
			((gGUISizer*)parent)->setControl(0, 1, panes[i]);
			break;
		}
	}
}

