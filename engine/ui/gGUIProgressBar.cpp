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
	radius = 30;
	valuemax = 100.0f;
	valuemin = 0.0f;
	value = 25.0f;
	thickness = 1.0f;
	spinthickness = 5.0f;
	type = TYPE_LINE;
	textdisplayed = false;
	progressbarcolor = gColor(0.1f, 0.45f, 0.87f);
	backgroundcolor = middlegroundcolor;
	linetextcolor = gColor(9.9f, 9.9f, 9.9f);
	spintextcolor = gColor(0.1f, 0.1f, 0.1f);
	texth = font->getStringHeight("100");
}

gGUIProgressBar::~gGUIProgressBar() {
}

void gGUIProgressBar::draw() {
	gColor oldcolor = renderer->getColor();
	renderer->setColor(backgroundcolor);// color of the background of progressbar
	if(type == TYPE_LINE) gDrawRectangle(left, top, progressbarw, progressbarh, true);
	else if(type == TYPE_CIRCULAR) gDrawCircle((left + right) / 2, (top + bottom) / 2, radius, true);
	else gDrawCircle((left + right) / 2, (top + bottom) / 2, radius, true);
	renderer->setColor(progressbarcolor); // progressbar color
	if(type == TYPE_LINE) gDrawRectangle(left + thickness / 2, top + thickness / 2, (progressbarw - thickness) * value / valuemax, progressbarh - thickness, true);
	else if(type == TYPE_CIRCULAR) gDrawArc((left + right) / 2, (top + bottom) / 2, radius - thickness, true, 60, 360 * value / valuemax, 270);
	else {
		gDrawArc((left + right) / 2, (top + bottom) / 2, radius, true, 60, 360 * value / valuemax, 270);
		renderer->setColor(oldcolor);// return to old color
		gDrawCircle((left + right) / 2, (top + bottom) / 2, radius - spinthickness, true);
	}

	//progress text display
	if(textdisplayed) {
		renderer->setColor(linetextcolor); //set color to text color
		std::string progresstext = std::to_string(int(value / valuemax * 100)) + "%";
		if(type == TYPE_LINE) {
			font->drawText(progresstext, left + thickness / 2 + ((progressbarw - thickness) * value / valuemax - font->getStringWidth(progresstext)) / 2,
										top + thickness / 2 + (progressbarh - thickness) / 2 + texth / 4);
		} else if(type == TYPE_SPIN || type == TYPE_CIRCULAR) {
			renderer->setColor(spintextcolor);
			font->drawText(progresstext, (left + right) / 2 - spinthickness / 2 - font->getStringWidth(progresstext) / 2, (top + bottom) / 2 + texth / 4);
		}
	}

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
	if(std::abs(valuemax - 0.001) <= value) this->value = valuemax; //for roundoff errors
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
	if(thickness < progressbarh / 2 && thickness < progressbarw / 2 && type == TYPE_LINE) {
		this->thickness = thickness;
	}
}

float gGUIProgressBar::getBorderThickness() {
	return thickness;
}

void gGUIProgressBar::setProgressBarSize(int w, int h) {
	progressbarw = w;
	progressbarh = h;
}

int gGUIProgressBar::getProgressBarWidth() {
	return progressbarw;
}

int gGUIProgressBar::getProgressBarHeight() {
	return progressbarh;
}

void gGUIProgressBar::setType(TYPE type) {
	this->type = type;
}

gGUIProgressBar::TYPE gGUIProgressBar::getType() {
	return type;
}

void gGUIProgressBar::setLineTextColor(float r, float g, float b) {
	linetextcolor = gColor(r,g,b);
}

void gGUIProgressBar::setSpinTextColor(float r, float g, float b) {
	spintextcolor = gColor(r,g,b);
}

void gGUIProgressBar::setSpinThickness(float thickness) {
	if(thickness < radius / 2 && type == TYPE_CIRCULAR)
		spinthickness = thickness;
}

float gGUIProgressBar::getSpinThickness() {
	return spinthickness;
}

void gGUIProgressBar::displayProgressText(bool display) {
	textdisplayed = display;
}
