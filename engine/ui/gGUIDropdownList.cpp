/*
 * gGUIDropdownList.cpp
 *
 *  Created on: 19 Aðu 2022
 *      Author: sevval
 */

#include "gGUIDropdownList.h"


gGUIDropdownList::gGUIDropdownList() {
	listsizer.setSize(1, 2);
	float columnproportions[2] = {0.8f, 0.2f};
	listsizer.setColumnProportions(columnproportions);
	setSizer(&listsizer);
	button.setButtonColor(gColor(0.1f, 0.45f, 0.87f));
	button.setSize(24, 24);
	button.setTitle("-");
	textbox.setEditable(false);
	listsizer.setControl(0, 1, &button);
	listsizer.setControl(0, 0, &textbox);

	actionmanager.addAction(&button, G_GUIEVENT_BUTTONPRESSED, &textbox, G_GUIEVENT_TEXTBOXENTRY, "TEST");
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
//    gGUIScrollable::set(root, topParentGUIObject, parentGUIObject, parentSlotLineNo, parentSlotColumnNo, x, y, w, h);
//    gGUIScrollable::setDimensions(w, h);
    guisizer->set(root, topParentGUIObject, parentGUIObject, parentSlotLineNo, parentSlotColumnNo, x, y + topbarh, w, h - topbarh);
	gGUIControl::set(root, topParentGUIObject, parentGUIObject, parentSlotLineNo, parentSlotColumnNo, x, y, w, h);
}

//void gGUIDropdownList::draw() {

//	gColor* oldcolor = renderer->getColor();
//	renderer->setColor(textbackgroundcolor);
//	gDrawRectangle(0, top + 24, boxw, boxh , true);
//
//	renderer->setColor(oldcolor);

//}


