/*
 * gGUISlider.cpp
 *
 *  Created on: 2022 Aug 02
 *      Author: Yaren Cetinkaya, Murat Ergin
 */
#include "gGUISlider.h"
#include "gBaseApp.h"
#include "gBaseCanvas.h"
#include <iostream>
#include <cmath>
#include <iomanip>


gGUISlider::gGUISlider() {
	sliderbarh = 3;
	sliderbarw = 160;
	sliderh = 30;
	sliderw = 10;
	sliderx = 0;
	slidery = 0;
	slidercenterx = 0;
	sliderbarx = 0;
	sliderbary = 0;
	textx = 0;
	texty = 0;
	tickx = 0;
	ticky = 0;
	currentvaluex = 0;
	currentvaluey = 0;

	ticknumbers.clear();

	ticknum = 5;
	spacelength = sliderbarw / ticknum;
	minvalue = 0;
	maxvalue = 100;
	currentvalue = (minvalue + maxvalue) / 2;
	direction = -1;
	counter = 0;
	counterlimit = 6;
	counterpressed = 0;
	counterpressedlimit = 6;
	mousex = 0;

	isdisabled = false;
	ispressedslider = false;
	istickvisible = false;
	ispressed = false;
	istextvisible = false;

	sliderbarcolor = *pressedbuttonfontcolor;
	disabledcolor = *disabledbuttoncolor;
	slidercolor = *buttoncolor;
	pressedslidercolor = *pressedbuttoncolor;
	tickcolor = sliderbarcolor;
}

gGUISlider::~gGUISlider() {
}

void gGUISlider::setTextVisibility(bool isVisible) {
	istextvisible = isVisible;
}

void gGUISlider::setTickVisibility(bool isVisible) {
	istickvisible = isVisible;
}

void gGUISlider::setTicknum(float tickNum) {
	ticknum = tickNum;
	spacelength = sliderbarw / ticknum;
}

float gGUISlider::getMinValue() {
	return minvalue;
}

void gGUISlider::setMinValue(float minValue) {
	minvalue = minValue;
}

float gGUISlider::getMaxValue() {
	return maxvalue;
}

void gGUISlider::setMaxValue(float maxValue) {
	minvalue = maxValue;
}

float gGUISlider::getCurrentValue() {
	return currentvalue;
}

float gGUISlider::getSliderbarWidth() {
	return sliderbarw;
}

float gGUISlider::getSliderbarHeight() {
	return sliderbarh;
}

void gGUISlider::setDisabled(bool isDisabled) {
	isdisabled = isDisabled;
}

void gGUISlider::setSliderColor(gColor color) {
	slidercolor = color;
}

gColor* gGUISlider::getSliderColor() {
	return &slidercolor;
}

void gGUISlider::setPressedSliderColor(gColor color) {
	pressedslidercolor = color;
}

gColor* gGUISlider::getPressedSliderColor() {
	return &pressedslidercolor;
}

void gGUISlider::setDisabledColor(gColor color) {
	disabledcolor = color;
}

gColor* gGUISlider::getDisabledColor() {
	return &disabledcolor;
}

void gGUISlider::setSliderbarColor(gColor color) {
	sliderbarcolor = color;
}

gColor* gGUISlider::getSliderbarColor() {
	return &sliderbarcolor;
}

void gGUISlider::setTickColor(gColor color) {
	tickcolor = color;
}

gColor* gGUISlider::getTickColor() {
	return &tickcolor;
}

bool gGUISlider::isTickVisible() {
	return istickvisible;
}

bool gGUISlider::isDisabled() {
	return isdisabled;
}

bool gGUISlider::isPressed() {
	return ispressed;
}

bool gGUISlider::isSliderPressed() {
	return ispressedslider;
}


void gGUISlider::set(gBaseApp* root, gBaseGUIObject* topParentGUIObject, gBaseGUIObject* parentGUIObject, int parentSlotLineNo, int parentSlotColumnNo, int x, int y, int w, int h) {
	gGUIControl::set(root, topParentGUIObject, parentGUIObject, parentSlotLineNo, parentSlotColumnNo, x, y, w, h);
	sliderx = left + (sliderbarw / 2);
	slidery = top;

	sliderbarx = left + (sliderw / 2);
	sliderbary = top + (sliderh / 2) - (sliderbarh / 2);

	tickx = left + (sliderw / 2) - 1;
	ticky = top + sliderh;
	textx = tickx;
	texty = ticky + 12 + font->getSize();

	currentvaluey = top - (sliderh / 2) + font->getSize() - 4;
}

