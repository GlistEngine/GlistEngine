/*
 * gGUILine.cpp
 *
 *  Created on: Aug 7, 2026
 *      Author: Antigravity
 */

#include "gGUILine.h"

gGUILine::gGUILine() {
	linecolor = gColor(0.85f, 0.85f, 0.85f, 1.0f); // Default light grey separator
	thickness = 1.0f; // Default 1 pixel
	isvertical = false; // Default horizontal
}

gGUILine::~gGUILine() {
}

void gGUILine::setLineColor(gColor color) {
	linecolor = color;
}

gColor gGUILine::getLineColor() {
	return linecolor;
}

void gGUILine::setThickness(float thickness) {
	this->thickness = thickness;
}

float gGUILine::getThickness() {
	return thickness;
}

void gGUILine::setVertical(bool isVertical) {
	isvertical = isVertical;
}

bool gGUILine::isVertical() {
	return isvertical;
}

void gGUILine::draw() {
	gColor* oldcolor = renderer->getColor();
	renderer->setColor(linecolor);

	if (isvertical) {
		gDrawRectangle(left + (width - thickness) / 2.0f, top, thickness, height, true);
	} else {
		gDrawRectangle(left, top + (height - thickness) / 2.0f, width, thickness, true);
	}

	renderer->setColor(oldcolor);
}
