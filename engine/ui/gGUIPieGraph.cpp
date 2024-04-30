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
	additionallabely = 0;
	radius = 180.0f;
	cursordegree = -1;
	radiusreduction = 0.45f;
	oncursorcolorreduction = - 0.1f;
	rotationforothers = 0.0f;
	isFilled = true;
	isshown = true;
	numberofsidesratio = 60;
	sideofothers = 2;
	infotextshift = 10;
	fontsize = 9;
	outlinecolor = color.WHITE;
	otherscolor = color.GRAY;
	infotextcolor = color.BLACK;
	loadFont();
}

gGUIPieGraph::~gGUIPieGraph() {

}

void gGUIPieGraph::set(gBaseApp* root, gBaseGUIObject* topParentGUIObject, gBaseGUIObject* parentGUIObject, int parentSlotLineNo, int parentSlotColumnNo, int x, int y, int w, int h) {
	gGUIControl::set(root, topParentGUIObject, parentGUIObject, parentSlotLineNo, parentSlotColumnNo, x, y, w, h);
	int smaller = height;
	if(width < height) smaller = width;
	radius = smaller * 7 / 18;
	widthhalf = width / 2;
	heighthalf = height / 2;
}

void gGUIPieGraph::draw() {
	gColor oldcolor = *renderer->getColor();
	renderer->setColor(textbackgroundcolor);
	gDrawRectangle(left, top, width, height, true);

	float degree = 0.0f;
	float rotationdegree = rotationforothers;
	int i = 0;
	for(i = 0; i < variablevalues.size(); i++){
//	for(; i < variablevalues.size() - othersindex.size() - othersindex.empty(); i++){
		degree = degree + valuesdegree.at(i);
		renderer->setColor(variablecolors.at(i));
		if(cursordegree > rotationdegree && cursordegree < degree) arrangeOnCursor(i);
		gDrawArc(widthhalf + left, heighthalf + top, radius - radiusreduction, isFilled, valuessides[i], valuesdegree[i], rotationdegree);
		renderer->setColor(outlinecolor);
		gDrawArc(widthhalf + left, heighthalf + top, radius, !isFilled, valuessides[i], valuesdegree[i], rotationdegree);
		rotationdegree = rotationdegree + valuesdegree.at(i);

		//Labels
		renderer->setColor(255, 255, 255);
		font->drawText(labelstr[i], labelx[i], labely[i]);
	}
/*
	if(!othersindex.empty()) {
		renderer->setColor(otherscolor);
		if(cursordegree > rotationdegree && cursordegree < 360)
			arrangeOnCursor(-1);
		gDrawArc(widthhalf + left, heighthalf + top, radius - radiusreduction, isFilled, sideofothers, 360.0f - degree, rotationdegree);
		renderer->setColor(outlinecolor);
		gDrawArc(widthhalf + left, heighthalf + top, radius, !isFilled, sideofothers, 360.0f - degree, rotationdegree);
	}
	else {
		renderer->setColor(variablecolors.at(i));
		if(cursordegree > rotationdegree && cursordegree < 360)
			arrangeOnCursor(i);
		gDrawArc(widthhalf + left, heighthalf + top, radius - radiusreduction, isFilled, valuessides.at(i), 360.0f - degree, rotationdegree);
		renderer->setColor(outlinecolor);
		gDrawArc(widthhalf + left, heighthalf + top, radius, !isFilled, valuessides.at(i), 360.0f - degree, rotationdegree);
	}
*/

	showInfoOnCursor();
	renderer->setColor(oldcolor);
}

void gGUIPieGraph::mouseMoved(int x, int y) {
	cursorx = x;
	cursory = y;
	if(pow((x - widthhalf - left), 2.0f) + pow((y - heighthalf - top), 2.0f) < pow(radius, 2.0f)) {
		float cosedge = x - widthhalf - left;
		float sinedge = y - heighthalf - top;
		float r = sqrt(pow(sinedge, 2.0) + pow(cosedge, 2.0));
		if(x >= widthhalf + left && y >= heighthalf + top) cursordegree = std::asin(sinedge/r) * 180 / PI;
		else if(x < widthhalf + left && y >= heighthalf + top) cursordegree = 180 - std::asin(sinedge/r) * 180 / PI;
		else if(x < widthhalf + left && y < heighthalf + top) cursordegree = 180 - std::asin(sinedge/r) * 180 / PI;
		else if (x >= widthhalf + left && y < heighthalf + top) cursordegree = 360 + std::asin(sinedge/r) * 180 / PI;
	}
	else
		cursordegree = -1;
	//gLogi("gGUIPieGraph") << "mouseMoved" << ", x:" << x << ", y:" << y;
	//gLogi("gGUIPieGraph") << "cursordegree -> " << cursordegree;*/
}

