/*
 * gGUIPieGraph.h
 *
 *  Created on: 27 Jul 2022
 *      Author: burakmeydan
 */

#ifndef UI_gGUIPieGraph_H_
#define UI_gGUIPieGraph_H_

#include "gGUIControl.h"

class gGUIPieGraph: public gGUIControl {
public:
	gGUIPieGraph();
	virtual ~gGUIPieGraph();

	void draw();
	void setRadius(float radius = 80.0f);
	void addVariable(std::string variableLabel, float variableValue);
	void addVariable(std::string variableLabel, float variableValue, gColor variableColor);
	void arrangePieGraph();
	void setOutLineColor(gColor color);
	void setInfoTextColor(gColor color);
	void setShowInfoOnCursor(bool isShown);
	void setInfoTextSize(int size);

private:
	float getTotalValue(std::vector<float> vector);
	void mouseMoved(int x, int y);
	void arrangeOnCursor(int index);
	void showInfoOnCursor();
	void loadFont();

	std::vector<std::string> variablelabels;
	std::vector<float> variablevalues;
	std::vector<gColor> variablecolors;

	std::vector<float> valuesdegree;
	std::vector<float> valuespercentage;
	std::vector<int> valuessides;
	std::vector<int> othersindex;

	std::vector<std::string> valuefortext;
	std::vector<std::string> percentagefortext;

	float radius, cursordegree = -1, radiusreduction = 0.45f, oncursorcolorreduction = - 0.1f, rotationforothers = 0.0f;
	bool isFilled = true, isshown = true;
	int numberofsidesratio = 60, sideofothers = 2, cursorx, cursory, infotextshift = 10, showinfoindex, fontsize = 9;
	gColor color;
	gColor outlinecolor = color.WHITE;
	gColor otherscolor = color.GRAY;
	gColor infotextcolor = color.BLACK;
	gFont fontforinfotext;
};

#endif /* UI_gGUIPieGraph_H_ */
