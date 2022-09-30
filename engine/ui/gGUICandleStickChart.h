/****************************************************************************
 * Copyright (c) 2014 Nitra Games Ltd., Istanbul, Turkey                    *
 *                                                                          *
 * Permission is hereby granted, free of charge, to any person obtaining a  *
 * copy of this software and associated documentation files (the            *
 * "Software"), to deal in the Software without restriction, including      *
 * without limitation the rights to use, copy, modify, merge, publish,      *
 * distribute, distribute with modifications, sublicense, and/or sell       *
 * copies of the Software, and to permit persons to whom the Software is    *
 * furnished to do so, subject to the following conditions:                 *
 *                                                                          *
 * The above copyright notice and this permission notice should not be      *
 * deleted from the source form of the Software.                            *
 *                                                                          *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS  *
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF               *
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.   *
 * IN NO EVENT SHALL THE ABOVE COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,   *
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR    *
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR    *
 * THE USE OR OTHER DEALINGS IN THE SOFTWARE.                               *
 *                                                                          *
 * Except as contained in this notice, the name(s) of the above copyright   *
 * holders shall not be used in advertising or otherwise to promote the     *
 * sale, use or other dealings in this Software without prior written       *
 * authorization.                                                           *
 ****************************************************************************/

/****************************************************************************
 * Author: Berke Adil, Utku Sarialan, 2022-on                               *
 ****************************************************************************/

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
	void setCandleWidth(float candleWidth);
	float getCandleWidth();

private:
	void drawGraph();
	void updatePoints();

	std::vector<std::array<float, 10>> graphline;
	gColor highcolor;
	gColor lowcolor;

	int candlew;
};



#endif /* UI_GGUICANDLESTICKCHART_H_ */
