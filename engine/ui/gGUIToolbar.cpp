/*
 * gGUIToolbar.cpp
 *
 *  Created on: Sep 25, 2021
 *      Author: noyan
 */

#include "gGUIToolbar.h"


gGUIToolbar::gGUIToolbar() {
	spaceLocation = 0;
	sizerrescaling = false;
	isSpaceAdded = false;
	toolbartype = TOOLBAR_HORIZONTAL;
}

gGUIToolbar::~gGUIToolbar() {
}

void gGUIToolbar::setToolbarType(int toolbarType) {
	toolbartype = toolbarType;
}

int gGUIToolbar::getToolbarType() {
	return toolbartype;
}

void gGUIToolbar::draw() {
//	gLogi("gGUIToolbar") << "draw";
//	gLogi("gGUIToolbar") << "l:" << left << ", t:" << top << ", w:" << width << ", h:" << height;
	gColor oldcolor = *renderer->getColor();
	if(toolbartype == TOOLBAR_HORIZONTAL) {
		renderer->setColor(foregroundcolor);
		gDrawRectangle(left, top, width, height, true);
		renderer->setColor(backgroundcolor);
		gDrawLine(left, bottom, right, bottom);
	//	gDrawRectangle(left, top, width, height, false);
	} else {
		renderer->setColor(foregroundcolor);
		gDrawRectangle(left, top, width, height, true);
		renderer->setColor(backgroundcolor);
		gDrawLine(right, top, right, bottom);
	}
	renderer->setColor(&oldcolor);
	if(guisizer) guisizer->draw();
}

void gGUIToolbar::addControl(gGUIControl *control) {
	if(typeid(*control) == typeid(gGUIToolbarButton)) {
		gGUIToolbarButton* derived = dynamic_cast<gGUIToolbarButton*>(control);
		addToolbarButton(derived);
	}
	else if(typeid(*control) == typeid(gGUICheckbox)) {
		gGUICheckbox* derived = dynamic_cast<gGUICheckbox*>(control);
		addCheckbox(derived);
	}
	else if(typeid(*control) == typeid(gGUINumberBox)) {
		gGUINumberBox* derived = dynamic_cast<gGUINumberBox*>(control);
		addNumberBox(derived);
	}
	else if(typeid(*control) == typeid(gGUISlider)) {
		gGUISlider* derived = dynamic_cast<gGUISlider*>(control);
		addSlider(derived);
	}
	else if(typeid(*control) == typeid(gGUISwitchButton)) {
		gGUISwitchButton* derived = dynamic_cast<gGUISwitchButton*>(control);
		addSwitchButton(derived);
	}
	else if(typeid(*control) == typeid(gGUIText)) {
		gGUIText* derived = dynamic_cast<gGUIText*>(control);
		addText(derived);
	}
	else if(typeid(*control) == typeid(gGUIDivider)) {
		gGUIDivider* derived = dynamic_cast<gGUIDivider*>(control);
		addDivider(derived, 0.05f);
	}
	else {
		controlObjects.push_back(control);
		sizerPrs.push_back(0.05f);
		resizeSizer();
	}
}

/*void gGUIToolbar::addControl(std::vector<gGUIControl*> control) {
	controls = control;
	int totalElements = controls.size();
	float totalLen = 0, margin;

	float *toolbarSizerprs = new float[totalElements + 1];
	for (int i = 0; i < totalElements; ++i) {
		if(typeid(*controls.at(i)) == typeid(gGUIToolbarButton)) {
			if(toolbartype == TOOLBAR_HORIZONTAL) margin = 0.03f;
			else margin = 0.05f;
		}
		else if(typeid(*controls.at(i)) == typeid(gGUIDropdownList)) {
			if(toolbartype == TOOLBAR_HORIZONTAL) margin = 0.15f;
			else margin = 0.05f;
		}
		else {
			margin = 0.05f;
		}
		toolbarSizerprs[i] = margin;
		totalLen += margin;
	}
	toolbarSizerprs[totalElements] = 1.0f - totalLen;

	if(toolbartype == TOOLBAR_HORIZONTAL) {
		guisizer->setSize(1, totalElements + 1);
		guisizer->setColumnProportions(toolbarSizerprs);

		for (int i = 0; i < totalElements; ++i) {
			guisizer->setControl(0, i, controls.at(i));
		}
	} else {
		guisizer->setSize(totalElements + 1, 1);
		guisizer->setLineProportions(toolbarSizerprs);

		for (int i = 0; i < totalElements; ++i) {
			guisizer->setControl(i, 0, controls.at(i));
		}
	}

	guisizer->enableBorders(true);
	delete[] toolbarSizerprs;
}*/

