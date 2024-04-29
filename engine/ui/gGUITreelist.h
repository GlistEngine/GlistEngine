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
 * Author: Sevval Bulburu, Aynur Dogan 2022-08-09                           *
 ****************************************************************************/

#ifndef UI_GGUITREELIST_H_
#define UI_GGUITREELIST_H_

#include "gGUIScrollable.h"
#include <deque>
#include <iostream>
#include "gImage.h"

/*
 * This class creats a tree list. Tree list is kind of a list that includes sub
 * titles. All sub titles can have their own sub titles. There is not any edge
 * for the numbers of sub titles. Uses a struct and its vector attribute which
 * includes struct type of elements for storing the sub elements.
 *
 * The devolopers can make operations on the tree list.This operations are basically
 * adding an object to the list, removing an object from the list, updating titles
 * of the objects from the list.
 *
 * Developers can choose a title on the list. If they clicked on '>' symbol and
 * the sub titles are  not on the draw list, the  parent element's sub vector's
 * titles adding to the list and the list will be updating, sub titles will be
 * shown. If the sub titles are shown on the list, draw list will be updating
 * and sub titles will be disabled.
 *
 * Also there is an icon mode for the tree list. If developer uses the setIconType()
 * function and actived the icons, '>' and '-' symbols will be 'Folder' and 'File'
 * icons. This icons can change for all elements with using gGUIResources class
 * or getting an image from outside.
 *
 * Developers can get the struct's attributes with getter functions.
 */
class gGUITreelist: public gGUIScrollable {
public:
	struct Element {
		gGUITreelist* parentlist;
		std::string title;
		std::deque<Element*> sub;
		Element* parent;
		bool isexpanded;
		bool isparent;
		bool isicon;
		bool isiconchanged;
		int orderno;
		gTexture* icon;

		Element() {
			parentlist = nullptr;
			isicon = false;
			isexpanded = false;
			orderno = 0;
			parent = nullptr;
			isparent = false;
			icon = nullptr;
			isiconchanged = false;
			title = "";
		}

		/*
		 * Used for print all the objects to the console in the struct.
		 */
		void logTitle() {
			if(parent!= nullptr) gLogi("Element") << title << (" iconid") << res.getIconNum();
			for(int i = 0; i < sub.size(); i++) {
				sub[i]->logTitle();
			}
		}

		/*
		 * Used for clear the list which used for draw the titles before updating
		 * list.
		 */
		void clearAllSubTitlesList() {
			parentlist->allsubtitles.clear();
			parentlist->allorderno.clear();
			parentlist->icons.clear();
		}

		/*
		 * Makes a list for drawing recursively. Updates the list when it's called.
		 * Operates when
		 * an element added to the list,
		 * an element removed from the list,
		 * making an update to the element's title,
		 * the user clicked to parent's title and opened the sub titles.
		 */
		void addSelfToList() {
			std::string linetext = "";
			if(isicon == false) {
				for(int i = 0; i < orderno; i++) linetext = "    " + linetext;
				if(sub.size() > 0) {
					linetext = linetext + "> ";
					isparent = true;
				}
				else linetext = linetext + "- ";
			}
			else{
				for(int i = 0; i < orderno + 1; i++) linetext = "  " + linetext;
				parentlist->allorderno.push_back(orderno + 1);
				parentlist->icons.push_back(this->icon);
				if(sub.size() > 0) isparent = true;
			}

			linetext += title;
			parentlist->allsubtitles.push_back(linetext);
			if(isexpanded) for(int i = 0; i < sub.size(); i++) sub[i]->addSelfToList();
			if(title == "Top") {
				parentlist->allsubtitles.erase(parentlist->allsubtitles.begin() + 0);
				if(parentlist->allorderno.size() > 0) parentlist->allorderno.erase(parentlist->allorderno.begin() + 0);
				if(parentlist->icons.size() > 0) parentlist->icons.erase(parentlist->icons.begin() + 0);
			}
		}

