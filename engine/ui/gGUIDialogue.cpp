/*
 * gGUIDialogue.cpp
 *
 *  Created on: 27 Jul 2022
 *      Author: Umut Can
 */

#include "gGUIDialogue.h"
#include "gGUISizer.h"


gGUIDialogue::gGUIDialogue() {
	guisizer = nullptr;

	titlebar = nullptr;
	buttonsbar = nullptr;

	title = "DIALOGUE BOX";
	message = "This is a dialogue box.";
	dialoguetype = DIALOGUETYPE_NONE;
	icontype = ICONTYPE_INFO;

	dialoguetypename[DIALOGUETYPE_NONE] = "none";
	dialoguetypename[DIALOGUETYPE_OK] = "ok";
	dialoguetypename[DIALOGUETYPE_OKCANCEL] = "okcancel";
	dialoguetypename[DIALOGUETYPE_YESNO] = "yesno";
	dialoguetypename[DIALOGUETYPE_YESNOCANCEL] = "yesnocancel";
	icontypename[ICONTYPE_INFO] = "info";
	icontypename[ICONTYPE_WARNING] = "warning";
	icontypename[ICONTYPE_ERROR] = "error";
	icontypename[ICONTYPE_QUESTION] = "question";

	minimizebutton = nullptr;
	maximizebutton = nullptr;
	exitbutton = nullptr;

	minimizeevent = false;
	maximizeevent = false;
	exitevent = false;

	exitbuttonexittrigger = false;
}

gGUIDialogue::~gGUIDialogue() {
}

void gGUIDialogue::update() {
	if (guisizer) guisizer->update();

	if (exitbutton) {
		if (exitbuttonexittrigger && !exitbutton->isPressed())  exitevent = true; exitbuttonexittrigger = false;
		if (exitbutton->isPressed()) exitbuttonexittrigger = true;
	}
}

void gGUIDialogue::draw() {
	if (guisizer) {

		gColor oldcolor = *renderer->getColor();
		renderer->setColor(textbackgroundcolor);
		gDrawRectangle(left, top - titlebar->height, width, titlebar->height, true);
		renderer->setColor(foregroundcolor);
		gDrawRectangle(left, top, width, height + buttonsbar->height, true);

		// DIALOGUE TITLE
		renderer->setColor(fontcolor);

		// DIALOGUE BORDERS
		gDrawLine(left, top - titlebar->height, right, top - titlebar->height);
		gDrawLine(left, bottom + buttonsbar->height, right, bottom + buttonsbar->height);
		gDrawLine(left, top - titlebar->height, left, bottom + buttonsbar->height);
		gDrawLine(right, top - titlebar->height, right, bottom + buttonsbar->height);
		// gDrawLine(left + width / 24, top + height / 8, right - width / 24, top + height / 8);
		// gDrawLine(left + width / 24, top + height * 3 / 4, right - width / 24, top + height * 3 / 4);

		renderer->setColor(&oldcolor);

		guisizer->draw();
	}

	if (titlebar) titlebar->draw();
	if (buttonsbar) buttonsbar->draw();
}

void gGUIDialogue::setMinimizeEvent(bool minimizeEvent) {
	this->minimizeevent = minimizeEvent;
}

bool gGUIDialogue::getMinimizeEvent() {
	return minimizeevent;
}

void gGUIDialogue::setMaximizeEvent(bool maximizeEvent) {
	this->maximizeevent = maximizeEvent;
}

bool gGUIDialogue::getMaximizeEvent() {
	return maximizeevent;
}

void gGUIDialogue::setExitEvent(bool exitEvent) {
	this->exitevent = exitEvent;
}

bool gGUIDialogue::getExitEvent() {
	return exitevent;
}

void gGUIDialogue::showDialogue(std::string title, std::string message, int dialogueType, int iconType) {

	this->title = title;
	this->message = message;
	this->dialoguetype = dialogueType;
	this->icontype = iconType;

	guisizer->enableBackgroundFill(false);

}

void gGUIDialogue::mouseDragged(int x, int y, int button) {
	if (x > left - width && x < left + width && y >= top - height && y < top + height / 8) {
		left = x;
		top = y;
		right = x + width;
		bottom = y + height;

		guisizer->left = x;
		guisizer->top = y;
		guisizer->right = x + guisizer->width;
		guisizer->bottom = y + guisizer->height;

		titlebar->left = x;
		titlebar->top = y;
		titlebar->right = x + titlebar->width;
		titlebar->bottom = y + titlebar->height;

		buttonsbar->left = x;
		buttonsbar->top = y;
		buttonsbar->right = x + buttonsbar->width;
		buttonsbar->bottom = y + buttonsbar->height;
	}
}

void gGUIDialogue::setTitleBar(gGUIContainer* titleBar) {
	this->titlebar = titleBar;
	titlebar->set(root, this, this, 0, 0, left, top - titlebar->height, width, titlebar->height);
}

void gGUIDialogue::setButtonsBar(gGUIContainer* buttonsBar) {
	this->buttonsbar = buttonsBar;
	buttonsbar->set(root, this, this, 0, 0, left, top + height, width, buttonsbar->height);
}

void gGUIDialogue::resetTitleBar() {
}

void gGUIDialogue::resetButtonsBar() {
}

void gGUIDialogue::setMinimizeButton(gGUIImageButton* minimizeButton) {
	this->minimizebutton = minimizeButton;
}

void gGUIDialogue::setMaximizeButton(gGUIImageButton* maximizeButton) {
	this->maximizebutton = maximizeButton;
}

void gGUIDialogue::setExitButton(gGUIImageButton* exitButton) {
	this->exitbutton = exitButton;
}

void gGUIDialogue::mouseMoved(int x, int y) {
	if(titlebar) titlebar->mouseMoved(x, y);
	if(guisizer) {
		if(x >= guisizer->left && x < guisizer->right && y >= guisizer->top && y < guisizer->bottom) {
			guisizer->iscursoron = true;
			guisizer->mouseMoved(x, y);
		}
	}
	if(buttonsbar) buttonsbar->mouseMoved(x, y);
}

void gGUIDialogue::mousePressed(int x, int y, int button) {
	if (titlebar) titlebar->mousePressed(x, y, button);
	if (guisizer) guisizer->mousePressed(x, y, button);
	if (buttonsbar) buttonsbar->mousePressed(x, y, button);
}

void gGUIDialogue::mouseReleased(int x, int y, int button) {
	if (titlebar) titlebar->mouseReleased(x, y, button);
	if (guisizer) guisizer->mouseReleased(x, y, button);
	if (buttonsbar) buttonsbar->mouseReleased(x, y, button);
}
