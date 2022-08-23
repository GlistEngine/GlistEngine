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
 * You can add menu items, seperators, change both menu width
 * and left margin width. You can directly change menu width
 * (Ex: menu.contextmenudefaultw = 100;) but
 * you have to use setter function to change left margin width.
 * (Ex: menu.setContextMenuLeftMargin(20);)
 *
 * HOW TO USE
 * - First, create an object from gGUIContextMenu class
 * (Ex: gGUIContextMenu menu;) and use addContextMenu method
 * to add your menu into the frame. (Ex: frame.addContextMenu(&menu);)
 *
 * - Second, add menu items with addItem method.
 * (Ex: menu.addItem("item name", nullptr, false);)
 * You can also add sub-menu items with this function.
 * (Ex: menu.getItem(0)->addItem("item name", &menuicon, false); or
 * menu.getItem(0)->getItem(0)->addItem("item name", nullptr, false);)
 * Parameter explanations can be found below.
 *
 * - To add an icon to menu you can load your own images or use the icons
 * provided by GlistEngine.
 * - To use your own image create a gImage object
 * (Ex: gImage icon;) and load it with loadImage method. Then, you can
 * send this object to add item. (Ex: menu.addItem("item name", &icon, false);)
 * - To use icons provided by GlistEngine create a gGUIResources object.
 * (Ex: gGUIResources resource;) Initialize it by initialize method.
 * (Ex: resource.initialize();) then you can send it to addItem by getIconImage
 * method. (Ex: menu.addItem("item name", icon.getIconImage(gGUIResources::ICON_FILE), false);)
 * - There are many different icons in GlistEngine you can see them all in gGUIResources.h
 *
 * - In order to add functionality to menu options you can use isPressed
 * method in GameCanvas' mousePressed method.
 * (Ex: if(menu.getItem(0)->isPressed()) {
 * 			// your code
 * 		})
 *
 * - Lastly, run your program. When you right click on the screen you will
 * see the context menu, then you can close it with a left click.
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
	 * Returns item id.
	 *
	 * @return the value of a menu items id.
	 */
	int getItemId();

	/**
	 * Sets parent item id of an item.
	 *
	 * @param itemId is the menu item's parent id to be set.
	 */
	void setParentItemId(int itemId);

	/**
	 * Returns parent item's id.
	 *
	 * @return the value of parent item's id.
	 */
	int getParentItemId();

	/**
	 * Returns selected menu item.
	 *
	 * @param itemNo is the number of the gGUIMenuItem object to return
	 *
	 * @return selected item as an gGUIMenuItem object.
	 */
	gGUIContextMenuItem* getItem(int itemNo);

	/**
	 * Returns the width of context menu item's left margin.
	 *
	 * @return the current width of left margin.
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
	 * @return the current size of items vector.
	 */
	int getContextMenuSize();

	/**
	 * Returns the visibility of context menu
	 *
	 * @return the visibility information for menu.
	 */
	bool getContextMenuShown();

	/*
	 * Returns the information for whether an item is
	 * selected or not.
	 *
	 * @return whether a menu item was clicked or not.
	 */
	bool isPressed();

	/*
	 * Returns the title of a context menu item.
	 *
	 * @param itemNo is the number of context menu item whose
	 * title will be returned
	 *
	 * @return a menu items title as a string.
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
	 * @param bool seperatorAdded determines if there will
	 * be a seperator under a menu item or not.
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
	bool isparent;
	gImage* menuicon;
	int menuiconx, menuicony, menuiconw, menuiconh;
	int itemno;
	int counter;
	int i;
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
