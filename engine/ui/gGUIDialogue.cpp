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
	restoreevent = false;
	exitevent = false;

	minimizebuttonminimizetrigger = false;
	maximizebuttonmaximizetrigger = false;
	maximizebuttonrestoretrigger = false;
	exitbuttonexittrigger = false;

	ismaximized = false;
	isdragged = false;
	dragposx = 0;
	dragposy = 0;
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

	if (maximizebutton) {
		if (ismaximized) {
			if (maximizebuttonrestoretrigger && !maximizebutton->isPressed())  restoreevent = true; maximizebuttonrestoretrigger = false;
			if (maximizebutton->isPressed()) maximizebuttonrestoretrigger = true;
		}
		else {
			if (maximizebuttonmaximizetrigger && !maximizebutton->isPressed())  maximizeevent = true; maximizebuttonmaximizetrigger = false;
			if (maximizebutton->isPressed()) maximizebuttonmaximizetrigger = true;
		}
	}
}

void gGUIDialogue::draw() {
	if (guisizer) {

		gColor oldcolor = *renderer->getColor();

		// TITLE BAR BACKGROUND
		renderer->setColor(textbackgroundcolor);
		gDrawRectangle(left, top - titlebar->height, width, titlebar->height, true);
		// BUTTONS BAR BACKGROUND
		renderer->setColor(foregroundcolor);
		gDrawRectangle(left, top, width, height + buttonsbar->height, true);
		// DIALOGUE BORDERS
		renderer->setColor(fontcolor);
		gDrawLine(left, top - titlebar->height, right, top - titlebar->height);
		gDrawLine(left, bottom + buttonsbar->height, right, bottom + buttonsbar->height);
		gDrawLine(left, top - titlebar->height, left, bottom + buttonsbar->height);
		gDrawLine(right, top - titlebar->height, right, bottom + buttonsbar->height);

		renderer->setColor(&oldcolor);

		guisizer->draw();
	}

	if (titlebar) titlebar->draw();
	if (buttonsbar) buttonsbar->draw();
}

void gGUIDialogue::setTitleBar(gGUIContainer* titleBar) {
	this->titlebar = titleBar;
	titlebar->set(root, this, this, 0, 0, left, top - titlebar->height, titlebar->width, titlebar->height);
}

gGUIContainer* gGUIDialogue::getTitleBar() {
	return titlebar;
}

void gGUIDialogue::setButtonsBar(gGUIContainer* buttonsBar) {
	this->buttonsbar = buttonsBar;
	buttonsbar->set(root, this, this, 0, 0, left, top + height, buttonsbar->width, buttonsbar->height);
}

gGUIContainer* gGUIDialogue::getButtonsBar() {
	return buttonsbar;
}

void gGUIDialogue::resetTitleBar() {
	deftitlebar.width = width;
	deftitlebar.height = deftitlebarheight;

	deftitlebar.setSizer(&deftitlebarsizer);
	deftitlebarsizer.setSize(1, 5);
	deftitlebarsizer.enableBorders(false);
	float tbbitp = (float)deftitlebarbitmapwidth / (float)deftitlebar.width;
	float tbbutp = (float)deftitlebarbuttonwidth / (float)deftitlebar.width;
	float tbtxtp = 1 - (tbbitp + 3 * tbbutp);
	float tbcolproportions[5] = {tbbitp, tbtxtp, tbbutp, tbbutp, tbbutp};
	deftitlebarsizer.setColumnProportions(tbcolproportions);

	setTitleBar(&deftitlebar);

	deftitlebarsizer.setControl(0, 0, &deftitlebarbitmap);
	deftitlebarbitmap.loadImage("gameicon/icon.png", false);
	deftitlebarbitmap.height = deftitlebar.height * 0.7f;
	deftitlebarbitmap.width = deftitlebarbitmap.height;
	deftitlebarbitmap.top += (deftitlebar.height - deftitlebarbitmap.height) / 2;
	deftitlebarbitmap.left += (deftitlebar.width * tbbitp - deftitlebarbitmap.width) / 2;

	deftitlebarsizer.setControl(0, 1, &deftitlebartext);
	deftitlebartext.setText("Properties for GlistEngine");
	deftitlebartext.height = deftitlebar.height * 0.6f;
	deftitlebartext.top += (deftitlebar.height - deftitlebartext.height) / 2;

	deftitlebarsizer.setControl(0, 2, &deftitlebarminimizebutton);
	deftitlebarminimizebutton.setSize(deftitlebar.height, deftitlebar.height);
	deftitlebarminimizebutton.setButtonImageFromIcon(gGUIResources::ICON_MINIMIZEBLACK);
	deftitlebarminimizebutton.setPressedButtonImageFromIcon(gGUIResources::ICON_MINIMIZEBLACK);
	setMinimizeButton(&deftitlebarminimizebutton);

	deftitlebarsizer.setControl(0, 3, &deftitlebarmaximizebutton);
	deftitlebarmaximizebutton.setSize(deftitlebar.height, deftitlebar.height);
	if (ismaximized) {
		deftitlebarmaximizebutton.setButtonImageFromIcon(gGUIResources::ICON_RESTOREBLACK);
		deftitlebarmaximizebutton.setPressedButtonImageFromIcon(gGUIResources::ICON_RESTOREBLACK);
	}
	else {
		deftitlebarmaximizebutton.setButtonImageFromIcon(gGUIResources::ICON_MAXIMIZEBLACK);
		deftitlebarmaximizebutton.setPressedButtonImageFromIcon(gGUIResources::ICON_MAXIMIZEBLACK);
	}
	setMaximizeButton(&deftitlebarmaximizebutton);

	deftitlebarsizer.setControl(0, 4, &deftitlebarexitbutton);
	deftitlebarexitbutton.setSize(deftitlebar.height, deftitlebar.height);
	deftitlebarexitbutton.setButtonImageFromIcon(gGUIResources::ICON_EXITBLACK);
	deftitlebarexitbutton.setPressedButtonImageFromIcon(gGUIResources::ICON_EXITBLACK);
	setExitButton(&deftitlebarexitbutton);
}