		/*
		 * Finds an element according to the titles. Compares the given title with
		 * the struct object's titles. When the titles are matched it returns the
		 * struct object which has the same title with the given. If there is no
		 * match then returns null.
		 *
		 * @param title is the string value which we want to compare with the
		 * struct object's title.
		 */
		Element* findElement(std::string title) {
			Element* tmp;
			if(title == this->title) return this;
			for(int i = 0; i < sub.size(); i++) {
				tmp = sub[i]->findElement(title);
				if(tmp) return tmp;
			}
			return nullptr;
		}

		/*
		 * Delete all the sub struct objects in the parent element's sub vectors.
		 * When it erase the sub vectors, it updates the number of total elements
		 * in the struct.
		 *
		 * @param element is the struct object which deleted from the list.
		 */
		void removeSubElement(Element* element) {
		    if(element->sub.size() != 0) {
		    	parentlist->nodenum -= element->sub.size();
		    	for(int i = 0; i < element->sub.size(); i++) removeSubElement(element->sub[i]);
		    }
		    element->sub.clear();
		}

		/*
		 * Delete the given struct object from the list. Before it erase the
		 * element, it calls the removeSubElement function and erase all the sub
		 * elements then delete the given element.
		 *
		 * @param element is the struct object which deleted from the list.
		 */
		void removeElement(Element* element) {
			removeSubElement(element);
			parentlist->nodenum -= 1;
			int index = 0;
			for(auto i: element->parent->sub) {
				if(i!= element) {
					index++;
				}
				else break;
			}
			element->parent->sub.erase(element->parent->sub.begin() + index);
		}

		/*
		 * Sets the isicon attributes of all elements.
		 */
		void setIconType(bool isicon) {
			this->isicon = isicon;
			for(int i = 0; i < sub.size(); i++) sub[i]->setIconType(isicon);
		}

		/*
		 * Sets the icon attributes of all elements. This function uses when
		 * isicon value of the elements is true.
		 * Sets the icons according to their isparent attributes. If the element
		 * has got a sub vector, it's icon will be the 'Folder icon'. If it has
		 * not got a sub vector, it's icon will be the 'File icon'.
		 *
		 */
		void setIcon() {
			if(sub.size() > 0) this->icon = res.getIconImage(gGUIResources::ICON_FOLDER);
			else this->icon = res.getIconImage(gGUIResources::ICON_FILE);
			for(int i = 0; i < sub.size(); i++) sub[i]->setIcon();
		}
	};


	gGUITreelist();
	virtual ~gGUITreelist();


	void set(gBaseApp* root, gBaseGUIObject* topParentGUIObject, gBaseGUIObject* parentGUIObject, int parentSlotLineNo, int parentSlotColumnNo, int x, int y, int w, int h);

	/*
	 * Delete the given struct object from the tree. Uses the struct's remove
	 * functions.
	 *
	 * @param element is the struct object which deleted from the list.
	 */
	void removeElement(Element* element);

	/*
	 * Used for add a struct object to the tree's sub vector.
	 * Developer should initialize the title first.
	 *
	 * @param element is the struct object which includes element's attributes.
	 */
	void addElement(Element* element);

	/*
	 * Used for add a sub element into the parent element's sub vector.
	 * Developer should initialize the title first. Element's orderno and the
	 * nodenum will be updates in the function.
	 *
	 * @param element is the struct object which includes element's attributes.
	 *
	 * @param parent is the struct object which element will be added to its
	 * sub vector.
	 */
	void addElement(Element* element, Element* parent);

	/*
	 * Renamed title of the given element with the given text.
	 *
	 * @param element is the struct object that renamed on the list.
	 * @param newtitle is the string value which updated to the element's title.
	 */
	void insertData(Element* element, std::string newtitle);

	/*
	 * Uses for update the draw list and some values about the draw box.
	 */
	void refreshList();