void gGUIToolbar::addToolbarButton(gGUIToolbarButton *toolbarButton) {
	controlObjects.push_back(toolbarButton);
	/*toolbarButton->setButtonh(guisizer->getSlotHeight(0, 0));
	toolbarButton->setButtonw(guisizer->getSlotHeight(0, 0));*/
	float margin = float(toolbarButton->getButtonWidth()) / 1000 - 0.002f;
	sizerPrs.push_back(margin);
	resizeSizer();
}

void gGUIToolbar::addDropdownList(gGUIDropdownList *dropdownList, gGUIFrame *frame) {
	controlObjects.push_back(dropdownList);
	dropdownList->setParentFrame(frame);
	dropdownList->button.setButtonh(guisizer->getSlotHeight(0, 0));
	dropdownList->button.setButtonw(guisizer->getSlotHeight(0, 0));
	sizerPrs.push_back(0.15f);
	resizeSizer();
}

void gGUIToolbar::addCheckbox(gGUICheckbox *checkbox) {
	controlObjects.push_back(checkbox);
	checkbox->setSize(guisizer->getSlotHeight(0, 0), guisizer->getSlotHeight(0, 0));
	float margin = float(checkbox->titlew + checkbox->buttonw) / 1000;
	sizerPrs.push_back(margin);
	resizeSizer();
}

void gGUIToolbar::addNumberBox(gGUINumberBox *numberBox) {
	controlObjects.push_back(numberBox);
	numberBox->smalboxheight = guisizer->getSlotHeight(0, 0) / 2 - 2;
	//numberBox->boxh = guisizer->getSlotHeight(0, 0);
	float margin = float(numberBox->numboxwidth + numberBox->boxwidth + numberBox->smalboxwidth) / 1000;
	sizerPrs.push_back(0.1f);
	resizeSizer();
}

void gGUIToolbar::addSlider(gGUISlider *slider) {
	controlObjects.push_back(slider);
	slider->sliderh = guisizer->getSlotHeight(0, 0);
	float margin = float(slider->sliderbarw) / 1150;
	sizerPrs.push_back(margin);
	resizeSizer();
}

void gGUIToolbar::addSwitchButton(gGUISwitchButton *switchButton) {
	controlObjects.push_back(switchButton);
	switchButton->toggleh = guisizer->getSlotHeight(0, 0);
	float margin = float(switchButton->togglew) / 1000;
	sizerPrs.push_back(margin);
	resizeSizer();
	switchButton->togglew = guisizer->getSlotWidth(0, 0) * 2;
}

void gGUIToolbar::addText(gGUIText *text) {
	controlObjects.push_back(text);
	float margin = text->getText().length() * 0.008;
	sizerPrs.push_back(margin);
	resizeSizer();
}

void gGUIToolbar::addDivider(gGUIDivider *divider, float margin) {
	controlObjects.push_back(divider);
	sizerPrs.push_back(margin);
	resizeSizer();
}

void gGUIToolbar::resizeSizer() {
	if(!isSpaceAdded) spaceLocation++;
	int totalElements = controlObjects.size();
	float spaceLen = 0, totalLen = 0;
	float *toolbarSizerprs = new float[totalElements + 1];

	for (int i = 0; i < totalElements; ++i) {
		totalLen += sizerPrs.at(i);
	}

	spaceLen = 1.0f - totalLen;

	int j = 0;
	for (int i = 0; i <= totalElements; ++i) {
		if(i == spaceLocation) {
			toolbarSizerprs[i] = spaceLen;
			continue;
		}

		toolbarSizerprs[i] = sizerPrs.at(j++);
	}

	j = 0;
	if(toolbartype == TOOLBAR_HORIZONTAL) {
		guisizer->setSize(1, totalElements + 1);
		guisizer->setColumnProportions(toolbarSizerprs);

		for (int i = 0; i <= totalElements; ++i) {
			if(i == spaceLocation) continue;
			guisizer->setControl(0, i, controlObjects.at(j++));
		}
	} else {
		guisizer->setSize(totalElements + 1, 1);
		guisizer->setLineProportions(toolbarSizerprs);

		for (int i = 0; i < totalElements; ++i) {
			guisizer->setControl(i, 0, controlObjects.at(i));
		}
	}

	guisizer->enableBorders(true);
	delete[] toolbarSizerprs;
}

void gGUIToolbar::addSpace() {
	isSpaceAdded = true;
}
