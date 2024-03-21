/*
 * gGUIPane.cpp
 *
 *  Created on: Apr 3, 2022
 *      Author: noyan
 */

#include "gGUIPane.h"
#include "gGUINavigation.h"


gGUIPane::gGUIPane() {
	navigation = nullptr;
	navorder = -1;
	setSizer(&panesizer);
	totalh = 0;
	title = "Pane";
	titlefontsize = 24;
	titlefont.loadFont("FreeSansBold.ttf", titlefontsize);
	titlecolor = gColor(128, 128, 128);
	topbarh = titlefontsize * 4 + font->getSize();
	panesizer.setSlotPadding(titlefontsize * 2);
	navbuttonsenabled = true;
	previouspane = nullptr;
	nextpane = nullptr;
	previousbutton.setTitle("Previous");
	previousbutton.setDisabled(true);
	nextbutton.setTitle("Next");
	nextbutton.setDisabled(true);
	buttonsizer.setSize(1, 3);
	float columnprs[3] = {0.65f, 0.15f, 0.2f};
	buttonsizer.setColumnProportions(columnprs);
	buttonsizer.setControl(0, 1, &previousbutton);
	buttonsizer.setControl(0, 2, &nextbutton);
	previousbuttonenabled = true;
	nextbuttonenabled = true;
	issubtitleset = false;
	subtitle = "";
	titlex = 0;
	titley = 0;
	subtitlex = 0;
	subtitley = 0;
}

gGUIPane::~gGUIPane() {
}

void gGUIPane::set(gBaseApp* root, gBaseGUIObject* topParentGUIObject, gBaseGUIObject* parentGUIObject, int parentSlotLineNo, int parentSlotColumnNo, int x, int y, int w, int h) {
	totalh = h;
	gGUIContainer::set(root, topParentGUIObject, parentGUIObject, parentSlotLineNo, parentSlotColumnNo, x, y, w, h);
	buttonsizer.set(root, topparent, this, 0, 0, left, top + height - 50, width, 50);
	titlex = left + titlefontsize * 2;
	titley = top + titlefontsize * 3;
	subtitlex = titlex;
	subtitley = titley + 3 * font->getSize();
}

void gGUIPane::setSubTitle(std::string subTitle) {
	subtitle = subTitle;
	issubtitleset = true;
	if(subtitle == "") issubtitleset = false;
}

void gGUIPane::setNavigation(gGUINavigation* nav) {
	navigation = nav;
}

void gGUIPane::setNavigationOrder(int orderNo) {
	navorder = orderNo;
}

int gGUIPane::getNavigationOrder() {
	return navorder;
}

void gGUIPane::setPreviousPane(gGUIPane* previousPane) {
	if(previouspane) actionmanager.removeAction(&previousbutton, G_GUIEVENT_BUTTONRELEASED, previouspane, G_GUIEVENT_PANEACTIVE);
	previouspane = previousPane;
	actionmanager.addAction(&previousbutton, G_GUIEVENT_BUTTONRELEASED, previouspane, G_GUIEVENT_PANEACTIVE);
	if(previouspane && previousbuttonenabled) previousbutton.setDisabled(false);
}

void gGUIPane::setNextPane(gGUIPane* nextPane) {
	if(nextpane) actionmanager.removeAction(&nextbutton, G_GUIEVENT_BUTTONRELEASED, nextpane, G_GUIEVENT_PANEACTIVE);
	nextpane = nextPane;
	actionmanager.addAction(&nextbutton, G_GUIEVENT_BUTTONRELEASED, nextpane, G_GUIEVENT_PANEACTIVE);
	if(nextpane && nextbuttonenabled) nextbutton.setDisabled(false);
}

void gGUIPane::enablePreviousButton(bool isEnabled) {
	previousbuttonenabled = isEnabled;
	if(!previousbuttonenabled) previousbutton.setDisabled(true);
	if(previouspane && previousbuttonenabled) previousbutton.setDisabled(false);
}

void gGUIPane::enableNextButton(bool isEnabled) {
	nextbuttonenabled = isEnabled;
	if(!nextbuttonenabled) nextbutton.setDisabled(true);
	if(nextpane && nextbuttonenabled) nextbutton.setDisabled(false);
}

void gGUIPane::draw() {
	gColor oldcolor = *renderer->getColor();

	renderer->setColor(255, 255, 255);
	gDrawRectangle(left, top, width, height, true);

	renderer->setColor(212, 212, 212);
	titlefont.drawText(title, titlex, titley);
	if(issubtitleset) {
		renderer->setColor(150, 150, 150);
		font->drawText(subtitle, subtitlex, subtitley);
	}

	renderer->setColor(&oldcolor);
	if(guisizer) guisizer->draw();

	if(navbuttonsenabled) {
		previousbutton.draw();
		nextbutton.draw();
	}
}

void gGUIPane::mouseMoved(int x, int y) {
	buttonsizer.mouseMoved(x, y);
	gGUIContainer::mouseMoved(x, y);
}

void gGUIPane::mousePressed(int x, int y, int button) {
	buttonsizer.mousePressed(x, y, button);
	gGUIContainer::mousePressed(x, y, button);
}

void gGUIPane::mouseReleased(int x, int y, int button) {
	buttonsizer.mouseReleased(x, y, button);
	gGUIContainer::mouseReleased(x, y, button);
}

void gGUIPane::onGUIEvent(int guiObjectId, int eventType, int sourceEventType, std::string value1, std::string value2) {
	gGUIContainer::onGUIEvent(guiObjectId, eventType, sourceEventType, value1, value2);
	if(eventType == G_GUIEVENT_PANEACTIVE) {
		if(navigation != nullptr) navigation->setSelectedPane(navorder);
		 ((gGUISizer*)parent)->setControl(0, 1, this);
	}
}

gGUIButton gGUIPane::getNextButton() {
	return nextbutton;
}