void gGUIPieGraph::setRadius(float radius) {
	this->radius = radius;
	calculateLabelPositions();
}

float gGUIPieGraph::getRadius() {
	return radius;
}

void gGUIPieGraph::addVariable(std::string variableLabel, float variableValue) {
	addVariable(variableLabel, variableValue, gColor(0.4f + gRandom(0.3f), 0.4f + gRandom(0.3f), 0.4f + gRandom(0.3f)));
}

void gGUIPieGraph::addVariable(std::string variableLabel, float variableValue, gColor variableColor) {
	variablelabels.push_back(variableLabel);
	variablevalues.push_back(variableValue);
	variablecolors.push_back(variableColor);
	arrangePieGraph();
}

float gGUIPieGraph::getTotalValue(std::vector<float> vector) {
	float total = 0;
	for(int i = 0; i < vector.size(); i++) {
		total = total + vector.at(i);
	}
	if(total == 0)
		return 1;
	else
		return total;
}

void gGUIPieGraph::arrangePieGraph() {
	valuesdegree.clear();
	valuespercentage.clear();
	valuessides.clear();
	othersindex.clear();
	valuefortext.clear();
	percentagefortext.clear();
	labelstr.clear();

	int size = variablevalues.size();
	float totalvalue = getTotalValue(variablevalues);
	float degree;
	float percentage;
	float orherspercentage = 0;
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
	variablelabels = tempvariablelabels;
	variablevalues = tempvariablevalues;
	variablecolors = tempvariablecolors;
	for(int i = 0; i < variablelabels.size(); i++) {
		degree = (360.0f * variablevalues.at(i)) / totalvalue;
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
		std::string percentagestr;
		if(variablevalues.at(i) - int(variablevalues.at(i)) > 0) {
			value = std::to_string(variablevalues.at(i));
			valuefortext.push_back(value.substr(0, value.find(".")+3));
		}
		else {
			value = std::to_string((int)variablevalues.at(i));
			valuefortext.push_back(value);
		}
		percentagestr = std::to_string(valuespercentage.at(i));
		percentagefortext.push_back(gToStr((int)round(percentage)));

		std::string labelstrdef = variablelabels[i] + " (" + percentagefortext[i] + "%)";
		labelstr.push_back(labelstrdef);
		calculateLabelPositions();
	}
	if(orherspercentage < 0.5f && !othersindex.empty()) {
		valuesdegree.at(0) -= 0.5f;
		rotationforothers = -0.5f;
	}
}

void gGUIPieGraph::calculateLabelPositions() {
	if(variablelabels.empty()) return;
	labelx.clear();
	labely.clear();
	float degree;
	float totaldegree = 0;
	float totalvalue = getTotalValue(variablevalues);
	for(int i = 0; i < variablelabels.size(); i++) {
		degree = (360.0f * variablevalues.at(i)) / totalvalue;
		float meandeg = totaldegree + degree / 2.0f;
		float lx = widthhalf + (std::cos(gDegToRad(meandeg)) * radius / 2) - (font->getStringWidth("Group 0 (50%") / 2);
		float ly = heighthalf + additionallabely + (std::sin(gDegToRad(meandeg)) * radius / 2);
		labelx.push_back(lx);
		labely.push_back(ly);
		totaldegree += degree;
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

void gGUIPieGraph::setAdditionalLabelY(float diff) {
	additionallabely = diff;
}

void gGUIPieGraph::loadFont() {
	fontforinfotext.load(font->getPath(), fontsize, font->isAntialised(), font->getDpi());
}

void gGUIPieGraph::clear() {
	variablelabels.clear();
	variablevalues.clear();
	variablecolors.clear();

	valuesdegree.clear();
	valuespercentage.clear();
	valuessides.clear();
	othersindex.clear();
	labelstr.clear();
	labelx.clear();
	labely.clear();

	valuefortext.clear();
	percentagefortext.clear();
}
