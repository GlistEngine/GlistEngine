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
	pointnum = 0;


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

	int a;							//Necessary calculations for locations
	a = max - min;
	a = (a / range);

	for(int i = 0; i <= range; i++) {						//LineX always push back in order to draw another range point
		std::vector<int> lineX;
		//lineX.draw(400 + min, 295, 400, 305);
		lineX.push_back(300 + min); //lines[i][0]
		lineX.push_back(395); //lines[i][1]
		lineX.push_back(300 + min); //lines[i][2]
		lineX.push_back(405); //lines[i][2]

		linesX.push_back(lineX);
		min = min + a;
	}
}

void gGUILineGraph::drawYAxis() {					//Drawed in here for Y Axis
	for(int i = 0; i < linesY.size(); i++) {
		renderer->setColor(gColor(0.0f, 0.0f, 0.0f, 1.0f));
		gDrawLine(linesY[i][0], linesY[i][1], linesY[i][2], linesY[i][3] );
	}
}

void gGUILineGraph::setYAxisValues(int min, int max, int range) {				//Add variables on the Y-Axis

	int a;						//Necessary calculations for locations
	a = max - min;
	a = (a / range);

	for(int i = 0; i <= range; i++) {				//LineX always push back in order to draw another range point
		std::vector<int> lineY;
		//lineX.draw(400 + min, 295, 400, 305);
		lineY.push_back(295); //lines[i][0]
		lineY.push_back(400 - min); //lines[i][1]
		lineY.push_back(305); //lines[i][2]
		lineY.push_back(400 - min); //lines[i][2]

		linesY.push_back(lineY);
		min = min + a;
	}
}

void gGUILineGraph::graph() {					// Graph with 2 straight lines

	gColor oldcolor = *renderer->getColor();
	renderer->setColor(&lgcolor);
	line1.draw(300, 400, 600, 400);
	line2.draw(300,400,300,100);

}

void gGUILineGraph::addValue(float x, float y) {  // points get their x and y locations on thee surface

	this->x[pointnum] = x;
	this->y[pointnum] = y;
	pointnum++;

}

void gGUILineGraph::addpoint() {				//adds points on graph
	for(int i = 0; i < pointnum; i++) {
		renderer->setColor(gColor(0.0f, 0.0f, 0.0f, 0.0f));
		points[i].draw((300+x[i]), (400-y[i]), lpw, lph, true);
	}

}

void gGUILineGraph::linedrawer() {			//connect points with one line
	for(int i = 0; i < pointnum; i++) {
		if(i==0) {
			renderer->setColor(gColor(1.0f, 0.0f, 1.0f, 1.0f));
			lines[i].draw(300, 400, 300 + x[i], 400 - y[i]);
		}

		else {
			renderer->setColor(gColor(1.0f, 0.0f, 1.0f, 1.0f));
			lines[i].draw(300 + x[i-1], 400 - y[i-1], 300 + x[i], 400 - y[i]);
		}
	}
}

void gGUILineGraph::setGraphColor(gColor color) {
	lgcolor = color;
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