void gGUIDialogue::resetButtonsBar() {
	defbuttonsbar.width = width;
	defbuttonsbar.height = defbuttonsbarheight;

	defbuttonsbar.setSizer(&defbuttonsbarsizer);
	defbuttonsbarsizer.setSize(1, 5);
	defbuttonsbarsizer.enableBorders(false);

	setButtonsBar(&defbuttonsbar);

	defbuttonsbarsizer.setControl(0, 4, &defbuttonsbarokbutton);
	defbuttonsbarokbutton.setTitle("OK");
	/* defbuttonsbarokbutton.setSize(defbuttonsbar.width * 0.14f, defbuttonsbar.height * 0.6f);
	defbuttonsbarokbutton.left += (defbuttonsbar.width * 0.2f - defbuttonsbar.width * 0.14f) / 2;
	defbuttonsbarokbutton.top += (defbuttonsbar.height - defbuttonsbar.height * 0.6f) / 2; */
	defbuttonsbarokbutton.setSize(defbuttonsbarbuttonwidth, defbuttonsbarbuttonheight);
	defbuttonsbarokbutton.left += (defbuttonsbar.width * 0.2f - defbuttonsbarbuttonwidth) / 2;
	defbuttonsbarokbutton.top += (defbuttonsbar.height - defbuttonsbarbuttonheight) / 2;
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

void gGUIDialogue::setRestoreEvent(bool restoreEvent) {
	this->restoreevent = restoreEvent;
}

bool gGUIDialogue::getRestoreEvent() {
	return restoreevent;
}

void gGUIDialogue::setExitEvent(bool exitEvent) {
	this->exitevent = exitEvent;
}

bool gGUIDialogue::getExitEvent() {
	return exitevent;
}

void gGUIDialogue::setIsMaximized(bool isMaximized) {
	this->ismaximized = isMaximized;
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
	if (!ismaximized && x > titlebar->left - titlebar->width && x < titlebar->left + titlebar->width && y >= titlebar->top - titlebar->height && y < titlebar->top + titlebar->height) {
		if ((minimizebutton || maximizebutton || exitbutton) && (minimizebutton->isPressed() || maximizebutton->isPressed() || exitbutton->isPressed())) {
			isdragged = false;
		}
		else {
			isdragged = true;
			dragposx = x;
			dragposy = y;
		}
	}
}

void gGUIDialogue::mouseDragged(int x, int y, int button) {
	if (x >= titlebar->left - titlebar->width && x < titlebar->left + titlebar->width && y >= titlebar->top - titlebar->height - guisizer->height && y < titlebar->top + titlebar->height + guisizer->height && isdragged) {
		int dx = x - dragposx;
		int dy = y - dragposy;

		left += dx;
		top += dy;
		right = left + width;
		bottom = top + height;

		titlebar->left += dx;
		titlebar->top += dy;
		titlebar->right = titlebar->left + titlebar->width;
		titlebar->bottom = titlebar->top + titlebar->height;

		guisizer->left += dx;
		guisizer->top += dy;
		guisizer->right = guisizer->left + guisizer->width;
		guisizer->bottom = guisizer->top + guisizer->height;

		buttonsbar->left += dx;
		buttonsbar->top += dy;
		buttonsbar->right = buttonsbar->left + buttonsbar->width;
		buttonsbar->bottom = buttonsbar->top + buttonsbar->height;

		for (int i = 0; i < titlebar->getSizer()->getLineNum(); i++) {
			for (int j = 0; j < titlebar->getSizer()->getColumnNum(); j++) {
				gGUIControl* guicontrol = titlebar->getSizer()->getControl(i, j);
				if (guicontrol != nullptr) {
					guicontrol->left += dx;
					guicontrol->top += dy;
					guicontrol->right = guicontrol->left + guicontrol->width;
					guicontrol->bottom = guicontrol->top + guicontrol->height;
				}
			}
		}

		for (int i = 0; i < guisizer->getLineNum(); i++) {
			for (int j = 0; j < guisizer->getColumnNum(); j++) {
				gGUIControl* guicontrol = guisizer->getControl(i, j);
				if (guicontrol != nullptr) {
					guicontrol->left += dx;
					guicontrol->top += dy;
					guicontrol->right = guicontrol->left + guicontrol->width;
					guicontrol->bottom = guicontrol->top + guicontrol->height;
				}
			}
		}

		for (int i = 0; i < buttonsbar->getSizer()->getLineNum(); i++) {
			for (int j = 0; j < buttonsbar->getSizer()->getColumnNum(); j++) {
				gGUIControl* guicontrol = buttonsbar->getSizer()->getControl(i, j);
				if (guicontrol != nullptr) {
					guicontrol->left += dx;
					guicontrol->top += dy;
					guicontrol->right = guicontrol->left + guicontrol->width;
					guicontrol->bottom = guicontrol->top + guicontrol->height;
				}
			}
		}

		dragposx += dx;
		dragposy += dy;
	}

}

void gGUIDialogue::mouseReleased(int x, int y, int button) {
	if (titlebar) titlebar->mouseReleased(x, y, button);
	if (guisizer) guisizer->mouseReleased(x, y, button);
	if (buttonsbar) buttonsbar->mouseReleased(x, y, button);
	if (isdragged) isdragged = false;
}
