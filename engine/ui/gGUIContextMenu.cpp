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
	menuicon = menuIcon;
	menuiconx = 0;
	menuicony = 0;
	menuiconw = 0;
	menuiconh = 0;
	itemno = 0;
	counter = 0;
	i = 0;
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

bool gGUIContextMenuItem::isPressed() {
	return ispressed;
}

std::string gGUIContextMenuItem::getItemTitle(int itemNo) {
	return items[itemNo].title;
}

void gGUIContextMenuItem::setContextMenuLeftMargin(int leftMargin) {
	contextmenuleftmargin = leftMargin;
}

int gGUIContextMenuItem::getContextMenuSize() {
	i = 0;
	for(i = 0; i < items.size(); i++) if(!items.empty()) items[i].getContextMenuSize();
	return items.size();
}

void gGUIContextMenuItem::addItem(std::string text,  gImage* menuIcon, bool seperatorAdded) {
	itemno = items.size();
	items.push_back(gGUIContextMenuItem(text, menuIcon, seperatorAdded));
	items[itemno].setParentItemId(itemid);
	if(itemid != lastparentitemid) parentitems.push_back(itemid);
	lastparentitemid = itemid;
	contextmenuh = items.size() * contextmenulineh;
//	gLogi("MenuItem") << "text:" << text << ", parentitemid:" << items[itemno].getParentItemId() << ", itemid:" << items[itemno].getItemId();
//	gLogi("MenuItem") << "size:" << items.size();
}

