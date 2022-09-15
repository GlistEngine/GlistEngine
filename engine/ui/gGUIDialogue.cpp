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

	isdragenabled = true;
	isresizeenabled = true;

	ismaximized = false;
	isdragged = false;
	isleftresized = false; isrightresized = false; istopresized = false; isbottomresized = false;
	dragposx = 0; dragposy = 0; sizeposx = 0; sizeposy = 0;

	initDefTitleBar();
	initDefButtonsBar();
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
	gColor oldcolor = *renderer->getColor();

	if (!(isleftresized || isrightresized || istopresized || isbottomresized)) {
		// TITLE BAR BACKGROUND
		renderer->setColor(textbackgroundcolor);
		gDrawRectangle(left, top - titlebar->height, width, titlebar->height, true);
		// BUTTONS BAR BACKGROUND
		renderer->setColor(foregroundcolor);
		gDrawRectangle(left, top, width, height + buttonsbar->height, true);

		if (guisizer) guisizer->draw();
		if (titlebar) titlebar->draw();
		if (buttonsbar) buttonsbar->draw();
	}

	// DIALOGUE BORDERS
	renderer->setColor(fontcolor);
	gDrawLine(left, top - titlebar->height, right, top - titlebar->height);
	gDrawLine(left, bottom + buttonsbar->height, right, bottom + buttonsbar->height);
	gDrawLine(left, top - titlebar->height, left, bottom + buttonsbar->height);
	gDrawLine(right, top - titlebar->height, right, bottom + buttonsbar->height);

	renderer->setColor(&oldcolor);
}

void gGUIDialogue::initDefTitleBar() {
	deftitlebar.setSizer(&deftitlebarsizer);
	deftitlebarsizer.setSize(1, 5);

	deftitlebarbitmap.loadImage("gameicon/icon.png", false);
	deftitlebarbitmap.width = deftitlebarbitmapwidth;
	deftitlebarbitmap.height = deftitlebarbitmapwidth;

	deftitlebartext.setText("Properties for GlistEngine");

	deftitlebarminimizebutton.setButtonImageFromIcon(gGUIResources::ICON_MINIMIZEBLACK);
	deftitlebarminimizebutton.setPressedButtonImageFromIcon(gGUIResources::ICON_MINIMIZEBLACK);

	deftitlebarmaximizebutton.setButtonImageFromIcon(gGUIResources::ICON_RESTOREBLACK);
	deftitlebarmaximizebutton.setPressedButtonImageFromIcon(gGUIResources::ICON_RESTOREBLACK);

	deftitlebarexitbutton.setButtonImageFromIcon(gGUIResources::ICON_EXITBLACK);
	deftitlebarexitbutton.setPressedButtonImageFromIcon(gGUIResources::ICON_EXITBLACK);

	setMinimizeButton(&deftitlebarminimizebutton);
	setMaximizeButton(&deftitlebarmaximizebutton);
	setExitButton(&deftitlebarexitbutton);
}

void gGUIDialogue::initDefButtonsBar() {
	defbuttonsbar.setSizer(&defbuttonsbarsizer);
	defbuttonsbarsizer.setSize(1, 2);

	defbuttonsbarokbutton.setTitle("OK");
}

void gGUIDialogue::setTitleBar(gGUIContainer* titleBar) {
	this->titlebar = titleBar;
	titlebar->width = width;
	if (titlebar->height == 0) titlebar->height = deftitlebarheight;
	titlebar->set(root, this, this, 0, 0, left, top - titlebar->height, titlebar->width, titlebar->height);
}

gGUIContainer* gGUIDialogue::getTitleBar() {
	return titlebar;
}

void gGUIDialogue::setButtonsBar(gGUIContainer* buttonsBar) {
	this->buttonsbar = buttonsBar;
	buttonsbar->width = width;
	if (buttonsbar->height == 0) buttonsbar->height = defbuttonsbarheight;
	buttonsbar->set(root, this, this, 0, 0, left, top + height, buttonsbar->width, buttonsbar->height);
}

