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
	maxx = -2147483647.0f;
	maxy = -2147483647.0f;
	minx = 0;
	miny = 0;
	largestvaluex = 0;
	largestvaluey = 0;
	smallestvaluex = 0;
	smallestvaluey = 0;

	rangeenabled = false;
	rangestart = 0;
	rangeend = 0;

	labelwidthx = 0.0f;
	labelwidthy = 0.0f;
	gridlinesxenabled = true;
	gridlinesyenabled = true;
	isxaxislinenable = true;
	yaxislenable = true;
	floatlabelsxenabled = false;
	floatlabelsyenabled = false;
	isbackgroundenabled = true;
	labelcountx = 5;
	labelcounty = 7;

	axisxtitle = "x-axis";
	axisytitle = "y-axis";

	for (int i = 0; i < labelcounty; i++) {
		labelsx.push_back(i);
		labelsy.push_back(i);
	}

	for (int i = 0; i < 8; i++) {
		int labelstep = std::pow(10, i);
		labelsteps.push_back(labelstep);
		labelsteps.push_back(labelstep * 2);
		labelsteps.push_back(labelstep * 5);
	}
}

gGUIGraph::~gGUIGraph() {
}

void gGUIGraph::set(gBaseApp* root, gBaseGUIObject* topParentGUIObject, gBaseGUIObject* parentGUIObject, int parentSlotLineNo, int parentSlotColumnNo, int x, int y, int w, int h) {
	gGUIControl::set(root, topParentGUIObject, parentGUIObject, parentSlotLineNo, parentSlotColumnNo, x, y, w, h);
	axisxw = width * 9 / 10;
	axisyh = height * 3 / 4;
	axisx1 = left + 80;
	axisy1 = top + 0.14f * height;
	axisx2 = axisx1 + axisxw;
	axisy2 = axisy1 + axisyh;
	axisxstart = axisy2;
	if (miny < 0) axisxstart -= axisyh * ((-miny) / (maxy - miny));
	axisystart = axisx1;
	if (minx < 0) axisystart += axisxw * ((-minx) / (maxx - minx));
	if (labelcountx > 1) labelwidthx = axisxw / (labelcountx - 1);
	else labelwidthx = 0;
	if (labelcounty > 1) labelwidthy = axisyh / (labelcounty - 1);
	else labelwidthy = 0;
}

void gGUIGraph::setMaxX(float maxX) {
	maxx = maxX;
	largestvaluex = maxX;
	updateLabelsX();
}

int gGUIGraph::getMaxX() {
	return maxx;
}

void gGUIGraph::setMinX(float minX) {
	minx = minX;
	smallestvaluex = minX;
	updateLabelsX();
}

int gGUIGraph::getMinX() {
	return minx;
}

void gGUIGraph::setMaxY(float maxY) {
	maxy = maxY;
	largestvaluey = maxY;
	updateLabelsY();
}

int gGUIGraph::getMaxY() {
	return maxy;
}

