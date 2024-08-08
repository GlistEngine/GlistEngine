/*
 * gGUIToolbar.h
 *
 *  Created on: Sep 25, 2021
 *      Author: noyan
 */

#ifndef UI_GGUITOOLBAR_H_
#define UI_GGUITOOLBAR_H_

#include "gGUIContainer.h"
#include "gGUIToolbarButton.h"
#include "gGUIControl.h"
#include "gGUIDropdownList.h"
#include "gGUICheckbox.h"
#include "gGUINumberBox.h"
#include "gGUISlider.h"
#include "gGUISwitchButton.h"
#include "gGUIText.h"
#include "gGUIBitmap.h"
#include "gGUIDivider.h"
#include "gGUIFrame.h"
#include <deque>
#include <typeinfo>


class gGUIToolbar: public gGUIContainer {
public:
	gGUIToolbar();
	virtual ~gGUIToolbar();

	enum {
		TOOLBAR_HORIZONTAL,
		TOOLBAR_VERTICAL
	};

	void setToolbarType(int toolbarType);
	int getToolbarType();
	void addControl(gGUIControl* control);
	//void addControl(std::vector<gGUIControl*> control);
	void addToolbarButton(gGUIToolbarButton* toolbarButton);
	void addDropdownList(gGUIDropdownList* dropdownList, gGUIFrame* frame);
	void addCheckbox(gGUICheckbox* checkbox);
	void addNumberBox(gGUINumberBox* numberBox);
	void addSlider(gGUISlider* slider);
	void addSwitchButton(gGUISwitchButton* switchButton);
	void addText(gGUIText* text);
	void addImage(gGUIBitmap* image);
	void addDivider(gGUIDivider* divider, float margin);
	void addSpace();
	void setToolbarForegroundColor(gColor* color);
	void setToolbarBottomLineColor(gColor* color);


	void draw();

private:
	int toolbartype;
	int spaceLocation;
	bool isSpaceAdded;
	std::deque<gGUIControl*> controlObjects;
	std::deque<float> sizerPrs;
	gColor* tbforegroundcolor, tbbottomlinecolor;

	void resizeSizer();
};

#endif /* UI_GGUITOOLBAR_H_ */
