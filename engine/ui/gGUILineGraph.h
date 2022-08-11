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


	gColor* getLineDrawerColor();
	gColor* getLinePartColor();
	gColor* getGraphColor();
	gColor* getDisabledLinePartColor();
	gColor* getDisabledLinePartFontColor();

	virtual void update();

	void draw();
	void graph();
	void addValue(float x, float y);
	void addpoint();
	void linedrawer();


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
	gColor lpcolor, pressedlpcolor, disabledlpcolor, lgcolor, ldcolor; //LP=linepart, LG=linegraph, LD=linedrawer
	gColor lpfcolor, pressedlpfcolor, disabledlpfcolor; //font

	void resetTitlePosition();


private:
	gLine line1;
	gLine line2;
	gRectangle points[300];
	float x[300];
	float y[300];
	int pointnum;
	gLine lines[300];
	gLine line3;

	std::vector<std::vector<int>> linesX;
	std::vector<std::vector<int>> linesY;

	bool shown;
};


#endif /* UI_GGUILINEGRAPH_H_ */
