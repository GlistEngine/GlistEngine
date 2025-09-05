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
	columnsw = 0;
	chartaspect = 0;
	chartliney = 0;
	numberrange = 0;
	numberrangey = 0;
	numberrangeaspect = 0;
	numbersinchart = "";
	colorindex = 0;
	columnnumber = 0;
	gridlineison = 1;
	numbersison = 1;
	textison = 1;
}

gGUIColumnChart::~gGUIColumnChart() {

}

void gGUIColumnChart::draw() {
	drawLinesAndNumbers();
	int chartyorigin = top + charty - (chartaspect * 10);
	gColor oldcolor = renderer->getColor();
	renderer->setColor(middlegroundcolor);
	gDrawLine(left + chartx , top + charty, left + chartx , top + charty - charth); //vertical line
	gDrawLine(left+ chartx , chartyorigin, left + chartx + chartw, chartyorigin); //horizontal
	drawColumns();
	drawMultiColumns();
	renderer->setColor(oldcolor);

}

void gGUIColumnChart::mouseMoved(int x, int y) {

}

void gGUIColumnChart::setChartProp(int chartx, int charty, int chartw, int charth,int columnsw, int columnsmargin, int chartnumberrange) {
	this->chartx =  chartx;
	this->charty = charty;
	this->chartw = chartw;
	this->charth = charth;
	this->columnx = chartx;
	this->columny = charty;
	this->multicolumnx = chartx + 20;
	this->multicolumny = charty;
	this->numberrange = chartnumberrange;
	this->columnsw = columnsw;
	this->columnsmargin = columnsmargin;
}

void gGUIColumnChart::setChartProp(int chartX, int chartY, int chartW, int chartH, int columnsW, int columnsMargin, int minNumRangeX, int maxNumRangeX, int minNumRangeY, int maxNumRangeY) {
	this->chartx = chartX;
	this->charty = chartY;
	this->chartw = chartW;
	this->charth = chartH;
	this->columnx = chartX;
	this->columny = chartY;
	this->multicolumnx = chartX + 20;
	this->multicolumny = chartY;
	this->numberrange = maxNumRangeY - minNumRangeY;
	this->columnsw = columnsW;
	this->columnsmargin = columnsMargin;
	this->minnumrangex = minNumRangeX;
	this->maxnumrangex = maxNumRangeX;
	this->minnumrangey = minNumRangeY;
	this->maxnumrangey = maxNumRangeY;
}

void gGUIColumnChart::addColumn(int columnh, gColor color) {
	std::vector<int> colors;
	colors.push_back(color.r);
	colors.push_back(color.g);
	colors.push_back(color.b);
	singlecolumscolor.push_back(colors);
	columnhe.push_back(columnh);
	this->columnh = columnh;
	columnx += columnsmargin / 2 + 5;
	columnxe.push_back(columnx);
	indexcolumn += 1;

}

void gGUIColumnChart::drawColumns() {
	int minnumberrangeaspect = charth / numberrange;
	int chartyorigin = top + charty - (abs(minnumrangey) * minnumberrangeaspect);
	for(int i = 0 ; i < indexcolumn; i++) {
		gColor oldcolor = renderer->getColor();
		renderer->setColor(singlecolumscolor[i][0], singlecolumscolor[i][1], singlecolumscolor[i][2]);
		gDrawRectangle(left + columnxe[i], chartyorigin - columnhe[i], columnsw, columnhe[i], true);
		renderer->setColor(oldcolor);
	}
}

void gGUIColumnChart::addMultiColumn(int columnnum,int columnh, int columnh2, int columnh3, int columnh4, gColor color, gColor color2, gColor color3, gColor color4, std::string text, int multicolumnspadding) {
	this->multicolumnspadding = multicolumnspadding;
	multicolumnspaddingvector.push_back(multicolumnspadding);
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
	if(columnnum == 1) multicolumnx += columnsw + columnsmargin;
	else if(columnnum == 2) multicolumnx += columnsw * columnnum + ((columnnum - 1) * (initialmcspadding)) + columnsmargin;     //55
	else if(columnnum == 3) multicolumnx += columnsw * columnnum + ((columnnum - 1) * (initialmcspadding)) + columnsmargin;     //90
	else if(columnnum == 4) multicolumnx += columnsw * columnnum + ((columnnum - 1) * (initialmcspadding)) + columnsmargin;    //130
	//multicolumnx += 130;


	//gLogi("paramters") << newmulticolumnh[0] << newmulticolumnh[1] << newmulticolumnh[2] << newmulticolumnh[3];
	indexmulticolumn += 1;
}

void gGUIColumnChart::drawMultiColumns() {
	for(int i = 0; i < indexmulticolumn; i++) {
		gColor oldcolor = renderer->getColor();
		renderer->setColor(middlegroundcolor);
		if(textison) {
			this->title = textvector[i];
			font->drawText(title, left + multicolumnxe[i] + ((columnsw * columnnumber + ((columnnumber - 1) * initialmcspadding ) - font->getStringWidth(title)) / 2), top + charty + 15); //columnnumberý vectore dönüþtür
		}
		renderer->setColor(oldcolor);
		colorindex = 0;
		for(int j = 0; j < 4; j++) {
			renderer->setColor(multicolumscolor[i][colorindex], multicolumscolor[i][colorindex + 1], multicolumscolor[i][colorindex + 2]);
			gDrawRectangle(left + initialmcspadding + multicolumnxe[i] , top + columny - multicolumnh[i][j], columnsw, multicolumnh[i][j], true );
			initialmcspadding += multicolumnspaddingvector[i];
			colorindex +=3;
		}
		renderer->setColor(oldcolor);
		initialmcspadding = 0;
	}
}

void gGUIColumnChart::drawLinesAndNumbers() {
	numbersinchart = "";
	numberrangey = 0;
	chartliney = 0;
	chartaspect = charth / 20;
	numberrangeaspect = numberrange / 20;
	if(gridlineison) { for(int i = 0; i < (charth / chartaspect) - 1; i++) {
			chartliney += chartaspect;
			gColor oldcolor = renderer->getColor();
			renderer->setColor(middlegroundcolor);
			gDrawLine(left+ chartx , top + charty - chartliney, left + chartx + chartw, top + charty - chartliney);
			renderer->setColor(oldcolor);
			//gLogi("chartliney : ") << chartliney;
		}
	}
	chartliney = 0;
	if(numbersison)	{ for(int i = 1; i < (charth / chartaspect); i++) {
			chartliney += chartaspect;
			numberrangey += numberrangeaspect;
			numbersinchart = gToStr(numberrangey);
			gColor oldcolor = renderer->getColor();
			renderer->setColor(middlegroundcolor);
			this->title = numbersinchart;
			font->drawText(title, left + chartx - font->getStringWidth(title) - 8, top + charty - chartliney);
			renderer->setColor(oldcolor);
			//gLogi("chartliney : ") << chartliney;
		}
	}
}

void gGUIColumnChart::setVisibility(bool gridlineison, bool numbersison, bool textison) { // If this function doesn't use, boolean variables will be accepted as TRUE
	this->gridlineison = gridlineison;
	this->numbersison = numbersison;
	this->textison = textison;
}

void gGUIColumnChart::clear() {
	columnhe.clear();
	columnxe.clear();
	multicolumnh.clear();
	multicolumnxe.clear();
	singlecolumscolor.clear();
	multicolumscolor.clear();
	textvector.clear();
	multicolumnspaddingvector.clear();
}
