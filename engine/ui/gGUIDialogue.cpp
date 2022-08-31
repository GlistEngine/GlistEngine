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

	minimizebutton = nullptr;
	maximizebutton = nullptr;
	exitbutton = nullptr;

	minimizeevent = false;
	maximizeevent = false;
	exitevent = false;

	minimizebuttonminimizetrigger = false;
	maximizebuttonmaximizetrigger = false;
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

	if (minimizebutton) {
		if (minimizebuttonminimizetrigger && !minimizebutton->isPressed())  minimizeevent = true; minimizebuttonminimizetrigger = false;
		if (minimizebutton->isPressed()) minimizebuttonminimizetrigger = true;
	}
}

void gGUIDialogue::draw() {
	if (guisizer) {

		gColor oldcolor = *renderer->getColor();
		renderer->setColor(textbackgroundcolor);
		gDrawRectangle(left, top - titlebar->height, width, titlebar->height, true);
		renderer->setColor(foregroundcolor);
		gDrawRectangle(left, top, width, height + buttonsbar->height, true);

		// DIALOGUE BORDERS
		renderer->setColor(fontcolor);
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

void gGUIDialogue::setTitleBar(gGUIContainer* titleBar) {
	this->titlebar = titleBar;
	titlebar->set(root, this, this, 0, 0, left, top - titlebar->height, width, titlebar->height);
}

void gGUIDialogue::setButtonsBar(gGUIContainer* buttonsBar) {
	this->buttonsbar = buttonsBar;
	buttonsbar->set(root, this, this, 0, 0, left, top + height, width, buttonsbar->height);
}

void gGUIDialogue::resetTitleBar() {
	deftitlebar.width = width;
	deftitlebar.height = height * 0.06f;

	deftitlebar.setSizer(&deftitlebarsizer);
	deftitlebarsizer.setSize(1, 5);
	deftitlebarsizer.enableBorders(false);
	float tbcolproportions[5] = {0.04f, 0.81f, 0.05f, 0.05f, 0.05f};
	deftitlebarsizer.setColumnProportions(tbcolproportions);

	setTitleBar(&deftitlebar);

	deftitlebarsizer.setControl(0, 0, &deftitlebarbitmap);
	deftitlebarbitmap.loadImage("gameicon/icon.png", false);
	deftitlebarbitmap.height = deftitlebar.height * 0.7f;
	deftitlebarbitmap.width = deftitlebarbitmap.height;
	deftitlebarbitmap.top += (deftitlebar.height - deftitlebarbitmap.height) / 2;
	deftitlebarbitmap.left += (deftitlebar.width * 0.04 - deftitlebarbitmap.width) / 2;

	deftitlebarsizer.setControl(0, 1, &deftitlebartext);
	deftitlebartext.setText("Properties for GlistEngine");
	deftitlebartext.height = deftitlebar.height * 0.6f;
	deftitlebartext.top += (deftitlebar.height - deftitlebartext.height) / 2;

	deftitlebarsizer.setControl(0, 2, &deftitlebarminimizebutton);
	deftitlebarminimizebutton.setSize(deftitlebar.height, deftitlebar.height);
	// deftitlebarminimizebutton.loadButtonImages("dicons/nzwmin16.png");
	deftitlebarminimizebutton.setButtonImageFromIcon(gGUIResources::ICON_MINIMIZEBLACK);
	deftitlebarminimizebutton.setPressedButtonImageFromIcon(gGUIResources::ICON_MINIMIZEBLACK);
	setMinimizeButton(&deftitlebarminimizebutton);

	deftitlebarsizer.setControl(0, 3, &deftitlebarmaximizebutton);
	deftitlebarmaximizebutton.setSize(deftitlebar.height, deftitlebar.height);
	// deftitlebarmaximizebutton.loadButtonImages("dicons/nzwr16.png");
	deftitlebarmaximizebutton.setButtonImageFromIcon(gGUIResources::ICON_MAXIMIZEBLACK);
	deftitlebarmaximizebutton.setPressedButtonImageFromIcon(gGUIResources::ICON_MAXIMIZEBLACK);
	setMaximizeButton(&deftitlebarmaximizebutton);

	deftitlebarsizer.setControl(0, 4, &deftitlebarexitbutton);
	deftitlebarexitbutton.setSize(deftitlebar.height, deftitlebar.height);
	// deftitlebarexitbutton.loadButtonImages("dicons/nzwe16.png");
	deftitlebarexitbutton.setButtonImageFromIcon(gGUIResources::ICON_EXITBLACK);
	deftitlebarexitbutton.setPressedButtonImageFromIcon(gGUIResources::ICON_EXITBLACK);
	setExitButton(&deftitlebarexitbutton);
}

void gGUIDialogue::resetButtonsBar() {
	defbuttonsbar.width = width;
	defbuttonsbar.height = height * 0.1f;

	defbuttonsbar.setSizer(&defbuttonsbarsizer);
	defbuttonsbarsizer.setSize(1, 5);
	defbuttonsbarsizer.enableBorders(false);

	setButtonsBar(&defbuttonsbar);

	defbuttonsbarsizer.setControl(0, 4, &defbuttonsbarokbutton);
	defbuttonsbarokbutton.setTitle("OK");
	defbuttonsbarokbutton.setSize(defbuttonsbar.width * 0.14f, defbuttonsbar.height * 0.6f);
	defbuttonsbarokbutton.left += (defbuttonsbar.width * 0.2f - defbuttonsbar.width * 0.14f) / 2;
	defbuttonsbarokbutton.top += (defbuttonsbar.height - defbuttonsbar.height * 0.6f) / 2;
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

void gGUIDialogue::mouseDragged(int x, int y, int button) {
	/* if (x > left - width && x < left + width && y >= top - height && y < top + height / 8) {
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
	} */
}

void gGUIDialogue::mouseReleased(int x, int y, int button) {
	if (titlebar) titlebar->mouseReleased(x, y, button);
	if (guisizer) guisizer->mouseReleased(x, y, button);
	if (buttonsbar) buttonsbar->mouseReleased(x, y, button);
}