void gGUIGraph::setMinY(float minY) {
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

void gGUIGraph::setAxislinesEnable(bool xaxis, bool yaxis) {
	isxaxislinenable = xaxis;
	yaxislenable = yaxis;
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
	if (labelcountx > 1) {
		labelwidthx = axisxw / (labelcountx - 1);
	} else {
		labelwidthx = 0;
	}
}

int gGUIGraph::getLabelCountX() {
	return labelcountx;
}

void gGUIGraph::setLabelCountY(int labelCount) {
	labelcounty = labelCount;
	if (labelcounty > 1) {
		labelwidthy = axisyh / (labelcounty - 1);
	} else {
		labelwidthy = 0;
	}
}

int gGUIGraph::getLabelCountY() {
	return labelcounty;
}

void gGUIGraph::enableRange(bool isRangeEnabled) {
	rangeenabled = isRangeEnabled;
}

void gGUIGraph::setRange(float rangeStart, float rangeEnd) {
	rangestart = rangeStart;
	rangeend = rangeEnd;
}

int gGUIGraph::getRangeStart() {
	return rangestart;
}

int gGUIGraph::getRangeEnd() {
	return rangeend;
}

void gGUIGraph::enableBackground(bool isEnabled) {
	isbackgroundenabled = isEnabled;
}

bool gGUIGraph::isBackgroundEnabled() {
	return isbackgroundenabled;
}

void gGUIGraph::draw() {
	gColor oldcolor = *renderer->getColor();

	if (title != "") {
		renderer->setColor(fontcolor);
		getFont()->drawText(title, (axisx1 + axisx2) / 2, axisy1 - 10);
	}

	drawBackground();
	drawLabels();
	drawGraph();

	renderer->setColor(oldcolor);
}

void gGUIGraph::drawBackground() {
	// First draw the background
	if (isbackgroundenabled) {
		renderer->setColor(textbackgroundcolor);
		gDrawRectangle(left, top, width, height, true);
	}

	// Draw the axis
	renderer->setColor(backgroundcolor);
	if (isxaxislinenable) gDrawLine(axisx1, axisxstart, axisx2, axisxstart);
	if (yaxislenable) gDrawLine(axisystart, axisy2, axisystart, axisy1);
}

void gGUIGraph::drawLabels() {
	renderer->setColor(foregroundcolor);

	// Draw the labels for x-axis
	if (labelcountx > 0) {
		float xpoint = 0;
		float labely = axisxstart + 0.04f * height + 4.0f;// small fixed gap below the tick mark, independent of the control's size
		for (int i = 0; i < labelcountx; i++) {
			xpoint = i * labelwidthx;
			if (isxaxislinenable) {
				renderer->setColor(foregroundcolor);
				if (!gridlinesxenabled) gDrawLine(axisx1 + xpoint, axisy2 + 0.04f * height, axisx1 + xpoint, axisy2);
				else gDrawLine(axisx1 + xpoint, axisy2 + 0.04f * height, axisx1 + xpoint, axisy2 - axisyh);
			}

			renderer->setColor(fontcolor);
			std::string labeltext;
			if (floatlabelsxenabled) {
				if (i < floatlabelsx.size()) labeltext = gToStr(floatlabelsx[i]);
			} else {
				if (i < labelsx.size()) labeltext = std::to_string(labelsx[i]);
			}
			if (!labeltext.empty()) getFont()->drawText(labeltext, axisx1 + xpoint - getFont()->getStringWidth(labeltext) / 2.0f, labely);
		}
	} else if (isxaxislinenable) {
		renderer->setColor(foregroundcolor);
		float yend = gridlinesxenabled ? (axisy2 - axisyh) : axisy2;
		gDrawLine(axisx1, axisy2 + 0.04f * height, axisx1, yend);
		gDrawLine(axisx2, axisy2 + 0.04f * height, axisx2, yend);
	}

	// Draw the labels for y-axis
	if (labelcounty > 0) {
		int ypoint = 0;
		float lineheight = getFont()->getLineHeight();
		for (int i = 0; i < labelcounty; i++) {
			ypoint = i * labelwidthy;
			if (yaxislenable) {
				renderer->setColor(foregroundcolor);
				if (!gridlinesyenabled) gDrawLine(axisx1 - 0.02f * width, axisy2 - ypoint, axisx1, axisy2 - ypoint);
				else gDrawLine(axisx1 - 0.02f * width, axisy2 - ypoint, axisx1 + axisxw, axisy2 - ypoint);
			}

			renderer->setColor(fontcolor);
			std::string labeltext;
			if (floatlabelsyenabled) {
				if (i < floatlabelsy.size()) labeltext = gToStr(floatlabelsy[i]);
			} else {
				if (i < labelsy.size()) labeltext = std::to_string(labelsy[i]);
			}
			if (!labeltext.empty()) {
				float labelw = getFont()->getStringWidth(labeltext);
				getFont()->drawText(labeltext, axisystart - labelw - 8.0f, axisy2 - ypoint - lineheight / 2.0f);
			}
		}
	} else if (yaxislenable) {
		renderer->setColor(foregroundcolor);
		float xend = gridlinesyenabled ? (axisx1 + axisxw) : axisx1;
		gDrawLine(axisx1 - 0.02f * width, axisy2, xend, axisy2);
		gDrawLine(axisx1 - 0.02f * width, axisy2 - axisyh, xend, axisy2 - axisyh);
	}
}

void gGUIGraph::drawGraph() {
}

void gGUIGraph::updateLabelsX() {
	if (floatlabelsxenabled) {
		float step = (maxx - minx) / (labelcountx - 1);
		floatlabelsx.resize(labelcountx);
		for (int i = 0; i < labelcountx; i++) {
			floatlabelsx[i] = minx + i * step;
		}
	} else {
		int step = abs(largestvaluex) / 10;
		for (int i = 0; i < labelsteps.size(); i++) {
			if (step < labelsteps[i]) {
				step = labelsteps[i];
				break;
			}
		}

		if (minx < 0) minx = (int(smallestvaluex / step) - 1) * step;
		maxx = (int)std::ceil(largestvaluex / (float)step) * step;// round up only as far as needed to cover the data, no extra step
		labelcountx = (maxx - minx) / step + 1;
		//		gLogi("labelx") << labelcountx << " " << maxx << " " << minx << " " << step;
		labelsx.clear();
		for (int i = 0; i < labelcountx + 1; i++) {
			labelsx.push_back(minx + i * step);
		}
	}
}

void gGUIGraph::updateLabelsY() {
	if (floatlabelsyenabled) {
		float step = (maxy - miny) / (labelcounty - 1);
		floatlabelsy.resize(labelcounty);
		for (int i = 0; i < labelcounty; i++) {
			floatlabelsy[i] = miny + i * step;
		}
	} else {
		int step = std::abs((int) largestvaluey) / 10;
		for (int i = 0; i < labelsteps.size(); i++) {
			if (step < labelsteps[i]) {
				step = labelsteps[i];
				break;
			}
		}
		if (step <= 0) step = 1;

		if (smallestvaluey >= 0) {
			miny = 0;
		} else {
			miny = (int(smallestvaluey / step) - 1) * step;
		}

		maxy = (int)std::ceil(largestvaluey / (float)step) * step;// round up only as far as needed to cover the data, no extra step

		labelcounty = (maxy - miny) / step + 1;
		labelsy.clear();
		for (int i = 0; i < labelcounty; i++) {
			labelsy.push_back(miny + i * step);
		}
	}
}

void gGUIGraph::setLabelsYForRange(float rangeMinY, float rangeMaxY) {
	float span = rangeMaxY - rangeMinY;
	if (span < 1e-6f) span = 1.0f;
	int targetcount = (labelcounty > 1) ? labelcounty : 7;

	// snap the step to a "nice" 1/2/5 x 10^n value so labels land on whole numbers whenever the
	// data range allows it (e.g. EUR-SEK's ~7-12), falling back to a nice decimal step only when
	// the range is too narrow for whole numbers to be meaningful (e.g. EUR-GBP's ~0.66-0.92)
	float rawstep = span / (targetcount - 1);
	float magnitude = std::pow(10.0f, std::floor(std::log10(rawstep)));
	float normalized = rawstep / magnitude;
	float nicestep = ((normalized <= 1.0f) ? 1.0f : (normalized <= 2.0f) ? 2.0f : (normalized <= 5.0f) ? 5.0f : 10.0f) * magnitude;

	float nicemin = std::floor(rangeMinY / nicestep) * nicestep;
	float nicemax = std::ceil(rangeMaxY / nicestep) * nicestep;

	miny = nicemin;
	maxy = nicemax;
	labelcounty = (int)std::round((nicemax - nicemin) / nicestep) + 1;

	floatlabelsy.resize(labelcounty);
	for (int i = 0; i < labelcounty; i++) {
		floatlabelsy[i] = nicemin + i * nicestep;
	}
	labelwidthy = (labelcounty > 1) ? axisyh / (labelcounty - 1) : 0.0f;
}

int gGUIGraph::countDigits(int number) {
	return std::to_string(number).size();
}

void gGUIGraph::clear() {
}
