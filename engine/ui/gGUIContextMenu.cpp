/*
 * gGUIContextMenu.cpp
 *
 *  Created on: 29 Jul 2022
 *      Author: Ipek Senturk
 */

#include "gGUIContextMenu.h"


int gGUIContextMenuItem::lastitemid;


gGUIContextMenuItem::gGUIContextMenuItem(std::string text, gImage* menuIcon) {
	itemid = lastitemid++;
	parentitemid = 0;
	this->text = text;
	title = text;
	contextmenux = 0;
	contextmenuy = 0;
	contextmenudefaultw = 295;
	contextmenuh = 0;
	contextmenuleftmargin = 45;
	contextmenulineh = 2 * font->getSize() + 2;
	datady = (contextmenulineh - font->getStringHeight("ae")) / 2 + 1;
	contextmenuw = 0;
	contextmenushown = false;
	hovered = false;
	seperatoradded = false;
	nullicon = nullptr;
	menuicon = menuIcon;
	menuiconx = 0;
	menuicony = 0;
	menuiconw = 0;
	menuiconh = 0;
}

gGUIContextMenuItem::~gGUIContextMenuItem() {

}

int gGUIContextMenuItem::getItemId() {
	return itemid;
}

void gGUIContextMenuItem::setParentItemId(int itemId) {
	parentitemid = itemId;
}

int gGUIContextMenuItem::getParentItemId() {
	return parentitemid;
}

gGUIContextMenuItem* gGUIContextMenuItem::getItem(int itemNo) {
	return &items[itemNo];
}

int gGUIContextMenuItem::getContextMenuLeftMargin() {
	return contextmenuleftmargin;
}

bool gGUIContextMenuItem::getContextMenuShown() {
	return contextmenushown;
}

void gGUIContextMenuItem::setContextMenuLeftMargin(int leftMargin) {
	contextmenuleftmargin = leftMargin;
}

int gGUIContextMenuItem::addItem(std::string text,  gImage* menuIcon) {
	int itemno = items.size();
	menuicon = menuIcon;
	items.push_back(gGUIContextMenuItem(text, menuicon));
	items[itemno].setParentItemId(itemid);
	contextmenuh = items.size() * contextmenulineh;
	gLogi("MenuItem") << "text:" << text << ", parentitemid:" << items[itemno].getParentItemId();
	return items[itemno].getItemId();
}

void gGUIContextMenuItem::addSeperator(int itemNo) {
	items[itemNo].seperatoradded = true;
}

void gGUIContextMenuItem::drawMenuItem() {
	if(contextmenushown) {
//		context menu on the right
		if((contextmenux + contextmenudefaultw) >= getScreenWidth()) {
			contextmenux -= contextmenudefaultw;
			renderer->setColor(middlegroundcolor);
			gDrawRectangle(contextmenux, contextmenuy, contextmenudefaultw, contextmenuh, true);
//		context menu at the bottom
		} else if((contextmenuy + contextmenuh) >= getScreenHeight()) {
			contextmenuy -= contextmenuh;
			renderer->setColor(middlegroundcolor);
			gDrawRectangle(contextmenux, contextmenuy, contextmenudefaultw, contextmenuh, true);
		} else {
			renderer->setColor(middlegroundcolor);
			gDrawRectangle(contextmenux, contextmenuy, contextmenudefaultw, contextmenuh, true);
		}

		if(parentitemid == 0) {
			for(int i = 0; i < items.size(); i++) {
				items[i].set(root, topparent, this, 0, 0, contextmenux, contextmenuy + i * contextmenulineh, contextmenudefaultw, contextmenulineh);
				items[i].contextmenux = items[i].left;
				items[i].contextmenuy = items[i].top;
				items[i].contextmenuw = contextmenudefaultw;
				items[i].contextmenuh = contextmenulineh;
				if(items[i].menuicon != nullptr) {
					items[i].menuiconh = contextmenulineh * 2 / 3;
					items[i].menuiconw = items[i].menuiconh;
					items[i].menuiconx = items[i].left + items[i].menuiconw * 3 / 4;
					items[i].menuicony = items[i].top + items[i].menuiconh / 4;
				}
//				gLogi("item") << i << ": x:" << items[i].left << ", y:" << items[i].top << ", right:" << items[i].right << ", bottom:" << items[i].bottom << ", w:" << items[i].width << ", h:" << items[i].height;
				items[i].drawMenuItem();
//				highlights
				if(items[i].hovered) {
					renderer->setColor(foregroundcolor);
					gDrawRectangle(items[i].left, items[i].top, items[i].contextmenuw, items[i].contextmenuh, true);
				}
				if(items[i].menuicon != nullptr) items[i].menuicon->draw(items[i].menuiconx, items[i].menuicony, items[i].menuiconw, items[i].menuiconh);
			}
//			text
			for(int i = 0; i < items.size(); i++) {
				renderer->setColor(fontcolor);
				font->drawText(items[i].title, items[i].left + contextmenuleftmargin, items[i].top + items[i].contextmenuh - datady);
			}
//			seperators
			for(int i = 0; i < items.size(); i++) {
				if(items[i].seperatoradded == true) {
					renderer->setColor(backgroundcolor);
					gDrawLine(items[i].contextmenux + 10, items[i].bottom, items[i].right - 10, items[i].bottom);
				}
			}
// 		submenu part
//		incomplete at the moment
		} else {
			gLogi("parentitemid != 0");
			for(int i = 0; i < items.size(); i++) {
				items[i].set(root, topparent, this, 0, 0, items[i].contextmenux + contextmenudefaultw - 4, items[i].contextmenuy + i * contextmenulineh, contextmenudefaultw, contextmenulineh);
				items[i].contextmenux = items[i].left;
				items[i].contextmenuy = items[i].top;
				items[i].contextmenuw = items[i].width;
				items[i].contextmenuh = items[i].height;
				items[i].drawMenuItem();
			}
		}
	}
}

void gGUIContextMenuItem::mouseMoved(int x, int y) {
	if(contextmenushown) {
		for(int i = 0; i < items.size(); i++) {
			if(x >= items[i].left && x < items[i].right && y >= items[i].top && y < items[i].bottom) items[i].hovered = true;
			else items[i].hovered = false;
		}
	}
}

void gGUIContextMenuItem::mousePressed(int x, int y, int button) {
//	gLogi("ContextMenu") << "mousePressed" << ", x:" << x << ", y:" << y << ", b:" << button;
	if(!contextmenushown && button == 1) {
		contextmenux = x;
		contextmenuy = y;
		contextmenushown = true;
	} else if((x != contextmenux || y != contextmenuy) && button == 1) {
		contextmenux = x;
		contextmenuy = y;
	} else if(button == 0) contextmenushown = false;
}

gGUIContextMenu::gGUIContextMenu() : gGUIContextMenuItem("", nullicon) {

}

gGUIContextMenu::~gGUIContextMenu() {
}

void gGUIContextMenu::draw() {
	gGUIContextMenuItem::drawMenuItem();
//	context menu borders
	if(getContextMenuShown() == true) {
		renderer->setColor(backgroundcolor);
		gDrawRectangle(contextmenux, contextmenuy, contextmenudefaultw + 1, contextmenuh + 1, false);
	}
}
