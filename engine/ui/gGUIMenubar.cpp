/*
 * gGUIMenubar.cpp
 *
 *  Created on: Sep 20, 2021
 *      Author: noyan
 */

#include "gGUIMenubar.h"
#include "gBaseCanvas.h"

bool gGUIMenuItem::isresinitialized = false;
int gGUIMenuItem::lastitemid;
int gGUIMenuItem::lastparentitemid;
int gGUIMenuItem::totaltextw;
std::vector<int> gGUIMenuItem::parentitems;


gGUIMenuItem::gGUIMenuItem(std::string text) {
	itemid = lastitemid++;
//	gLogi("MenuItem") << "itemid:" << itemid;
	parentitemid = 0;
	this->text = text;
	title = text;
	selected = false;
	hovered = false;
	menuboxdefaultw = 150;
	menuboxdefaulth = font->getSize() * 2 / 3;
	menuboxx = 0;
	menuboxy = 0;
	menuboxw = 0;
	menuboxh = 0;
	menuboxlineh = font->getSize() * 9 / 4;
	menuboxtextextrah = 0;
	menuboxshown = false;
	texth = font->getStringHeight("Ap");
	texty = 0;
	isparent = false;
	counter = 0;
	isicon = false;
	seperator = false;
	iconh = 16;
	iconw = 16;
	isparentpressed = false;

}

gGUIMenuItem::~gGUIMenuItem() {

}

int gGUIMenuItem::getItemId() {
	return itemid;
}

void gGUIMenuItem::setParentItemId(int itemId) {
	parentitemid = itemId;
	menuboxtextextrah = 0;
	if(parentitemid == 0) menuboxtextextrah = font->getSize() * 3 / 4;
}

int gGUIMenuItem::getParentItemId() {
	return parentitemid;
}

int gGUIMenuItem::addChild(std::string text, bool addSeperator) {
	int childno = childs.size();
	childs.push_back(gGUIMenuItem(text));
	childs[childno].setParentItemId(itemid);
	childs[childno].seperator = addSeperator;
	if(itemid != lastparentitemid) parentitems.push_back(itemid);
	lastparentitemid = itemid;
	childs[childno].menuicon = new gImage();

	int tw = font->getStringWidth(text) + 6 + font->getSize();
	if(itemid == 0) {
		childs[childno].set(root, topparent, this, 0, 0, totaltextw, 0, tw, height);
		childs[childno].menuboxx = childs[childno].left;
		childs[childno].menuboxy = childs[childno].bottom;
		childs[childno].menuboxw = menuboxdefaultw;
		childs[childno].menuboxh = menuboxdefaulth;
		childs[childno].texty = childs[childno].menuboxy + childs[childno].menuboxh - ((childs[childno].menuboxh - texth) / 2);
	} else if(parentitemid == 0) {
		childs[childno].set(root, topparent, this, 0, 0, menuboxx, (menuboxlineh * 5 / 3) + childno * menuboxlineh, menuboxw - 1, menuboxlineh);
		childs[childno].menuboxw = menuboxdefaultw;
		childs[childno].menuboxh = menuboxdefaulth;
		childs[childno].menuboxx = childs[childno].left + childs[childno].menuboxw;
		childs[childno].menuboxy = childs[childno].top - texth;
		childs[childno].texty = childs[childno].menuboxy + childs[childno].menuboxh - ((childs[childno].menuboxh - texth) / 2);
	} else {
		childs[childno].set(root, topparent, this, 0, 0, menuboxx,texth + menuboxy + (childno * menuboxlineh), menuboxw, menuboxlineh);
		childs[childno].menuboxw = menuboxdefaultw;
		childs[childno].menuboxh = menuboxdefaulth;
		childs[childno].menuboxx = childs[childno].left + childs[childno].menuboxw;
		childs[childno].menuboxy = childs[childno].top;
		childs[childno].texty = childs[childno].menuboxy + childs[childno].menuboxh - ((childs[childno].menuboxh - texth) / 2);
	}
	totaltextw += tw;
	menuboxh = 0;
//	if(font->getStringWidth(text) > menuboxw) menuboxw = font->getStringWidth(text);
	return childs[childno].getItemId();
}

