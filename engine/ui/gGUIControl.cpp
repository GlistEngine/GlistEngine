/*
 * gGUIControl.cpp
 *
 *  Created on: Aug 20, 2021
 *      Author: noyan
 */

#include "gGUIControl.h"


gGUIControl::gGUIControl() {
	countasspace = false;
	oldwidth = 0;
	oldheight = 0;
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
	oldwidth = width;
	if(oldwidth == 0) oldwidth = w;
	oldheight = height;
	if(oldheight == 0) oldheight = h;
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

bool gGUIControl::countAsSpace() {
	return countasspace;
}
