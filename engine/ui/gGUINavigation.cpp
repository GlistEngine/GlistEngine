/*
 * gGUINavigation.cpp
 *
 *  Created on: Apr 3, 2022
 *      Author: noyan
 */

#include "gGUINavigation.h"
#include "gBaseApp.h"


gGUINavigation::gGUINavigation() {
	panetoph = 100;
	panelineh = 40;
	panelinepad = 20;
	selectedpane = 0;
	toolbarenabled = false;
}

gGUINavigation::~gGUINavigation() {
}

void gGUINavigation::set(gBaseApp* root, gBaseGUIObject* topParentGUIObject, gBaseGUIObject* parentGUIObject, int parentSlotLineNo, int parentSlotColumnNo, int x, int y, int w, int h) {
	totalh = h;
	gGUIScrollable::set(root, topParentGUIObject, parentGUIObject, parentSlotLineNo, parentSlotColumnNo, x, y, w, h);
	gGUIScrollable::setDimensions(w, h);
}

void gGUINavigation::update() {

}

void gGUINavigation::draw() {
	gColor* oldcolor = renderer->getColor();
	gGUIScrollable::drawContent();
	renderer->setColor(navigationbackgroundcolor);
	gDrawRectangle(0, 0, boxw, boxh + panetoph, true);

	renderer->setColor(255, 255, 255);
	for(int i = 0; i < panes.size(); i++) {
		if(i == selectedpane) root->getAppManager()->getGUIManager()->getFont(0, 1)->drawText(gToStr(i + 1) + ". " + panes[i]->getTitle(), panelinepad, panetoph + i * panelineh);
		else font->drawText(gToStr(i + 1) + ". " + panes[i]->getTitle(), panelinepad, panetoph + i * panelineh);
	}

	if(toolbarenabled) toolbar.draw();

	renderer->setColor(oldcolor);
}

void gGUINavigation::addPane(gGUIPane* newPane) {
	newPane->setNavigation(this);
	panes.push_back(newPane);
	newPane->setNavigationOrder(panes.size() - 1);
}

void gGUINavigation::setPane(int paneNo, gGUIPane* newPane) {
	newPane->setNavigation(this);
	panes.insert(panes.begin() + paneNo, newPane);
	newPane->setNavigationOrder(paneNo);
	for(int i = paneNo + 1; i < panes.size(); i++) panes[i]->setNavigationOrder(i);
}

void gGUINavigation::removePane(int paneNo) {
	panes[paneNo]->setNavigation(nullptr);
	panes.erase(panes.begin() + paneNo);
	for(int i = paneNo; i < panes.size(); i++) panes[i]->setNavigationOrder(i);
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

void gGUINavigation::setSelectedPane(int paneNo) {
	if(paneNo < 0 || paneNo >= panes.size()) return;
	selectedpane = paneNo;
}

void gGUINavigation::showPane(gGUIPane* paneToShow) {
	((gGUISizer*)parent)->setControl(0, 1, paneToShow);
}

void gGUINavigation::mousePressed(int x, int y, int button) {
	gGUIScrollable::mousePressed(x, y, button);
}

void gGUINavigation::mouseReleased(int x, int y, int button) {
	gGUIScrollable::mouseReleased(x, y, button);
	int selected = -1;
	for(int i = 0; i < panes.size(); i++) {
		if(x >= panelinepad && x < width - panelinepad && y >= panetoph + i * panelineh - font->getSize() && y < panetoph + i * panelineh + font->getSize() / 2) {
			selectedpane = i;
			((gGUISizer*)parent)->setControl(0, 1, panes[selectedpane]);
			break;
		}
	}

	if(toolbarenabled && x >= 0 && y >= height - 40 && x < width && y < height - 40 + 32) {
		toolbar.mouseReleased(x, y, button);
	}
}

void gGUINavigation::enableToolbar() {
//	this->toolbar = toolbar;
	maintoolbarsizer.set(root, topparent, this, 0, 0, 0, height - 40, width, 32);
	maintoolbarsizer.setSize(1, 1);
	toolbar.setSizer(&toolbarsizer);
	maintoolbarsizer.setControl(0, 0, &toolbar);
	toolbarsizer.setRootApp(root);
	toolbarsizer.setSize(1, 10);
	toolbar.setToolbarForegroundColor(navigationbackgroundcolor);
	toolbar.setToolbarBottomLineColor(navigationbackgroundcolor);
	toolbarenabled = true;
}

gGUISizer* gGUINavigation::getToolbarSizer() {
	return &toolbarsizer;
}

