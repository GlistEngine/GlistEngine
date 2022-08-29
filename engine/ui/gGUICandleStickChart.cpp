/*
 * gGUICandleStickChart.cpp
 *
 *  Created on: 15 August 2022
 *      Author: Berke Adil
 */

#include "gGUICandleStickChart.h"

gGUICandleStickChart::gGUICandleStickChart() {


	title = "Button";
	cscolor = *buttoncolor;
	pointnum=0;
	candlew=10;
	locationnum=0;
	xx=110;


	resetTitlePosition();
}

gGUICandleStickChart::~gGUICandleStickChart() {

}

void gGUICandleStickChart::draw() {

	graph();
	addpoint();
	drawXAxis();
	drawYAxis();
}

void gGUICandleStickChart::drawXAxis() {				//Drawed in here for X Axis
	for(int i = 0; i < linesX.size(); i++) {
		renderer->setColor(gColor(0.0f, 0.0f, 0.0f, 1.0f));
		gDrawLine(linesX[i][0], linesX[i][1], linesX[i][2], linesX[i][3] );
	}
}

void gGUICandleStickChart::setXAxisValues(int min, int max, int range) {				//Add variables on the X-Axis
									//lineGraph.setXAxisValues(min, max, range);
	maxx=max;						//get maxx value
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

void gGUICandleStickChart::drawYAxis() {					//Drawed in here for Y Axis
	for(int i = 0; i < linesY.size(); i++) {
		renderer->setColor(gColor(0.0f, 0.0f, 0.0f, 1.0f));
		gDrawLine(linesY[i][0], linesY[i][1], linesY[i][2], linesY[i][3] );
	}
}

void gGUICandleStickChart::setYAxisValues(int min, int max, int range) {				//Add variables on the Y-Axis
								//	lineGraph.setYAxisValues(min, max, range);
	maxy=max;					//get maxy value
	int a;						//Necessary calculations for locations
	a = max - min;
	a = (a / range);

	for(int i = 0; i <= range; i++) {				//LineX always push back in order to draw another range point
		std::vector<int> lineY;
		//lineX.draw(400 + min, 295, 400, 305);
		lineY.push_back(95); //lines[i][0]
		lineY.push_back(500 - min); //lines[i][1]
		lineY.push_back(105); //lines[i][2]
		lineY.push_back(500 - min); //lines[i][2]

		linesY.push_back(lineY);
		min = min + a;
	}
}

void gGUICandleStickChart::graph() {					// Graph with 2 straight lines
														// get maxx and maxy values for draw with accurate

	gColor oldcolor = *renderer->getColor();
	renderer->setColor(&cscolor);
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

void gGUICandleStickChart::addValue(float y, float ch, float open, float close) {
	xAxis[pointnum]=xx;						//get x axis
	this->y[pointnum] = 500-y;
	this->ch[pointnum] = -ch;				// used minus value for y axis.(y axi start from top ends on bot but graph needs the opposite
	this->open[pointnum] = -open;
	this->close[pointnum] = close;
	pointnum++;
	xx=xx+10;							//body has 10pixel width. every candlestick has to 10pixel away from each other
}


void gGUICandleStickChart::addpoint() {				//adds points on graph
													//for everypointnum draw a candlestick
	for(int i = 0; i < pointnum; i++) {

		if(ch[i]<0 && i==0) {															//body is larger means draw green body. first value
			renderer->setColor(gColor(0.0f,	1.0f, 0.0f, 1.0f));							//color
			gDrawRectangle(xAxis[i], y[i], candlew, ch[i], true);						//rectangle. body of candlestick
			gDrawLine(xAxis[i]+5, y[i]+ch[i], xAxis[i]+5, y[i]+ch[i]+open[i]);			//open line
			gDrawLine(xAxis[i]+5, y[i], xAxis[i]+5, y[i]+close[i]);						// close line
		}
		if(ch[i]>0 && i==0) {															// body is larger means draw red body. first value
			renderer->setColor(gColor(1.0f,	0.0f, 0.0f, 1.0f));							// color
			gDrawRectangle(xAxis[i]+5, y[i], candlew, ch[i], true);						//rectangle body
			gDrawLine(xAxis[i]+5, y[i]+ch[i], xAxis[i]+5, y[i]+ch[i]-open[i]);			//open line
			gDrawLine(xAxis[i]+5, y[i], xAxis[i]+5, y[i]-close[i]);						//close line
		}
		if(ch[i]<0 && i!=0) {															//body is larger. other values.
			renderer->setColor(gColor(0.0f,	1.0f, 0.0f, 1.0f));
			gDrawRectangle(xAxis[i]+10, y[i], candlew, ch[i], true);
			gDrawLine(xAxis[i]+15, y[i]+ch[i], xAxis[i]+15, y[i]+ch[i]+open[i]);
			gDrawLine(xAxis[i]+15, y[i], xAxis[i]+15, y[i]+close[i]);
		}
		if(ch[i]>0 && i!=0) {															//body is smaller. other values.
			renderer->setColor(gColor(1.0f,	0.0f, 0.0f, 1.0f));
			gDrawRectangle(xAxis[i]+10, y[i], candlew, ch[i], true);
			gDrawLine(xAxis[i]+15, y[i]+ch[i], xAxis[i]+15, y[i]+ch[i]-open[i]);
			gDrawLine(xAxis[i]+15, y[i], xAxis[i]+15, y[i]-close[i]);
		}
	}
}

void gGUICandleStickChart::setGraphColor(gColor color) {			//.setGraphColor(gColor(1.0f, 1.0f, 0.0f, 1.0f));
	cscolor = color;
}

void gGUICandleStickChart::resetTitlePosition() {

}

void gGUICandleStickChart::update() {

}

void gGUICandleStickChart::setSize(int width, int height) {
	candlew = width;
	candleh = height;
	resetTitlePosition();
}


gColor* gGUICandleStickChart::getGraphColor() {
	return &cscolor;
}
