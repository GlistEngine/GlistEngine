/*
 * tooltipText.h
 *
 *  Created on: 03 Jan 2024
 *  Author    : Furkan
 */


#include "gGUITooltipText.h"

gGUITooltipText::gGUITooltipText() {
	text = "";
	textlist.push_back("");
	texth = 0;
	textw = 0;
	margin = 2;
}

gGUITooltipText::~gGUITooltipText() {
}

void gGUITooltipText::setText(std::string text) {
	this->text = text;
	textlist.push_back(this->text);
}

std::string gGUITooltipText::getText() {
	return this->text;
}

void gGUITooltipText::update() {
}

void gGUITooltipText::draw() {
	if(cursorx >= sizerx && cursorx <= sizerw && cursory >= sizery && cursory <= sizerh) {
		gColor oldcolor = *renderer->getColor();
		color.set(0.85f, 0.85f, 0.0f);
		renderer->setColor(&color);
		gDrawRectangle(sizerx, sizery + objecth + margin, textw + tx * 3, 2 * texth, true);
		renderer->setColor(fontcolor);
		for(int i = 0; i < textlist.size(); i++) { font->drawText(textlist[i], sizerx + tx - 2, sizery + objecth + margin + texth + 2);}
		renderer->setColor(&oldcolor);
	}
}

void gGUITooltipText::activate(gGUISizer* sizer, objects guiobject, int lineNo, int columnNo, std::string text) {
	setText(text);
	sizerx = sizer->getSlotX(lineNo, columnNo);
	sizery = sizer->getSlotY(lineNo, columnNo);
	switch(guiobject) {
		case Button:
			sizerw = sizerx + 96;
			sizerh = sizery + 32;
			objecth = 32;
			break;
		case Textbox:
			sizerw = sizerx + sizer->getSlotWidth(lineNo, columnNo);
			sizerh = sizery + 32;
			objecth = 32;
			break;
		case Numberbox:
			sizerw = sizerx + sizer->getSlotWidth(lineNo, columnNo) * 0.5;
			sizerh = sizery + 32;
			objecth = 32;
			break;
		case Panel:
			sizerw = sizerx + sizer->getSlotWidth(lineNo, columnNo);
			sizerh = sizery + 21;
			objecth = 21;
			break;
		default:
			gLogi("tooltipText") << "Check the name of the object!!";
		}
	setTitleLength(text);
}

void gGUITooltipText::setTitleLength(std::string text) {
	texth = font->getStringHeight("A");
	textw = font->getStringWidth(text);
	tx = font->getStringWidth("a") / 2;
	//gLogi("gCanvas") << texth << " " << textw << " " << tx;
}

void gGUITooltipText::mouseMoved(int x, int y) {
	cursorx = x;
	cursory = y;
}
