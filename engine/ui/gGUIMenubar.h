/*
 * gGUIMenubar.h
 *
 *  Created on: Sep 20, 2021
 *      Author: noyan
 */

#ifndef UI_GGUIMENUBAR_H_
#define UI_GGUIMENUBAR_H_

#include "gGUIControl.h"


class gGUIMenuItem : public gGUIControl {
public:
	static const int TYPE_ITEM = 0, TYPE_SEPERATOR = 1;

	gGUIMenuItem(std::string text);
	~gGUIMenuItem();

	int getItemId();
	void setParentItemId(int itemId);
	int getParentItemId();

	int addChild(std::string text);
	int addChild(gGUIMenuItem childItem);
	void removeChild(int childNo);
	int getChildNum();
	gGUIMenuItem* getChild(int childNo);
	gGUIMenuItem* findChild(int itemId);

	virtual void draw();

	void mouseMoved(int x, int y);
	void mousePressed(int x, int y, int button);

	int menuboxx, menuboxy, menuboxw, menuboxh;
	int texty;

protected:
	static int totaltextw;

private:
	static int lastitemid;
	int itemid, parentitemid;
	std::string text;
	std::vector<gGUIMenuItem> childs;
	bool selected;
	bool hovered;
	int menuboxlineh, menuboxtextextrah;
	int menuboxdefaultw;
	bool menuboxshown;
	int menuboxdefaulth;
	int texth;
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
