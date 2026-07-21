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
	setSizer(&listsizer);
	guisizer->setSize(1, 2);
	float columnproportions[2] = {0.8f, 0.2f};
	guisizer->setColumnProportions(columnproportions);
	guisizer->enableBorders(false);
#if GLIST_ANDROID || GLIST_IOS
	// Tight internal packing, not the page's 24 unit finger gap: the textbox and
	// the arrow button sit side by side as one control. At 24 they are inset from
	// each other and from the box, and the arrow button - whose release opens the
	// list - ends up nowhere near where it is drawn. See the note in gGUINumberBox.
	guisizer->setSlotPadding(2, 0);
#endif
	button.setButtonColor(pressedbuttoncolor);
	button.setSize(buttonw, buttonw);
	button.setTitle("");
	textbox.setEditable(false);
	textbox.enableVerticalMargin(false);
	guisizer->setControl(0, 0, &textbox);
	guisizer->setControl(0, 1, &button);
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
	list.setTitleOn(false);
	ispressed = false;
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
    gGUIScrollable::set(root, topParentGUIObject, parentGUIObject, parentSlotLineNo, parentSlotColumnNo, x, y, w, h);
    gGUIScrollable::setDimensions(w, h);
    guisizer->set(root, topParentGUIObject, parentGUIObject, parentSlotLineNo, parentSlotColumnNo, x, y + topbarh, w, h - topbarh);
//	gGUIControl::set(root, topParentGUIObject, parentGUIObject, parentSlotLineNo, parentSlotColumnNo, x, y, w, h);
	textboxw = textbox.width;
	textboxh = textbox.getTextboxh();
	buttonw = textboxh;
	button.setSize(buttonw, buttonw);
	listw = textboxw + buttonw + 5;
	listx = textbox.left;
	listy = textbox.top + textboxh - list.getTitleTop() + 5;
}

void gGUIDropdownList::onGUIEvent(int guiObjectId, int eventType, int sourceEventType, std::string value1, std::string value2) {
	if(sourceEventType == G_GUIEVENT_BUTTONRELEASED) {
		buttonpressed = listopened;
		ispressed = false;
		frame->addTreelist(&list, listx, listy, listw);
		root->getCurrentCanvas()->onGuiEvent(id, G_GUIEVENT_TREELISTOPENEDONDROPDOWNLIST);
		actionmanager.onGUIEvent(id, G_GUIEVENT_TREELISTOPENEDONDROPDOWNLIST);
		listopened = !listopened;
	}
	if(sourceEventType == G_GUIEVENT_BUTTONPRESSED){
		ispressed = true;
	}
	if(sourceEventType == G_GUIEVENT_TREELISTSELECTED) {
		selectedline = true;
		setSelectedTitle();
		listopened = false;
		frame->addTreelist(nullptr, listx, listy, listw);
	}
	if(sourceEventType == G_GUIEVENT_TREELISTEXPANDED) {
		listexpanded = true;
	}
	if(sourceEventType == G_GUIEVENT_MOUSEPRESSEDONTREELIST) {
		pressedonlist = true;
	}
}

#if GLIST_ANDROID || GLIST_IOS
int gGUIDropdownList::getNaturalHeight() {
	// The collapsed textbox, not the open list: the drop-down overlays whatever
	// is beneath it rather than pushing the layout taller when it opens.
	return textboxh;
}
#endif

void gGUIDropdownList::draw() {
	gGUIContainer::draw();

//		if(listopened) {
//		list.draw();
//	}

	gColor* oldcolor = renderer->getColor();
	if(isdisabled) renderer->setColor(disabledbuttonfontcolor);
	else renderer->setColor(buttonfontcolor);
	gDrawTriangle((button.left + (buttonw/2)) - 6.5,
	                (top) + ((buttonw/2) - 3),
	                (button.left + (buttonw/2)) + 6.5,
	                (top) + ((buttonw/2) - 3),
	                (button.left + (buttonw/2)),
	                (button.top) + ((buttonw/2) + 3),
	                true);
	renderer->setColor(oldcolor);
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
	if(isdisabled) return;
	gGUIContainer::mousePressed(x, y, button);
	if(listopened)
		list.mousePressed(x, y, button);
	ispressed = true;
}

void gGUIDropdownList::mouseReleased(int x, int y, int button) {
	if(isdisabled) return;
    lopened = listopened;
    gGUIContainer::mouseReleased(x, y, button);
    if(listopened) {
    	list.mouseReleased(x, y, button);
    }
    setSelectedTitle();
    //Clicking on the Textbox opens the Treelist.
#if GLIST_ANDROID || GLIST_IOS
    // The original lower bound was textbox.height + buttonw - a size (~48 units),
    // not a coordinate. Near the top of a desktop form a small y falls under it and
    // it happens to work, but on a scrolling mobile page the box sits hundreds of
    // units down, y is never <= 48, and the list never opened. The real bottom of
    // the collapsed box is textbox.bottom.
    if (x >= textbox.left && x <= textbox.right && y >= textbox.top + 5 && y <= textbox.bottom) {
#else
    if (x >= textbox.left && x <= textbox.right && y >= textbox.top + 5 && y <= textbox.height + buttonw) {
#endif
        buttonpressed = true;
        frame->addTreelist(&list, listx, listy, listw);
        root->getCurrentCanvas()->onGuiEvent(id, G_GUIEVENT_TREELISTOPENEDONDROPDOWNLIST);
        actionmanager.onGUIEvent(id, G_GUIEVENT_TREELISTOPENEDONDROPDOWNLIST);
        listopened = true;
    }
    // Clicking outside the Textbox closes the Treelist.
    else if (lopened) {
        listopened = false;
        frame->addTreelist(nullptr, listx, listy, listw);
    }
    pressedonlist = false;
    ispressed = false;
}

void gGUIDropdownList::mouseScrolled(int x, int y) {
	if(isdisabled) return;
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

int gGUIDropdownList::getSelectedLineNo() {
	return list.getSelectedLineNumber();
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

int gGUIDropdownList::calculateContentHeight() {
	return textbox.calculateContentHeight();
}
