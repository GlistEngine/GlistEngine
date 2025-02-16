/*
 * gGUIDialogue.cpp
 *
 *  Created on: 27 Jul 2022
 *      Author: Umut Can
 */

#include "gGUIDialogue.h"
#include "gGUISizer.h"
#include "gGUIMenubar.h"
#include "gGUIStatusBar.h"
#include "gGUIContextMenu.h"
#include "gGUITreelist.h"
#include "gBaseApp.h"


gGUIDialogue::gGUIDialogue() {
	isdialogueshown = false;

	guisizer = new gGUISizer();
	menubar = nullptr;
	statusbar = nullptr;
	contextmenu = nullptr;
	treelist = nullptr;

	buttontrigger = EVENT_NONE; buttonevent = EVENT_NONE;

	isdragenabled = true; isresizeenabled = true;
	ismaximized = false; isdragged = false;
	isiconenabled = false;
	icontypeid = ICONTYPE_NONE;

	dragposx = 0; dragposy = 0; sizeposx = 0; sizeposy = 0;

	dialoguetype = DIALOGUETYPE_OK;
	titletype = TITLETYPE_EXITMINMAX;

	resizeposition = RESIZE_NONE;

	initTitleBar();
	initButtonsBar();
	initDefMessageBar();
}

gGUIDialogue::~gGUIDialogue() {
	if(guisizer) delete guisizer;
}

void gGUIDialogue::setSizer(gGUISizer* guiSizer) {
	if(guisizer) delete guisizer;
	gGUIForm::setSizer(guiSizer);
}

void gGUIDialogue::update() {
	if(guisizer) guisizer->update();
	messagebar.update();
}

void gGUIDialogue::draw() {
	gColor oldcolor = *renderer->getColor();

	if(resizeposition == RESIZE_NONE) {
		// TITLE BAR BACKGROUND
		renderer->setColor(textbackgroundcolor);
		gDrawRectangle(left, top - titlebar.height, width, titlebar.height, true);
		// BUTTONS BAR BACKGROUND
		renderer->setColor(foregroundcolor);
		gDrawRectangle(left, top, width, height + buttonsbar.height, true);

		if(guisizer) guisizer->draw();
		titlebar.draw();
		buttonsbar.draw();
		messagebar.draw();
	}

	// DIALOGUE BORDERS
	renderer->setColor(fontcolor);
	gDrawLine(left, top - titlebar.height, right, top - titlebar.height);
	gDrawLine(left, bottom + buttonsbar.height, right, bottom + buttonsbar.height);
	gDrawLine(left, top - titlebar.height, left, bottom + buttonsbar.height);
	gDrawLine(right, top - titlebar.height, right, bottom + buttonsbar.height);

	renderer->setColor(&oldcolor);

	if(menubar) menubar->draw();
	if(statusbar) statusbar->draw();
	if(contextmenu) contextmenu->draw();
	if(treelist) treelist->draw();
}

bool gGUIDialogue::show() {
	if(isdialogueshown) return false;
	if(root->getAppManager()->getGUIManager()->showDialogue(this)) isdialogueshown = true;
	else isdialogueshown = false;
	return isdialogueshown;
}

bool gGUIDialogue::hide() {
	if(!isdialogueshown) return true;
	if(root->getAppManager()->getGUIManager()->hideDialogue(this)) isdialogueshown = false;
	return !isdialogueshown;
}

bool gGUIDialogue::isShown() {
	return isdialogueshown;
}

void gGUIDialogue::initTitleBar() {
	titlebar.setSizer(&titlebarsizer);
	titlebarsizer.setSize(1, 5);

	titlebarbitmap.loadImage("appicon/icon.png", false);
	titlebarbitmap.width = titlebarbitmapw;
	titlebarbitmap.height = titlebarbitmapw;

	titlebartext.setText("Properties for GlistEngine");
	titlebartext.setTextVerticalAlignment(gGUIText::TEXTVERTICALALIGNMENT_TOP);

	minimizebutton.setButtonImageFromIcon(gGUIResources::ICON_MINIMIZEBLACK);
	minimizebutton.setPressedButtonImageFromIcon(gGUIResources::ICON_MINIMIZEBLACK);

	maximizebutton.setButtonImageFromIcon(gGUIResources::ICON_MAXIMIZEBLACK);
	maximizebutton.setPressedButtonImageFromIcon(gGUIResources::ICON_MAXIMIZEBLACK);

	exitbutton.setButtonImageFromIcon(gGUIResources::ICON_EXITBLACK);
	exitbutton.setPressedButtonImageFromIcon(gGUIResources::ICON_EXITBLACK);
}

