/*
 * gGUIPieGraph.cpp
 *
 *  Created on: 27 Jul 2022
 *      Author: burakmeydan
 */

#include "gGUIPieGraph.h"
#include "gBaseApp.h"
#include "gBaseCanvas.h"

gGUIPieGraph::gGUIPieGraph() {
	setRadius();
	loadFont();
}

gGUIPieGraph::~gGUIPieGraph() {

}

void gGUIPieGraph::draw() {
	float degree = 0.0f;
	float rotationdegree = rotationforothers;
	int i = 0;
	for(; i < variablevalues.size() - othersindex.size() - othersindex.empty(); i++){
		degree = degree + valuesdegree.at(i);
		renderer->setColor(variablecolors.at(i));
		if(cursordegree > rotationdegree && cursordegree < degree) arrangeOnCursor(i);
		gDrawArc((width/2) + left, (height/2) + top, radius - radiusreduction, isFilled, valuessides.at(i), valuesdegree.at(i), rotationdegree);
		renderer->setColor(outlinecolor);
		gDrawArc((width/2) + left, (height/2) + top, radius, !isFilled, valuessides.at(i), valuesdegree.at(i), rotationdegree);
		rotationdegree = rotationdegree + valuesdegree.at(i);
	}
	if(!othersindex.empty()) {
		renderer->setColor(otherscolor);
		if(cursordegree > rotationdegree && cursordegree < 360) arrangeOnCursor(-1);
		gDrawArc((width/2) + left, (height/2) + top, radius - radiusreduction, isFilled, sideofothers, 360.0f - degree, rotationdegree);
		renderer->setColor(outlinecolor);
		gDrawArc((width/2) + left, (height/2) + top, radius, !isFilled, sideofothers, 360.0f - degree, rotationdegree);
	}
	else {
		renderer->setColor(variablecolors.at(i));
		if(cursordegree > rotationdegree && cursordegree < 360) arrangeOnCursor(i);
		gDrawArc((width/2) + left, (height/2) + top, radius - radiusreduction, isFilled, valuessides.at(i), 360.0f - degree, rotationdegree);
		renderer->setColor(outlinecolor);
		gDrawArc((width/2) + left, (height/2) + top, radius, !isFilled, valuessides.at(i), 360.0f - degree, rotationdegree);
	}
	showInfoOnCursor();
}

void gGUIPieGraph::mouseMoved(int x, int y) {
	cursorx = x;
	cursory = y;
	if(pow((x - (width/2) - left), 2.0f) + pow((y - (height/2) - top), 2.0f) < pow(radius, 2.0f)) {
		float cosedge = x - (width/2) - left;
		float sinedge = y - (height/2) - top;
		float r = sqrt(pow(sinedge, 2.0) + pow(cosedge, 2.0));
		if(x >= (width/2) + left && y >= (height/2) + top) cursordegree = std::asin(sinedge/r) * 180 / PI;
		else if(x < (width/2) + left && y >= (height/2) + top) cursordegree = 180 - std::asin(sinedge/r) * 180 / PI;
		else if(x < (width/2) + left && y < (height/2) + top) cursordegree = 180 - std::asin(sinedge/r) * 180 / PI;
		else if (x >= (width/2) + left && y < (height/2) + top) cursordegree = 360 + std::asin(sinedge/r) * 180 / PI;
	}
	else cursordegree = -1;
	//gLogi("gGUIPieGraph") << "mouseMoved" << ", x:" << x << ", y:" << y;
	//gLogi("gGUIPieGraph") << "cursordegree -> " << cursordegree;
}

void gGUIPieGraph::setRadius(float radius) {
	this->radius = radius;
}

void gGUIPieGraph::addVariable(std::string variableLabel, float variableValue) {
	variablelabels.push_back(variableLabel);
	variablevalues.push_back(variableValue);
	float randomr = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
	float randomg = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
	float randomb = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
	color.set(randomr, randomg, randomb, 1.0f);
	variablecolors.push_back(color);
	arrangePieGraph();
	//gLogi("gGUIPieGraph.h") << "The added variable -> " << variableLabel << ", " << variableValue;
	//gLogi("gGUIPieGraph.h") << "Color rgb -> " << color.r << ", " << color.g << ", " << color.b << ", " << color.a;
}

void gGUIPieGraph::addVariable(std::string variableLabel, float variableValue, gColor variableColor) {
	variablelabels.push_back(variableLabel);
	variablevalues.push_back(variableValue);
	variablecolors.push_back(variableColor);
	arrangePieGraph();
	//gLogi("gGUIPieGraph.h") << "The added variable -> " << variableLabel << ", " << variableValue;
	//gLogi("gGUIPieGraph.h") << "Color rgb -> " << variableColor.r << ", " << variableColor.g << ", " << variableColor.b << ", " << variableColor.a;
}

float gGUIPieGraph::getTotalValue(std::vector<float> vector) {
	float total = 0;
	for(int i = 0; i < vector.size(); i++) {
		total = total + vector.at(i);
	}
	if(total == 0) return 1;
	else return total;
}

