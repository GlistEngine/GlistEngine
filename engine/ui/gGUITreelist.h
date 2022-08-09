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

/*
 * This class creats a tree list. Tree list is kinf of a list that includes sub
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
 * Developers can get the struct's attributes with getter functions.
 */
class gGUITreelist: public gGUIScrollable {
public:
	struct Element {
		std::string title;
		std::deque<Element*> sub;
		Element* parent;
		static int nodenum;
		bool isexpanded;
		bool isparent;
		int orderno;
		static std::vector<std::string> allsubtitles;

		Element() {
			nodenum = 0;
			isexpanded = false;
			orderno = 0;
			parent = nullptr;
			isparent = false;
		}

		/*
		 * Used for add a sub element into the parent element's sub vector.
		 * Developer should initialize the title first. Element's orderno and the
		 * nodenum will be updates in the function.
		 *
		 * @param element is the struct object which includes element's attributes.
		 */
		void addElement(Element* element) {
			sub.push_back(element);
			element->parent = this;
			element->orderno = orderno + 1;
			element->nodenum = this->nodenum + 1;
		}

		/*
		 * Used for print all the objects to the console in the struct.
		 */
		void logTitle() {
			if(parent!= nullptr) gLogi("Element") << title << (" Nodenumber") << nodenum;
			for(int i = 0; i < sub.size(); i++) {
				sub[i]->logTitle();
			}
		}

		/*
		 * Used for clear the list which used for draw the titles before updating
		 * list.
		 */
		void clearAllSubTitlesList() {
			allsubtitles.clear();
		}

		/*
		 * Makes a list for drawing recursively. Updates the list when it's called.
		 * Operates when
		 * an element added to the list,
		 * an element removed from the list,
		 * making a update to the element's title,
		 * the user clicked to parent title and opened the sub titles.
		 */
		void addSelfToList() {
			std::string linetext = "";
			for(int i = 0; i < orderno; i++) linetext = "    " + linetext;
			if(sub.size() > 0) {
				linetext = linetext + "> ";
				isparent = true;
			}
			else linetext = linetext + "- ";
			linetext += title;

			allsubtitles.push_back(linetext);
			if(isexpanded) for(int i = 0; i < sub.size(); i++) sub[i]->addSelfToList();
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
		    	nodenum -= element->sub.size();
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
			nodenum -= 1;
			int index = 0;
			for(auto i: element->parent->sub) {
				if(i!= element) {
					index++;
				}
				else break;
			}
			element->parent->sub.erase(element->parent->sub.begin() + index);
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
	 * Renamed title of the given element with the given text.
	 *
	 * @param element is the struct object that renamed on the list.
	 * @param newtitle is the string value which updated to the element's title.
	 */
	void insertData(Element* element, std::string newtitle);

	void drawContent();

	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);

	/*
	 * Returns given struct object's title.
	 *
	 * @param element is the struct object which returned title's node.
	 */
	std::string getTitle(Element* element);

	/*
	 * Returns given struct object's node number. Node number is a static integer
	 * value. It contains the total number of the elements.
	 *
	 *  @param element is the struct object which has the node number value.
	 */
	int getNodenum(Element* element);

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
private:
	int listboxh;
	int lineh, linenum;
	int minlinenum, minboxh;
	int maxlinenum;
	int fldy, flno;
	float datady;
	int firstlineno;
	int selectedno;

	float arrowsize;
	bool mousepressedonlist;
	Element topelement;
};

#endif /* UI_GGUITREELIST_H_ */
