/*
 * gGUITopContainer.cpp
 *
 *  Created on: Aug 20, 2021
 *      Author: noyan
 */

#include "gGUIForm.h"
#include "gGUISizer.h"
#include "gGUIMenubar.h"
#include "gGUIToolbar.h"
#include "gGUIContextMenu.h"
#include "gGUIStatusBar.h"
#include "gGUITooltipText.h"
#include "gGUITreelist.h"


gGUIForm::gGUIForm() {
	topparent = this;
	parent = this;
	menuh = 0;
	toolbarh = 0;
	verticaltoolbarw = 0;
	toolbarnum = 0;
	verticaltoolbarnum = 0;
	statush = 0;
	sizerh = 0;
	focusid = 0;
	previousfocusid = 0;
	contextmenuw = 0;
	contextmenuh = 0;
	contextmenux = 0;
	contextmenuy = 0;
	treelisth = 0;
	treelistw = 0;
	menubar = nullptr;
	isshown = true;
	contextmenu = nullptr;
	statusw = 0;
	statusbar = nullptr;
	guisizer = nullptr;
	treelist = nullptr;
	isprogressshown = false;
	cpcolor = gColor(1.0f, 1.0f, 1.0f);
	cprdeg = 0.0f;
	cpw = 256;
	cph = 256;
	cpx = 0;
	cpy = 0;
	cpspeed = 1.0f;
	temporaryemptysizer = new gGUISizer();
	setSizer(temporaryemptysizer);
	temporaryemptysizer->setSize(1, 1);
}

gGUIForm::~gGUIForm() {
	if(temporaryemptysizer != nullptr) delete temporaryemptysizer;
}

void gGUIForm::setMenuBar(gGUIMenubar* menuBar) {
	menubar = menuBar;
	menuh = 30;
	menubar->set(root, this, this, 0, 0,
				left,
				top,
				width,
				menuh
		);
	resizeAll(RESIZE_MENUBAR);
}

void gGUIForm::resizeMenuBar() {
	menubar->set(root, this, this, 0, 0,
				left,
				top,
				width,
				menuh
		);
//	resizeAll(RESIZE_MENUBAR);
}

void gGUIForm::addToolBar(gGUIToolbar* toolBar) {
	if(toolBar->getToolbarType() == gGUIToolbar::TOOLBAR_HORIZONTAL) {
		if(toolbarnum >= maxtoolbarnum) return;

		toolbarh = 32;

		toolbars[toolbarnum] = toolBar;
		toolbars[toolbarnum]->set(left,
					top + menuh,
					width,
					toolbarh
			);
		toolbars[toolbarnum]->setTopParent(this);
		toolbars[toolbarnum]->setParent(this);
		toolbars[toolbarnum]->setParentSlotNo(0, 0);
		toolbars[toolbarnum]->setRootApp(root);
		toolbarnum++;
		resizeAll(RESIZE_TOOLBAR_HORIZONTAL);
	} else {
		if(verticaltoolbarnum >= maxtoolbarnum) return;

		verticaltoolbarw = 34;

		verticaltoolbars[verticaltoolbarnum] = toolBar;
		verticaltoolbars[verticaltoolbarnum]->set(left,
					top + menuh + toolbarh * toolbarnum,
					verticaltoolbarw,
					height
			);
		verticaltoolbars[verticaltoolbarnum]->setTopParent(this);
		verticaltoolbars[verticaltoolbarnum]->setParent(this);
		verticaltoolbars[verticaltoolbarnum]->setParentSlotNo(0, 0);
		verticaltoolbars[verticaltoolbarnum]->setRootApp(root);
		verticaltoolbarnum++;

		resizeAll(RESIZE_TOOLBAR_VERTICAL);
	}
}

void gGUIForm::resizeToolbars() {
	for(int i = 0; i < toolbarnum; i++) {
		toolbars[i]->set(left,
				top + menuh,
				width,
				toolbarh
			);
	}
//	resizeAll(RESIZE_TOOLBAR_HORIZONTAL);
}

void gGUIForm::resizeVerticalToolbars() {
	for(int i = 0; i < verticaltoolbarnum; i++) {
		verticaltoolbars[i]->set(left,
				top + menuh + toolbarh * toolbarnum,
				verticaltoolbarw,
				height
			);
	}
//	resizeAll(RESIZE_TOOLBAR_VERTICAL);
}


void gGUIForm::setStatusBar(gGUIStatusBar* statusBar) {
	statusbar = statusBar;
	statush = 30;
	statusbar->set(root, this, this, 0, 0,
					left,
					height - statush,
					width,
					statush
			);
	if(guisizer) updateSizer();
}

