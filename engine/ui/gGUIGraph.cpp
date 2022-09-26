/*
 * gGUIGraph.cpp
 *
 *  Created on: Sep 23, 2022
 *      Author: noyan
 */

#include "gGUIGraph.h"

gGUIGraph::gGUIGraph() {
	axisx1 = 0;
	axisx2 = 0;
	axisy1 = 0;
	axisy2 = 0;
	axisxw = 0;
	axisyh = 0;
	maxx = -2147483647;
	maxy = -2147483647;
	minx = 2147483647;
	miny = 0;

	labelwidthx = 0.0f;
	labelwidthy = 0.0f;
	gridlinesxenabled = false;
	gridlinesyenabled = true;
	labelcountx = 5;
	labelcounty = 6;

	axisxtitle = "x-axis";
	axisytitle = "y-axis";

	for(int i = 0; i < labelcounty; i++) {
		labelsx.push_back(i);
		labelsy.push_back(i);
	}
}

gGUIGraph::~gGUIGraph() {
}

void gGUIGraph::set(gBaseApp *root, gBaseGUIObject *topParentGUIObject, gBaseGUIObject *parentGUIObject, int parentSlotLineNo, int parentSlotColumnNo, int x, int y, int w, int h) {
	gGUIControl::set(root, topParentGUIObject, parentGUIObject, parentSlotLineNo, parentSlotColumnNo, x, y, w, h);
	axisxw = width * 3 / 4;
	axisyh = height * 3 / 4;
	axisx1 = left + 60;
	axisy1 = top + 40;
	axisx2 = axisx1 + axisxw;
	axisy2 = axisy1 + axisyh;
//	labelwidthx = axisxw / (maxx - minx);
	labelwidthx = axisxw / (labelcountx - 1);
	labelwidthy = axisyh / (labelcounty - 1);
}

void gGUIGraph::setMaxX(int maxX) {
	maxx = maxX;
	updateLabelsX();
}

int gGUIGraph::getMaxX() {
	return maxx;
}

void gGUIGraph::setMinX(int minX) {
	minx = minX;
	updateLabelsX();
}

int gGUIGraph::getMinX() {
	return minx;
}

void gGUIGraph::setMaxY(int maxY) {
	maxy = maxY;
	updateLabelsY();
}

int gGUIGraph::getMaxY() {
	return maxy;
}

void gGUIGraph::setMinY(int minY) {
	miny = minY;
	updateLabelsY();
}

int gGUIGraph::getMinY() {
	return miny;
}

void gGUIGraph::enableGridlinesX(bool isEnabled) {
	gridlinesxenabled = isEnabled;
}

void gGUIGraph::enableGridlinesY(bool isEnabled) {
	gridlinesyenabled = isEnabled;
}

void gGUIGraph::setTitleX(std::string titleX) {
	axisxtitle = titleX;
}

std::string gGUIGraph::getTitleX() {
	return axisxtitle;
}

void gGUIGraph::setTitleY(std::string titleY) {
	axisytitle = titleY;
}

std::string gGUIGraph::getTitleY() {
	return axisytitle;
}

void gGUIGraph::setLabelCountX(int labelCount) {
	labelcountx = labelCount;
	labelwidthx = axisxw / (labelcountx - 1);
}

int gGUIGraph::getLabelCountX() {
	return labelcountx;
}

void gGUIGraph::setLabelCountY(int labelCount) {
	labelcounty = labelCount;
	labelwidthy = axisyh / (labelcounty - 1);
}

int gGUIGraph::getLabelCountY() {
	return labelcounty;
}

void gGUIGraph::draw() {
	gColor oldcolor = *renderer->getColor();

	if(title != "") {
		renderer->setColor(fontcolor);
		font->drawText(title, (axisx1 + axisx2) / 2, axisy1 - 10);
	}

	drawBackground();
	drawLabels();
	drawGraph();

	renderer->setColor(oldcolor);
}

void gGUIGraph::drawBackground() {
	// First draw the background
	renderer->setColor(textbackgroundcolor);
	gDrawRectangle(left, top, width, height, true);

	// Draw the axis
	renderer->setColor(backgroundcolor);
	gDrawLine(axisx1, axisy2, axisx2, axisy2);
	gDrawLine(axisx1, axisy2, axisx1, axisy1);
}

void gGUIGraph::drawLabels() {
	// Draw the labels for x-axis
	float xpoint = 0;
	int labelcountx = labelsx.size() - 1;
	for(int i = 0; i < labelcountx; i++) {
		xpoint = i * labelwidthx;
		renderer->setColor(backgroundcolor);
		if(!gridlinesxenabled) gDrawLine(axisx1 + xpoint, axisy2 + 10, axisx1 + xpoint, axisy2);
		else gDrawLine(axisx1 + xpoint, axisy2 + 10, axisx1 + xpoint, axisy2 - axisyh);

		renderer->setColor(fontcolor);
		if(i > 0) {
			font->drawText(std::to_string(labelsx[i]), axisx1 + xpoint - 6, axisy2 + 25);
		}
	}
//	font->drawText(axisxtitle, axisx1 + axisxw / 2, axisy2 + 40);
	// Draw the labels for y-axis
	int ypoint = 0;
	int labelcounty = labelsy.size();
	for(int i = 0; i < labelcounty; i++) {
		ypoint = i * labelwidthy;
		renderer->setColor(backgroundcolor);
		if(!gridlinesyenabled) gDrawLine(axisx1 - 10, axisy2 - ypoint, axisx1, axisy2 - ypoint);
		else gDrawLine(axisx1 - 10, axisy2 - ypoint, axisx1 + axisxw, axisy2 - ypoint);

		renderer->setColor(fontcolor);
		font->drawText(std::to_string(labelsy[i]), axisx1 - 30, axisy2 - ypoint);
	}
}

void gGUIGraph::drawGraph() {

}

void gGUIGraph::updateLabelsX() {
	int step = int(maxx - minx) / (labelcountx - 1);
	for(int i = 0; i < labelcountx; i++) {
		labelsx[i] = minx + i * step;
	}
}

void gGUIGraph::updateLabelsY() {
	int step = int(maxy - miny) / (labelcounty - 1);
	for(int i = 0; i < labelcounty; i++) {
		labelsy[i] = miny + i * step;
	}
}
