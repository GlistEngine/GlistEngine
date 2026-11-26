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
 *  Created on: 15 Agu 2022
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
#include "gCircle.h"
#include "gGUIGraph.h"
#include <array>
#include <cstdint>
#include <deque>
#include <unordered_map>
#include <vector>


class gGUICandleStickChart: public gGUIGraph {

public:
	gGUICandleStickChart();
	virtual ~gGUICandleStickChart();

	/**
	 * Sets the positioning, slot indexing, and dimensions of the candlestick chart control.
	 */
	void set(gBaseApp* root, gBaseGUIObject* topParentGUIObject, gBaseGUIObject* parentGUIObject, int parentSlotLineNo, int parentSlotColumnNo, int x, int y, int w, int h) override;

	/**
	 * Sets the maximum value limit of the X axis.
	 */
	void setMaxX(float maxX) override;

	/**
	 * Sets the minimum value limit of the X axis.
	 */
	void setMinX(float minX) override;

	/**
	 * Sets the maximum value limit of the Y axis.
	 */
	void setMaxY(float maxY) override;

	/**
	 * Sets the minimum value limit of the Y axis.
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
	 * Enables or disables drawing only data within the configured X range.
	 */
	void enableRange(bool isRangeEnabled) override;

	/**
	 * Configures the visible X range when range filtering is enabled.
	 */
	void setRange(float rangeStart, float rangeEnd) override;

	void setHighColor(gColor highColor);
	gColor getHighColor();
	void setLowColor(gColor lowColor);
	gColor getLowColor();

	/**
	 * Appends a candlestick data point (x, high, low, open, close) to the chart.
	 */
	void addPointToLine(float x, float high, float low, float open, float close);
	void setCandleWidth(float candleWidth);
	float getCandleWidth();

	/**
	 * Enables or disables drawing circles/points at overlay line data coordinates.
	 * Defaults to false on candlestick charts to keep candle bodies unobstructed.
	 */
	void enablePoints(bool arePointsEnabled);

	/**
	 * Returns whether overlay line point markers are enabled.
	 */
	bool isPointsEnabled() const;

	/**
	 * Sets the drawing color for a specific overlay line index.
	 */
	void setLineColor(int lineIndex, gColor lineColor);

	/**
	 * Gets the drawing color of a specific overlay line index.
	 */
	gColor getLineColor(int lineIndex);

	/**
	 * Sets the filled area color under a specific overlay line index.
	 */
	void setFilledAreaColor(int lineIndex, gColor fillColor);

	/**
	 * Gets the filled area color under a specific overlay line index.
	 */
	gColor getFilledAreaColor(int lineIndex);

	/**
	 * Adds a new overlay line series data container to the chart.
	 */
	void addLine();

	/**
	 * Appends a batch of (x, y) points to a specific overlay line series.
	 */
	void addData(int lineIndex, std::deque<std::array<float, 2>> dataToAdd);

	/**
	 * Appends a single (x, y) data point to a specific overlay line series.
	 */
	void addPointToLine(int lineIndex, float x, float y);

	/**
	 * Updates the coordinates of a data point at a specific 0-based index in an overlay line series.
	 */
	void setPointValues(int lineIndex, int pointIndex, float newX, float newY);

	/**
	 * Finds a data point by old coordinates in an overlay line series and updates it to new coordinates.
	 */
	void setPointValues(int lineIndex, float oldX, float oldY, float newX, float newY);

	/**
	 * Truncates the oldest points from the beginning of an overlay line series if its size meets or exceeds pointNumLimit.
	 */
	void removeFirstPointsFromLine(int lineIndex, int pointNumLimit);

	/**
	 * Gets the total number of overlay line series in the chart.
	 */
	int getLineNum() const;

	/**
	 * Gets the total number of points in a specific overlay line series.
	 */
	int getPointNum(int lineIndex);

	/**
	 * Gets the raw X value of a point inside a specific overlay line series.
	 */
	float getPointXValue(int lineIndex, int pointIndex);

	/**
	 * Gets the raw Y value of a point inside a specific overlay line series.
	 */
	float getPointYValue(int lineIndex, int pointIndex);

	/**
	 * Clears only the overlay line series and their cached geometry while keeping candlesticks intact.
	 */
	void clearLines();

	/**
	 * Clears all candlesticks, overlay line series, and cached geometry from the chart.
	 */
	void clear() override;

	/**
	 * Configures outer margins/padding around the chart axes boundary.
	 */
	void setOffset(int leftOffset, int rightOffset, int topOffset, int bottomOffset);

	/**
	 * Enables or disables drawing a filled area beneath overlay lines.
	 */
	void enableFilledArea(bool isEnabled);

	/**
	 * Configures the thickness of overlay line segments.
	 */
	void setLineThickness(float thickness);

	/**
	 * Gets the configured thickness of overlay line segments.
	 */
	float getLineThickness() const;

	/**
	 * Enables or disables rounded corners and edge smoothing on overlay lines.
	 * Disabled (false) by default for sharp, unsmoothed corners.
	 */
	void setRoundedCorners(bool isRounded);

	/**
	 * Returns whether rounded corners and edge smoothing are enabled on overlay lines.
	 */
	bool getRoundedCorners() const;

	/**
	 * Enables or disables auto-scaling of the Y axis tightly to the actual min/max data range
	 * across both candlesticks and overlay lines.
	 */
	void enableAutoScale(bool isEnabled);

protected:
	void drawGraph() override;
	void drawFilledAreas();
	void drawCandles();
	void drawOverlayLines();
	void updatePoints();
	void updateOffset();

private:
	static const int defaultlinecolornum = 6;
	static gColor defaultLineColor(int lineIndex);
	void ensureLineColorSlot(int lineIndex);
	void clearLineGeometryCache();

	std::vector<std::array<float, 10>> graphline;
	gColor highcolor;
	gColor lowcolor;
	float candlew;

	std::deque<std::deque<std::array<float, 4>>> graphlines;
	std::deque<PolylineStroke> cachedstrokes;
	std::vector<gCircle> cachedcircles;
	std::unordered_map<uint64_t, size_t> circlesmap;
	std::vector<gColor> linecolors;
	std::vector<gColor> filledcolors;

	bool pointsenabled;
	bool isfilledarea;
	bool isautoscaled;
	bool roundedcorners;
	float linethickness;
	bool needsupdate;
	bool hascustomoffset;
	int offsetleft;
	int offsetright;
	int offsettop;
	int offsetbottom;

	uint64_t hash(uint32_t a, uint32_t b) const {
		return (static_cast<uint64_t>(a) << 32) | b;
	}
};



#endif /* UI_GGUICANDLESTICKCHART_H_ */
