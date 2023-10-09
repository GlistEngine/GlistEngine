/*
 * gGUIDropdownList.h
 *
 *  Created on: 19 A�u 2022
 *      Author: sevval
 */


#ifndef UI_GGUIDROPDOWNLIST_H_
#define UI_GGUIDROPDOWNLIST_H_

#include "gGUIContainer.h"
#include "gGUISizer.h"
#include "gGUIButton.h"
#include "gGUIImageButton.h"
#include "gGUITextbox.h"
#include "gGUITreelist.h"
#include "gGUIFrame.h"
#include "gBaseCanvas.h"
#include "gBaseApp.h"

class gGUIDropdownList: public gGUIContainer {
public:
	gGUITreelist list;
	virtual void onGUIEvent(int guiObjectId, int eventType, int sourceEventType, std::string value1 = "", std::string value2 = "");
	gGUIDropdownList();
	virtual ~gGUIDropdownList();
	void set(gBaseApp* root, gBaseGUIObject* topParentGUIObject, gBaseGUIObject* parentGUIObject, int parentSlotLineNo, int parentSlotColumnNo, int x, int y, int w, int h);
	void draw();
	void setParentFrame(gGUIFrame *frame);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void mouseScrolled(int x, int y);
	void setfirstTitle();
	void setSelectedTitle();
	void addElement(gGUITreelist::Element* element);
	void addElement(gGUITreelist::Element* element, gGUITreelist::Element* parentelement);
	void clearTitle();
	void setSelectLastTitle();

	std::string& getSelectedTitle();

	friend class gGUIToolbar;

private:
	gGUISizer listsizer;
	gGUIButton button;
	gGUIImageButton ibutton;
	gGUITextbox textbox;
	gGUITreelist::Element* rootelement;
	int textboxw, buttonw, buttonh;
	int textboxh;
	int listx, listy, listw;
	bool listopened, selectedline, listexpanded, pressedonlist, buttonpressed, ispressed;
	bool lopened;
	gGUIFrame *frame;
	std::string fTitle;
};

#endif /* UI_GGUIDROPDOWNLIST_H_ */
