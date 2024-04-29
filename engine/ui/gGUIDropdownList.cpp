 /*
  * gGUIDropdownList.cpp
  *
  *  Created on: 19 Aug 2022
  *      Author: sevval
 */

#include "gGUIDropdownList.h"
#include "gBaseCanvas.h"
#include "gBaseApp.h"

gGUIDropdownList::gGUIDropdownList() {
	buttonw = 24;
	listsizer.setSize(1, 2);
	float columnproportions[2] = {0.8f, 0.2f};
	listsizer.setColumnProportions(columnproportions);
	listsizer.enableBorders(false);
	setSizer(&listsizer);
	button.setButtonColor(pressedbuttoncolor);
	button.setSize(buttonw, buttonw);
	button.setTitle("");
	textbox.setEditable(false);
	textbox.enableVerticalMargin(false);
	listsizer.setControl(0, 0, &textbox);
	listsizer.setControl(0, 1, &button);
	listx = textbox.left;
	listy = textbox.top + textbox.height;
	listw = textbox.width + button.width + 1;
	listopen = false;
	selectedline = false;
	frame = nullptr;
	textboxh = 0;
	textboxw = 0;
	rootelement = nullptr;
	list.setTitleOn(false);
	isdisabled = false;

	actionmanager.addAction(&button, G_GUIEVENT_BUTTONRELEASED, this, G_GUIEVENT_TREELISTOPENEDONDROPDOWNLIST);
	actionmanager.addAction(&list, G_GUIEVENT_TREELISTSELECTED, this, G_GUIEVENT_TREELISTOPENEDONDROPDOWNLIST);
	actionmanager.addAction(&list, G_GUIEVENT_TREELISTEXPANDED, this, G_GUIEVENT_TREELISTOPENEDONDROPDOWNLIST);
	actionmanager.addAction(&list, G_GUIEVENT_MOUSEPRESSEDONTREELIST, this, G_GUIEVENT_TREELISTOPENEDONDROPDOWNLIST);
}

gGUIDropdownList::~gGUIDropdownList() {
}


void gGUIDropdownList::set(gBaseApp* root, gBaseGUIObject* topParentGUIObject, gBaseGUIObject* parentGUIObject, int parentSlotLineNo, int parentSlotColumnNo, int x, int y, int w, int h) {
    totalh = h;
    left = x;
    top = y;
    right = x + w;
    bottom = y + h;
    width = w;
    height = h;
    gGUIScrollable::set(root, topParentGUIObject, parentGUIObject, parentSlotLineNo, parentSlotColumnNo, x, y, w, h);
    gGUIScrollable::setDimensions(w, h);
    guisizer->set(root, topParentGUIObject, parentGUIObject, parentSlotLineNo, parentSlotColumnNo, x, y + topbarh, w, h - topbarh);
//	gGUIControl::set(root, topParentGUIObject, parentGUIObject, parentSlotLineNo, parentSlotColumnNo, x, y, w, h);
	textboxw = textbox.width;
	textboxh = textbox.getTextboxh();
	buttonw = textboxh;
	listw = textboxw + buttonw + 5;
	listx = textbox.left;
	listy = textbox.top + textboxh - list.getTitleTop() + 5;
}

void gGUIDropdownList::onGUIEvent(int guiObjectId, int eventType, int sourceEventType, std::string value1, std::string value2) {
	if(sourceEventType == G_GUIEVENT_TREELISTSELECTED) {
		selectedline = true;
		setSelectedTitle();
		listopen = false;
		frame->addTreelist(nullptr, listx, listy, listw);
	}
}

void gGUIDropdownList::draw() {
	gGUIContainer::draw();

//		if(listopen) {
//		list.draw();
//	}

	gColor* oldcolor = renderer->getColor();
	if(isdisabled) {
		renderer->setColor(disabledbuttonfontcolor);
	} else {
		renderer->setColor(buttonfontcolor);
	}
	gDrawTriangle((button.left + (buttonw/2)) - 6.5,
	                (top) + ((buttonw/2) - 3),
	                (button.left + (buttonw/2)) + 6.5,
	                (top) + ((buttonw/2) - 3),
	                (button.left + (buttonw/2)),
	                (button.top) + ((buttonw/2) + 3),
	                true);
	renderer->setColor(oldcolor);

	if (!isdisabled) {
		BIND_GUI_EVENT(gMouseButtonPressedEvent, onMousePressedEvent);
		BIND_GUI_EVENT(gMouseButtonReleasedEvent, onMouseReleasedEvent);
	}
}

