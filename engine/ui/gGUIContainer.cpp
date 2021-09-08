/*
 * gGUIContainer.cpp
 *
 *  Created on: Aug 20, 2021
 *      Author: noyan
 */

#include "gGUIContainer.h"


gGUIContainer::gGUIContainer() {
	topbarh = 0;
}

gGUIContainer::~gGUIContainer() {
}

void gGUIContainer::set(int x, int y, int w, int h) {
	left = x;
	top = y;
	right = x + w;
	bottom = y + h;
	width = w;
	height = h;
	guisizer->set(x, y + topbarh, w, h - topbarh);
}

void gGUIContainer::setSizer(gGUISizer* guiSizer) {
	guisizer = guiSizer;
	guisizer->setParent(this);
	guisizer->setParentSlotNo(0, 0);
	guisizer->setRootApp(root);
//	guisizer->set(left, top, width, height);
	guisizer->left = left;
	guisizer->top = top + topbarh;
	guisizer->right = right;
	guisizer->bottom = bottom;
	guisizer->width = width;
	guisizer->height = height - topbarh;
}

gGUISizer* gGUIContainer::getSizer() {
	return guisizer;
}

void gGUIContainer::update() {
	guisizer->update();
}

void gGUIContainer::draw() {
	guisizer->draw();
}

int gGUIContainer::getCursor(int x, int y) {
	return guisizer->getCursor(x, y);
}

void gGUIContainer::keyPressed(int key) {
	guisizer->keyPressed(key);
}

void gGUIContainer::keyReleased(int key) {
	guisizer->keyReleased(key);
}

void gGUIContainer::charPressed(unsigned int codepoint) {
	guisizer->charPressed(codepoint);
}

void gGUIContainer::mouseMoved(int x, int y) {
	guisizer->mouseMoved(x, y);
}

void gGUIContainer::mousePressed(int x, int y, int button) {
	guisizer->mousePressed(x, y, button);
}

void gGUIContainer::mouseDragged(int x, int y, int button) {
	guisizer->mouseDragged(x, y, button);
}

void gGUIContainer::mouseReleased(int x, int y, int button) {
	guisizer->mouseReleased(x, y, button);
}

void gGUIContainer::mouseScrolled(int x, int y) {
	guisizer->mouseScrolled(x, y);
}

