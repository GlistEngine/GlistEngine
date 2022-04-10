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


gGUIForm::gGUIForm() {
	menuh = 0;
	toolbarh = 0;
	toolbarnum = 0;
}

gGUIForm::~gGUIForm() {
}

void gGUIForm::setMenuBar(gGUIMenubar* menuBar) {
	menubar = menuBar;
	menuh = 30;
	menubar->set(root, this, 0, 0,
				left,
				top,
				width,
				menuh
		);
}

void gGUIForm::resizeMenuBar() {
	menubar->set(root, this, 0, 0,
				left,
				top,
				width,
				menuh
		);
}

void gGUIForm::addToolBar(gGUIToolbar* toolBar) {
	if(toolbarnum >= maxtoolbarnum) return;

	toolbarh = 32;

	toolbars[toolbarnum] = toolBar;
	toolbars[toolbarnum]->set(left,
				top + menuh,
				width,
				toolbarh
		);
	toolbars[toolbarnum]->setParent(this);
	toolbars[toolbarnum]->setParentSlotNo(0, 0);
	toolbars[toolbarnum]->setRootApp(root);
	toolbarnum++;
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

void gGUIForm::setSizer(gGUISizer* guiSizer) {
	guisizer = guiSizer;
	guisizer->setParent(this);
	guisizer->setParentSlotNo(0, 0);
	guisizer->setRootApp(root);
	guisizer->enableBackgroundFill(true);
	guisizer->iscursoron = false;
	guisizer->left = left;
	guisizer->top = top + menuh + toolbarh;
	guisizer->right = right;
	guisizer->bottom = bottom;
	guisizer->width = width;
	guisizer->height = height;
	guisizer->setSlotPadding(0);
}

gGUISizer* gGUIForm::getSizer() {
	return guisizer;
}

int gGUIForm::getCursor(int x, int y) {
	if(guisizer) return guisizer->getCursor(x, y);
	return CURSOR_ARROW;
}

void gGUIForm::show() {
	isshown = true;
}

void gGUIForm::hide() {
	isshown = false;
}

void gGUIForm::keyPressed(int key) {
	if(guisizer) guisizer->keyPressed(key);
}

void gGUIForm::keyReleased(int key) {
	if(guisizer) guisizer->keyReleased(key);
}

void gGUIForm::charPressed(unsigned int codepoint) {
	if(guisizer) guisizer->charPressed(codepoint);
}

void gGUIForm::mouseMoved(int x, int y) {
	if(menubar) menubar->mouseMoved(x, y);
	for(int i = 0; i < toolbarnum; i++) toolbars[i]->mouseMoved(x, y);
	if(guisizer) guisizer->mouseMoved(x, y);
}

void gGUIForm::mousePressed(int x, int y, int button) {
	if(menubar) menubar->mousePressed(x, y, button);
	for(int i = 0; i < toolbarnum; i++) toolbars[i]->mousePressed(x, y, button);
	if(guisizer) guisizer->mousePressed(x, y, button);
}

void gGUIForm::mouseDragged(int x, int y, int button) {
	if(guisizer) guisizer->mouseDragged(x, y, button);
}

void gGUIForm::mouseReleased(int x, int y, int button) {
	for(int i = 0; i < toolbarnum; i++) toolbars[i]->mouseReleased(x, y, button);
	if(guisizer) guisizer->mouseReleased(x, y, button);
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
	if(menubar) {
		resizeMenuBar();
		menubar->windowResized(w, h);
	}
	if(toolbarnum > 0) {
		resizeToolbars();
		for(int i = 0; i < toolbarnum; i++) toolbars[i]->windowResized(w, h);
	}
	if(guisizer) {
		guisizer->set(guisizer->left, guisizer->top, w, h);
		guisizer->windowResized(w, h);
	}
}


