/*
 * gGUIGraph.h
 *
 *  Created on: Sep 23, 2022
 *      Author: noyan
 */

#ifndef UI_GGUIGRAPH_H_
#define UI_GGUIGRAPH_H_

#include "gGUIControl.h"

/**
 * This is a base class for all Graph classes. Developers can use the child classes
 * of this class for their graph needs.
 */
class gGUIGraph: public gGUIControl {
public:
	gGUIGraph();
	virtual ~gGUIGraph();

	virtual void set(gBaseApp* root, gBaseGUIObject* topParentGUIObject, gBaseGUIObject* parentGUIObject, int parentSlotLineNo, int parentSlotColumnNo, int x, int y, int w, int h);

	virtual void setMaxX(int maxX);
	int getMaxX();
	virtual void setMinX(int minX);
	int getMinX();
	virtual void setMaxY(int maxY);
	int getMaxY();
	virtual void setMinY(int minY);
	int getMinY();

	void enableGridlinesX(bool isEnabled);
	void enableGridlinesY(bool isEnabled);

	void setTitleX(std::string titleX);
	std::string getTitleX();
	void setTitleY(std::string titleY);
	std::string getTitleY();

	virtual void setLabelCountX(int labelCount);
	int getLabelCountX();
	virtual void setLabelCountY(int labelCount);
	int getLabelCountY();

	int calculateStepSize(int step);

	void draw();

protected:
	float axisx1, axisy1, axisx2, axisy2, axisxstart, axisystart;
	float axisxw, axisyh;
	float maxy, miny, maxx, minx;
	float largestvaluex, largestvaluey, smallestvaluex, smallestvaluey;
	bool gridlinesxenabled, gridlinesyenabled;
	bool floatlabelsenabled;
	int labelcountx, labelcounty;


private:
	void drawBackground();
	void drawLabels();
	virtual void drawGraph();
	void updateLabelsX();
	void updateLabelsY();
	int countDigits(int number);
	float labelwidthx, labelwidthy;
	std::string axisytitle, axisxtitle;
	std::vector<int> labelsx;
	std::vector<int> labelsy;
	std::vector<float> floatlabelsx;
	std::vector<float> floatlabelsy;
	std::vector<int> labelsteps;
};

#endif /* UI_GGUIGRAPH_H_ */
