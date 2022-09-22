/*
 * gGUILineGraph.cpp
 *
 *  Created on: 28 July 2022
 *      Author: Berke Adil
 */


#include "gGUILineGraph.h"
#include "gGUISlider.h"

gGUILineGraph::gGUILineGraph() {
	title = "Graph";

	axisx1 = 0;
	axisx2 = 0;
	axisy1 = 0;
	axisy2 = 0;
	axisxw = 0;
	axisyh = 0;
	maxx = 0;
	maxy = 0;
	minx = 0;
	miny = 0;
	labelwidthx = 0.0f;
	labelwidthy = 0.0f;
	drawgridlinesx = false;
	drawgridlinesy = true;
	labelcountx = 5;
	labelcounty = 6;

	axisxtitle = "x-axis";
	axisytitle = "y-axis";

	linecolors[0] = {0.96f, 0.46f, 0.55f};
	linecolors[1] = {0.62f, 0.80f, 0.41f};
	linecolors[2] = {0.47f, 0.63f, 0.96f};
	linecolors[3] = {0.87f, 0.68f, 0.40f};
	linecolors[4] = {0.73f, 0.60f, 0.96f};

	for(int i = 0; i < labelcounty; i++) {
		labelsx.push_back(i);
		labelsy.push_back(i);
	}
}

gGUILineGraph::~gGUILineGraph() {

}

void gGUILineGraph::set(gBaseApp* root, gBaseGUIObject* topParentGUIObject, gBaseGUIObject* parentGUIObject, int parentSlotLineNo, int parentSlotColumnNo, int x, int y, int w, int h) {
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
	updatePoints();
}

void gGUILineGraph::setMaxX(int maxX){
	maxx = maxX;
	updateLabelsX();
	updatePoints();
//	labelwidthx = axisxw / (maxx - minx);
}

int gGUILineGraph::getMaxX() {
	return maxx;
}

void gGUILineGraph::setMinX(int minX) {
	minx = minX;
	updateLabelsX();
	updatePoints();
//	labelwidthx = axisxw / (maxx - minx);
}

int gGUILineGraph::getMinX() {
	return minx;
}

void gGUILineGraph::setMaxY(int maxY) {
	maxy = maxY;
	updateLabelsY();
	updatePoints();
//	labelwidthy = axisyh / (maxy - miny);
}

int gGUILineGraph::getMaxY() {
	return maxy;
}

void gGUILineGraph::setMinY(int minY) {
	miny = minY;
	updateLabelsY();
	updatePoints();
//	labelwidthy = axisyh / (maxy - miny);
}

int gGUILineGraph::getMinY() {
	return miny;
}

void gGUILineGraph::setGridlinesX(bool gridlinesX) {
	drawgridlinesx = gridlinesX;
}

void gGUILineGraph::setGridlinesY(bool gridlinesY) {
	drawgridlinesy = gridlinesY;
}

void gGUILineGraph::setTitleX(std::string titleX) {
	axisxtitle = titleX;
}

std::string gGUILineGraph::getTitleX() {
	return axisxtitle;
}

void gGUILineGraph::setTitleY(std::string titleY) {
	axisytitle = titleY;
}

std::string gGUILineGraph::getTitleY() {
	return axisytitle;
}

void gGUILineGraph::setLabelCountX(int labelCount){
	labelcountx = labelCount;
	labelwidthx = axisxw / (labelcountx - 1);
	updatePoints();
}

int gGUILineGraph::getLabelCountX() {
	return labelcountx;
}

void gGUILineGraph::setLabelCountY(int labelCount) {
	labelcounty = labelCount;
	labelwidthy = axisyh / (labelcounty - 1);
	updatePoints();
}

int gGUILineGraph::getLabelCountY() {
	return labelcounty;
}

void gGUILineGraph::addLine() {
	std::vector<std::array<float, 4>> newline;
	graphlines.push_back(newline);
}

