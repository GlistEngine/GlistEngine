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
#include "gGUITooltipText.h"
#include "gGUITreelist.h"


gGUIFrame::gGUIFrame() {
	guisizer = nullptr;
	menubar = nullptr;
	statusbar = nullptr;
	contextmenu = nullptr;
	treelist = nullptr;
	for(int i = 0; i < vectooltiptext.size(); i++) { vectooltiptext[i] = nullptr;}
}

gGUIFrame::gGUIFrame(gBaseApp* root) {
	guisizer = nullptr;
	menubar = nullptr;
	statusbar = nullptr;
	contextmenu = nullptr;
	treelist = nullptr;
	setParentSlotNo(0, 0);
	left = 0;
	top = 0;
	right = root->getAppManager()->getCurrentCanvas()->getScreenWidth();
	bottom = root->getAppManager()->getCurrentCanvas()->getScreenHeight();
	width = root->getAppManager()->getCurrentCanvas()->getScreenWidth();
	height = root->getAppManager()->getCurrentCanvas()->getScreenHeight();
	setRootApp(root);
}

gGUIFrame::~gGUIFrame() {
}

void gGUIFrame::update() {
//	gLogi("gGUIFrame") << "update";
	if(guisizer) {
		guisizer->update();
	}
}

void gGUIFrame::draw() {
//	gLogi("gGUIFrame") << "draw";
	if(guisizer) {
		guisizer->draw();
	}
	for(int i = 0; i < toolbarnum; i++) {
		toolbars[i]->draw();
	}
	for(int i = 0; i < verticaltoolbarnum; i++) {
		verticaltoolbars[i]->draw();
	}
	if(menubar) {
		menubar->draw();
	}
	if(statusbar) {
		statusbar->draw();
	}
	if(contextmenu) {
		contextmenu->draw();
	}
	if(treelist) {
		treelist->draw();
	}
	for(int i = 0; i < vectooltiptext.size(); i++) {
		vectooltiptext[i]->draw();
	}
}

int gGUIFrame::getCursor(int x, int y) {
	int cursor = CURSOR_KEEP;
	if(guisizer) {
		cursor = guisizer->getCursor(x, y);
		if (cursor != CURSOR_KEEP) {
			return cursor;
		}
	}
	for(int i = 0; i < toolbarnum; i++) {
		cursor = toolbars[i]->getCursor(x, y);
		if (cursor != CURSOR_KEEP) {
			return cursor;
		}
	}
	for(int i = 0; i < verticaltoolbarnum; i++) {
		cursor = verticaltoolbars[i]->getCursor(x, y);
		if (cursor != CURSOR_KEEP) {
			return cursor;
		}
	}
	if(menubar) {
		cursor = menubar->getCursor(x, y);
		if (cursor != CURSOR_KEEP) {
			return cursor;
		}
	}
	if(statusbar) {
		cursor = statusbar->getCursor(x, y);
		if (cursor != CURSOR_KEEP) {
			return cursor;
		}
	}
	if(contextmenu) {
		cursor = contextmenu->getCursor(x, y);
		if (cursor != CURSOR_KEEP) {
			return cursor;
		}
	}
	if(treelist) {
		cursor = treelist->getCursor(x, y);
		if (cursor != CURSOR_KEEP) {
			return cursor;
		}
	}
	for(int i = 0; i < vectooltiptext.size(); i++) {
		cursor = vectooltiptext[i]->getCursor(x, y);
		if (cursor != CURSOR_KEEP) {
			return cursor;
		}
	}
	return cursor;
}

