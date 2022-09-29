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
	axisxstart = 0;
	axisystart = 0;
	axisxw = 0;
	axisyh = 0;
	maxx = -2147483647;
	maxy = -2147483647;
	minx = 0;
	miny = 0;
	largestvaluex = 0;
	largestvaluey = 0;
	smallestvaluex = 0;
	smallestvaluey = 0;

	labelwidthx = 0.0f;
	labelwidthy = 0.0f;
	gridlinesxenabled = false;
	gridlinesyenabled = true;
	floatlabelsenabled = false;
	labelcountx = 5;
	labelcounty = 7;

	axisxtitle = "x-axis";
	axisytitle = "y-axis";

	for(int i = 0; i < labelcounty; i++) {
		labelsx.push_back(i);
		labelsy.push_back(i);
	}

	for(int i = 0; i < 8; i++) {
		int labelstep = std::pow(10, i);
		labelsteps.push_back(labelstep);
		labelsteps.push_back(labelstep * 2);
		labelsteps.push_back(labelstep * 5);
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
	axisxstart = axisy2;
	if(miny < 0) axisxstart -= axisyh * ((-miny) / (maxy - miny));
	axisystart = axisx1;
	if(minx < 0) axisystart += axisxw * ((-minx) / (maxx - minx));
	labelwidthx = axisxw / (labelcountx - 1);
	labelwidthy = axisyh / (labelcounty - 1);
}

void gGUIGraph::setMaxX(int maxX) {
	maxx = maxX;
	largestvaluex = maxX;
	updateLabelsX();
}

int gGUIGraph::getMaxX() {
	return maxx;
}

void gGUIGraph::setMinX(int minX) {
	minx = minX;
	smallestvaluex = minX;
	updateLabelsX();
}

int gGUIGraph::getMinX() {
	return minx;
}

void gGUIGraph::setMaxY(int maxY) {
	maxy = maxY;
	largestvaluey = maxY;
	updateLabelsY();
}

int gGUIGraph::getMaxY() {
	return maxy;
}

void gGUIGraph::setMinY(int minY) {
	miny = minY;
	smallestvaluey = minY;
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

int gGUIGraph::calculateStepSize(int step) {
	return step;
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
	gDrawLine(axisx1, axisxstart, axisx2, axisxstart);
	gDrawLine(axisystart, axisy2, axisystart, axisy1);
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
		if(floatlabelsenabled) font->drawText(std::to_string(floatlabelsx[i]), axisx1 + xpoint - 6, axisxstart + 20);
		else font->drawText(std::to_string(labelsx[i]), axisx1 + xpoint - 6, axisxstart + 20);
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
		if(floatlabelsenabled) font->drawText(std::to_string(floatlabelsy[i]), axisystart - 30, axisy2 - ypoint + 3);
		else font->drawText(std::to_string(labelsy[i]), axisystart - 30, axisy2 - ypoint + 3);
	}
}

void gGUIGraph::drawGraph() {

}

void gGUIGraph::updateLabelsX() {
	if(floatlabelsenabled) {
		float step = (maxx - minx) / (labelcountx - 1);
		for(int i = 0; i < labelcountx; i++) {
			floatlabelsx[i] = minx + i * step;
		}
	}
	else{
		int step = abs(largestvaluex) / 10;
		for(int i = 0; i < labelsteps.size(); i++) {
			if(step < labelsteps[i]) {
				step = labelsteps[i];
				break;
			}
		}

		if(minx < 0) minx = (int(smallestvaluex / step) - 1) * step;
		maxx = (int(largestvaluex / step) + 1) * step;
		labelcountx = (maxx - minx) / step + 1;
//		gLogi("labelx") << labelcountx << " " << maxx << " " << minx << " " << step;
		labelsx.clear();
		for(int i = 0; i < labelcountx; i++) {
			labelsx.push_back(minx + i * step);
		}
	}
}

void gGUIGraph::updateLabelsY() {
	if(floatlabelsenabled) {
		float step = (maxy - miny) / (labelcounty - 1);
		for(int i = 0; i < labelcounty; i++) {
			floatlabelsy[i] = miny + i * step;
		}
	}
	else{
		int step = abs(largestvaluey) / 10;
		for(int i = 0; i < labelsteps.size(); i++) {
			if(step < labelsteps[i]) {
				step = labelsteps[i];
				break;
			}
		}

		if(miny < 0) miny = (int(smallestvaluey / step) - 1) * step;
		maxy = (int(largestvaluey / step) + 1) * step;
		labelcounty = (maxy - miny) / step + 1;
		labelsy.clear();
		for(int i = 0; i < labelcounty; i++) {
			labelsy.push_back(miny + i * step);
		}
	}
}

int gGUIGraph::countDigits(int number) {
	return std::to_string(number).size();
}