void gGUIDropdownList::setParentFrame(gGUIForm* form) {
	setParentForm(form);
}

void gGUIDropdownList::setParentForm(gGUIForm* form) {
	this->frame = form;
}

void gGUIDropdownList::addElement(gGUITreelist::Element* element) {
	list.addElement(element);
	setfirstTitle();
}

void gGUIDropdownList::addElement(gGUITreelist::Element* element, gGUITreelist::Element* parentelement) {
	list.addElement(element, parentelement);
}

void gGUIDropdownList::mousePressed(int x, int y, int button) {
	if(isdisabled) {
		return;
	}
	gGUIContainer::mousePressed(x, y, button);
	if(listopen) {
		list.mousePressed(x, y, button);
	}
}

void gGUIDropdownList::mouseReleased(int x, int y, int button) {
	if(isdisabled) {
		return;
	}
    gGUIContainer::mouseReleased(x, y, button);
    if(listopen) {
    	list.mouseReleased(x, y, button);
    }
}

void gGUIDropdownList::mouseScrolled(int x, int y) {
	if(isdisabled) {
		return;
	}
	list.mouseScrolled(x, y);
}

void gGUIDropdownList::setfirstTitle() {
	rootelement = list.getRootElement();
	if(rootelement->sub.size() > 0) {
		textbox.setText(rootelement->sub[0]->title);
		title = rootelement->sub[0]->title;
		fTitle = title;
	}
}

void gGUIDropdownList::setSelectedTitle() {
	std::string title = "";
	bool arrow = false;
	if(selectedline) {
		title = rootelement->parentlist->allsubtitles[list.getSelectedLineNumber()];
		if(rootelement->isicon) {
			int i = 0;
			while(i < title.size() && !arrow) {
				if(title[i] == ' ') {
					i++;
				} else {
					arrow = true;
				}
			}
			title = title.substr(i, title.size() - i);
		} else {
			int i = 0;
			while(i < title.size() && !arrow) {
				if(title[i] == '>' || title[i] == '-') {
					arrow = true;
					title = title.substr(i + 2, title.size() - i);
				}
				i++;
			}
		}
		textbox.setText(title);

		fTitle = title;
	}
}

std::string& gGUIDropdownList::getSelectedTitle() {
	return fTitle;
}

void gGUIDropdownList::clearTitle() {
	textbox.setText("");
}

void gGUIDropdownList::clear() {
	list.clear();
	clearTitle();
}

void gGUIDropdownList::setDisabled(bool isDisabled) {
	isdisabled = isDisabled;
	textbox.setDisabled(isDisabled);
}

bool gGUIDropdownList::onMousePressedEvent(gMouseButtonPressedEvent& event) {
	int x = event.getX();
	int y = event.getY();

	if (listopen && x >= left && x <= right && y >= top && y <= bottom) {
		return true;
	}
	return false;
}

bool gGUIDropdownList::onMouseReleasedEvent(gMouseButtonReleasedEvent& event) {
	int x = event.getX();
	int y = event.getY();

	setSelectedTitle();
	//Clicking on the Textbox opens the Treelist.
	if (!listopen && x >= left && x <= right && y >= top && y <= bottom) {
		frame->addTreelist(&list, listx, listy, listw);
		root->getCurrentCanvas()->onGuiEvent(id, G_GUIEVENT_TREELISTOPENEDONDROPDOWNLIST);
		actionmanager.onGUIEvent(id, G_GUIEVENT_TREELISTOPENEDONDROPDOWNLIST);
		listopen = true;
		list.isfocused = true;
		return true;
	}

	if (listopen) {
		// Clicking outside the Textbox closes the Treelist.
		listopen = false;
		frame->addTreelist(nullptr, listx, listy, listw);
		return true;
	}
	return false;
}