void gGUISlider::update() {
    if(ispressed) {
        if(mousex <= sliderbarx) {
        	sliderx = sliderbarx - (sliderw / 2);
        }else if(mousex >= sliderbarx + sliderbarw) {
        	sliderx = sliderbarx + sliderbarw - (sliderw / 2);
        }else {
            counterpressed++;
            if(counterpressed >= counterpressedlimit && abs(sliderx - (mousex + sliderw / 2)) > spacelength / 2) {
                direction = -1;
                if(mousex > sliderx) direction = 1;
                sliderx = (sliderx + spacelength * direction);
                counterpressed = 0;
			}
        }
    }
}

void gGUISlider::draw() {
	gColor oldcolor = renderer->getColor();
	if(isdisabled) renderer->setColor(&disabledcolor);
	else renderer->setColor(pressedbuttonfontcolor);
	drawSliderBar();
	if(istickvisible) drawTick();
	if(istextvisible) drawText();

	if(isdisabled) renderer->setColor(&disabledcolor);
	else {
		if(ispressedslider) renderer->setColor(&pressedslidercolor);
		else renderer->setColor(&slidercolor);
	}
	drawSlider();
	renderer->setColor(oldcolor);
}

void gGUISlider::drawSliderBar() {
	gDrawRectangle(sliderbarx, sliderbary, sliderbarw, sliderbarh, true);
}

void gGUISlider::drawTick() {
	for(int i = 0; i <= ticknum; i++) {
		if(i == 0 || i == ticknum) {
			//Beginning and ending long ticks
			gDrawLine(tickx + spacelength * i, ticky + 12, tickx + spacelength * i, ticky - (sliderh / 2));
		}else {
			//In-between ticks
			gDrawLine(tickx + spacelength * i, ticky + 12, tickx + spacelength * i, ticky + 4);
		}
	}
}

void gGUISlider::drawText() {
    for(int i = 0; i <= ticknum; i++) {
    	//Tick number texts
    	float temp_ticknumbers = floor((i * (maxvalue - minvalue) / ticknum) * 100.0) / 100.0;
    	ticknumbers = gToStr(temp_ticknumbers);
        font->drawText(ticknumbers, textx - 8 + (spacelength * i), texty);
    }
    //Current value text
    float tempvalue = floor((currentvalue * 100.0)) / 100.0;
    font->drawText(gToStr(tempvalue), currentvaluex, currentvaluey);
}

void gGUISlider::drawSlider() {
	float modvalue = std::fmod(sliderx, spacelength);
	if(modvalue >= spacelength / 2) sliderx = sliderx + spacelength - modvalue;
	else if(modvalue < spacelength / 2) sliderx = sliderx - modvalue;

	slidercenterx = sliderx + (sliderw / 2);
	currentvaluex = slidercenterx - (sliderw / 2);
	currentvalue = floor((minvalue + (slidercenterx - sliderbarx) / sliderbarw * ticknum * ((maxvalue - minvalue) / ticknum)  + 1.25f) * 100.0f) / 100.0f;
	currentvaluetext = gToStr(getCurrentValue());

	gDrawRectangle(sliderx, slidery, sliderw, sliderh, true);
}

void gGUISlider::mousePressed(int x, int y, int button) {
    if(isdisabled) return;
    mousex = x;
    if(x > sliderx && x < sliderx + sliderw && y > sliderbary - (sliderh / 2) && y < sliderbary + sliderbarh + (sliderh / 2)) {
        ispressedslider = true;
    }else if(x > sliderbarx && x < sliderbarx + sliderbarw && y > sliderbary - (sliderh / 2) && y < sliderbary + sliderbarh + (sliderh / 2) && !(x >= sliderx && x < (sliderx + sliderw))) {
        ispressed = true;
    }
}

void gGUISlider::mouseReleased(int x, int y, int button) {
    if(isdisabled) return;
    ispressedslider = false;
    ispressed = false;
}

void gGUISlider::mouseDragged(int x, int y, int button) {
    if(isdisabled) return;
    mousex = x;

    if(ispressedslider) {
        if(x <= sliderbarx) sliderx = sliderbarx - (sliderw / 2);
        else if(x >= sliderbarx + sliderbarw) sliderx = sliderbarx + sliderbarw - (sliderw / 2);
        else sliderx = x;
    }
}