void gGUIDialogue::initButtonsBar() {
	dialoguetype = DIALOGUETYPE_OK;
	buttonsbar.setSizer(&buttonsbarsizer);

	buttonsbarokbutton.setTitle("OK");
	buttonsbaryesbutton.setTitle("Yes");
	buttonsbarnobutton.setTitle("No");
	buttonsbarcancelbutton.setTitle("Cancel");
}

void gGUIDialogue::initDefMessageBar() {
	messagebar.setSizer(&defmessagebarsizer);
	defmessagebarsizer.setSize(1, 4);
//	defmessagebarsizer.enableBorders(true);
//	defmessagebarsizer.setForegroundColor(backgroundcolor);

	defmessagetext.setText("This is a placeholder text.");
	defdialogueicon.setDisabled(true);

	defmessagebartopspace = 25;
	defmessagebarrightspace = 15;
}

gGUIContainer* gGUIDialogue::getTitleBar() {
	return &titlebar;
}

gGUIContainer* gGUIDialogue::getButtonsBar() {
	return &buttonsbar;
}

gGUIContainer* gGUIDialogue::getMessageBar() {
	return &messagebar;
}

void gGUIDialogue::resetTitleBar() {
	titlebar.width = width;
	if(titlebar.height == 0) titlebar.height = titlebarh;
	titlebar.set(root, this, this, 0, 0, left, top - titlebar.height, titlebar.width, titlebar.height);

	float tbbitp = ((float)titlebarbitmapw + 15) / (float)titlebar.width;
	float tbbutp = (float)titlebarbuttonw / (float)titlebar.width;
	float tbtxtp = 1 - (tbbitp + 3 * tbbutp);
	// 5th one is for the exit button
	float tbcolproportions[5] = {tbtxtp, tbbutp, tbbutp, tbbutp, 0};
	titlebarsizer.setColumnProportions(tbcolproportions);

	titlebarbitmap.top += (titlebar.height - titlebarbitmap.height) / 2;
	titlebarbitmap.left += (titlebar.width * tbbitp - titlebarbitmap.width) / 2;

	titlebarsizer.setControl(0, 0, &titlebartext);
	titlebartext.height = titlebarbitmapw / 1.5f;
	titlebartext.top += (titlebar.height - titlebartext.height) / 2;

	if(titletype == TITLETYPE_EXITMINMAX || titletype == TITLETYPE_EXITMIN) {
		titlebarsizer.setControl(0, 2, &minimizebutton);
		minimizebutton.setSize(titlebar.height, titlebar.height);
	} else {
		titlebarsizer.removeControl(0, 2);
	}

	if(ismaximized) {
		maximizebutton.setButtonImageFromIcon(gGUIResources::ICON_RESTOREBLACK);
		maximizebutton.setPressedButtonImageFromIcon(gGUIResources::ICON_RESTOREBLACK);
	} else {
		maximizebutton.setButtonImageFromIcon(gGUIResources::ICON_MAXIMIZEBLACK);
		maximizebutton.setPressedButtonImageFromIcon(gGUIResources::ICON_MAXIMIZEBLACK);
	}

	if(titletype == TITLETYPE_EXITMINMAX || titletype == TITLETYPE_EXITMAX) {
		titlebarsizer.setControl(0, 3, &maximizebutton);
		maximizebutton.setSize(titlebar.height, titlebar.height);
	} else {
		titlebarsizer.removeControl(0, 3);
	}

	if(titletype != TITLETYPE_NONE) {
		titlebarsizer.setControl(0, 4, &exitbutton);
		exitbutton.setSize(titlebar.height, titlebar.height);
	} else {
		titlebarsizer.removeControl(0, 4);
	}
}

