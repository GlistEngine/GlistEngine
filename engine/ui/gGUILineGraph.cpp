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

	linecolors[0] = {0.96f, 0.46f, 0.55f};
	linecolors[1] = {0.62f, 0.80f, 0.41f};
	linecolors[2] = {0.47f, 0.63f, 0.96f};
	linecolors[3] = {0.87f, 0.68f, 0.40f};
	linecolors[4] = {0.73f, 0.60f, 0.96f};
}

gGUILineGraph::~gGUILineGraph() {

}

void gGUILineGraph::set(gBaseApp* root, gBaseGUIObject* topParentGUIObject, gBaseGUIObject* parentGUIObject, int parentSlotLineNo, int parentSlotColumnNo, int x, int y, int w, int h) {
	gGUIGraph::set(root, topParentGUIObject, parentGUIObject, parentSlotLineNo, parentSlotColumnNo, x, y, w, h);
	updatePoints();
}

void gGUILineGraph::setMaxX(int maxX){
	gGUIGraph::setMaxX(maxX);
	updatePoints();
}


void gGUILineGraph::setMinX(int minX) {
	gGUIGraph::setMinX(minX);
	updatePoints();
}

void gGUILineGraph::setMaxY(int maxY) {
	gGUIGraph::setMaxY(maxY);
	updatePoints();
}


void gGUILineGraph::setMinY(int minY) {
	gGUIGraph::setMinY(minY);
	updatePoints();
}

void gGUILineGraph::setLabelCountX(int labelCount){
	gGUIGraph::setLabelCountX(labelCount);
	updatePoints();
}

void gGUILineGraph::setLabelCountY(int labelCount) {
	gGUIGraph::setLabelCountY(labelCount);
	updatePoints();
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

void gGUILineGraph::drawGraph() {
	if(graphlines.empty()) return;

	gColor oldcolor = *renderer->getColor();

	int linecount = graphlines.size();
	for(int i = 0; i < graphlines.size(); i++) {
		renderer->setColor(linecolors[i]);
		int pointcount = graphlines[i].size();
		for(int j = 0; j < graphlines[i].size(); j++) {
			gDrawCircle(graphlines[i][j][2], graphlines[i][j][3], 5, true);
			if(j != pointcount - 1) gDrawLine(graphlines[i][j][2], graphlines[i][j][3], graphlines[i][j+1][2], graphlines[i][j+1][3]);
		}
	}

	renderer->setColor(oldcolor);
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
