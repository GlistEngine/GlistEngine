/*
 * gGUIControl.cpp
 *
 *  Created on: Aug 20, 2021
 *      Author: noyan
 */

#include "gGUIControl.h"


gGUIControl::gGUIControl() {
}

gGUIControl::~gGUIControl() {
}

void gGUIControl::set(gBaseApp* root, gBaseGUIObject* parentGUIObject, int parentSlotLineNo, int parentSlotColumnNo, int x, int y, int w, int h) {
	this->root = root;
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

