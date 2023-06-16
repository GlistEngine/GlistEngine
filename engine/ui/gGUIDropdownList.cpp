/*
 * gGUIDropdownList.cpp
 *
 *  Created on: 19 A�u 2022
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
	setSizer(&listsizer);
	button.setButtonColor(pressedbuttoncolor);
	button.setSize(buttonw, buttonw);
	button.setTitle("-");
	textbox.setEditable(false);
	listsizer.setControl(0, 0, &textbox);
	listsizer.setControl(0, 1, &button);
	listx = textbox.left;
	listy = textbox.top + textbox.height;
	listw = textbox.width + button.width + 1;
	listopened = false;
	selectedline = false;
	listexpanded = false;
	pressedonlist = false;
	buttonpressed = false;
	frame = nullptr;
	lopened = false;
	textboxh = 0;
	textboxw = 0;
	rootelement = nullptr;


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
	listy = textbox.top + textboxh - list.getTitleTop();
}

void gGUIDropdownList::onGUIEvent(int guiObjectId, int eventType, int sourceEventType, std::string value1, std::string value2) {
	if(sourceEventType == G_GUIEVENT_BUTTONRELEASED) {
		buttonpressed = true;
		frame->addTreelist(&list, listx, listy, listw);
		root->getCurrentCanvas()->onGuiEvent(id, G_GUIEVENT_TREELISTOPENEDONDROPDOWNLIST);
		actionmanager.onGUIEvent(id, G_GUIEVENT_TREELISTOPENEDONDROPDOWNLIST);
		listopened = !listopened;
	}
	if(sourceEventType == G_GUIEVENT_TREELISTSELECTED) {
		selectedline = listopened;
	}
	if(sourceEventType == G_GUIEVENT_TREELISTEXPANDED) {
		listexpanded = true;
	}
	if(sourceEventType == G_GUIEVENT_MOUSEPRESSEDONTREELIST) {
		pressedonlist = true;
	}
}

void gGUIDropdownList::draw() {
	gGUIContainer::draw();
	if(listopened) {
//		list.draw();
	}
//	gColor* oldcolor = renderer->getColor();
//	renderer->setColor(oldcolor);

}

void gGUIDropdownList::setParentFrame(gGUIFrame *frame) {
	this->frame = frame;
//	(*frame).addTreelist(&list, listx, listy, listw);
}


void gGUIDropdownList::addElement(gGUITreelist::Element* element) {
	list.addElement(element);
	setfirstTitle();
}

void gGUIDropdownList::addElement(gGUITreelist::Element* element, gGUITreelist::Element* parentelement) {
	list.addElement(element, parentelement);
}

void gGUIDropdownList::mousePressed(int x, int y, int button) {
	gGUIContainer::mousePressed(x, y, button);
	list.mousePressed(x, y, button);
}

void gGUIDropdownList::mouseReleased(int x, int y, int button) {
	lopened = listopened;
	gGUIContainer::mouseReleased(x, y, button);
	list.mouseReleased(x, y, button);
	setSelectedTitle();
	if(lopened && !pressedonlist) {
		listopened = false;
		frame->addTreelist(nullptr, listx, listy, listw);
	}
	pressedonlist = false;
}

void gGUIDropdownList::mouseScrolled(int x, int y) {
	list.mouseScrolled(x, y);
}

void gGUIDropdownList::setfirstTitle() {
	rootelement = list.getRootElement();
	if(rootelement->sub.size() > 0) {
		textbox.setText(rootelement->sub[0]->title);
	}
}

void gGUIDropdownList::setSelectedTitle() {
	std::string title = "";
	bool arrow = false;
	if(selectedline) {
		title = rootelement->parentlist->allsubtitles[list.getSelectedLineNumber()];
		if(rootelement->isicon) {
			int i = 0;
			while(i < title.size() && arrow == false) {
				if(title[i] == ' ') {
					i++;
				}
				else arrow = true;
			}
			title = title.substr(i, title.size() - i);
		}
		else {
			int i = 0;
			while(i < title.size() && arrow == false) {
				if(title[i] == '>' || title[i] == '-') {
					arrow = true;
					title = title.substr(i + 2, title.size() - i);
				}
				i++;
			}
		}
		textbox.setText(title);
	}
}
