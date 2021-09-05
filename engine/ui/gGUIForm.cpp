/*
 * gGUITopContainer.cpp
 *
 *  Created on: Aug 20, 2021
 *      Author: noyan
 */

#include "gGUIForm.h"
#include "gGUISizer.h"


gGUIForm::gGUIForm() {
}

gGUIForm::~gGUIForm() {
}

void gGUIForm::setSizer(gGUISizer* guiSizer) {
	guisizer = guiSizer;
	guisizer->setParent(this);
	guisizer->setParentSlotNo(0, 0);
	guisizer->setRootApp(root);
	guisizer->enableBackgroundFill(true);
	guisizer->iscursoron = true;
	guisizer->left = left;
	guisizer->top = top;
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

void gGUIForm::charPressed(unsigned int key) {
	guisizer->charPressed(key);
}

void gGUIForm::mouseMoved(int x, int y) {
	guisizer->mouseMoved(x, y);
}

void gGUIForm::mousePressed(int x, int y, int button) {
	guisizer->mousePressed(x, y, button);
}

void gGUIForm::mouseDragged(int x, int y, int button) {
	guisizer->mouseDragged(x, y, button);
}

void gGUIForm::mouseReleased(int x, int y, int button) {
	guisizer->mouseReleased(x, y, button);
}


