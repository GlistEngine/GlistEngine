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

	void draw();

protected:
	float axisx1, axisy1, axisx2, axisy2;
	float axisxw, axisyh;
	float maxy, miny, maxx, minx;
	bool gridlinesxenabled, gridlinesyenabled;
	int labelcountx, labelcounty;


private:
	void drawBackground();
	void drawLabels();
	virtual void drawGraph();
	void updateLabelsX();
	void updateLabelsY();
	float labelwidthx, labelwidthy;
	std::string axisytitle, axisxtitle;
	std::vector<int> labelsx;
	std::vector<int> labelsy;
};

#endif /* UI_GGUIGRAPH_H_ */