void gGUILineGraph::addPointToLine(int lineindex, float x, float y) {
	if(graphlines.size() - 1 < lineindex) return;
	if(x > maxx) {
		int newmax = int(x) + 1;
		if(newmax % (labelcountx - 1) == 0) setMaxX(newmax);
		else setMaxX(newmax + labelcountx - 1 - (newmax % (labelcountx - 1)));
	}
	if(y > maxy) {
		int newmax = int(y) + 1;
		if(newmax % (labelcounty - 1) == 0) setMaxY(newmax);
		else setMaxY(newmax + labelcounty - 1 - (newmax % (labelcounty - 1)));
	}
	gLogi("max") << maxx << " " << maxy;
	int pointcount = graphlines[lineindex].size();
	if(pointcount == 0) {
		graphlines[lineindex].push_back({x, y, axisx1 + axisxw * x / (maxx - minx), axisy2 - axisyh * y / (maxy - miny)});
		return;
	}
	int index = 0;
	while(index < pointcount) {
		if(graphlines[lineindex][index++][0] < x) continue;
		index--;
		break;
	}
	graphlines[lineindex].insert(graphlines[lineindex].begin() + index, {x, y, axisx1 + axisxw * x / (maxx - minx), axisy2 - axisyh * y / (maxy - miny)});
}

void gGUILineGraph::draw() {
	gColor oldcolor = *renderer->getColor();

	if(title != "") {
		renderer->setColor(fontcolor);
		font->drawText(title, (axisx1 + axisx2) / 2, axisy1 - 10);
	}

	drawGraph();
	drawLabels();
	drawLines();

	renderer->setColor(oldcolor);
}

void gGUILineGraph::drawGraph() {

	// First draw the background
	renderer->setColor(textbackgroundcolor);
	gDrawRectangle(left, top, width, height, true);

	// Draw the axis
	renderer->setColor(backgroundcolor);
	gDrawLine(axisx1, axisy2, axisx2, axisy2);
	gDrawLine(axisx1, axisy2, axisx1, axisy1);

}

void gGUILineGraph::drawLabels() {

	// Draw the labels for x-axis
	float xpoint = 0;
	int labelcountx = labelsx.size();
	for(int i = 0; i < labelcountx; i++) {
		xpoint = i * labelwidthx;
		renderer->setColor(backgroundcolor);
		if(!drawgridlinesx) gDrawLine(axisx1 + xpoint, axisy2 + 10, axisx1 + xpoint, axisy2);
		else gDrawLine(axisx1 + xpoint, axisy2 + 10, axisx1 + xpoint, axisy2 - axisyh);

		renderer->setColor(fontcolor);
		if(i > 0) font->drawText(std::to_string(labelsx[i]), axisx1 + xpoint - 6, axisy2 + 25);
	}
//	font->drawText(axisxtitle, axisx1 + axisxw / 2, axisy2 + 40);
	// Draw the labels for y-axis
	int ypoint = 0;
	int labelcounty = labelsy.size();
	for(int i = 0; i < labelcounty; i++) {
		ypoint = i * labelwidthy;
		renderer->setColor(backgroundcolor);
		if(!drawgridlinesy) gDrawLine(axisx1 - 10, axisy2 - ypoint, axisx1, axisy2 - ypoint);
		else gDrawLine(axisx1 - 10, axisy2 - ypoint, axisx1 + axisxw, axisy2 - ypoint);

		renderer->setColor(fontcolor);
		font->drawText(std::to_string(labelsy[i]), axisx1 - 30, axisy2 - ypoint);
	}

}

void gGUILineGraph::drawLines() {
	if(graphlines.empty()) return;

	int linecount = graphlines.size();
	for(int i = 0; i < graphlines.size(); i++) {
		renderer->setColor(linecolors[i]);
		int pointcount = graphlines[i].size();
		for(int j = 0; j < graphlines[i].size(); j++) {
			gDrawCircle(graphlines[i][j][2], graphlines[i][j][3], 5, true);
			if(j != pointcount - 1) gDrawLine(graphlines[i][j][2], graphlines[i][j][3], graphlines[i][j+1][2], graphlines[i][j+1][3]);
		}
	}
}

void gGUILineGraph::updatePoints() {
	if(graphlines.empty()) return;

	int linecount = graphlines.size();
	for(int i = 0; i < graphlines.size(); i++) {
		int pointcount = graphlines[i].size();
		for(int j = 0; j < graphlines[i].size(); j++) {
			graphlines[i][j][2] = axisx1 + axisxw * graphlines[i][j][0] / (maxx - minx);
			graphlines[i][j][3] = axisy2 - axisyh * graphlines[i][j][1] / (maxy - miny);
		}
	}
}

void gGUILineGraph::updateLabelsX() {
	int step = int(maxx - minx) / (labelcountx - 1);
	for(int i = 0; i < labelcountx; i++) {
		labelsx[i] = i * step;
	}
}

void gGUILineGraph::updateLabelsY() {
	int step = int(maxy - miny) / (labelcounty - 1);
	for(int i = 0; i < labelcounty; i++) {
		labelsy[i] = i * step;
	}
}