void gGUIContextMenuItem::drawMenuItem() {
	if(contextmenushown) {
//		context menu on the right
		if((contextmenux + contextmenudefaultw) >= getScreenWidth()) {
			contextmenux -= contextmenudefaultw;
			renderer->setColor(backgroundcolor);
//			menu shadow
			gDrawRectangle(items[0].left + 2, items[0].top + 2, contextmenudefaultw, items.size() * contextmenulineh, true);
//			menu borders
			gDrawRectangle(items[0].left, items[0].top, contextmenudefaultw + 1, items.size() * contextmenulineh + 1, false);
			renderer->setColor(middlegroundcolor);
//			menu background
			gDrawRectangle(items[0].left, items[0].top, contextmenudefaultw, items.size() * contextmenulineh, true);
//		context menu at the bottom
		} else if((contextmenuy + contextmenuh) >= getScreenHeight()) {
			contextmenuy -= contextmenuh;
			renderer->setColor(backgroundcolor);
//			menu shadow
			gDrawRectangle(items[0].left + 2, items[0].top + 2, contextmenudefaultw, items.size() * contextmenulineh, true);
//			menu border
			gDrawRectangle(items[0].left, items[0].top, contextmenudefaultw + 1, items.size() * contextmenulineh + 1, false);
//			menu background
			renderer->setColor(middlegroundcolor);
			gDrawRectangle(items[0].left, items[0].top, contextmenudefaultw, items.size() * contextmenulineh, true);
		} else {
			renderer->setColor(backgroundcolor);
//			menu shadows
			gDrawRectangle(items[0].left + 2, items[0].top + 2, contextmenudefaultw, items.size() * contextmenulineh, true);
//			menu borders
			gDrawRectangle(items[0].left, items[0].top, contextmenudefaultw + 1, items.size() * contextmenulineh + 1, false);
			renderer->setColor(middlegroundcolor);
//			menu background
			gDrawRectangle(items[0].left, items[0].top, contextmenudefaultw, items.size() * contextmenulineh, true);
		}
//		if(!items.empty() && hovered) {
//			for(int i = 0; i < items.size(); i++) items[i].drawMenuItem();
//		}
//		set position for the main menu items
		i = 0;
		for(i = 0; i < items.size(); i++) {
			items[i].set(root, topparent, this, 0, 0, contextmenux, contextmenuy + i * contextmenulineh, contextmenudefaultw, contextmenulineh);
//			sub-menus aren't positioned correctly without declerations below.
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
//		set position for the sub-menu items
		i = 0;
		for(i = 0; i < items.size(); i++) {
			if(items[i].parentitemid > 0) {
//				Set sub menu positions first, this one will be just next to main menu
				items[i].set(root, topparent, this, 0, 0, contextmenux + contextmenudefaultw - 4, contextmenuy + (i * contextmenulineh), contextmenudefaultw, contextmenulineh);
//				Then check if it's at the bottom right corner
				if(contextmenuy + (contextmenulineh * items.size()) >= getScreenHeight() && items[i].left + contextmenudefaultw >= getScreenWidth())
					items[i].set(root, topparent, this, 0, 0, items[i].left - (2 * contextmenudefaultw) + 4, contextmenuy + (i * contextmenulineh) - (contextmenulineh * (items.size() - 1)), contextmenudefaultw, contextmenulineh);
//				Check if it's on the right
				else if(items[i].left + contextmenudefaultw >= getScreenWidth())
					items[i].set(root, topparent, this, 0, 0, items[i].left - (2 * contextmenudefaultw) + 4, contextmenuy + (i * contextmenulineh), contextmenudefaultw, contextmenulineh);
//				Check if it's at the bottom
				else if(contextmenuy + (contextmenulineh * items.size()) >= getScreenHeight())
					items[i].set(root, topparent, this, 0, 0, contextmenux + contextmenudefaultw - 4, contextmenuy + (i * contextmenulineh) - (contextmenulineh * (items.size() - 1)), contextmenudefaultw, contextmenulineh);
//				If it doesn't extend beyond the screen set it to the first position again.
				else
					items[i].set(root, topparent, this, 0, 0, contextmenux + contextmenudefaultw - 4, contextmenuy + (i * contextmenulineh), contextmenudefaultw, contextmenulineh);
//				sub-menus aren't positioned correctly without the declarations below.
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
			}
		}
//		seperators
		i = 0;
		for(i = 0; i < items.size(); i++) {
			if(items[i].seperatoradded == true && items[i].parentitemid >= 0) {
				renderer->setColor(backgroundcolor);
				gDrawLine(items[i].left + 10, items[i].bottom, items[i].right - 10, items[i].bottom);
			}
		}
//		highlights
		i = 0;
		for(i = 0; i < items.size(); i++) {
			if(items[i].hovered) {
				renderer->setColor(foregroundcolor);
				gDrawRectangle(items[i].left, items[i].top, items[i].width, items[i].height, true);
			}
		}
//		menu icons
		i = 0;
		for(i = 0; i < items.size(); i++) {
			if(items[i].parentitemid >= 0 && items[i].menuicon != nullptr) {
				 renderer->setColor(gColor(1.0f, 1.0f, 1.0f, 1.0f));
				items[i].menuicon->draw(items[i].menuiconx, items[i].menuicony, items[i].menuiconw, items[i].menuiconh);
			}
		}
//		condition for > symbol
		i = 0;
		for(i = 0; i < parentitems.size(); i++) if(itemid == parentitems[i]) isparent = true;
//		menu item titles
		i = 0;
		for(i = 0; i < items.size(); i++) {
			if(items[i].parentitemid >= 0) {
				renderer->setColor(fontcolor);
//				">" symbol for parent items
				if(items[i].isparent) font->drawText(">", items[i].right - 20, items[i].top + items[i].height - datady - 2);
				font->drawText(items[i].title, items[i].left + contextmenuleftmargin, items[i].top + items[i].height - datady);
			}
		}
		i = 0;
		for(i = 0; i < items.size(); i++) if(parentitemid >= 0 && items[i].hovered) items[i].drawMenuItem();
	}
}

void gGUIContextMenuItem::mouseMoved(int x, int y) {
	for(int i = 0; i < items.size(); i++) {
		if(items[i].contextmenushown && items[i].parentitemid >= 0 && x >= items[i].left && x < items[i].right && y >= items[i].top && y < items[i].bottom) {
//			gLogi("Item") << i << ", left:" << items[i].left << ", top:" << items[i].top << ", right:" << items[i].right << ", bottom:" << items[i].bottom;
			hovered = true;
			items[i].hovered = true;
		} else if(items[i].parentitemid != 0 && x >= items[i].left && x < items[i].right && y >= items[i].top && y < items[i].bottom) {
			hovered = true;
			items[i].hovered = true;
		} else {
			items[i].hovered = false;
		}
		items[i].mouseMoved(x, y);
	}
}

void gGUIContextMenuItem::mousePressed(int x, int y, int button) {
//	gLogi("ContextMenu") << "mousePressed" << ", x:" << x << ", y:" << y << ", b:" << button;
	i = 0;
	for(i = 0; i < items.size(); i++) {
//		button == 1 is right click
//		button == 0 is left click
		if(!contextmenushown && button == 1) {
			contextmenux = x;
			contextmenuy = y;
			contextmenushown = true;
		} else if((x != contextmenux || y != contextmenuy) && button == 1) {
			contextmenux = x;
			contextmenuy = y;
		} else if(button == 0) {
			if(x >= items[i].left && x < items[i].right - 4 && y >= items[i].top && y < items[i].bottom) {
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
}
