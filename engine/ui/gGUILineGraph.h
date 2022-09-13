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

	void setSize(int width, int height);
	void setDisabled(bool isDisabled);
	void setTextVisibility(bool isVisible);

	bool isDisabled();

	void setLinePartColor(gColor color);
	void setDisabledLinePartColor(gColor color);
	void setGraphColor(gColor color);
	void setLineDrawerColor(gColor color);
	void setmultiLineColor(gColor color);


	gColor* getLineDrawerColor();
	gColor* getLinePartColor();
	gColor* getGraphColor();
	gColor* getDisabledLinePartColor();
	gColor* getDisabledLinePartFontColor();
	gColor* getmultiLineColor();

	virtual void update();

	void draw();
	void graph();
	void addValue(float x, float y);
	void addMultiValue(float title, float mx, float my);
	void addpoint();
	void addMultiPoint();
	void drawLine();
	void drawMultiline();


	void drawXAxis();
	void drawYAxis();
	void setXAxisValues(int min, int max, int range);
	void setYAxisValues(int min, int max, int range);


protected:
	bool ispressed;
	int lpw, lph;
	bool istextvisible;
	bool istoggle;
	bool ispressednow;
	bool isdisabled;
	gColor lpcolor, pressedlpcolor, disabledlpcolor, lgcolor, ldcolor, mlcolor; //LP=linepart, LG=linegraph, LD=linedrawer, ML=multiline
	gColor lpfcolor, pressedlpfcolor, disabledlpfcolor; //font

	void resetTitlePosition();


private:
	gLine line1;
	gLine line2;
	gRectangle points[300];
	float x[300];
	float y[300];
	float mx[300];
	float my[300];
	int pointnum;
	int multinum;
	int rangepointx, rangepointy;
	gLine lines[300];
	int maxy;
	int maxx;

	std::vector<std::vector<int>> linesX;
	std::vector<std::vector<int>> linesY;

	bool shown;
};


#endif /* UI_GGUILINEGRAPH_H_ */
