/*
 * gGUIActionManager.cpp
 *
 *  Created on: 22 Aðu 2022
 *      Author: sevva
 */

#include <gGUIActionManager.h>
#include "gGUIControl.h"

gGUIActionManager::gGUIActionManager() {
	// TODO Auto-generated constructor stub

}

gGUIActionManager::~gGUIActionManager() {
	// TODO Auto-generated destructor stub
}

void gGUIActionManager::addAction(gGUIControl* srcControl, int srcEvent, gGUIControl* dstControl, int dstEvent, std::string value) {
	Action a;
	a.sourceControl = srcControl;
	a.sourceEvent = srcEvent;
	a.targetControl = dstControl;
	a.targetEvent = dstEvent;
	a.value = value;
	actions.push_back(a);
}

void gGUIActionManager::onGUIEvent(int guiObjectId, int eventType, std::string value1, std::string value2) {
	for(int i = 0; i < actions.size(); i++) {
		if(actions[i].sourceEvent == eventType && actions[i].sourceControl->id == guiObjectId) {
			actions[i].targetControl->onGUIEvent(guiObjectId, actions[i].targetEvent, value1, value2);
		}
	}
}
