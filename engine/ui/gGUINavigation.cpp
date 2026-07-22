/*
 * gGUINavigation.cpp
 *
 *  Created on: Apr 3, 2022
 *      Author: noyan
 */

#include "gGUINavigation.h"
#include "gBaseApp.h"
#include <algorithm>


gGUINavigation::gGUINavigation() {
	panetoph = 100;
	panelineh = 40;
	panelinepad = 20;
	selectedpane = 0;
	toolbarenabled = false;
	bottombarenabled = false;
	bottombarpressactive = false;
	bottombarhorizontalpadding = 16;
	bottombarmaximumcontentwidth = 720;
	modernbottombarenabled = false;
	bottombarsurfaceleft = 0;
	bottombarsurfacetop = 0;
	bottombarsurfacewidth = 0;
	bottombarsurfaceheight = 0;
	bottombarsurfacecolor = gColor(0.98f, 0.985f, 1.0f, 1.0f);
	bottombarshadowcolor = gColor(0.02f, 0.05f, 0.12f, 0.16f);
}

gGUINavigation::~gGUINavigation() {
}

void gGUINavigation::set(gBaseApp* root, gBaseGUIObject* topParentGUIObject, gBaseGUIObject* parentGUIObject, int parentSlotLineNo, int parentSlotColumnNo, int x, int y, int w, int h) {
	totalh = h;
	gGUIScrollable::set(root, topParentGUIObject, parentGUIObject, parentSlotLineNo, parentSlotColumnNo, x, y, w, h);
	gGUIScrollable::setDimensions(w, h);
	if(bottombarenabled) layoutBottomBar();
}

void gGUINavigation::update() {
	if(bottombarenabled) {
		layoutBottomBar();
	} else {
		// Legacy navigation fills the desktop window. Keep this exact behavior
		// unless the new bottom-bar mode was explicitly enabled.
		height = renderer->getHeight();
		maintoolbarsizer.set(0, height - 40, width, 32);
	}
}

