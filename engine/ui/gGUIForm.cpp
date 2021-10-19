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
	return guisizer->getCursor(x, y);
}

void gGUIForm::show() {
	isshown = true;
}

void gGUIForm::hide() {
	isshown = false;
}

void gGUIForm::keyPressed(int key) {
	guisizer->keyPressed(key);
}

void gGUIForm::keyReleased(int key) {
	guisizer->keyReleased(key);
}

void gGUIForm::charPressed(unsigned int codepoint) {
	guisizer->charPressed(codepoint);
}

void gGUIForm::mouseMoved(int x, int y) {
	menubar->mouseMoved(x, y);
	for(int i = 0; i < toolbarnum; i++) toolbars[i]->mouseMoved(x, y);
	guisizer->mouseMoved(x, y);
}

void gGUIForm::mousePressed(int x, int y, int button) {
	menubar->mousePressed(x, y, button);
	for(int i = 0; i < toolbarnum; i++) toolbars[i]->mousePressed(x, y, button);
	guisizer->mousePressed(x, y, button);
}

void gGUIForm::mouseDragged(int x, int y, int button) {
	guisizer->mouseDragged(x, y, button);
}

void gGUIForm::mouseReleased(int x, int y, int button) {
	for(int i = 0; i < toolbarnum; i++) toolbars[i]->mouseReleased(x, y, button);
	guisizer->mouseReleased(x, y, button);
}