gGUIContainer* gGUIDialogue::getButtonsBar() {
	return buttonsbar;
}

void gGUIDialogue::resetTitleBar() {
	setTitleBar(&deftitlebar);

	float tbbitp = ((float)deftitlebarbitmapwidth + 10) / (float)deftitlebar.width;
	float tbbutp = (float)deftitlebarbuttonwidth / (float)deftitlebar.width;
	float tbtxtp = 1 - (tbbitp + 3 * tbbutp);
	float tbcolproportions[5] = {tbbitp, tbtxtp, tbbutp, tbbutp, tbbutp};
	deftitlebarsizer.setColumnProportions(tbcolproportions);

	deftitlebarsizer.setControl(0, 0, &deftitlebarbitmap);
	deftitlebarbitmap.top += (deftitlebar.height - deftitlebarbitmap.height) / 2;
	deftitlebarbitmap.left += (deftitlebar.width * tbbitp - deftitlebarbitmap.width) / 2;

	deftitlebarsizer.setControl(0, 1, &deftitlebartext);
	deftitlebartext.height = deftitlebarbitmapwidth / 1.5f;
	deftitlebartext.top += (deftitlebar.height - deftitlebartext.height) / 2;

	deftitlebarsizer.setControl(0, 2, &deftitlebarminimizebutton);
	deftitlebarminimizebutton.setSize(deftitlebar.height, deftitlebar.height);

	if (ismaximized) {
		deftitlebarmaximizebutton.setButtonImageFromIcon(gGUIResources::ICON_RESTOREBLACK);
		deftitlebarmaximizebutton.setPressedButtonImageFromIcon(gGUIResources::ICON_RESTOREBLACK);
	}
	else {
		deftitlebarmaximizebutton.setButtonImageFromIcon(gGUIResources::ICON_MAXIMIZEBLACK);
		deftitlebarmaximizebutton.setPressedButtonImageFromIcon(gGUIResources::ICON_MAXIMIZEBLACK);
	}

	deftitlebarsizer.setControl(0, 3, &deftitlebarmaximizebutton);
	deftitlebarmaximizebutton.setSize(deftitlebar.height, deftitlebar.height);

	deftitlebarsizer.setControl(0, 4, &deftitlebarexitbutton);
	deftitlebarexitbutton.setSize(deftitlebar.height, deftitlebar.height);
}

