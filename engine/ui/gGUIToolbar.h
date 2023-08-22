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
#include "gGUIDivider.h"
#include "gGUIFrame.h"
#include <vector>
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
	void addDivider(gGUIDivider* divider, float margin);
	void addSpace();

	void draw();

private:
	int toolbartype;
	int spaceLocation;
	bool isSpaceAdded;
	std::vector<gGUIControl*> controlObjects;
	std::vector<float> sizerPrs;

	void resizeSizer();
};

#endif /* UI_GGUITOOLBAR_H_ */
