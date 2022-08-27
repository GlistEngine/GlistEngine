/*
 * gGUIMenubar.h
 *
 *  Created on: Sep 20, 2021
 *      Author: noyan
 */

#ifndef UI_GGUIMENUBAR_H_
#define UI_GGUIMENUBAR_H_

#include "gGUIControl.h"
#include "gImage.h"
#include "gGUIResources.h"


class gGUIMenuItem : public gGUIControl {
public:
	static const int TYPE_ITEM = 0, TYPE_SEPERATOR = 1;

	gGUIMenuItem(std::string text);
	~gGUIMenuItem();

	int getItemId();
	void setParentItemId(int itemId);
	int getParentItemId();

	int addChild(std::string text, bool addSeperator = false);
	int addChild(gGUIMenuItem childItem);
	void removeChild(int childNo);
	int getChildNum();
	gGUIMenuItem* getChild(int childNo);
	gGUIMenuItem* findChild(int itemId);

	virtual void draw();
	void setMenuicon(int menuItemid, std::string icon);
	void setMenuicon(int menuItemId, int icon);

	void mouseMoved(int x, int y);
	void mousePressed(int x, int y, int button);
	void update();
	void resInitialize();
	int menuboxx, menuboxy, menuboxw, menuboxh;
	int texty;

protected:
	static int totaltextw;

private:
	static int lastitemid;
	static int lastparentitemid;
	static std::vector<int> parentitems;
	int itemid, parentitemid;
	std::string text;
	std::vector<gGUIMenuItem> childs;
	bool selected;
	bool hovered;
	int menuboxlineh, menuboxtextextrah;
	int menuboxdefaultw;
	bool menuboxshown;
	bool isparent;
	int menuboxdefaulth;
	int texth;
	int counter;
	gImage* menuicon;
	bool isicon;
	int iconh, iconw;
	gGUIResources res;
	bool seperator;
	static bool isresinitialized;
};


class gGUIMenubar : public gGUIMenuItem {
public:

	gGUIMenubar();
	virtual ~gGUIMenubar();

	void draw();

/*
	int addMenuItem(std::string text);
	void removeMenuItem(int childNo);
	int getChildNum();
	gGUIMenuItem* getChild(int childNo);
	gGUIMenuItem* findItem(int itemId);
*/
private:
	std::vector<gGUIMenuItem> childs;
};

#endif /* UI_GGUIMENUBAR_H_ */