void gGUIDialogue::resetButtonsBar() {
	buttonsbar.width = width;
	if(buttonsbar.height == 0) buttonsbar.height = buttonsbarh;
	buttonsbar.set(root, this, this, 0, 0, left, top + height, buttonsbar.width, buttonsbar.height);

	if(dialoguetype == DIALOGUETYPE_OK) buttonsbarsizer.setSize(1, 2);
	if(dialoguetype == DIALOGUETYPE_YESNOCANCEL) buttonsbarsizer.setSize(1, 4);
	if(dialoguetype == DIALOGUETYPE_OKCANCEL || dialoguetype == DIALOGUETYPE_YESNO) buttonsbarsizer.setSize(1, 3);

	if(dialoguetype == DIALOGUETYPE_OK){
		float bbbutp = ((float)buttonsbarbuttonw + 10) / (float)buttonsbar.width;
		float bbempp = 1 - bbbutp;
		float bbcolproportions[2] = {bbempp, bbbutp};
		buttonsbarsizer.setColumnProportions(bbcolproportions);
		// OK BUTTON
		buttonsbarsizer.setControl(0, 1, &buttonsbarokbutton);
		buttonsbarokbutton.setSize(buttonsbarbuttonw, buttonsbarbuttonh);
		buttonsbarokbutton.left += (buttonsbar.width * bbbutp - buttonsbarbuttonw) / 2 - 5;
		buttonsbarokbutton.top += (buttonsbar.height - buttonsbarbuttonh) / 2;
	}

	if(dialoguetype == DIALOGUETYPE_YESNOCANCEL){
		float bbbutp = ((float)buttonsbarbuttonw + 10) / (float)buttonsbar.width;
		float bbempp = 1 - bbbutp * 3;
		float bbcolproportions[4] = {bbempp, bbbutp, bbbutp, bbbutp};
		buttonsbarsizer.setColumnProportions(bbcolproportions);
		// YES BUTTON
		buttonsbarsizer.setControl(0, 1, &buttonsbaryesbutton);
		buttonsbaryesbutton.setSize(buttonsbarbuttonw, buttonsbarbuttonh);
		buttonsbaryesbutton.left += (buttonsbar.width * bbbutp - buttonsbarbuttonw) / 2 + 5;
		buttonsbaryesbutton.top += (buttonsbar.height - buttonsbarbuttonh) / 2;
		// NO BUTTON
		buttonsbarsizer.setControl(0, 2, &buttonsbarnobutton);
		buttonsbarnobutton.setSize(buttonsbarbuttonw, buttonsbarbuttonh);
		buttonsbarnobutton.left += (buttonsbar.width * bbbutp - buttonsbarbuttonw) / 2;
		buttonsbarnobutton.top += (buttonsbar.height - buttonsbarbuttonh) / 2;
		// CANCEL BUTTON
		buttonsbarsizer.setControl(0, 3, &buttonsbarcancelbutton);
		buttonsbarcancelbutton.setSize(buttonsbarbuttonw, buttonsbarbuttonh);
		buttonsbarcancelbutton.left += (buttonsbar.width * bbbutp - buttonsbarbuttonw) / 2 - 5;
		buttonsbarcancelbutton.top += (buttonsbar.height - buttonsbarbuttonh) / 2;
	}

	if(dialoguetype == DIALOGUETYPE_OKCANCEL){
		float bbbutp = ((float)buttonsbarbuttonw + 10) / (float)buttonsbar.width;
		float bbempp = 1 - bbbutp * 2;
		float bbcolproportions[4] = {bbempp, bbbutp, bbbutp, bbbutp};
		buttonsbarsizer.setColumnProportions(bbcolproportions);
		// OK BUTTON
		buttonsbarsizer.setControl(0, 1, &buttonsbarokbutton);
		buttonsbarokbutton.setSize(buttonsbarbuttonw, buttonsbarbuttonh);
		buttonsbarokbutton.left += (buttonsbar.width * bbbutp - buttonsbarbuttonw) / 2;
		buttonsbarokbutton.top += (buttonsbar.height - buttonsbarbuttonh) / 2;
		// CANCEL BUTTON
		buttonsbarsizer.setControl(0, 2, &buttonsbarcancelbutton);
		buttonsbarcancelbutton.setSize(buttonsbarbuttonw, buttonsbarbuttonh);
		buttonsbarcancelbutton.left += (buttonsbar.width * bbbutp - buttonsbarbuttonw) / 2 - 5;
		buttonsbarcancelbutton.top += (buttonsbar.height - buttonsbarbuttonh) / 2;
	}

	if(dialoguetype == DIALOGUETYPE_YESNO){
		float bbbutp = ((float)buttonsbarbuttonw + 10) / (float)buttonsbar.width;
		float bbempp = 1 - bbbutp * 2;
		float bbcolproportions[4] = {bbempp, bbbutp, bbbutp, bbbutp};
		buttonsbarsizer.setColumnProportions(bbcolproportions);
		// YES BUTTON
		buttonsbarsizer.setControl(0, 1, &buttonsbaryesbutton);
		buttonsbaryesbutton.setSize(buttonsbarbuttonw, buttonsbarbuttonh);
		buttonsbaryesbutton.left += (buttonsbar.width * bbbutp - buttonsbarbuttonw) / 2;
		buttonsbaryesbutton.top += (buttonsbar.height - buttonsbarbuttonh) / 2;
		// NO BUTTON
		buttonsbarsizer.setControl(0, 2, &buttonsbarnobutton);
		buttonsbarnobutton.setSize(buttonsbarbuttonw, buttonsbarbuttonh);
		buttonsbarnobutton.left += (buttonsbar.width * bbbutp - buttonsbarbuttonw) / 2 - 5;
		buttonsbarnobutton.top += (buttonsbar.height - buttonsbarbuttonh) / 2;
	}
}

