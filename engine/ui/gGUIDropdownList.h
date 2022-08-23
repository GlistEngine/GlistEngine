/*
 * gGUIDropdownList.h
 *
 *  Created on: 19 Aðu 2022
 *      Author: sevval
 */

#ifndef UI_GGUIDROPDOWNLIST_H_
#define UI_GGUIDROPDOWNLIST_H_

#include "gGUIContainer.h"
#include "gGUISizer.h"
#include "gGUIButton.h"
#include "gGUITextbox.h"
#include "gGUITreelist.h"

class gGUIDropdownList: public gGUIContainer {
public:
	gGUITreelist list;
	gGUIDropdownList();
	virtual ~gGUIDropdownList();
	void set(gBaseApp* root, gBaseGUIObject* topParentGUIObject, gBaseGUIObject* parentGUIObject, int parentSlotLineNo, int parentSlotColumnNo, int x, int y, int w, int h);
//	void draw();


private:
	gGUISizer listsizer;
	gGUIButton button;
	gGUITextbox textbox;

};



#endif /* UI_GGUIDROPDOWNLIST_H_ */