void gGUIForm::resizeStatusBar() {
	statusbar->set(root, this, this, 0, 0,
					left,
					height - statush,
					width,
					statush
		);
//	resizeAll(RESIZE_STATUSBAR);
}

void gGUIForm::addContextMenu(gGUIContextMenu* contextMenu) {
	contextmenu = contextMenu;
	contextmenuw = 50;
	contextmenuh = 50;
	contextmenu->set(root, this, this, 0, 0, contextmenux, contextmenuy, contextmenuw, contextmenuh);
}

void gGUIForm::addTreelist(gGUITreelist* treeList, int treeListx, int treeListy, int treeListw) {
	treelist = treeList;
	if(treelist) {
		treelistw = treeListw;
		treelisth = 188;
		treelist->set(root, this, this, 0, 0, treeListx, treeListy, treeListw, treelisth);
	}
}

void gGUIForm::resizeAll(int resizeCode) {
	if(resizeCode == RESIZE_MENUBAR || resizeCode == RESIZE_STATUSBAR) {
		resizeToolbars();
		resizeVerticalToolbars();
	} else if(resizeCode == RESIZE_TOOLBAR_HORIZONTAL) resizeVerticalToolbars();

	if(guisizer) updateSizer();
}

void gGUIForm::setToolbarHeight(int toolbarHeight) {
	toolbarh = toolbarHeight;
	resizeToolbars();
}

int gGUIForm::getToolbarHeight() {
	return toolbarh;
}

void gGUIForm::setVerticalToolbarWidth(int verticalToolbarWidth) {
	verticaltoolbarw = verticalToolbarWidth;
	resizeVerticalToolbars();
}

int gGUIForm::getVerticalToolbarWidth() {
	return verticaltoolbarw;
}

void gGUIForm::setSizer(gGUISizer* guiSizer) {
	guisizer = guiSizer;
	guisizer->setTopParent(this);
	guisizer->setParent(this);
	guisizer->setParentSlotNo(0, 0);
	guisizer->setRootApp(root);
	guisizer->enableBackgroundFill(true);
	guisizer->iscursoron = false;
	guisizer->left = left + verticaltoolbarnum * verticaltoolbarw;
	guisizer->top = top + menuh + toolbarnum * toolbarh;
	guisizer->right = right;
	guisizer->bottom = bottom - statush;
	guisizer->width = guisizer->right - guisizer->left;
	guisizer->height = guisizer->bottom - guisizer->top;
	guisizer->setSlotPadding(0, 0);
}

void gGUIForm::updateSizer() {
	guisizer->set(root, this, this, 0, 0,
			left + (verticaltoolbarnum ? verticaltoolbarnum * verticaltoolbars[0]->width : 0),
			top + (menubar ? menubar->height : 0) + (toolbarnum ? toolbarnum * toolbars[0]->height : 0),
			width - (verticaltoolbarnum ? verticaltoolbarnum * verticaltoolbars[0]->width : 0),
			height - (statusbar ? statusbar->height : 0) - (menubar ? menubar->height : 0) - (toolbarnum ? toolbarnum * toolbars[0]->height : 0));
}


gGUISizer* gGUIForm::getSizer() {
	return guisizer;
}

int gGUIForm::getCursor(int x, int y) {
	if(guisizer) return guisizer->getCursor(x, y);
	return CURSOR_ARROW;
}

int gGUIForm::getFocusId() {
	return focusid;
}

int gGUIForm::getPreviousFocusId() {
	return previousfocusid;
}


void gGUIForm::show() {
	isshown = true;
}

void gGUIForm::hide() {
	isshown = false;
}

void gGUIForm::keyPressed(int key) {
	if(guisizer) guisizer->keyPressed(key);
	if(statusbar) statusbar->keyPressed(key);
}

void gGUIForm::keyReleased(int key) {
	if(guisizer) guisizer->keyReleased(key);
	if(statusbar) statusbar->keyPressed(key);
}

void gGUIForm::charPressed(unsigned int codepoint) {
	if(guisizer) guisizer->charPressed(codepoint);
}

void gGUIForm::mouseMoved(int x, int y) {
	if(statusbar) statusbar->mouseMoved(x, y);
	if(menubar) menubar->mouseMoved(x, y);
	for(int i = 0; i < toolbarnum; i++) toolbars[i]->mouseMoved(x, y);
	for(int i = 0; i < verticaltoolbarnum; i++) verticaltoolbars[i]->mouseMoved(x, y);
	if(guisizer) {
		if(x >= guisizer->left && x < guisizer->right && y >= guisizer->top && y < guisizer->bottom) {
			guisizer->iscursoron = true;
			guisizer->mouseMoved(x, y);
		}
	}
	if(contextmenu) contextmenu->mouseMoved(x, y);
	if(treelist) treelist->mouseMoved(x, y);
	for(int i = 0; i < vectooltiptext.size(); i++) { vectooltiptext[i]->mouseMoved(x, y);}
}

