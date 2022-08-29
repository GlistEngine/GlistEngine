/*
 * gGUILineGraph.cpp
 *
 *  Created on: 28 July 2022
 *      Author: Berke Adil
 */


#include "gGUILineGraph.h"
#include "gGUISlider.h"


gGUILineGraph::gGUILineGraph() {

	ispressed = false;
	lpw = 2;
	lph = 2;
	istoggle = false;
	title = "Button";
	istextvisible = true;
	ispressednow = false;
	isdisabled = false;
	lpcolor = *buttoncolor;
	lgcolor = *buttoncolor;
	mlcolor = *buttoncolor;
	pointnum = 0;
	multinum = 0;
	maxx=0;
	maxy=0;


	resetTitlePosition();
}

gGUILineGraph::~gGUILineGraph() {

}

void gGUILineGraph::draw() {

	graph();
	addpoint();
	linedrawer();
	drawXAxis();
	drawYAxis();

}

void gGUILineGraph::drawXAxis() {				//Drawed in here for X Axis
	for(int i = 0; i < linesX.size(); i++) {
		renderer->setColor(gColor(0.0f, 0.0f, 0.0f, 1.0f));
		gDrawLine(linesX[i][0], linesX[i][1], linesX[i][2], linesX[i][3] );
	}
}

void gGUILineGraph::setXAxisValues(int min, int max, int range) {				//Add variables on the X-Axis
									//lineGraph.setXAxisValues(min, max, range);
	maxx=max;						// get maxx value
	int a;							//Necessary calculations for locations
	a = max - min;
	a = (a / range);


	for(int i = 0; i <= range; i++) {						//LineX always push back in order to draw another range point
		std::vector<int> lineX;
		lineX.push_back(100 + min);
		lineX.push_back(495);
		lineX.push_back(100 + min);
		lineX.push_back(505);

		linesX.push_back(lineX);
		min = min + a;
	}
}

void gGUILineGraph::drawYAxis() {					//Drawed in here for Y Axis  ölçkelendir renklendir... addvalue 4 parametreli olacak.
	for(int i = 0; i < linesY.size(); i++) {
		renderer->setColor(gColor(0.0f, 0.0f, 0.0f, 1.0f));
		gDrawLine(linesY[i][0], linesY[i][1], linesY[i][2], linesY[i][3] );
	}
}

void gGUILineGraph::setYAxisValues(int min, int max, int range) {				//Add variables on the Y-Axis
								//	lineGraph.setYAxisValues(min, max, range);
	maxy=max;					// get maxy value
	int a;						//Necessary calculations for locations
	a = max - min;
	a = (a / range);

	for(int i = 0; i <= range; i++) {				//LineX always push back in order to draw another range point
		std::vector<int> lineY;
		lineY.push_back(95);
		lineY.push_back(500 - min);
		lineY.push_back(105);
		lineY.push_back(500 - min);

		linesY.push_back(lineY);
		min = min + a;
	}
}

void gGUILineGraph::graph() {					// Graph with 2 straight lines
												// get maxx and maxy values for draw with accurate

	gColor oldcolor = *renderer->getColor();
	renderer->setColor(&lgcolor);
	this->maxy = maxy;
	this->maxx = maxx;
	if(maxx>0) {
		gDrawLine(100, 500, maxx+100, 500);
	}
	if(maxy>0) {
		gDrawLine(100, 500, 100, 500-maxy);
	}
	else
		gDrawLine(100, 500, 500, 500);
		gDrawLine(100, 500, 100, 100);

}

void gGUILineGraph::addValue(float x, float y) {  // points get their x and y locations on thee surface
													// 	lineGraph.addValue(x location, y location);
	this->x[pointnum] = x+100;
	this->y[pointnum] = 500-y;
	pointnum++;
}

void gGUILineGraph::addpoint() {				//adds points on graph
	for(int i = 0; i < pointnum; i++) {
		renderer->setColor(gColor(0.0f, 0.0f, 0.0f, 0.0f));
		gDrawCircle((x[i]), (y[i]), 5, true, 10);
	}
}

void gGUILineGraph::linedrawer() {
	//connect points with one line
	gColor oldcolor = *renderer->getColor();
	renderer->setColor(&lgcolor);
	for(int i = 0; i < pointnum; i++) {
		if(i==0) {
			gDrawCircle((x[i]), (y[i]), 5, true, 10);			//draw circle to the first point to look like a line without start from origin.
		}

		else {
			gDrawLine(x[i-1], y[i-1], x[i], y[i]);
		}
	}
}

void gGUILineGraph::addMultiValue(float title, float mx, float my) {  // points get their x and y locations on thee surface
													// 	lineGraph.addValue(x location, y location);
	this->mx[multinum] = mx+100;
	this->my[multinum] = 500-my;
	multinum++;
}

void gGUILineGraph::multiline() {				//not working

	/*for(int i = 0; i <multinum; i++) {
		if(i==0) {
			gDrawCircle(mx[i], my[i], 5, true, 10);
			}
		else {
			gDrawLine(mx[i-1], my[i-1], mx[i], my[i]);
			gDrawCircle(mx[i], my[i], 5, true, 10);
			}
	}*/
}

void gGUILineGraph::addMultiPoint() {					//not working
	for(int i = 0; i < multinum; i++) {
		renderer->setColor(gColor(1.0f, 1.0f, 1.0f, 1.0f));
		gDrawCircle(mx[i], my[i], 5, true, 10);
	}
}


void gGUILineGraph::setGraphColor(gColor color) {			//lineGraph.setGraphColor(gColor(1.0f, 1.0f, 0.0f, 1.0f));
	lgcolor = color;
}

void gGUILineGraph::setLineDrawerColor(gColor color) {			//lineGraph.setGraphColor(gColor(1.0f, 1.0f, 0.0f, 1.0f));
	ldcolor = color;
}

void gGUILineGraph::setmultiLineColor(gColor color) {			//for multiline color


}


void gGUILineGraph::resetTitlePosition() {

}

void gGUILineGraph::setSize(int width, int height) {
	lpw = width;
	lph = height;
	resetTitlePosition();
}

void gGUILineGraph::setDisabled(bool isDisabled) {
	isdisabled = isDisabled;
}

bool gGUILineGraph::isDisabled() {
	return isdisabled;
}

void gGUILineGraph::update() {
//	gLogi("gGUIButton") << "update";
}


void gGUILineGraph::setLinePartColor(gColor color) {
	lpcolor = color;
}

void gGUILineGraph::setDisabledLinePartColor(gColor color) {
	disabledlpcolor = color;
}

gColor* gGUILineGraph::getGraphColor() {
	return &lgcolor;
}

gColor* gGUILineGraph::getLinePartColor() {
	return &lpcolor;
}

gColor* gGUILineGraph::getDisabledLinePartColor() {
	return &disabledlpcolor;
}

gColor* gGUILineGraph::getmultiLineColor() {
	return &mlcolor;
}
