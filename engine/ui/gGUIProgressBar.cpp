/*
 * gGUIProgressBar.cpp
 *
 *  Created on: 19 Tem 2022
 *      Author: sevval
 *      Revised: 16 March 2022 Umutcan Turkmen
 */

#include "gGUIProgressBar.h"


gGUIProgressBar::gGUIProgressBar() {
	progressbarw = 100;
	progressbarh = 25;
	valuemax = 100.0f;
	valuemin = 0.0f;
	value = 25.0f;
	progressbarcolor = gColor(0.1f, 0.45f, 0.87f);
	backgroundcolor = middlegroundcolor;
	thickness = 10.0f;
}

gGUIProgressBar::~gGUIProgressBar() {
}

void gGUIProgressBar::draw() {
	gColor oldcolor = renderer->getColor();
	renderer->setColor(backgroundcolor);// color of the background of progressbar
	gDrawRectangle(left, top, progressbarw, progressbarh, true);
	renderer->setColor(progressbarcolor); // progressbar color
	gDrawRectangle(left + thickness / 2, top + thickness / 2, (progressbarw - thickness) * value / valuemax, progressbarh - thickness, true);
	renderer->setColor(oldcolor);
}


void gGUIProgressBar::setMaxValue(float value) {
	if(value > valuemin) valuemax = value;
}

void gGUIProgressBar::setMinValue(float value) {
	if(value < valuemax) valuemin = value;
}

void gGUIProgressBar::setProgressBarColor(float r, float g, float b) {
	progressbarcolor = gColor(r,g,b);
}

void gGUIProgressBar::setBackgroundColor(float r, float g, float b) {
	backgroundcolor = gColor(r,g,b);
}

void gGUIProgressBar::setValue(float value) {
	if(value <= valuemax && value > valuemin) this->value = value;
}

gColor* gGUIProgressBar::getProgressBarColor() {
	return &progressbarcolor;
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

void gGUIProgressBar::setBorderThickness(float thickness) {
	if(thickness < width / 2) {
		this->thickness = thickness;
	}
}

float gGUIProgressBar::getBorderThickness() {
	return thickness;
}
