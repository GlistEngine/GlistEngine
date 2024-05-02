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

	for(int i = 0; i < panes.size(); i++) {
		if(!paneenabled[i]) {
			renderer->setColor(196, 196, 196);
			font->drawText(gToStr(i + 1) + ". " + panes[i]->getTitle(), panelinepad, panetoph + i * panelineh);
		} else if(i == selectedpane) {
			renderer->setColor(255, 255, 255);
			root->getAppManager()->getGUIManager()->getFont(0, 1)->drawText(gToStr(i + 1) + ". " + panes[i]->getTitle(), panelinepad, panetoph + i * panelineh);
		} else {
			renderer->setColor(255, 255, 255);
			font->drawText(gToStr(i + 1) + ". " + panes[i]->getTitle(), panelinepad, panetoph + i * panelineh);
		}
	}

	if(toolbarenabled) {
		renderer->setColor(255, 255, 255);
		toolbar.draw();
	}

	renderer->setColor(oldcolor);
}

void gGUINavigation::addPane(gGUIPane* newPane, bool isEnabled) {
	newPane->setNavigation(this);
	panes.push_back(newPane);
	paneenabled.push_back(isEnabled);
	newPane->setNavigationOrder(panes.size() - 1);
}

void gGUINavigation::setPane(int paneNo, gGUIPane* newPane, bool isEnabled) {
	newPane->setNavigation(this);
	panes.insert(panes.begin() + paneNo, newPane);
	paneenabled.insert(paneenabled.begin() + paneNo, isEnabled);
	newPane->setNavigationOrder(paneNo);
	for(int i = paneNo + 1; i < panes.size(); i++) panes[i]->setNavigationOrder(i);
}

void gGUINavigation::removePane(int paneNo) {
	panes[paneNo]->setNavigation(nullptr);
	panes.erase(panes.begin() + paneNo);
	paneenabled.erase(paneenabled.begin() + paneNo);
	for(int i = paneNo; i < panes.size(); i++) panes[i]->setNavigationOrder(i);
}

gGUIPane* gGUINavigation::getPane(int paneNo) {
	return panes[paneNo];
}

int gGUINavigation::getPaneNum() {
	return panes.size();
}

void gGUINavigation::setPaneEnabled(int paneNo, bool isEnabled) {
	paneenabled[paneNo] = isEnabled;
}

bool gGUINavigation::isPaneEnabled(int paneNo) {
	return paneenabled[paneNo];
}

void gGUINavigation::clear() {
	panes.clear();
	paneenabled.clear();
}

void gGUINavigation::setSelectedPaneNo(int paneNo) {
	if(paneNo < 0 || paneNo >= panes.size()) return;
	selectedpane = paneNo;
}

int gGUINavigation::getSelectedPaneNo() {
	return selectedpane;
}

gGUIPane* gGUINavigation::getSelectedPane() {
	return panes[selectedpane];
}


void gGUINavigation::showPane(gGUIPane* paneToShow) {
	((gGUISizer*)parent)->setControl(0, 1, paneToShow);
}

void gGUINavigation::mousePressed(int x, int y, int button) {
	gGUIScrollable::mousePressed(x, y, button);

	if(toolbarenabled && x >= 0 && y >= height - 40 && x < width && y < height - 40 + 32) {
		toolbar.mousePressed(x, y, button);
	}
}

void gGUINavigation::mouseReleased(int x, int y, int button) {
	gGUIScrollable::mouseReleased(x, y, button);
	for(int i = 0; i < panes.size(); i++) {
		if(!paneenabled[i]) continue;
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

void gGUINavigation::mouseMoved(int x, int y) {
	if(toolbarenabled && x >= 0 && y >= height - 40 && x < width && y < height - 40 + 32) {
		toolbar.mouseMoved(x, y);
	}
}

void gGUINavigation::mouseDragged(int x, int y, int button) {
	if(toolbarenabled && x >= 0 && y >= height - 40 && x < width && y < height - 40 + 32) {
		toolbar.mouseDragged(x, y, button);
	}
}

void gGUINavigation::mouseEntered() {
	if(toolbarenabled) {
		toolbar.mouseEntered();
	}
}

void gGUINavigation::mouseExited() {
	if(toolbarenabled) {
		toolbar.mouseExited();
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

