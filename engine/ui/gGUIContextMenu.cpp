/*
 * gGUIContextMenu.cpp
 *
 *  Created on: 29 Jul 2022
 *      Author: Ipek Senturk
 */

#include "gGUIContextMenu.h"
#include "gBaseApp.h"
#include "gBaseCanvas.h"


int gGUIContextMenuItem::lastitemid;
int gGUIContextMenuItem::lastparentitemid;
std::vector<int> gGUIContextMenuItem::parentitems;


gGUIContextMenuItem::gGUIContextMenuItem(std::string text, gImage* menuIcon, bool seperatorAdded) {
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
	seperatoradded = seperatorAdded;
	ispressed = false;
	isparent = false;
	selected = false;
	menuicon = menuIcon;
	menuiconx = 0;
	menuicony = 0;
	menuiconw = 0;
	menuiconh = 0;
	itemno = 0;
	counter = 0;
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

bool gGUIContextMenuItem::getIsPressed() {
	return ispressed;
}

std::string gGUIContextMenuItem::getItemTitle(int itemNo) {
	return items[itemNo].title;
}

void gGUIContextMenuItem::setContextMenuLeftMargin(int leftMargin) {
	contextmenuleftmargin = leftMargin;
}

int gGUIContextMenuItem::getContextMenuSize() {
	return items.size();
}

void gGUIContextMenuItem::addItem(std::string text,  gImage* menuIcon, bool seperatorAdded) {
	itemno = items.size();
	items.push_back(gGUIContextMenuItem(text, menuIcon, seperatorAdded));
	items[itemno].setParentItemId(itemid);
	if(itemid != lastparentitemid) parentitems.push_back(itemid);
	lastparentitemid = itemid;
	contextmenuh = items.size() * contextmenulineh;
	gLogi("MenuItem") << "text:" << text << ", parentitemid:" << items[itemno].getParentItemId() << ", itemid:" << items[itemno].getItemId();
//	gLogi("MenuItem") << "size:" << items.size();
}

void gGUIContextMenuItem::update() {
	counter ++;
//	gLogi("Counter") << counter;
	for(int i = 0; i < items.size(); i++) {
//		items[i].update();
		if(items[i].counter >= 45 && items[i].hovered) {
			gLogi("inside if");
//			items[i].selected = true;
			items[i].contextmenushown = true;
			items[i].counter = 0;
		}
	}
}

void gGUIContextMenuItem::drawMenuItem() {
	if(contextmenushown) {
		for(int i = 0; i < items.size(); i++) {
//			context menu on the right
			if((contextmenux + contextmenudefaultw) >= getScreenWidth()) {
				contextmenux -= contextmenudefaultw;
				renderer->setColor(backgroundcolor);
				gDrawRectangle(contextmenux, contextmenuy, contextmenudefaultw, items.size() * contextmenulineh, false);
				renderer->setColor(middlegroundcolor);
				gDrawRectangle(items[i].left, items[i].top, items[i].width, items[i].height, true);
//			context menu at the bottom
			} else if((contextmenuy + contextmenuh) >= getScreenHeight()) {
				contextmenuy -= contextmenuh;
				renderer->setColor(backgroundcolor);
				gDrawRectangle(contextmenux, contextmenuy, contextmenudefaultw, items.size() * contextmenulineh, false);
				renderer->setColor(middlegroundcolor);
				gDrawRectangle(items[i].left, items[i].top, items[i].width, items[i].height, true);
			} else {
				renderer->setColor(backgroundcolor);
				gDrawRectangle(contextmenux, contextmenuy, contextmenudefaultw, items.size() * contextmenulineh, false);
				renderer->setColor(middlegroundcolor);
				gDrawRectangle(items[i].left, items[i].top, items[i].width, items[i].height, true);
			}
		}

//		if(!items.empty()) {
//			for(int i = 0; i < items.size(); i++) items[i].drawMenuItem();
//		}

//		set position for the main menu items
		for(int i = 0; i < items.size(); i++) {
			items[i].set(root, topparent, this, 0, 0, contextmenux, contextmenuy + i * contextmenulineh, contextmenudefaultw, contextmenulineh);
			items[i].contextmenux = items[i].left;
			items[i].contextmenuy = items[i].top;
			items[i].contextmenuw = contextmenudefaultw;
			items[i].contextmenuh = contextmenulineh;
			if(items[i].menuicon != nullptr && items[i].parentitemid == 0) {
				items[i].menuiconh = contextmenulineh * 2 / 3;
				items[i].menuiconw = items[i].menuiconh;
				items[i].menuiconx = items[i].left + items[i].menuiconw * 3 / 4;
				items[i].menuicony = items[i].top + items[i].menuiconh / 4;
			}
		}
//		sets poisiton for the sub-menu items
		for(int i = 0; i < items.size(); i++) {
			if(items[i].parentitemid > 0) {
				items[i].set(root, topparent, this, 0, 0, contextmenux + contextmenudefaultw - 4, contextmenuy + (i * contextmenulineh), contextmenudefaultw, contextmenulineh);
				if(items[i].left + contextmenudefaultw >= getScreenWidth()) {
					items[i].left -= 2 * contextmenudefaultw - 4;
				}
				items[i].contextmenuw = contextmenudefaultw;
				items[i].contextmenuh = contextmenulineh;
				items[i].contextmenux = items[i].left;
				items[i].contextmenuy = items[i].top;
				if(items[i].menuicon != nullptr) {
					items[i].menuiconh = contextmenulineh * 2 / 3;
					items[i].menuiconw = items[i].menuiconh;
					items[i].menuiconx = items[i].left + items[i].menuiconw * 3 / 4;
					items[i].menuicony = items[i].top + items[i].menuiconh / 4;
				}
//				items[i].drawMenuItem();
//				gLogi("Sub item") << i << ", x" << items[i].left << ", y" << items[i].top;
			} //else if(items[i].parentitemid == 0) {
//				gLogi("Item") << i << ", left:" << items[i].left << ", top:" << items[i].top << ", right:" << items[i].right << ", bottom:" << items[i].bottom;
//				items[i].drawMenuItem();
//			}
		}
//		highlights
		for(int i = 0; i < items.size(); i++) {
			if(items[i].hovered) {
				renderer->setColor(foregroundcolor);
				gDrawRectangle(items[i].left, items[i].top, items[i].width, items[i].height, true);
			}
		}
//		menu icons
		for(int i = 0; i < items.size(); i++) {
			if(items[i].parentitemid == 0 && items[i].menuicon != nullptr) {
				items[i].menuicon->draw(items[i].menuiconx, items[i].menuicony, items[i].menuiconw, items[i].menuiconh);
			} else if(items[i].parentitemid > 0 && items[i].menuicon != nullptr/* && hovered*/) {
				items[i].menuicon->draw(items[i].menuiconx, items[i].menuicony, items[i].menuiconw, items[i].menuiconh);
			}
		}
//		condition for > symbol
		for(int i = 0; i < parentitems.size(); i++)  {
			if(itemid == parentitems[i]){
				isparent = true;
			}
		}
//		menu item titles
		for(int i = 0; i < items.size(); i++) {
			if(items[i].parentitemid > 0) {
				renderer->setColor(fontcolor);
				font->drawText(items[i].title, items[i].left + contextmenuleftmargin, items[i].top + items[i].height - datady);
				if(items[i].isparent) font->drawText(">", items[i].right - 20, items[i].top + items[i].height - datady - 2);
			} else if(items[i].parentitemid == 0){
				renderer->setColor(fontcolor);
				font->drawText(items[i].title, items[i].left + contextmenuleftmargin, items[i].top + items[i].height - datady);
				if(items[i].isparent) font->drawText(">", items[i].right - 20, items[i].top + items[i].height - datady - 2);
			}
		}

//		seperators
		for(int i = 0; i < items.size(); i++) {
			if(items[i].seperatoradded == true) {
				renderer->setColor(backgroundcolor);
				gDrawLine(items[i].left + 10, items[i].bottom, items[i].right - 10, items[i].bottom);
			}
		}

		for(int i = 0; i < items.size(); i++) {
			if(parentitemid >= 0 && items[i].hovered) items[i].drawMenuItem();
		}
	}
}

void gGUIContextMenuItem::mouseMoved(int x, int y) {
	if(contextmenushown) {
		for(int i = 0; i < items.size(); i++) {
			if(/*items[i].parentitemid >= 0 && */x >= items[i].left && x < items[i].right && y >= items[i].top && y < items[i].bottom) {
				gLogi("Item") << i << ", left:" << items[i].left << ", top:" << items[i].top << ", right:" << items[i].right << ", bottom:" << items[i].bottom;
				items[i].hovered = true;
				items[i].selected = true;
				items[i].update();
			} else {
//				gLogi("false");
				items[i].hovered = false;
			}
			items[i].mouseMoved(x, y);
		}
	}
}

void gGUIContextMenuItem::mousePressed(int x, int y, int button) {
//	gLogi("ContextMenu") << "mousePressed" << ", x:" << x << ", y:" << y << ", b:" << button;
	for(int i = 0; i < items.size(); i++) {
		if(!contextmenushown && button == 1) {
			contextmenux = x;
			contextmenuy = y;
			contextmenushown = true;
		} else if((x != contextmenux || y != contextmenuy) && button == 1) {
			contextmenux = x;
			contextmenuy = y;
		} else if(button == 0) {
			if(x >= items[i].left && x < items[i].right && y >= items[i].top && y < items[i].bottom) {
				items[i].ispressed = true;
				root->getCurrentCanvas()->onGuiEvent(id, G_GUIEVENT_BUTTONPRESSED);
			} else {
				items[i].left = 0;
				items[i].right = 0;
				items[i].bottom = 0;
				items[i].top = 0;
				contextmenushown = false;
			}
		}
		items[i].mousePressed(x, y, button);
	}
}

void gGUIContextMenuItem::mouseReleased(int x, int y, int button) {
//	gLogi("Button") << "released, id:" << id;
	for(int i = 0; i < items.size(); i++) {
		if(x >= items[i].left && x < items[i].right && y >= items[i].top && y < items[i].bottom) {
			items[i].ispressed = false;
			root->getCurrentCanvas()->onGuiEvent(id, G_GUIEVENT_BUTTONRELEASED);
		} else items[i].ispressed = false;
		items[i].mouseReleased(x, y, button);
	}
}

gGUIContextMenu::gGUIContextMenu() : gGUIContextMenuItem("", nullptr, false) {

}

gGUIContextMenu::~gGUIContextMenu() {
}

void gGUIContextMenu::draw() {
	gGUIContextMenuItem::drawMenuItem();
//	context menu borders
	if(getContextMenuShown()) {
		renderer->setColor(backgroundcolor);
		gDrawRectangle(contextmenux, contextmenuy, contextmenudefaultw + 1, contextmenuh + 1, false);
	}
}
