/*
 * gGUIActionManager.cpp
 *
 *  Created on: 22 Aðu 2022
 *      Author: sevva
 */

#include <gGUIActionManager.h>
#include "gGUIControl.h"

gGUIActionManager::gGUIActionManager() {
}

gGUIActionManager::~gGUIActionManager() {
}

void gGUIActionManager::addAction(gBaseGUIObject* srcControl, int srcEvent, gBaseGUIObject* dstControl, int dstEvent) {
	Action a;
	a.sourceControl = srcControl;
	a.sourceEvent = srcEvent;
	a.targetControl = dstControl;
	a.targetEvent = dstEvent;
	actions.push_back(a);
}

void gGUIActionManager::onGUIEvent(int guiObjectId, int eventType, std::string value1, std::string value2) {
	for(int i = 0; i < actions.size(); i++) {
		if(actions[i].sourceEvent == eventType && actions[i].sourceControl->id == guiObjectId) {
			actions[i].targetControl->onGUIEvent(guiObjectId, actions[i].targetEvent, value1, value2);
		}
	}
}
