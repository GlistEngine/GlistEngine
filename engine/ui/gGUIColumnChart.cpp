/*
 * gGUIColumnChart.cpp
 *
 *  Created on: 11 Aðu 2022
 *      Author: Batuhan
 */

#include "gGUIColumnChart.h"

gGUIColumnChart::gGUIColumnChart() {
	chartx = 0;
	charty = 0;
	chartw = 0;
	charth = 0;
	columnh = 0;
	columnx = 0;
	columny = 0;
	indexcolumn = 0;
	indexmulticolumn = 0;
	initialmcspadding = 5;
	columnsw = 15;
	chartaspect = 0;
	chartliney = 0;
	numberrange = 0;
	numberrangey = 0;
	numberrangeaspect = 0;
	numbersinchart = "";
	colorindex = 0;
	columnnumber = 0;
}

gGUIColumnChart::~gGUIColumnChart() {

}

void gGUIColumnChart::draw() {
	drawLinesAndNumbers();
	gColor oldcolor = renderer->getColor();
	renderer->setColor(middlegroundcolor);
	gDrawLine(left + chartx , top + charty, left + chartx , top + charty - charth); //vertical line
	gDrawLine(left+ chartx , top + charty, left + chartx + chartw, top + charty); //horizontal
	drawColumns();
	drawMultiColumns();
	//drawLines();
	//gLogi("paramters") << multicolumnh[1][1];
	renderer->setColor(oldcolor);

}

void gGUIColumnChart::mouseMoved(int x, int y) {

}

void gGUIColumnChart::setChartProp(int chartx, int charty, int chartw, int charth, int chartnumberrange) {
	this->chartx =  chartx;
	this->charty = charty;
	this->chartw = chartw;
	this->charth = charth;
	this->columnx = chartx;
	this->columny = charty;
	this->multicolumnx = chartx + 20;
	this->multicolumny = charty;
	this->numberrange = chartnumberrange;
}

void gGUIColumnChart::addColumn(int columnh, gColor color) {
	std::vector<int> colors;
	colors.push_back(color.r);
	colors.push_back(color.g);
	colors.push_back(color.b);
	singlecolumscolor.push_back(colors);
	columnhe.push_back(columnh);
	this->columnh = columnh;
	//gDrawRectangle(left + columnx + 5, top + columny - columnh, 15, columnh, true);
	//gDrawRectangle(left + columnx + 5, top + columny - columnh, 15, columnh, true);
	columnx += 25;
	columnxe.push_back(columnx);
	indexcolumn += 1;

}

void gGUIColumnChart::drawColumns() {
	for(int i = 0 ; i < indexcolumn; i++) {
		gColor oldcolor = renderer->getColor();
		renderer->setColor(singlecolumscolor[i][0], singlecolumscolor[i][1], singlecolumscolor[i][2]);
		gDrawRectangle(left + columnxe[i], top + columny - columnhe[i], columnsw, columnhe[i], true);
		renderer->setColor(oldcolor);
	}
}

void gGUIColumnChart::addMultiColumn(int columnnum,int columnh, int columnh2, int columnh3, int columnh4, gColor color, gColor color2, gColor color3, gColor color4, std::string text) {
	columnnumber = columnnum;
	std::vector<int> colors;
	textvector.push_back(text);
	colors.push_back(color.r);
	colors.push_back(color.g);
	colors.push_back(color.b);
	colors.push_back(color2.r);
	colors.push_back(color2.g);
	colors.push_back(color2.b);
	colors.push_back(color3.r);
	colors.push_back(color3.g);
	colors.push_back(color3.b);
	colors.push_back(color4.r);
	colors.push_back(color4.b);
	colors.push_back(color4.g);
	multicolumscolor.push_back(colors);
	std::vector<int> newmulticolumnh;
	newmulticolumnh.push_back(columnh);
	newmulticolumnh.push_back(columnh2);
	newmulticolumnh.push_back(columnh3);
	newmulticolumnh.push_back(columnh4);
	multicolumnh.push_back(newmulticolumnh);
	multicolumnxe.push_back(multicolumnx);
	//multicolumnx += (columnh + columnh2 + columnh3 + columnh4) / 4;
	if(columnnum == 1) multicolumnx += columnsw + 20;
	else if(columnnum == 2) multicolumnx += columnsw * 2 + ((columnnum - 1) * (initialmcspadding)) + 35;     //55
	else if(columnnum == 3) multicolumnx += columnsw * 3 + ((columnnum - 1) * (initialmcspadding)) + 35;     //90
	else if(columnnum == 4) multicolumnx += columnsw * 4 + ((columnnum - 1) * (initialmcspadding)) + 35;    //130
	//multicolumnx += 130;


	//gLogi("paramters") << newmulticolumnh[0] << newmulticolumnh[1] << newmulticolumnh[2] << newmulticolumnh[3];
	indexmulticolumn += 1;
}

void gGUIColumnChart::drawMultiColumns() {
	for(int i = 0; i < indexmulticolumn; i++) {
		gColor oldcolor = renderer->getColor();
		renderer->setColor(middlegroundcolor);
		this->title = textvector[i];
		font->drawText(title, left + multicolumnxe[i] + ((columnsw * columnnumber + ((columnnumber - 1) * initialmcspadding ) - font->getStringWidth(title)) / 2), top + charty + 15);
		renderer->setColor(oldcolor);
		colorindex = 0;
		for(int j = 0; j < 4; j++) {
			renderer->setColor(multicolumscolor[i][colorindex], multicolumscolor[i][colorindex + 1], multicolumscolor[i][colorindex + 2]);
			gDrawRectangle(left + initialmcspadding + multicolumnxe[i] , top + columny - multicolumnh[i][j], columnsw, multicolumnh[i][j], true );
			initialmcspadding += 20;
			colorindex +=3;
		}
		renderer->setColor(oldcolor);
		initialmcspadding = 0;
	}
}


/*void gGUIColumnChart::drawLines() {
	for(int i = 0; i < indexmulticolumn; i++) {
		for(int j = 0; j < 4; j++) {
			gColor oldcolor = renderer->getColor();
			renderer->setColor(middlegroundcolor);
			gDrawLine(left+ chartx , top + charty - multicolumnh[i][j], left + chartx + chartw, top + charty - multicolumnh[i][j]);
			renderer->setColor(oldcolor);
		}
	}
}*/

void gGUIColumnChart::drawLinesAndNumbers() {
	numbersinchart = "";
	numberrangey = 0;
	chartliney = 0;
	chartaspect = charth / 10;
	numberrangeaspect = numberrange / 10;
	for(int i = 0; i < (charth / chartaspect) - 1; i++) {
		chartliney += chartaspect;
		numberrangey += numberrangeaspect;
		numbersinchart = gToStr(numberrangey);
		gColor oldcolor = renderer->getColor();
		renderer->setColor(middlegroundcolor);
		gDrawLine(left+ chartx , top + charty - chartliney, left + chartx + chartw, top + charty - chartliney);
		this->title = numbersinchart;
		font->drawText(title, left + chartx - font->getStringWidth(title) - 8, top + charty - chartliney);
		renderer->setColor(oldcolor);
		//gLogi("chartliney : ") << chartliney;
	}
}
