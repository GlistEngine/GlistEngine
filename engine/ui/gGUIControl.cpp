/*
 * gGUIControl.cpp
 *
 *  Created on: Aug 20, 2021
 *      Author: noyan
 */

#include "gGUIControl.h"

gGUIActionManager gGUIControl::actionmanager;


gGUIControl::gGUIControl() {
}

gGUIControl::~gGUIControl() {
}

void gGUIControl::set(gBaseApp* root, gBaseGUIObject* topParentGUIObject, gBaseGUIObject* parentGUIObject, int parentSlotLineNo, int parentSlotColumnNo, int x, int y, int w, int h) {
	this->root = root;
	topparent = topParentGUIObject;
	parent = parentGUIObject;
	parentslotlineno = parentSlotLineNo;
	parentslotcolumnno = parentSlotColumnNo;
	left = x;
	top = y;
	right = x + w;
	bottom = y + h;
	width = w;
	height = h;
}

int gGUIControl::getCursor(int x, int y) {
	return 0;
}

void gGUIControl::update() {

}

void gGUIControl::draw() {
//	gLogi("gGUIControl") << "draw";
}

void gGUIControl::onGUIEvent(int guiObjectId, int eventType, std::string value1, std::string value2) {

}
