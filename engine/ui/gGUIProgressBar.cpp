/*
 * gGUIProgressBar.cpp
 *
 *  Created on: 19 Tem 2022
 *      Author: sevval
 */

#include "gGUIProgressBar.h"


gGUIProgressBar::gGUIProgressBar() {
	progressbarw = 180;
	progressbarh = 25;
	progressbarinh = progressbarh - 5;
	valuemax = 100.0f;
	valuemin = 0.0f;
	value = 25.0f;
	pcolor = gColor(0.1f, 0.45f, 0.87f);
}

gGUIProgressBar::~gGUIProgressBar() {
}

void gGUIProgressBar::draw() {
	gColor oldcolor = renderer->getColor();
	renderer->setColor(middlegroundcolor);
	gDrawRectangle(left, top, progressbarw, progressbarh);
	renderer->setColor(pcolor);
	gDrawRectangle(left + 3 , top + 2, ((value * 180) / (valuemax - valuemin)) - 8, progressbarinh, true);
	renderer->setColor(oldcolor);
}


void gGUIProgressBar::setMaxValue(float value) {
	if(value > valuemin) valuemax = value;
}

void gGUIProgressBar::setMinValue(float value) {
	if(value < valuemax) valuemin = value;
}

void gGUIProgressBar::setProgressBarColor(gColor color) {
	pcolor = color;
}

void gGUIProgressBar::setValue(float value) {
	if(value <= valuemax && value > valuemin) this->value = value;
}

gColor* gGUIProgressBar::getProgressBarColor() {
	return &pcolor;
}

float gGUIProgressBar::getMaxValue() {
	return valuemax;
}

float gGUIProgressBar::getMinValue() {
	return valuemin;
}

float gGUIProgressBar::getValue() {
	return value;
}
