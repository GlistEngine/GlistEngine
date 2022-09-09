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
	isleftresized = false;
	isrightresized = false;
	istopresized = false;
	isbottomresized = false;
	dragposx = 0; dragposy = 0;
	sizeposx = 0; sizeposy = 0;
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

	float tbbitp = ((float)deftitlebarbitmapwidth + 10) / (float)deftitlebar.width;
	float tbbutp = (float)deftitlebarbuttonwidth / (float)deftitlebar.width;
	float tbtxtp = 1 - (tbbitp + 3 * tbbutp);
	float tbcolproportions[5] = {tbbitp, tbtxtp, tbbutp, tbbutp, tbbutp};
	deftitlebarsizer.setColumnProportions(tbcolproportions);

	setTitleBar(&deftitlebar);

	deftitlebarsizer.setControl(0, 0, &deftitlebarbitmap);
	deftitlebarbitmap.loadImage("gameicon/icon.png", false);
	deftitlebarbitmap.height = deftitlebarbitmapwidth;
	deftitlebarbitmap.width = deftitlebarbitmapwidth;
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
	defbuttonsbarsizer.setSize(1, 2);
	defbuttonsbarsizer.enableBorders(false);

	float bbbutp = ((float)defbuttonsbarbuttonwidth + 30) / (float)defbuttonsbar.width;
	float bbempp = 1 - bbbutp;
	float bbcolproportions[2] = {bbempp, bbbutp};
	defbuttonsbarsizer.setColumnProportions(bbcolproportions);

	setButtonsBar(&defbuttonsbar);

	defbuttonsbarsizer.setControl(0, 1, &defbuttonsbarokbutton);
	defbuttonsbarokbutton.setTitle("OK");
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
	if ((x > left - 5 && x < left + 5) || (x > right - 5 && x < right + 5)) return CURSOR_HRESIZE;
	if ((y > titlebar->top - 5 && y < titlebar->top + 5) || (y > buttonsbar->bottom - 5 && y < buttonsbar->bottom + 5)) return CURSOR_VRESIZE;
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

	if (!ismaximized && x > titlebar->left && x < titlebar->left + titlebar->width && y > titlebar->top + 5 && y < titlebar->top + titlebar->height) {
		if ((minimizebutton || maximizebutton || exitbutton) && (minimizebutton->isPressed() || maximizebutton->isPressed() || exitbutton->isPressed())) {
			isdragged = false;
		}
		else {
			isdragged = true;
			dragposx = x;
			dragposy = y;
		}
	}

	if (x > left - 5 && x < left + 5) isleftresized = true; sizeposx = x;
	if (x > right - 5 && x < right + 5) isrightresized = true; sizeposx = x;
	if (y > titlebar->top - 5 && y < titlebar->top + 5) istopresized = true; sizeposy = y;
	if (y > buttonsbar->bottom - 5 && y < buttonsbar->bottom + 5) isbottomresized = true; sizeposy = y;
}