void gGUIDialogue::resetMessageBar() {
	messagebar.width = width;
	messagebar.height = height;
	messagebar.set(root, this, this, 0, 0, left, top + defmessagebartopspace, messagebar.width - defmessagebarrightspace, messagebar.height - defmessagebartopspace * 2);

	float mbspace = 0.05f;
	float mbdtp = 0.07f;
	float mbsmgp = 0.9f;
	if(isiconenabled){
		float mbcolproportions[4] = {mbspace, mbdtp, mbspace, mbsmgp};
		defmessagebarsizer.setColumnProportions(mbcolproportions);
	}
	else{
		float mbcolproportions[4] = {mbspace, 0.0f, 0.0f, mbdtp};
		defmessagebarsizer.setColumnProportions(mbcolproportions);
	}

	defmessagebarsizer.setControl(0, 3, &defmessagetext);
	if(isiconenabled) defmessagebarsizer.setControl(0, 1, &defdialogueicon);
	defdialogueicon.setSize(titlebar.height, titlebar.height);
	defdialogueicon.top += (messagebar.height - defdialogueicon.width) / 2;
}

void gGUIDialogue::setTitle(std::string title) {
	titlebartext.setText(title);
}

void gGUIDialogue::setMessageBarSizer(gGUISizer* sizer) {
	messagebar.setSizer(sizer);
}

void gGUIDialogue::setButtonEvent(int buttonEvent) {
	this->buttonevent = buttonEvent;
}