void gGUINavigation::draw() {
	gColor oldcolor = *renderer->getColor();
	gGUIScrollable::drawContent();
	if(modernbottombarenabled) {
		renderer->setColor(backgroundcolor);
		gDrawRectangle(left, top, width, height, true);
		const int radius = bottombarsurfaceheight / 2;
		renderer->setColor(&bottombarshadowcolor);
		gDrawRectangle(bottombarsurfaceleft + radius, bottombarsurfacetop + 3,
				std::max(0, bottombarsurfacewidth - radius * 2), bottombarsurfaceheight, true);
		gDrawCircle(bottombarsurfaceleft + radius, bottombarsurfacetop + radius + 3, radius, true, 40);
		gDrawCircle(bottombarsurfaceleft + bottombarsurfacewidth - radius,
				bottombarsurfacetop + radius + 3, radius, true, 40);
		renderer->setColor(&bottombarsurfacecolor);
		gDrawRectangle(bottombarsurfaceleft + radius, bottombarsurfacetop,
				std::max(0, bottombarsurfacewidth - radius * 2), bottombarsurfaceheight, true);
		gDrawCircle(bottombarsurfaceleft + radius, bottombarsurfacetop + radius, radius, true, 40);
		gDrawCircle(bottombarsurfaceleft + bottombarsurfacewidth - radius,
				bottombarsurfacetop + radius, radius, true, 40);
	} else if(bottombarenabled) {
		renderer->setColor(navigationbackgroundcolor);
		gDrawRectangle(left, top, width, height, true);
	} else {
		renderer->setColor(navigationbackgroundcolor);
		gDrawRectangle(0, 0, boxw, boxh + panetoph, true);
	}

	for(int i = 0; !bottombarenabled && i < panes.size(); i++) {
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

	renderer->setColor(&oldcolor);
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
	root->getCurrentCanvas()->onGuiEvent(paneToShow->getId(), G_GUIEVENT_PANEACTIVE);
}

void gGUINavigation::mousePressed(int x, int y, int button) {
	gGUIScrollable::mousePressed(x, y, button);

	if(bottombarenabled && x >= left && y >= top && x < right && y < bottom) {
		// Bottom-bar controls are laid out in absolute GUI coordinates. Forward
		// directly to their sizer so gGUIContainer does not localize them twice.
		// Refresh hover first because mobile touch streams may not send a move.
		toolbarsizer.mouseMoved(x, y);
		toolbarsizer.mousePressed(x, y, button);
		bottombarpressactive = true;
	} else if(toolbarenabled && x >= 0 && y >= height - 40 && x < width && y < height - 8) {
		toolbar.mousePressed(x, y, button);
	}
}

void gGUINavigation::mouseReleased(int x, int y, int button) {
	gGUIScrollable::mouseReleased(x, y, button);
	for(int i = 0; !bottombarenabled && i < panes.size(); i++) {
		if(!paneenabled[i]) continue;
		if(x >= panelinepad && x < width - panelinepad && y >= panetoph + i * panelineh - font->getSize() && y < panetoph + i * panelineh + font->getSize() / 2) {
			selectedpane = i;
			showPane(panes[selectedpane]);
			break;
		}
	}

	if(bottombarenabled && bottombarpressactive) {
		const bool isinside = x >= left && y >= top && x < right && y < bottom;
		// A control which received press must always receive release. Passing an
		// outside coordinate cancels the click while reliably clearing its state.
		toolbarsizer.mouseReleased(isinside ? x : -100000, isinside ? y : -100000, button);
		bottombarpressactive = false;
	} else if(toolbarenabled && x >= 0 && y >= height - 40 && x < width && y < height - 8) {
		toolbar.mouseReleased(x, y, button);
	}
}

void gGUINavigation::mouseMoved(int x, int y) {
	if(bottombarenabled && x >= left && y >= top && x < right && y < bottom) {
		toolbarsizer.mouseMoved(x, y);
	} else if(toolbarenabled && x >= 0 && y >= height - 40 && x < width && y < height - 8) {
		toolbar.mouseMoved(x, y);
	}
}

void gGUINavigation::mouseDragged(int x, int y, int button) {
	if(bottombarenabled && x >= left && y >= top && x < right && y < bottom) {
		toolbarsizer.mouseDragged(x, y, button);
	} else if(toolbarenabled && x >= 0 && y >= height - 40 && x < width && y < height - 8) {
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

void gGUINavigation::enableBottomBar() {
	enableToolbar();
	bottombarenabled = true;
	layoutBottomBar();
}

void gGUINavigation::enableModernBottomBar() {
	enableBottomBar();
	modernbottombarenabled = true;
	toolbar.enableBackgroundFill(false);
	layoutBottomBar();
}

void gGUINavigation::setBottomBarLayout(int horizontalPadding, int maximumContentWidth) {
	bottombarhorizontalpadding = std::max(0, horizontalPadding);
	bottombarmaximumcontentwidth = std::max(0, maximumContentWidth);
	if(bottombarenabled) layoutBottomBar();
}

void gGUINavigation::layoutBottomBar() {
	const int verticalmargin = modernbottombarenabled ? 6 : 0;
	const int surfacemargin = modernbottombarenabled ? 12 : 0;
	bottombarsurfacewidth = std::max(0, width - surfacemargin * 2);
	if(modernbottombarenabled && bottombarmaximumcontentwidth > 0) {
		bottombarsurfacewidth = std::min(bottombarsurfacewidth,
				bottombarmaximumcontentwidth + bottombarhorizontalpadding * 2);
	}
	bottombarsurfaceheight = std::max(0, height - verticalmargin * 2);
	bottombarsurfaceleft = left + (width - bottombarsurfacewidth) / 2;
	bottombarsurfacetop = top + verticalmargin;

	int contentwidth = std::max(0, bottombarsurfacewidth - bottombarhorizontalpadding * 2);
	if(bottombarmaximumcontentwidth > 0) {
		contentwidth = std::min(contentwidth, bottombarmaximumcontentwidth);
	}
	const int contentleft = left + (width - contentwidth) / 2;
	maintoolbarsizer.set(contentleft, bottombarsurfacetop, contentwidth, bottombarsurfaceheight);
}

gGUISizer* gGUINavigation::getToolbarSizer() {
	return &toolbarsizer;
}
