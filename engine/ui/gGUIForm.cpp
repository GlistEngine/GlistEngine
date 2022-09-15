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
	issizerset = false;
}

gGUIForm::~gGUIForm() {
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
	if(issizerset) updateSizer();
}

void gGUIForm::resizeMenuBar() {
	menubar->set(root, this, this, 0, 0,
				left,
				top,
				width,
				menuh
		);
}

void gGUIForm::addToolBar(gGUIToolbar* toolBar) {
	if(toolBar->getToolbarType() == gGUIToolbar::HORIZONTAL_TOOLBAR) {
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
		if (verticaltoolbarnum > 0) resizeVerticalToolbars();
	} else {
		if(verticaltoolbarnum >= maxtoolbarnum) return;

		verticaltoolbarw = 32;

		verticaltoolbars[verticaltoolbarnum] = toolBar;
		verticaltoolbars[verticaltoolbarnum]->set(left,
					top + menuh + toolbarh,
					verticaltoolbarw,
					height
			);
		verticaltoolbars[verticaltoolbarnum]->setTopParent(this);
		verticaltoolbars[verticaltoolbarnum]->setParent(this);
		verticaltoolbars[verticaltoolbarnum]->setParentSlotNo(0, 0);
		verticaltoolbars[verticaltoolbarnum]->setRootApp(root);
		verticaltoolbarnum++;
	}

	if(issizerset) updateSizer();
}

void gGUIForm::resizeToolbars() {
	for(int i = 0; i < toolbarnum; i++) {
		toolbars[i]->set(left,
				top + menuh,
				width,
				toolbarh
			);
	}
}

void gGUIForm::resizeVerticalToolbars() {
	for(int i = 0; i < verticaltoolbarnum; i++) {
		verticaltoolbars[i]->set(left,
				top + menuh + toolbarh,
				verticaltoolbarw,
				height
			);
	}
}

void gGUIForm::addContextMenu(gGUIContextMenu* contextMenu) {
	contextmenu = contextMenu;
	contextmenuw = 50;
	contextmenuh = 50;
	contextmenu->set(root, this, this, 0, 0, contextmenux, contextmenuy, contextmenuw, contextmenuh);
}

void gGUIForm::addTreelist(gGUITreelist* treeList, int treeListx, int treeListy, int treeListw) {
	treelist = treeList;
	treelistw = 10;
	treelisth = 188;
	treelist->set(root, this, this, 0, 0, treeListx, treeListy, treeListw, treelisth);
}

void gGUIForm::setToolbarHeight(int toolbarHeight) {
	toolbarh = toolbarHeight;
	resizeToolbars();
	if(issizerset) updateSizer();
}

int gGUIForm::getToolbarHeight() {
	return toolbarh;
}

void gGUIForm::setVerticalToolbarWidth(int verticalToolbarWidth) {
	verticaltoolbarw = verticalToolbarWidth;
	resizeVerticalToolbars();
	if(issizerset) updateSizer();
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
	guisizer->left = left + verticaltoolbarw;
	guisizer->top = top + menuh + toolbarh;
	guisizer->right = right;
	guisizer->bottom = bottom;
	guisizer->width = right - guisizer->left;
	guisizer->height = bottom - guisizer->top;
	guisizer->setSlotPadding(0);
	issizerset = true;
}

void gGUIForm::updateSizer() {
	guisizer->left = left + verticaltoolbarw;
	guisizer->top = top + menuh + toolbarh;
	guisizer->right = right;
	guisizer->bottom = bottom;
	guisizer->width = right - guisizer->left;
	guisizer->height = bottom - guisizer->top;
}

void gGUIForm::setStatusBar(gGUIStatusBar* statusBar) {
	statusbar = statusBar;
	statush = 30;
	statusbar->set(root, this, this, 0, 0,
					left,
					statush,
					width,
					statush
			);
	if(issizerset) updateSizer();
}

void gGUIForm::resizeStatusBar() {
	statusbar->set(root, this, this, 0, 0,
					left,
					statush,
					width,
					statush
		);
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
}

void gGUIForm::mousePressed(int x, int y, int button) {
	if(statusbar) statusbar->mousePressed(x, y, button);
	if(menubar) menubar->mousePressed(x, y, button);
	for(int i = 0; i < toolbarnum; i++) toolbars[i]->mousePressed(x, y, button);
	for(int i = 0; i < verticaltoolbarnum; i++) verticaltoolbars[i]->mousePressed(x, y, button);
	if(guisizer) guisizer->mousePressed(x, y, button);
	if(contextmenu) contextmenu->mousePressed(x, y, button);
}

void gGUIForm::mouseDragged(int x, int y, int button) {
	if(statusbar) statusbar->mouseDragged(x, y, button);
	if(menubar) menubar->mouseDragged(x, y, button);
	for(int i = 0; i < toolbarnum; i++) toolbars[i]->mouseDragged(x, y, button);
	for(int i = 0; i < verticaltoolbarnum; i++) verticaltoolbars[i]->mouseDragged(x, y, button);
	if(guisizer) guisizer->mouseDragged(x, y, button);
	if(contextmenu) contextmenu->mouseDragged(x, y, button);
}

void gGUIForm::mouseReleased(int x, int y, int button) {
	if(statusbar) statusbar->mouseReleased(x, y, button);
	if(menubar) menubar->mouseReleased(x, y, button);
	for(int i = 0; i < toolbarnum; i++) toolbars[i]->mouseReleased(x, y, button);
	for(int i = 0; i < verticaltoolbarnum; i++) verticaltoolbars[i]->mouseReleased(x, y, button);
	if(guisizer) guisizer->mouseReleased(x, y, button);
	if(contextmenu) contextmenu->mouseReleased(x, y, button);
}

void gGUIForm::mouseScrolled(int x, int y) {
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
	if(statusbar) {
		resizeStatusBar();
		statusbar->windowResized(w, h);
	}
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
	if(guisizer) {
		guisizer->set(root, this, this, 0, 0, left + (verticaltoolbarnum?verticaltoolbarnum*verticaltoolbars[0]->width:0), top + (menubar?menubar->height:0) + (toolbarnum?toolbarnum*toolbars[0]->height:0), w, h);
		guisizer->windowResized(w, h);
	}
}