	void clear();

	void drawContent();

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
	 * default icons will be '>' and '-'.
	 *
	 * @param isicon is the boolean value that sets the icon types. For using new
	 * icons, it should be 'true'.
	 */
	void setIconType(bool isicon);

	/*
	 * Sets the given icon to the given object of struct. Developer can give any
	 * image from outside. The image must be loaded before using as a parameter
	 * for the function. Image's width and height will be arrange automatically.
	 *
	 * @param icon is the image that developer wants to use.
	 *
	 *  @param element is the struct object which the icon will be changed.
	 */
	void setIcon(gImage* icon, Element* element);


	/*
	 * Sets the given icon to the given object of the struct. Uses the icons in
	 * the gGUIResources class. This icons are storing in the base64.
	 *
	 * @param iconid is the icon number of the images in the gGUIResources class.
	 * This numbers are defined with enumeration method.
	 *
	 * @param element is the struct object which the icon will be changed.
	 */
	void setIcon(int iconid, Element* element);

	/*
 	 * Sets the color of the icons with the given color. Colors consist of RGB
 	 * values float between 0.0f-1.0f.
 	 *
 	 * @param color The given color consist of (r, g, b) float values.
	 */
	void setIconsColor(float r, float g, float b);


	/*
	 * Returns given struct object's title.
	 *
	 * @param element is the struct object which returned title's node.
	 */
	std::string getTitle(Element* element);


	/*
	 * Returns given struct object's title.
	 *
	 * @param No of the element to get it's title
	 */
	std::string getTitle(int elementNo);


	/*
	 * Returns selected struct object's title.
	 *
	 */
	std::string getSelectedTitle();

	/*
	 * Returns given struct object's node number. Node number is a static integer
	 * value. It contains the total number of the elements.
	 *
	 *  @param element is the struct object which has the node number value.
	 */
	int getNodenum();

	/*
	 * Returns given struct object's orderno. Orderno is used for calculating the
	 * sub elements locations on the x axis on the list.
	 *
	 * @param element is the struct object which returned the orderno's node.
	 */
	int getOrderno(Element* element);

	/*
	 * Returns the given struct object's isexpanded value. If the sub titles of
	 * the element is on the list, it returns true. If is not, it returns false.
	 *
	 * @param element is the struct object which if it is isexpanded or not.
	 */
	bool isExpanded(Element* element);

	/*
	 * Returns the given struct object's isparent value. If given element is a
	 * parent, it returns true. If is not, it returns false.
	 *
	 * @param element is the struct object which if it is isparent or not.
	 */
	bool isParent(Element* element);

	/*
	 * Returns the isicon value of the struct object.
	 */
	bool getIconType();

	/*
	 * Returns the pointer of the image that uses for icons to the given struct
	 * object.
	 *
	 *  @param element is the struct object which returned icon image.
	 */
	gTexture* getIcon(Element* element);

	/*
	 * Returns the color that when an element is chosen.
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

	/*
	 * Returns root element of the struct object.
	 */
	Element* getRootElement();

	void setSelectedLineNumber(int lineNo);
	int getSelectedLineNumber();

	int nodenum;
	std::vector<std::string> allsubtitles;
	std::vector<int> allorderno;
	std::deque<gTexture*> icons;

private:
	int listboxh;
	int lineh;
	int visibilelinenum, minboxh;
	int fldy, flno;
	float textoffset;
	int firstlineno;
	int selectedno;
	float arrowsize, spacesize;
	bool mousepressedonlist;
	Element topelement;
	gColor chosencolor, iconcolor;
	int iconx, iconw, iconh;

private:
	bool onMousePressedEvent(gMouseButtonPressedEvent& event);
	bool onMouseReleasedEvent(gMouseButtonReleasedEvent& event);

	void updateTotalHeight();
};

#endif /* UI_GGUITREELIST_H_ */