void gGUIDialogue::resetButtonsBar() {
	setButtonsBar(&defbuttonsbar);

	float bbbutp = ((float)defbuttonsbarbuttonwidth + 30) / (float)defbuttonsbar.width;
	float bbempp = 1 - bbbutp;
	float bbcolproportions[2] = {bbempp, bbbutp};
	defbuttonsbarsizer.setColumnProportions(bbcolproportions);

	defbuttonsbarsizer.setControl(0, 1, &defbuttonsbarokbutton);
	defbuttonsbarokbutton.setSize(defbuttonsbarbuttonwidth, defbuttonsbarbuttonheight);
	defbuttonsbarokbutton.left += (defbuttonsbar.width * bbbutp - defbuttonsbarbuttonwidth) / 2;
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

void gGUIDialogue::enableDrag(bool isDragEnabled) {
	this->isdragenabled = isDragEnabled;
}

void gGUIDialogue::enableResize(bool isResizeEnabled) {
	this->isresizeenabled = isResizeEnabled;
}

void gGUIDialogue::setIsMaximized(bool isMaximized) {
	this->ismaximized = isMaximized;
}

int gGUIDialogue::getCursor(int x, int y) {
	if (!ismaximized) {
		if ((x > left - 5 && x < left + 5) || (x > right - 5 && x < right + 5)) return CURSOR_HRESIZE;
		if ((y > titlebar->top - 5 && y < titlebar->top + 5) || (y > buttonsbar->bottom - 5 && y < buttonsbar->bottom + 5)) return CURSOR_VRESIZE;
	}
	return CURSOR_ARROW;
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

	if (!ismaximized && x > titlebar->left + 5 && x < titlebar->left + titlebar->width - 5 && y > titlebar->top + 5 && y < titlebar->top + titlebar->height) {
		if ((minimizebutton || maximizebutton || exitbutton) && (minimizebutton->isPressed() || maximizebutton->isPressed() || exitbutton->isPressed())) {
			isdragged = false;
		}
		else {
			isdragged = true;
			dragposx = x; dragposy = y;
		}
	}

	if (!ismaximized) {
		if (x > left - 5 && x < left + 5) isleftresized = true; sizeposx = x;
		if (x > right - 5 && x < right + 5) isrightresized = true; sizeposx = x;
		if (y > titlebar->top - 5 && y < titlebar->top + 5) istopresized = true; sizeposy = y;
		if (y > buttonsbar->bottom - 5 && y < buttonsbar->bottom + 5) isbottomresized = true; sizeposy = y;
	}
}

void gGUIDialogue::mouseDragged(int x, int y, int button) {
	int dx = x - dragposx; int dy = y - dragposy;
	int sx = x - sizeposx; int sy = y - sizeposy;

	if ((isrightresized && sx < 0 && width < 400) || (isleftresized && sx > 0 && width < 400)) sx = 0;
	if ((isbottomresized && sy < 0 && height < 100) || (istopresized && sy > 0 && height < 100)) sy = 0;

	if (isdragged && x >= titlebar->left - titlebar->width && x < titlebar->left + titlebar->width && y >= titlebar->top - titlebar->height - guisizer->height && y < titlebar->top + titlebar->height + guisizer->height) {
		left += dx;
		top += dy;
		right = left + width;
		bottom = top + height;

		titlebar->left += dx;
		titlebar->top += dy;
		titlebar->right = titlebar->left + titlebar->width;
		titlebar->bottom = top;

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
	}

	if (isleftresized) {
		width -= sx; left += sx; right = left + width;
		titlebar->width -= sx; titlebar->left += sx; titlebar->right = titlebar->left + titlebar->width;
		buttonsbar->width -= sx; buttonsbar->left += sx; buttonsbar->right = buttonsbar->left + buttonsbar->width;
	}

	if (isrightresized) {
		width += sx; right += sx; left = right - width;
		titlebar->width += sx; titlebar->right += sx; titlebar->left = titlebar->right - titlebar->width;
		buttonsbar->width += sx; buttonsbar->right += sx; buttonsbar->left = buttonsbar->right - buttonsbar->width;
	}

	if (istopresized) {
		height -= sy; top += sy; bottom = top + height;
		titlebar->top += sy; titlebar->bottom = titlebar->top + titlebar->height;
		buttonsbar->top += sy; buttonsbar->bottom = buttonsbar->top + buttonsbar->height;
	}

	if (isbottomresized) {
		height += sy; bottom += sy; top = bottom - height;
		titlebar->bottom += sy; titlebar->top = titlebar->bottom - titlebar->height;
		buttonsbar->bottom += sy; buttonsbar->top = buttonsbar->bottom - buttonsbar->height;
	}

	guisizer->width = width; guisizer->height = height; guisizer->left = left; guisizer->right = right; guisizer->top = top; guisizer->bottom = bottom;

	dragposx += dx; dragposy += dy; sizeposx += sx; sizeposy += sy;
}

void gGUIDialogue::mouseReleased(int x, int y, int button) {
	if (titlebar) titlebar->mouseReleased(x, y, button);
	if (guisizer) guisizer->mouseReleased(x, y, button);
	if (buttonsbar) buttonsbar->mouseReleased(x, y, button);
	if (isdragged) isdragged = false;
	if (isleftresized || isrightresized || istopresized || isbottomresized) {
		isleftresized = false; isrightresized = false; istopresized = false; isbottomresized = false; resetTitleBar(); resetButtonsBar();
	}
}