int gGUIDialogue::getButtonEvent() {
	return buttonevent;
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

void gGUIDialogue::transformDialogue(int left, int top, int width, int height) {
	this->left = left; this->top = top; this->width = width; this->height = height;
	this->right = this->left + this->width; this->bottom = this->top + this->height;

	guisizer->left = this->left; guisizer->top = this->top; guisizer->width = this->width; guisizer->height = this->height;
	guisizer->right = guisizer->left + guisizer->width; guisizer->bottom = guisizer->top + guisizer->height;

	titlebar.left = left; titlebar.top = top - titlebarh; titlebar.width = width; titlebar.height = titlebarh;
	titlebar.right = titlebar.left + titlebar.width; titlebar.bottom = titlebar.top + titlebar.bottom;

	buttonsbar.left = left; buttonsbar.top = top + height; buttonsbar.width = width; buttonsbar.height = buttonsbarh;
	buttonsbar.right = buttonsbar.left + buttonsbar.width; buttonsbar.bottom = buttonsbar.top + buttonsbar.height;
	resetTitleBar();
	resetButtonsBar();
	resetMessageBar();
}

int gGUIDialogue::getCursor(int x, int y) {
	if(!ismaximized && isresizeenabled) {
		if((x > left - 5 && x < left + 5) && (y > titlebar.top && y < buttonsbar.bottom)) return CURSOR_HRESIZE;
		if((x > right - 5 && x < right + 5) && (y > titlebar.top && y < buttonsbar.bottom)) return CURSOR_HRESIZE;
		if((y > titlebar.top - 5 && y < titlebar.top + 5) && (x > left && x < right)) return CURSOR_VRESIZE;
		if((y > buttonsbar.bottom - 5 && y < buttonsbar.bottom + 5) && (x > left && x < right)) return CURSOR_VRESIZE;
	}
	return CURSOR_ARROW;
}

void gGUIDialogue::keyPressed(int key) {
	gGUIForm::keyPressed(key);
	messagebar.keyPressed(key);
}

void gGUIDialogue::keyReleased(int key) {
	gGUIForm::keyReleased(key);
	messagebar.keyReleased(key);
}

void gGUIDialogue::charPressed(unsigned int codepoint) {
	gGUIForm::charPressed(codepoint);
	messagebar.charPressed(codepoint);
}

void gGUIDialogue::mouseMoved(int x, int y) {
	titlebar.mouseMoved(x, y);
//	if(guisizer) {
//		if(x >= guisizer->left && x < guisizer->right && y >= guisizer->top && y < guisizer->bottom) {
//			guisizer->iscursoron = true;
//			guisizer->mouseMoved(x, y);
//		}
//	}
	buttonsbar.mouseMoved(x, y);
	messagebar.mouseMoved(x, y);
}

void gGUIDialogue::mousePressed(int x, int y, int button) {
	titlebar.mousePressed(x, y, button);
//	if(guisizer) guisizer->mousePressed(x, y, button);
	buttonsbar.mousePressed(x, y, button);
	messagebar.mousePressed(x, y, button);

	if(minimizebutton.isPressed()) buttontrigger = EVENT_MINIMIZE;
	if(!ismaximized && maximizebutton.isPressed()) buttontrigger = EVENT_MAXIMIZE;
	if(ismaximized && maximizebutton.isPressed()) buttontrigger = EVENT_RESTORE;
	if(exitbutton.isPressed()) buttontrigger = EVENT_EXIT;

	if(!ismaximized && isdragenabled && x > titlebar.left + 5 && x < titlebar.left + titlebar.width - 5 && y > titlebar.top + 5 && y < titlebar.top + titlebar.height) {
		if(minimizebutton.isPressed() || maximizebutton.isPressed() || exitbutton.isPressed()) {
			isdragged = false;
		}
		else {
			isdragged = true;
			dragposx = x; dragposy = y;
		}
	}

	if(!ismaximized && isresizeenabled) {
		if((x > left - 5 && x < left + 5) && (y > titlebar.top && y < buttonsbar.bottom)) {resizeposition = RESIZE_LEFT; sizeposx = x;}
		if((x > right - 5 && x < right + 5) && (y > titlebar.top && y < buttonsbar.bottom)) {resizeposition = RESIZE_RIGHT; sizeposx = x;}
		if((y > titlebar.top - 5 && y < titlebar.top + 5) && (x > left && x < right)) {resizeposition = RESIZE_TOP; sizeposy = y;}
		if((y > buttonsbar.bottom - 5 && y < buttonsbar.bottom + 5) && (x > left && x < right)) {resizeposition = RESIZE_BOTTOM; sizeposy = y;}
	}
}

void gGUIDialogue::mouseDragged(int x, int y, int button) {
	int dx = x - dragposx; int dy = y - dragposy; int sx = x - sizeposx; int sy = y - sizeposy;
	int tleft = left; int twidth = width; int theight = height; int ttop = top;

	if((resizeposition == RESIZE_RIGHT && sx < 0 && width < 400) || (resizeposition == RESIZE_LEFT && sx > 0 && width < 400)) sx = 0;
	if((resizeposition == RESIZE_BOTTOM && sy < 0 && height < 100) || (resizeposition == RESIZE_TOP && sy > 0 && height < 100)) sy = 0;

	if(isdragged && x >= titlebar.left - titlebar.width && x < titlebar.left + titlebar.width && y >= titlebar.top - titlebar.height - guisizer->height && y < titlebar.top + titlebar.height + guisizer->height) {
		tleft += dx; ttop += dy;

		for (int i = 0; i < titlebar.getSizer()->getLineNum(); i++) {
			for (int j = 0; j < titlebar.getSizer()->getColumnNum(); j++) {
				gGUIControl* guicontrol = titlebar.getSizer()->getControl(i, j);
				if(guicontrol != nullptr) {
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
				if(guicontrol != nullptr) {
					guicontrol->left += dx;
					guicontrol->top += dy;
					guicontrol->right = guicontrol->left + guicontrol->width;
					guicontrol->bottom = guicontrol->top + guicontrol->height;
				}
			}
		}

		for (int i = 0; i < buttonsbar.getSizer()->getLineNum(); i++) {
			for (int j = 0; j < buttonsbar.getSizer()->getColumnNum(); j++) {
				gGUIControl* guicontrol = buttonsbar.getSizer()->getControl(i, j);
				if(guicontrol != nullptr) {
					guicontrol->left += dx;
					guicontrol->top += dy;
					guicontrol->right = guicontrol->left + guicontrol->width;
					guicontrol->bottom = guicontrol->top + guicontrol->height;
				}
			}
		}
	}

	if(resizeposition == RESIZE_LEFT) {twidth -= sx; tleft += sx;}
	if(resizeposition == RESIZE_RIGHT) {twidth += sx;}
	if(resizeposition == RESIZE_TOP) {theight -= sy; ttop += sy;}
	if(resizeposition == RESIZE_BOTTOM) {theight += sy;}

	transformDialogue(tleft, ttop, twidth, theight);

	dragposx += dx; dragposy += dy; sizeposx += sx; sizeposy += sy;
}

void gGUIDialogue::mouseReleased(int x, int y, int button) {
	titlebar.mouseReleased(x, y, button);
	if(guisizer) guisizer->mouseReleased(x, y, button);
	buttonsbar.mouseReleased(x, y, button);
	messagebar.mouseReleased(x, y, button);
	if(isdragged) isdragged = false;

	if(resizeposition != RESIZE_NONE) {resizeposition = RESIZE_NONE; resetTitleBar(); resetButtonsBar(); resetMessageBar();}

	if(buttontrigger == EVENT_MINIMIZE) {buttonevent = EVENT_MINIMIZE; buttontrigger = EVENT_NONE;}
	if(buttontrigger == EVENT_MAXIMIZE) {buttonevent = EVENT_MAXIMIZE; buttontrigger = EVENT_NONE;}
	if(buttontrigger == EVENT_RESTORE) {buttonevent = EVENT_RESTORE; buttontrigger = EVENT_NONE;}
	if(buttontrigger == EVENT_EXIT) {buttonevent = EVENT_EXIT; buttontrigger = EVENT_NONE;}
}

void gGUIDialogue::setMessageText(std::string messageText) {
	defmessagetext.setText(messageText);
}

std::string gGUIDialogue::getMessageText() {
	return defmessagetext.getText();
}

void gGUIDialogue::setIconType(int iconId) {
	if(iconId == ICONTYPE_NONE) {
		isiconenabled = false;
		icontypeid = ICONTYPE_NONE;
	}
	else{
		isiconenabled = true;
		defdialogueicon.setPressedButtonImageFromIcon(iconId - 1, true);
		icontypeid = iconId;
	}
}

int gGUIDialogue::getIconType() {
	return icontypeid;
}

void gGUIDialogue::setDialogueType(int typeId) {
	dialoguetype = typeId;
	resetButtonsBar();
}

int gGUIDialogue::getDialogueType() {
	return dialoguetype;
}

void gGUIDialogue::setTitleType(int typeId) {
	titletype = typeId;
	resetTitleBar();
}

int gGUIDialogue::getTitleType() {
	return titletype;
}


int gGUIDialogue::getOKButtonId() {
	return buttonsbarokbutton.getId();
}

int gGUIDialogue::getCancelButtonId() {
	return buttonsbarcancelbutton.getId();
}

int gGUIDialogue::getYesButtonId() {
	return buttonsbaryesbutton.getId();
}

int gGUIDialogue::getNoButtonId() {
	return buttonsbarnobutton.getId();
}

gGUIButton* gGUIDialogue::getOKButton() {
	return &buttonsbarokbutton;
}

gGUIButton* gGUIDialogue::getCancelButton() {
	return &buttonsbarcancelbutton;
}

gGUIButton* gGUIDialogue::getYesButton() {
	return &buttonsbaryesbutton;
}

gGUIButton* gGUIDialogue::getNoButton() {
	return &buttonsbarnobutton;
}


