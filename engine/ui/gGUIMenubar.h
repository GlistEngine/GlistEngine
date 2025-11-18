/****************************************************************************
 * Copyright (c) 2014 Nitra Games Ltd., Istanbul, Turkey                    *
 *                                                                          *
 * Permission is hereby granted, free of charge, to any person obtaining a  *
 * copy of this software and associated documentation files (the            *
 * "Software"), to deal in the Software without restriction, including      *
 * without limitation the rights to use, copy, modify, merge, publish,      *
 * distribute, distribute with modifications, sublicense, and/or sell       *
 * copies of the Software, and to permit persons to whom the Software is    *
 * furnished to do so, subject to the following conditions:                 *
 *                                                                          *
 * The above copyright notice and this permission notice should not be      *
 * deleted from the source form of the Software.                            *
 *                                                                          *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS  *
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF               *
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.   *
 * IN NO EVENT SHALL THE ABOVE COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,   *
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR    *
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR    *
 * THE USE OR OTHER DEALINGS IN THE SOFTWARE.                               *
 *                                                                          *
 * Except as contained in this notice, the name(s) of the above copyright   *
 * holders shall not be used in advertising or otherwise to promote the     *
 * sale, use or other dealings in this Software without prior written       *
 * authorization.                                                           *
 ****************************************************************************/

/****************************************************************************
 * Author: Medine, Yasin 2022-on                                             *
 ****************************************************************************/
/*
 * gGUIMenubar.h
 *
 *  Created on: 29 Aug 2022
 *      Author: Medine, Yasin
 */
#ifndef UI_GGUIMENUBAR_H_
#define UI_GGUIMENUBAR_H_

#include "gGUIControl.h"
#include "gImage.h"


/**
 * The menubar is a thin, horizontal bar containing the labels of the menus in
 * GUI. This class provides the user with a place in the pop-up window to find
 * most of a program's core functions. To these functions; we can give examples
 * af adding new file or deleting file and exiting the app.
 */
class gGUIMenuItem : public gGUIControl {
public:
	static const int TYPE_ITEM = 0, TYPE_SEPERATOR = 1;

	gGUIMenuItem(std::string text);
	~gGUIMenuItem();

/**
 * Gets the menubar's items id.
 */
	int getItemId();

/**
 * Returns the parent items id.
 */
	int getParentItemId();

/**
 * Sets parent item id of an item.
 *
 * @param itemId is the menu item's parent id to be set.
 */
	void setParentItemId(int itemId);

/**
 * @param addChild Adds a child to the gGUIMenubar vector
 *
 * @param text is the title of menu item
 *
 * @return addSeperator determines if there will be a seperator
 * under a menu item or not.
 */
	int addChild(std::string text, bool addSeperator = false);

/**
 * @param addChild Adds a child to the gGUIMenubar vector
 *
 * @return childItem the number of items that will occur under parent item
 */
	int addChild(gGUIMenuItem childItem);

/**
 * @param getChildNum Returns the child items id.
 */
	int getChildNum();

/**
 * Removes the child item from the gGUIMenubar's vector
 *
 * @param childNo  number of child item to be removed.
 */
	void removeChild(int childNo);

/**
 * Returns selected child item
 *
 * @param childNo is the number of the gGUIMenuItem object to return
 *
 * @return selected item as an gGUIMenuItem object.
 */
	gGUIMenuItem* getChild(int childNo);

	gGUIMenuItem* findChild(int itemId);

	virtual void draw();

/**
 * Sets icon to the menubar items. User can load their own icons with this
 * command.
 *
 * @param menuItemId the id of the item to be added icon
 *
 * @param icon to be added with its file location.
 */
	void setMenuicon(int menuItemid, std::string icon);

/**
 * Sets the icons found in the game engine.
 *
 * @param menuItemId the id of the item to be added icon
 *
 * @param icon to be added from game engine.
 */
	void setMenuicon(int menuItemId, int icon);

	void mouseMoved(int x, int y);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
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
	gTexture* menuicon;
	bool isicon;
	int iconh, iconw;
	bool seperator;
	static bool isresinitialized;
	bool isparentpressed;
	bool pressed;
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
