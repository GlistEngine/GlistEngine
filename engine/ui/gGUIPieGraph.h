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

	void set(gBaseApp* root, gBaseGUIObject* topParentGUIObject, gBaseGUIObject* parentGUIObject, int parentSlotLineNo, int parentSlotColumnNo, int x, int y, int w, int h);

	void draw();

	void setRadius(float radius);
	float getRadius();

	void addVariable(std::string variableLabel, float variableValue);
	void addVariable(std::string variableLabel, float variableValue, gColor variableColor);

	void setOutLineColor(gColor color);
	void setInfoTextColor(gColor color);
	void setShowInfoOnCursor(bool isShown);
	void setInfoTextSize(int size);
	void setAdditionalLabelY(float diff);
	void setPredictedOutputs(const std::vector<int>& outs);
	void setPredictedOutputName(int id, const std::string& label);

	void clear();

private:
	float getTotalValue(std::vector<float> vector);
	void mouseMoved(int x, int y);
	void arrangeOnCursor(int index);
	void showInfoOnCursor();
	void loadFont();
	void arrangePieGraph();
	void calculateLabelPositions();

	std::vector<std::string> variablelabels;
	std::vector<float> variablevalues;
	std::vector<gColor> variablecolors;

	std::vector<float> valuesdegree;
	std::vector<float> valuespercentage;
	std::vector<int> valuessides;
	std::vector<int> othersindex;
	std::vector<std::string> labelstr;
	std::vector<int> labelx, labely;
	std::vector<int> predictedOutputs;
	std::vector<std::string> valuefortext;
	std::vector<std::string> percentagefortext;

	float radius, cursordegree, radiusreduction, oncursorcolorreduction, rotationforothers;
	bool isFilled, isshown;
	int numberofsidesratio, sideofothers, cursorx, cursory, infotextshift, showinfoindex, fontsize;
	gColor color;
	gColor outlinecolor, otherscolor, infotextcolor;
	gFont fontforinfotext;
	float additionallabely;
	int widthhalf, heighthalf;
	bool setPredictedOutputNameRequested;
};

#endif /* UI_gGUIPieGraph_H_ */
