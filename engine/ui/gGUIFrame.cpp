/*
 * gFrame.cpp
 *
 *  Created on: Aug 11, 2021
 *      Author: noyan
 */

#include "gGUIFrame.h"
#include "gGUISizer.h"
#include "gGUIMenubar.h"
#include "gGUIToolbar.h"


gGUIFrame::gGUIFrame() {
	guisizer = nullptr;
	menubar = nullptr;
}

gGUIFrame::~gGUIFrame() {
}

void gGUIFrame::update() {
//	gLogi("gGUIFrame") << "update";
	if(guisizer) guisizer->update();
}

void gGUIFrame::draw() {
//	gLogi("gGUIFrame") << "draw";
	if(guisizer) guisizer->draw();
	if(toolbarnum > 0) for(int i = 0; i < toolbarnum; i++) toolbars[i]->draw();
	if(menubar) menubar->draw();
}