int gGUIMenuItem::addChild(gGUIMenuItem childItem) {
	int childno = childs.size();
	childs.push_back(childItem);
	childs[childno].setParentItemId(itemid);
	int tw = font->getStringWidth(childItem.getTitle()) + 6 + font->getSize();
	if(itemid == 0) childs[childno].set(root, topparent, this, 0, 0, totaltextw, 0, tw, height);
	else childs[childno].set(root, topparent, this, 0, 0, menuboxx, (menuboxlineh * 5 / 3) + childno * menuboxlineh, menuboxw - 1, menuboxlineh);
	totaltextw += tw;
	childs[childno].menuboxx = left + childs[childno].left;
	childs[childno].menuboxy = childs[childno].bottom;
	childs[childno].menuboxw = menuboxdefaultw;
	childs[childno].menuboxh = menuboxdefaulth;
	childs[childno].texty = childs[childno].menuboxy + childs[childno].menuboxh - ((childs[childno].menuboxh - texth) / 2);
	menuboxh = 0;
	return childs[childno].getItemId();
}

void gGUIMenuItem::removeChild(int childNo) {
	childs.erase(childs.begin() + childNo);
	if(childs.empty()) menuboxh = menuboxdefaulth;
}

int gGUIMenuItem::getChildNum() {
	return childs.size();
}

gGUIMenuItem* gGUIMenuItem::getChild(int childNo) {
	return &childs[childNo];
}

gGUIMenuItem* gGUIMenuItem::findChild(int itemId) {
	return &childs[0];
}

void gGUIMenuItem::update() {
	counter ++;
	for(int x = 0; x < parentitems.size(); x++)  {
		if(itemid == parentitems[x]){
			isparent = true;
		}
	}
	for(int i = 0; i < childs.size(); i++) {
		childs[i].update();
		if(childs[i].counter >= 30 && childs[i].hovered && childs[i].isparent) {
			childs[i].selected = true;
			childs[i].menuboxshown = true;
			childs[i].counter = 0;
		}
	}
}

void gGUIMenuItem::setMenuicon(int MenuItemid, std::string icon){
	for(int i = 0; i < childs.size(); i++){
		if(childs[i].itemid == MenuItemid) {
			childs[i].menuicon->load(icon);
			childs[i].isicon = true;
			//gLogi("Menubar") << "icon" << i << " itemid" << MenuItemid;
		}
		childs[i].setMenuicon(MenuItemid, icon);
	}
}

void gGUIMenuItem::setMenuicon(int MenuItemid, int icon){
	for(int i = 0; i < childs.size(); i++){
		if(childs[i].itemid == MenuItemid) {
			childs[i].menuicon = res.getIconImage(icon);
			childs[i].isicon = true;
			//gLogi("Menubar") << "icon" << i << " itemid" << MenuItemid;
		}
		childs[i].setMenuicon(MenuItemid, icon);
	}
}
void gGUIMenuItem::draw() {
	if(selected || (parentitemid > 0 && hovered)) {

		if(parentitemid == 0) {
			renderer->setColor(middlegroundcolor);
			gDrawRectangle(left, top, width, height, true);
		}

		if(!childs.empty()) {
			renderer->setColor(foregroundcolor);
			gDrawRectangle(menuboxx, menuboxy, menuboxw + texth, menuboxh + childs.size() * menuboxlineh + texth, true);
			renderer->setColor(backgroundcolor);
			gDrawRectangle(menuboxx, menuboxy, menuboxw + texth, menuboxh + childs.size() * menuboxlineh + texth, false);

			for(int i = 0; i < childs.size(); i++) {
				if(childs[i].hovered) {
					renderer->setColor(middlegroundcolor);
					gDrawRectangle(childs[i].left, childs[i].top - texth / 2, childs[i].width + texth, childs[i].height - 1, true);
				}
				childs[i].draw();
			}
		}
	}

	if(isicon){
		renderer->setColor(gColor(1.0f, 1.0f, 1.0f));
		menuicon->draw(left, top - texth / 3, iconh, iconw);
	}

	renderer->setColor(fontcolor);
	if(parentitemid != 0)font->drawText(title, left + iconh, top + (font->getSize() - texth / 3) + menuboxtextextrah);
	else font->drawText(title, left + 8, top + (font->getSize() - texth / 3) + menuboxtextextrah);

	if(seperator){
		renderer->setColor(backgroundcolor);
		gDrawLine(left + iconh, bottom - texth / 2, right + 10, bottom - texth / 2);
	}

	if(isparent && parentitemid != 0) font->drawText(">", left + menuboxw, top + (font->getSize() - texth / 3) + menuboxtextextrah);

	if(itemid == 0) {
		for(int i = 0; i < childs.size(); i++) {
			childs[i].draw();
		}
	}
}

