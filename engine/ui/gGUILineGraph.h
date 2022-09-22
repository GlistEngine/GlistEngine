/*
 * gGUILineGraph.h
 *
 *  Created on: 28 July 2022
 *      Author: Berke Adil
 */

#ifndef UI_GGUILINEGRAPH_H_
#define UI_GGUILINEGRAPH_H_


#include "gImage.h"
#include "gGUIFrame.h"
#include "gGUIPanel.h"
#include "gGUIButton.h"
#include "gGUITextbox.h"
#include "gGUIControl.h"
#include "gLine.h"
#include "gRectangle.h"
#include "gCircle.h"

class gGUILineGraph: public gGUIControl  {

public:
	gGUILineGraph();
	virtual ~gGUILineGraph();

	void set(gBaseApp* root, gBaseGUIObject* topParentGUIObject, gBaseGUIObject* parentGUIObject, int parentSlotLineNo, int parentSlotColumnNo, int x, int y, int w, int h);

	void setMaxX(int maxX);
	int getMaxX();
	void setMinX(int minX);
	int getMinX();
	void setMaxY(int maxY);
	int getMaxY();
	void setMinY(int minY);
	int getMinY();
	void setGridlinesX(bool gridlinesX);
	void setGridlinesY(bool gridlinesY);
	void setTitleX(std::string titleX);
	std::string getTitleX();
	void setTitleY(std::string titleY);
	std::string getTitleY();
	void setLabelCountX(int labelCount);
	int getLabelCountX();
	void setLabelCountY(int labelCount);
	int getLabelCountY();

	void addLine();
	void addPointToLine(int lineindex, float x, float y);

	void draw();


private:
	void drawGraph();
	void drawLabels();
	void drawLines();
	void updatePoints();
	void updateLabelsX();
	void updateLabelsY();

	float axisx1, axisy1, axisx2, axisy2;
	float axisxw, axisyh;
	float maxy, miny, maxx, minx;
	std::vector<std::vector<std::array<float, 4>>> graphlines;
	float labelwidthx, labelwidthy;
	bool drawgridlinesx, drawgridlinesy;
	std::string axisytitle, axisxtitle;
	gColor linecolors[5];
	int labelcountx, labelcounty;
	std::vector<int> labelsx;
	std::vector<int> labelsy;
};


#endif /* UI_GGUILINEGRAPH_H_ */
