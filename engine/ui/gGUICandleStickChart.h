/*
 * gGUICandleStickChart.h
 *
 *  Created on: 15 Aðu 2022
 *      Author: Berke Adil
 */

#ifndef UI_GGUICANDLESTICKCHART_H_
#define UI_GGUICANDLESTICKCHART_H_

#include "gImage.h"
#include "gGUIFrame.h"
#include "gGUIPanel.h"
#include "gGUIButton.h"
#include "gGUITextbox.h"
#include "gGUIControl.h"
#include "gLine.h"
#include "gRectangle.h"
#include "gGUIGraph.h"


class gGUICandleStickChart: public gGUIGraph {

public:
	gGUICandleStickChart();
	virtual ~gGUICandleStickChart();

	void set(gBaseApp* root, gBaseGUIObject* topParentGUIObject, gBaseGUIObject* parentGUIObject, int parentSlotLineNo, int parentSlotColumnNo, int x, int y, int w, int h);

	void setMaxX(int maxX);
	void setMinX(int minX);
	void setMaxY(int maxY);
	void setMinY(int minY);

	void setLabelCountX(int labelCount);
	void setLabelCountY(int labelCount);

	void setHighColor(gColor highColor);
	gColor getHighColor();
	void setLowColor(gColor lowColor);
	gColor getLowColor();

	void addPointToLine(float x, float high, float low, float open, float close);

private:
	void drawGraph();
	void updatePoints();

	std::vector<std::array<float, 10>> graphline;
	gColor highcolor;
	gColor lowcolor;

	int candlew;
};



#endif /* UI_GGUICANDLESTICKCHART_H_ */
