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
 * Author: Ipek Senturk, 2022 Jul 29                                        *
 ****************************************************************************/
/*
 * gGUIContextMenu.h
 *
 *  Created on: 29 Jul 2022
 *      Author: Ipek Senturk
 */

#ifndef UI_GGUICONTEXTMENU_H_
#define UI_GGUICONTEXTMENU_H_

#include "gGUIControl.h"
#include "gImage.h"

/**
 * Beggining of gGUIContextMenuItem class which
 * creates a menu item according to given parameters.
 * Without any parameters the class will give an error.
 * You can add menu items and seperators, change the left
 * margin and the menu width.
 */
class gGUIContextMenuItem: public gGUIControl {
public:

	gGUIContextMenuItem(std::string text, gImage* menuIcon, bool seperatorAdded);
	~gGUIContextMenuItem();

	virtual void drawMenuItem();
	void mouseMoved(int x, int y);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);

	/**
	 * Returns itemid.
	 *
	 * @return itemid.
	 */
	int getItemId();

	/**
	 * Sets parentitemid to itemId.
	 *
	 * @param itemId is parenitemid to be set.
	 */
	void setParentItemId(int itemId);

	/**
	 * Returns parentitemid.
	 *
	 * @return parentitemid.
	 */
	int getParentItemId();

	/**
	 * Returns selected menu item.
	 *
	 * @return menuitems[itemNo].
	 *
	 * @param itemNo is the item's number that
	 * is going to be returned
	 */
	gGUIContextMenuItem* getItem(int itemNo);

	/**
	 * Returns the width of context menu item's left margin.
	 *
	 * @return contextmenuleftmargin.
	 */
	int getContextMenuLeftMargin();

	/**
	 * Sets context menu item's left margin to given number.
	 *
	 * @param leftMargin is the new margin width to be set.
	 */
	void setContextMenuLeftMargin(int leftMargin);

	/*
	 * Returns the total number of context menu items.
	 *
	 * @return items.size().
	 */
	int getContextMenuSize();

	/**
	 * Returns the visibility of context menu
	 *
	 * @return a bool value contextmenushown.
	 */
	bool getContextMenuShown();

	/*
	 * Returns a bool value. If you click on an item
	 * it will return true, otherwise false.
	 *
	 * @return the value of ispressed
	 */
	bool getIsPressed();

//	bool isDisabled();
//	void setDisabled();

	/*
	 * Returns the title of a context menu item.
	 *
	 * @param itemNo is the number of context menu item whose
	 * title will be returned
	 *
	 * @return the string item[itemNo].title.
	 */
	std::string getItemTitle(int itemNo);

	/**
	 * Adds an item to the gGUIContextMenuItem vector
	 *
	 * @param std::string text is the title of menu item
	 *
	 * @param gImage* menuIcon is the icon of menu item,
	 * if nullptr is sent an item without any icons will
	 * be added to the vector.
	 *
	 * @param bool seperatorAdded determines whether a seperator
	 * will added under a menu item or not.
	 */
	void addItem(std::string text, gImage* menuIcon, bool seperatorAdded);

	int contextmenux, contextmenuy, contextmenudefaultw, contextmenuh;

private:
	static int lastitemid;
	static int lastparentitemid;
	static std::vector<int> parentitems;
	int itemid, parentitemid;
	std::string text;
	std::vector<gGUIContextMenuItem> items;
	int contextmenuw;
	int contextmenuleftmargin;
	int contextmenulineh;
	int datady;
	bool contextmenushown;
	bool hovered;
	bool seperatoradded;
	bool ispressed;
	gImage* menuicon;
	int menuiconx, menuicony, menuiconw, menuiconh;

	int itemno;
};

class gGUIContextMenu : public gGUIContextMenuItem {
public:

	gGUIContextMenu();
	virtual ~gGUIContextMenu();

	void draw();

private:
	std::vector<gGUIContextMenuItem> items;
};

#endif /* UI_GGUICONTEXTMENU_H_ */
