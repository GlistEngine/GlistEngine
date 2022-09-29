/*
 * gGUICandleStickChart.cpp
 *
 *  Created on: 15 August 2022
 *      Author: Berke Adil
 */

#include "gGUICandleStickChart.h"

gGUICandleStickChart::gGUICandleStickChart() {
	title = "Candlestick Chart";
	lowcolor = {0.96f, 0.46f, 0.55f};
	highcolor = {0.62f, 0.80f, 0.41f};
	candlew = 10;
}

gGUICandleStickChart::~gGUICandleStickChart() {

}

void gGUICandleStickChart::set(gBaseApp* root, gBaseGUIObject* topParentGUIObject, gBaseGUIObject* parentGUIObject, int parentSlotLineNo, int parentSlotColumnNo, int x, int y, int w, int h) {
	gGUIGraph::set(root, topParentGUIObject, parentGUIObject, parentSlotLineNo, parentSlotColumnNo, x, y, w, h);
	updatePoints();
}

void gGUICandleStickChart::setMaxX(int maxX){
	gGUIGraph::setMaxX(maxX);
	updatePoints();
}


void gGUICandleStickChart::setMinX(int minX) {
	gGUIGraph::setMinX(minX);
	updatePoints();
}

void gGUICandleStickChart::setMaxY(int maxY) {
	gGUIGraph::setMaxY(maxY);
	updatePoints();
}


void gGUICandleStickChart::setMinY(int minY) {
	gGUIGraph::setMinY(minY);
	updatePoints();
}

void gGUICandleStickChart::setLabelCountX(int labelCount){
	gGUIGraph::setLabelCountX(labelCount);
	updatePoints();
}

void gGUICandleStickChart::setLabelCountY(int labelCount) {
	gGUIGraph::setLabelCountY(labelCount);
	updatePoints();
}

void gGUICandleStickChart::setHighColor(gColor highColor) {
	highcolor = highColor;
}

gColor gGUICandleStickChart::getHighColor(){
	return highcolor;
}

void gGUICandleStickChart::setLowColor(gColor lowColor) {
	lowcolor = lowColor;
}

gColor gGUICandleStickChart::getLowColor() {
	return lowcolor;
}

void gGUICandleStickChart::addPointToLine(float x, float high, float low, float open, float close) {
	if(x > largestvaluex) setMaxX(x);
	if(high > largestvaluey) setMaxY(high);

	int pointcount = graphline.size();
	float pointx = axisx1 + axisxw * (x - minx) / (maxx - minx);
	float lengthy = maxy - miny;
	float highy = axisy2 - (axisyh * (high - miny) / lengthy);
	float lowy = axisy2 - axisyh * (low - miny) / lengthy;
	float openy = axisy2 - axisyh * (open - miny) / lengthy;
	float closey = axisy2 - axisyh * (close - miny) / lengthy;

	if(pointcount == 0) {
		graphline.push_back({x, high, low, open, close, pointx, highy, lowy, openy, closey});
		return;
	}
	int index = 0;
	while(index < pointcount) {
		if(graphline[index++][0] < x) continue;
		index--;
		break;
	}
	graphline.insert(graphline.begin() + index, {x, high, low, open, close, pointx, highy, lowy, openy, closey});
//	gLogi("update") << graphline[0][5] << " " << graphline[0][6] << " " << graphline[0][7] << " " << graphline[0][8] << " " << graphline[0][9];

}

void gGUICandleStickChart::drawGraph() {
	if(graphline.empty()) return;

	gColor oldcolor = *renderer->getColor();
	int pointcount = graphline.size();
	for(int i = 0; i < pointcount; i++) {
		if(graphline[i][3] - graphline[i][4] < 0) renderer->setColor(lowcolor);
		else renderer->setColor(highcolor);
		gDrawLine(graphline[i][5], graphline[i][6], graphline[i][5], graphline[i][7]);
		gDrawRectangle(graphline[i][5] - candlew, graphline[i][8], candlew * 2, graphline[i][9] - graphline[i][8], true);
//		gLogi("Data#") << i << " x: " << graphline[i][0] << " high: " << graphline[i][1] << " low: " << graphline[i][2] << " open: " << graphline[i][3] << " close: " << graphline[i][4];
	}

	renderer->setColor(oldcolor);
}

void gGUICandleStickChart::updatePoints() {
	if(graphline.empty()) return;
//	gLogi("update") << axisx1 << " " << axisxw << " " << axisy1 << " " << axisyh;

	int pointcount = graphline.size();
	float lengthy = maxy - miny;
	for(int i = 0; i < pointcount; i++) {
		graphline[i][5] = axisx1 + axisxw * (graphline[i][0] - minx) / (maxx - minx);
//		gLogi("values") << " " << graphline[i][0] << " " << axisx1 << " " << axisxw << " " << maxx << " " << minx;
		graphline[i][6] = axisy2 - axisyh * (graphline[i][1] - miny) / lengthy;
		graphline[i][7] = axisy2 - axisyh * (graphline[i][2] - miny) / lengthy;
		graphline[i][8] = axisy2 - axisyh * (graphline[i][3] - miny) / lengthy;
		graphline[i][9] = axisy2 - axisyh * (graphline[i][4] - miny) / lengthy;
	}
//	gLogi("update") << graphline[0][5] << " " << graphline[0][6] << " " << graphline[0][7] << " " << graphline[0][8] << " " << graphline[0][9];

}