void gGUIForm::mousePressed(int x, int y, int button) {
	if(statusbar) statusbar->mousePressed(x, y, button);
	if(menubar) menubar->mousePressed(x, y, button);
	for(int i = 0; i < toolbarnum; i++) toolbars[i]->mousePressed(x, y, button);
	for(int i = 0; i < verticaltoolbarnum; i++) verticaltoolbars[i]->mousePressed(x, y, button);
	if(treelist) {
		treelist->mousePressed(x, y, button);
		return;
	}
	if(contextmenu) contextmenu->mousePressed(x, y, button);
	if(guisizer) guisizer->mousePressed(x, y, button);
}

void gGUIForm::mouseDragged(int x, int y, int button) {
	if(statusbar) statusbar->mouseDragged(x, y, button);
	if(menubar) menubar->mouseDragged(x, y, button);
	for(int i = 0; i < toolbarnum; i++) toolbars[i]->mouseDragged(x, y, button);
	for(int i = 0; i < verticaltoolbarnum; i++) verticaltoolbars[i]->mouseDragged(x, y, button);
	if(treelist) {
		treelist->mouseDragged(x, y, button);
		return;
	}
	if(contextmenu) contextmenu->mouseDragged(x, y, button);
	if(guisizer) guisizer->mouseDragged(x, y, button);
}

void gGUIForm::mouseReleased(int x, int y, int button) {
	if(statusbar) statusbar->mouseReleased(x, y, button);
	if(menubar) menubar->mouseReleased(x, y, button);
	for(int i = 0; i < toolbarnum; i++) toolbars[i]->mouseReleased(x, y, button);
	for(int i = 0; i < verticaltoolbarnum; i++) verticaltoolbars[i]->mouseReleased(x, y, button);
	if(treelist) {
		treelist->mouseReleased(x, y, button);
		return;
	}
	if(contextmenu) contextmenu->mouseReleased(x, y, button);
	if(guisizer) guisizer->mouseReleased(x, y, button);
}

void gGUIForm::mouseScrolled(int x, int y) {
	if(treelist) {
		treelist->mouseScrolled(x, y);
		return;
	}
	if(guisizer) guisizer->mouseScrolled(x, y);
}

void gGUIForm::mouseEntered() {
	if(guisizer) guisizer->mouseEntered();
}

void gGUIForm::mouseExited() {
	if(guisizer) guisizer->mouseExited();
}

void gGUIForm::windowResized(int w, int h) {
	if(w == 0 || h == 0) return;
	width = w;
	height = h;
	if(menubar) {
		resizeMenuBar();
		menubar->windowResized(w, h);
	}
	if(toolbarnum > 0) {
		resizeToolbars();
		for(int i = 0; i < toolbarnum; i++) toolbars[i]->windowResized(w, h);
	}
	if(verticaltoolbarnum > 0) {
		resizeVerticalToolbars();
		for(int i = 0; i < verticaltoolbarnum; i++) verticaltoolbars[i]->windowResized(w, h);
	}
	if(statusbar) {
		resizeStatusBar();
		statusbar->windowResized(w, h);
	}
	if(guisizer) {
//		updateSizer();
		guisizer->set(left + (verticaltoolbarnum ? verticaltoolbarnum * verticaltoolbars[0]->width : 0),
				top + (menubar ? menubar->height : 0) + (toolbarnum ? toolbarnum * toolbars[0]->height : 0),
				width - (verticaltoolbarnum ? verticaltoolbarnum * verticaltoolbars[0]->width : 0),
				height - (statusbar ? statusbar->height : 0) - (menubar ? menubar->height : 0) - (toolbarnum ? toolbarnum * toolbars[0]->height : 0));

		guisizer->windowResized(w, h);
	}
}

void gGUIForm::setTooltipText(gGUITooltipText *tooltiptext) {
	this->vectooltiptext.push_back(tooltiptext);
}

void gGUIForm::showProgressBar(int style, gColor col, float speed) {
	cpw = 256;
	cph = 256;
	cpx = (width - cpw) / 2;
	cpy = (height - cph) / 2;
	cprdeg = 0.0f;
	cpcolor = col;
	cpspeed = speed;
	isprogressshown = true;
}

void gGUIForm::hideProgressBar() {
	isprogressshown = false;
}

bool gGUIForm::isProgressShown() {
	return isprogressshown;
}

