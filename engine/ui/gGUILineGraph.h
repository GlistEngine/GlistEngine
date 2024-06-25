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
#include "gGUIGraph.h"
#include "gLine.h"
#include "gRectangle.h"
#include "gCircle.h"
#include <array>
#include <deque>
#include <unordered_map>


class gGUILineGraph: public gGUIGraph  {

public:
	gGUILineGraph();
	virtual ~gGUILineGraph();

	void set(gBaseApp* root, gBaseGUIObject* topParentGUIObject, gBaseGUIObject* parentGUIObject, int parentSlotLineNo, int parentSlotColumnNo, int x, int y, int w, int h) override;

	void setMaxX(int maxX) override;
	void setMinX(int minX) override;
	void setMaxY(int maxY) override;
	void setMinY(int minY) override;

	void setLabelCountX(int labelCount) override;
	void setLabelCountY(int labelCount) override;

	void enablePoints(bool arePointsEnabled);
	void setLineColor(int lineIndex, gColor lineColor);
	gColor getLineColor(int lineIndex);

	void addLine();
	void addData(int lineIndex, std::deque<std::array<float, 2>> dataToAdd);
	void addPointToLine(int lineIndex, float x, float y);
	void setPointValues(int lineIndex, int pointIndex, float newX, float newY);
	void setPointValues(int lineIndex, float oldX, float oldY, float newX, float newY);
	void removeFirstPointsFromLine(int lineIndex, int pointNumLimit);

	int getPointNum(int lineIndex);

	float getPointXValue(int lineIndex, int pointIndex);
	float getPointYValue(int lineIndex, int pointIndex);

	void clear() override;

private:
	static const int linecolornum = 6;
	void drawGraph() override;
	void updatePoints();

	std::deque<std::deque<std::array<float, 4>>> graphlines;
	std::vector<gLine> cachedlines;
	std::vector<gCircle> cachedcircles;
	std::unordered_map<uint64_t, gLine*> linesmap;
	std::unordered_map<uint64_t, gCircle*> circlesmap;
	gColor linecolors[linecolornum];

	bool pointsenabled;
	bool needsupdate = false;

	uint64_t hash(uint32_t a, uint32_t b) {
		return ((uint64_t)a << 32) | b;
	}
};


#endif /* UI_GGUILINEGRAPH_H_ */
