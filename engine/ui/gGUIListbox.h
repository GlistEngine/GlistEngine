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
 * Author: Noyan Culum, Sevval Bulburu, Aynur Dogan 2022-08-18                           *
 ****************************************************************************/

/*
 * gGUIListbox.h
 *
 *  Created on: Feb 5, 2022
 *      Author: noyan
 */

#ifndef UI_GGUILISTBOX_H_
#define UI_GGUILISTBOX_H_

#include "gGUIScrollable.h"
#include <deque>
#include "gImage.h"


/*
 * This class creates list boxes. List boxes can use when developer wants to listed
 * any kind of data. There isn't an edge for number of the datas.
 *
 * Listbox uses a vector for storing the datas. This vector stores the given
 * title's. Developer can make basic operations about this vector such as adding
 * a data to a vector, removing data from the vector or renamed the spesific line's
 * title.
 *
 * Also developers can use icons with the datas. SetIconType() function uses for
 * activeted the default icons. When the function activated, then deleloper can
 * change icons with images from outside or using gGUIResources class'es icons.
 * Icon's color can be change with setIconColor() function.
 *
 * Developer can click to a line. When the line clicked it's color will be orange.
 * This color can be changed with the setChosenColor() function.
 */

class gGUIListbox: public gGUIScrollable {
public:
	gGUIListbox();
	virtual ~gGUIListbox();

	void set(gBaseApp* root, gBaseGUIObject* topParentGUIObject, gBaseGUIObject* parentGUIObject, int parentSlotLineNo, int parentSlotColumnNo, int x, int y, int w, int h);

	/*
	 * Used for add an element to da data list with given title.
	 *
	 * @param lineData is the string value that stored in the data list.
	 */
	void addData(std::string lineData);

	/*
	 * Changes the data of the given line no.
	 *
	 * @param lineData is the string value that stored in the data list.
	 */
	void setData(int lineNo, std::string lineData);

	/*
	 *  Renamed title of the given line number with the given text.
	 *
	 *  @lineNo is the line number that changed the title.
	 *  @lineData is the new title for the given line number.
	 */
	void insertData(int lineNo, std::string lineData);

	/*
	 * Used for erase the given line number from the data list.
	 *
	 * @param lineNo is the line number that developer wants to erase from the
	 * list.
	 */
	void removeData(int lineNo);

	/*
	 * Used for erase the click and selected line from the list.
	 */
	void removeSelected();

	/*
	 * Used for clean all the datas in the class.
	 */
	void clear();

	void drawContent();

	/*
	 * Sets the click and selected line with the given line number.
	 *
	 * @param lineNo is the number of line that selected.
	 */
	void setSelected(int lineNo);

	/*
	 * Returns selected line's number.
	 */
	int getSelected();

	/*
	 * Returns the title of the selected line.
	 */
	std::string getSelectedData();

	/*
	 * Returns the title of the given line number.
	 *
	 * @lineNo is the number of line that the title's given.
	 */
	std::string getData(int lineNo);

	/*
	 * Returns all the titles in a vector.
	 *
	 */
	std::vector<std::string> getData();

	/*
	 * Returns the size of data list.
	 */
	int getDataNum();

	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);

	/*
 	 * Sets the color of the row which developer click and select, with the
 	 * given color. Colors consist of RGB values float between 0.0f-1.0f.
 	 *
 	 * @param color The given color consist of (r, g, b) float values.
	 */
	void setChosenColor(float r, float g, float b);


	/*
	 * Sets the number of the lines that visible on the tree list. This function
	 * should be use before Treelist added to the panel with setControl function.
	 *
	 * @param linenumber is an integer value which define the number of minimum
	 * lines. Developer should give a number bigger than 0.
	 */
	void setVisibleLineNumber(int linenumber);

	/*
	 * Sets the icon type at the top of the titles. If developer wants to use new
	 * icons, should sets the isicon attribute. If the attribute is not setted,
	 * default list will be drawn.
	 *
	 * @param isicon is the boolean value that sets the icon types. For using new
	 * icons, it should be 'true'.
	 */
	void setIconType(bool isicon);

	/*
	 * Sets the icons with default 'File icon' for the all elements of the data
	 * list.
	 */
	void setIcons();

	/*
 	 * Sets the color of the icons with the given color. Colors consist of RGB
 	 * values float between 0.0f-1.0f.
 	 *
 	 * @param color The given color consist of (r, g, b) float values.
	 */
	void setIconsColor(float r, float g, float b);

	/*
	 * Sets the given icon to the given title. Developer can give any
	 * image from outside. The image must be loaded before using as a parameter
	 * for the function. Image's width and height will be arrange automatically.
	 *
	 * @param icon is the image that developer wants to use.
	 *
	 * @param title is the string data value which the icon will be changed.
	 */
	void setIcon(gImage* icon, std::string title);


	/*
	 * Sets the given icon to the given object of the struct. Uses the icons in
	 * the gGUIResources class. This icons are storing in the base64. Icon's
	 * width and height will be arrange automatically.
	 *
	 * @param iconid is the icon number of the images in the gGUIResources class.
	 * This numbers are defined with enumeration method.
	 *
	 * @param title is the string data value which the icon will be changed.
	 */
	void setIcon(int iconid, std::string title);

	/*
	 * Returns the isicon value of the given data list.
	 */
	bool getIconType();

	/*
	 * Returns the pointer of the image that uses for icons to the given data
	 * list
	 *
	 *  @param title is the string data value which returned icon image on the
	 *  list.
	 */
	gTexture* getIcon(std::string title);

	/*
	 * Returns the color that when an data is chosen.
	 */
	gColor getChosenColor();

	/*
	 * Returns the icons color.
	 */
	gColor getIconsColor();

	/*
	 * Returns the number of the lines which is visible in the box.
	 */
	int getVisibleLineNumber();

	int getTotalHeight();

private:
	int listboxh;
	int lineh, linenum;
	int minlinenum, minboxh;
	int maxlinenum;
	std::vector<std::string> data;
	std::deque<gTexture*> icons;
	float datady;
	int firstlineno, flno, fldy;
	int selectedno;
	bool mousepressedonlist;
	bool isicon;
	gColor chosencolor, iconcolor;
	int iconw;
};

#endif /* UI_GGUILISTBOX_H_ */
