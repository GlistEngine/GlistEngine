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
#include "gGUIStatusBar.h"
#include "gGUIContextMenu.h"


gGUIFrame::gGUIFrame() {
	guisizer = nullptr;
	menubar = nullptr;
	statusbar = nullptr;
	contextmenu = nullptr;
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
	if(verticaltoolbarnum > 0) for(int i = 0; i < verticaltoolbarnum; i++) verticaltoolbars[i]->draw();
	if(menubar) menubar->draw();
	if(statusbar) statusbar->draw();
	if(contextmenu) contextmenu->draw();
}