void gGUIMenuItem::mouseMoved(int x, int y) {
	if(menuboxshown && isparentpressed) {
		for(int i = 0; i < childs.size(); i++) {
			childs[i].hovered = false;
//			if(parentitemid == 0 && x >= childs[i].left && x < childs[i].right && y >= childs[i].top && y < childs[i].bottom){
//				childs[i].selected = false;
//				childs[i].menuboxshown = false;
//				childs[i].update();
//			}
			if(x >= childs[i].left && x < childs[i].right && y >= childs[i].top && y < childs[i].bottom) {
				childs[i].hovered = true;
			}
			else{
				childs[i].counter = 0;
			}
			childs[i].mouseMoved(x, y);
		}
	}

	for(int i = 0; i < childs.size(); i++) {
		if(childs[i].selected) {
			childs[i].hovered = false;
			if(x >= childs[i].left && x < childs[i].right && y >= childs[i].top && y < childs[i].bottom) {
				childs[i].hovered = true;
			}
			else{
				childs[i].counter = 0;
			}
			childs[i].mouseMoved(x, y);
		}
	}
}

void gGUIMenuItem::mousePressed(int x, int y, int button) {
	for(int i = 0; i < parentitems.size(); i++){
		isparentpressed = true;
		menuboxshown = true;
	}

	for(int i = 0; i < childs.size(); i++) {
			childs[i].selected = false;
			childs[i].menuboxshown = false;
		if(parentitemid == 0 && x >= childs[i].left && x < childs[i].right && y >= childs[i].top && y < childs[i].bottom){
			childs[i].hovered = true;
			childs[i].selected = true;
			childs[i].menuboxshown = true;
		}
		if(childs[i].hovered && x >= childs[i].left && x < childs[i].right && y >= childs[i].top && y < childs[i].bottom) {
			childs[i].selected = true;
			childs[i].menuboxshown = true;
			childs[i].hovered = false;
			root->getCurrentCanvas()->onGuiEvent(id, G_GUIEVENT_MENUBARSELECTED, gToStr(childs[i].itemid));
		}

		childs[i].mousePressed(x, y, button);
	}
}

void gGUIMenuItem::mouseReleased(int x, int y, int button) {
	isparentpressed = false;
	menuboxshown = true;
}

gGUIMenubar::gGUIMenubar() : gGUIMenuItem("") {
	totaltextw = 0;
}

gGUIMenubar::~gGUIMenubar() {
}

void gGUIMenubar::draw() {
//	gLogi("Menubar") << "draw";

	gColor oldcolor = *renderer->getColor();
	renderer->setColor(foregroundcolor);
	gDrawRectangle(left, top, width, height, true);
	renderer->setColor(foregroundcolor->r - 0.05f, foregroundcolor->g - 0.05f, foregroundcolor->b - 0.05f);
	gDrawLine(left, top + 29, right, top + 29);

	gGUIMenuItem::draw();
	gGUIMenuItem::update();

	renderer->setColor(&oldcolor);
}

/*
int gGUIMenubar::addMenuItem(std::string text) {
	childs.push_back(gGUIMenuItem(text));
	return childs.size() - 1;
}

void gGUIMenubar::removeMenuItem(int childNo) {
	childs.erase(childs.begin() + childNo);
}

int gGUIMenubar::getChildNum() {
	return childs.size();
}

gGUIMenuItem* gGUIMenubar::getChild(int childNo) {
	return childs[childNo];
}

gGUIMenuItem* gGUIMenubar::findItem(int itemId) {
	return childs[0];
}
*/
