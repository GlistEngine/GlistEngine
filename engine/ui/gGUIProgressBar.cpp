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
	scolor = gColor(0.1f * 0.82f, 0.45f * 0.82f, 0.87f * 0.82f);
	thickness = 1.0f;
}

gGUIProgressBar::~gGUIProgressBar() {
}

void gGUIProgressBar::draw() {
	gColor oldcolor = renderer->getColor();
	renderer->setColor(middlegroundcolor);// window
	gDrawRectangle(left, top, progressbarw, progressbarh, false, thickness);


	renderer->setColor(scolor); // progressbar shadow
	gDrawRectangle(left + (thickness / 2) + 2.26f, top + (thickness / 2) + 0.66f , ((value * 180) / (valuemax - valuemin)) - (thickness / 2) - 7.22f, progressbarinh - thickness , true, 1.26f);
	renderer->setColor(pcolor); // progress bar
	gDrawRectangle(left + (thickness / 2) + 2, top + ((thickness / 2) + 0.4f ), ((value * 180) / (valuemax - valuemin)) - (thickness / 2) - 8, progressbarinh - thickness - 1.78f, true);


	renderer->setColor(oldcolor);
}


void gGUIProgressBar::setMaxValue(float value) {
	if(value > valuemin) valuemax = value;
}

void gGUIProgressBar::setMinValue(float value) {
	if(value < valuemax) valuemin = value;
}

void gGUIProgressBar::setProgressBarColor(float r, float g, float b) {
	pcolor = gColor(r,g,b);
	scolor = gColor(r * 0.82f, g * 0.82f, b * 0.82f);
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

void gGUIProgressBar::setBorderThickness(float thickness) {
	if(thickness < 25.0f) {
		this->thickness = thickness;
	}
}

float gGUIProgressBar::getBorderThickness() {
	return thickness;
}
