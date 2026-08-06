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

	/**
	 * Sets the positioning, slot indexing, and dimensions of the line graph control.
	 */
	void set(gBaseApp* root, gBaseGUIObject* topParentGUIObject, gBaseGUIObject* parentGUIObject, int parentSlotLineNo, int parentSlotColumnNo, int x, int y, int w, int h) override;

	/**
	 * Sets the maximum value limits of the X axis.
	 */
	void setMaxX(float maxX) override;

	/**
	 * Sets the minimum value limits of the X axis.
	 */
	void setMinX(float minX) override;

	/**
	 * Sets the maximum value limits of the Y axis.
	 */
	void setMaxY(float maxY) override;

	/**
	 * Sets the minimum value limits of the Y axis.
	 */
	void setMinY(float minY) override;

	/**
	 * Sets the number of labels to be drawn on the X axis.
	 */
	void setLabelCountX(int labelCount) override;

	/**
	 * Sets the number of labels to be drawn on the Y axis.
	 */
	void setLabelCountY(int labelCount) override;

	/**
	 * Enables or disables drawing circles/points at the data coordinates.
	 */
	void enablePoints(bool arePointsEnabled);

	/**
	 * Sets the drawing color for a specific line index.
	 */
	void setLineColor(int lineIndex, gColor lineColor);

	/**
	 * Gets the drawing color of a specific line index.
	 */
	gColor getLineColor(int lineIndex);

	/**
	 * Sets the filled area color under the graph line for a specific line index.
	 */
	void setFilledAreaColor(int lineIndex, gColor fillColor);

	/**
	 * Gets the filled area color under the graph line for a specific line index.
	 */
	gColor getFilledAreaColor(int lineIndex);

	/**
	 * Adds a new line series data container to the graph.
	 */
	void addLine();

	/**
	 * Appends a batch of points to a specific line series.
	 */
	void addData(int lineIndex, std::deque<std::array<float, 2>> dataToAdd);

	/**
	 * Appends a single data point to a specific line series.
	 */
	void addPointToLine(int lineIndex, float x, float y);

	/**
	 * Updates the coordinates of a data point at a specific index in a line series.
	 */
	void setPointValues(int lineIndex, int pointIndex, float newX, float newY);

	/**
	 * Finds a data point by old coordinates and updates it to new coordinates.
	 */
	void setPointValues(int lineIndex, float oldX, float oldY, float newX, float newY);

	/**
	 * Truncates the oldest points from the beginning of a line series if it exceeds a limit.
	 */
	void removeFirstPointsFromLine(int lineIndex, int pointNumLimit);

	/**
	 * Gets the total number of points in a specific line series.
	 */
	int getPointNum(int lineIndex);

	/**
	 * Gets the raw X value of a point inside a specific line series.
	 */
	float getPointXValue(int lineIndex, int pointIndex);

	/**
	 * Gets the raw Y value of a point inside a specific line series.
	 */
	float getPointYValue(int lineIndex, int pointIndex);

	/**
	 * Clears all data lines, points, and customized colors from the graph.
	 */
	void clear() override;

	/**
	 * Configures outer margins/padding around the graph axes boundary.
	 */
	void setOffset(int leftOffset, int rightOffset, int topOffset, int bottomOffset);

	/**
	 * Enables or disables drawing a filled area beneath the graph lines.
	 */
	void enableFilledArea(bool isEnabled);

	/**
	 * Configures the thickness of the line segments on the graph.
	 */
	void setLineThickness(float thickness);

	/**
	 * Enables or disables auto-scaling of the Y axis tightly to the actual min/max data range.
	 */
	void enableAutoScale(bool isEnabled);
private:
	static const int linecolornum = 6;
	void drawGraph() override;
	void updatePoints();
	void updateOffset();

	std::deque<std::deque<std::array<float, 4>>> graphlines;
	std::vector<gLine> cachedlines;
	std::vector<gCircle> cachedcircles;
	std::unordered_map<uint64_t, gLine*> linesmap;
	std::unordered_map<uint64_t, gCircle*> circlesmap;
	gColor linecolors[linecolornum];
	gColor filledcolors[linecolornum];

	bool pointsenabled;
	bool isfilledarea;
	bool isautoscaled;
	float linethickness;
	bool needsupdate = false;
	int offsetleft;
	int offsetright;
	int offsettop;
	int offsetbottom;


	uint64_t hash(uint32_t a, uint32_t b) {
		return ((uint64_t)a << 32) | b;
	}
};


#endif /* UI_GGUILINEGRAPH_H_ */