void gGUIDialogue::mouseDragged(int x, int y, int button) {
	if (isdragged && x >= titlebar->left - titlebar->width && x < titlebar->left + titlebar->width && y >= titlebar->top - titlebar->height - guisizer->height && y < titlebar->top + titlebar->height + guisizer->height) {
		int dx = x - dragposx;
		int dy = y - dragposy;

		left += dx;
		top += dy;
		right = left + width;
		bottom = top + height;

		guisizer->left = left;
		guisizer->top = top;
		guisizer->right = right;
		guisizer->bottom = bottom;

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

		dragposx += dx;
		dragposy += dy;
	}

	/* if (isleftresized || isrightresized || istopresized || isbottomresized) {
		int dx, dy;
		if (isleftresized || isrightresized) dx = x - sizeposx;
		if (istopresized || isbottomresized) dy = y - sizeposy;

		if (isleftresized) width -= dx; left = x; right = left + width;
		if (isrightresized) width += dx; right = x; left = right - width;
		if (istopresized) height -= dy; top = y + titlebar->height; bottom = top + height;
		if (isbottomresized) height += dy; bottom = y - buttonsbar->height; top = bottom - height;

		guisizer->width = width;
		guisizer->height = height;
		guisizer->left = left;
		guisizer->right = right;
		guisizer->top = top;
		guisizer->bottom = bottom;

		if (isleftresized) titlebar->width -= dx; titlebar->left = x; titlebar->right = left + width;
		if (isrightresized) titlebar->width += dx; titlebar->right = x; titlebar->left = titlebar->right - titlebar->width;
		if (istopresized) titlebar->top = y; titlebar->bottom = titlebar->top + titlebar->height;
		if (isbottomresized) titlebar->bottom = y - buttonsbar->height - height; titlebar->top = titlebar->bottom - titlebar->height;

		if (isleftresized) buttonsbar->width -= dx; buttonsbar->left = x; buttonsbar->right = buttonsbar->left + buttonsbar->width;
		if (isrightresized) buttonsbar->width += dx; buttonsbar->right = x; buttonsbar->left = buttonsbar->right - buttonsbar->width;
		if (istopresized) buttonsbar->top = y + titlebar->height + height; buttonsbar->bottom = buttonsbar->top + buttonsbar->height;
		if (isbottomresized) buttonsbar->bottom = y; buttonsbar->top = buttonsbar->bottom - buttonsbar->height;

		if (isleftresized || isrightresized) sizeposx += dx;
		if (istopresized || isbottomresized) sizeposy += dy;
	} */

	if (isleftresized) {
		int sx = x - sizeposx;
		width -= sx; left = x; right = left + width;
		titlebar->width -= sx; titlebar->left = x; titlebar->right = titlebar->left + titlebar->width;
		guisizer->width = width; guisizer->height = height; guisizer->left = left; guisizer->right = right; guisizer->top = top; guisizer->bottom = bottom;
		buttonsbar->width -= sx; buttonsbar->left = x; buttonsbar->right = buttonsbar->left + buttonsbar->width;
		sizeposx += sx;
	}

	if (isrightresized) {
		int sx = x - sizeposx;
		width += sx; right = x; left = right - width;
		titlebar->width += sx; titlebar->right = x; titlebar->left = titlebar->right - titlebar->width;
		guisizer->width = width; guisizer->height = height; guisizer->left = left; guisizer->right = right; guisizer->top = top; guisizer->bottom = bottom;
		buttonsbar->width += sx; buttonsbar->right = x; buttonsbar->left = buttonsbar->right - buttonsbar->width;
		sizeposx += sx;
	}

	if (istopresized) {
		int sy = y - sizeposy;
		height -= sy; top = y + titlebar->height; bottom = top + height;
		titlebar->top = y; titlebar->bottom = titlebar->top + titlebar->height;
		guisizer->width = width; guisizer->height = height; guisizer->left = left; guisizer->right = right; guisizer->top = top; guisizer->bottom = bottom;
		buttonsbar->top = y + titlebar->height + height; buttonsbar->bottom = buttonsbar->top + buttonsbar->height;
		sizeposy += sy;
	}

	if (isbottomresized) {
		int sy = y - sizeposy;
		height += sy; bottom = y - buttonsbar->height; top = bottom - height;
		titlebar->bottom = y - buttonsbar->height - height; titlebar->top = titlebar->bottom - titlebar->height;
		guisizer->width = width; guisizer->height = height; guisizer->left = left; guisizer->right = right; guisizer->top = top; guisizer->bottom = bottom;
		buttonsbar->bottom = y; buttonsbar->top = buttonsbar->bottom - buttonsbar->height;
		sizeposy += sy;
	}

}

void gGUIDialogue::mouseReleased(int x, int y, int button) {
	if (titlebar) titlebar->mouseReleased(x, y, button);
	if (guisizer) guisizer->mouseReleased(x, y, button);
	if (buttonsbar) buttonsbar->mouseReleased(x, y, button);
	if (isdragged) isdragged = false;
	if (isleftresized) isleftresized = false; resetTitleBar(); resetButtonsBar();
	if (isrightresized) isrightresized = false; resetTitleBar(); resetButtonsBar();
	if (istopresized) istopresized = false; resetTitleBar(); resetButtonsBar();
	if (isbottomresized) isbottomresized = false; resetTitleBar(); resetButtonsBar();
}