void gGUIPieGraph::arrangePieGraph() {
	this->valuesdegree.clear();
	this->valuespercentage.clear();
	this->valuessides.clear();
	this->othersindex.clear();
	this->valuefortext.clear();
	this->percentagefortext.clear();

	int size = variablevalues.size();
	float totalvalue = getTotalValue(variablevalues);
	float degree;
	float percentage;
	float orherspercentage;
	int maxelementindex;
	int side;

	std::vector<std::string> tempvariablelabels;
	std::vector<float> tempvariablevalues;
	std::vector<gColor> tempvariablecolors;
	for(int i = 0; i < size; i++) {
		maxelementindex = std::max_element(variablevalues.begin(), variablevalues.end()) - variablevalues.begin();
		tempvariablelabels.push_back(variablelabels.at(maxelementindex));
		tempvariablevalues.push_back(variablevalues.at(maxelementindex));
		tempvariablecolors.push_back(variablecolors.at(maxelementindex));
		variablevalues.erase(variablevalues.begin() + maxelementindex);
		variablelabels.erase(variablelabels.begin() + maxelementindex);
		variablecolors.erase(variablecolors.begin() + maxelementindex);
	}
	this->variablelabels = tempvariablelabels;
	this->variablevalues = tempvariablevalues;
	this->variablecolors = tempvariablecolors;
	for(int i = 0; i < variablelabels.size(); i++) {
		degree = 360 * variablevalues.at(i) / totalvalue;
		percentage = degree / 360 * 100;
		side = percentage < 1 ? 1 : ceil((degree / 360) * numberofsidesratio);
		if(percentage < 0.5f) {
			othersindex.push_back(i);
			orherspercentage += percentage;
		}
		valuesdegree.push_back(degree);
		valuespercentage.push_back(percentage);
		valuessides.push_back(side);
		std::string value;
		std::string percentage;
		if(variablevalues.at(i) - int(variablevalues.at(i)) > 0) {
			value = std::to_string(variablevalues.at(i));
			valuefortext.push_back(value.substr(0, value.find(".")+3));
		}
		else {
			value = std::to_string((int)variablevalues.at(i));
			valuefortext.push_back(value);
		}
		percentage = std::to_string(valuespercentage.at(i));
		percentagefortext.push_back(percentage.substr(0, percentage.find(".")+3));
		//gLogi("gGUIPieGraph.h") << "Label -> " << variablelabels.at(i) << ", Value -> " << variablevalues.at(i) << ", Degree -> " << degree << ", Percantage -> " << percentage << ", Side -> " << side;
		//gLogi("gGUIPieGraph.h") << "Color rgb -> " << variablecolors.at(i).r << ", " << variablecolors.at(i).g << ", " << variablecolors.at(i).b << ", " << variablecolors.at(i).a;
	}
	if(orherspercentage < 0.5f) {
		valuesdegree.at(0) -= 0.5f;
		rotationforothers = -0.5f;
	}
}

void gGUIPieGraph::setOutLineColor(gColor color) {
	outlinecolor = color;
}

void gGUIPieGraph::setInfoTextColor(gColor color) {
	infotextcolor = color;
}

void gGUIPieGraph::setShowInfoOnCursor(bool isShown) {
	isshown = isShown;
}

void gGUIPieGraph::arrangeOnCursor(int index) {
		color = renderer->getColor();
		color.set(color.r + oncursorcolorreduction, color.g + oncursorcolorreduction, color.b + oncursorcolorreduction);
		renderer->setColor(color);
		showinfoindex = index;
}

void gGUIPieGraph::showInfoOnCursor() {
	if(isshown && cursordegree != -1) {
		renderer->setColor(infotextcolor);
		std::string text = "";
		std::string infotext = "";
		int maxwidth;
		if(showinfoindex == -1)
			for(int i = 0; i < othersindex.size(); i++) {
				text = variablelabels.at(othersindex.at(i)) + ": " + valuefortext.at(othersindex.at(i)) + " (%" + percentagefortext.at(othersindex.at(i)) + ")\n";
				infotext += text;
				if(maxwidth < fontforinfotext.getStringWidth(text))
					maxwidth = fontforinfotext.getStringWidth(text);
			}
		else {
			infotext = variablelabels.at(showinfoindex) + ": " + valuefortext.at(showinfoindex) + " (%" + percentagefortext.at(showinfoindex) + ")";
			maxwidth = fontforinfotext.getStringWidth(infotext);
		}
		if(cursorx + infotextshift + maxwidth <= left + width)
			fontforinfotext.drawText(infotext, cursorx + infotextshift, cursory + infotextshift);
		else if(cursorx - infotextshift - maxwidth > left)
			fontforinfotext.drawText(infotext, cursorx - infotextshift - maxwidth, cursory + infotextshift);
		else
			fontforinfotext.drawText(infotext, left, cursory - infotextshift);
	}
}

void gGUIPieGraph::setInfoTextSize(int size) {
	fontsize = size;
	loadFont();
}

void gGUIPieGraph::loadFont() {
	fontforinfotext.load(font->getPath(), fontsize, font->isAntialised(), font->getDpi());
}
