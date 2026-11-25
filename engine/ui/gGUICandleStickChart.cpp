/*
 * gGUICandleStickChart.cpp
 *
 *  Created on: 15 August 2022
 *      Author: Berke Adil
 */

#include "gGUICandleStickChart.h"
#include <algorithm>
#include <cmath>
#include <limits>

gGUICandleStickChart::gGUICandleStickChart() {
	title = "Candlestick Chart";
	lowcolor = {0.96f, 0.46f, 0.55f};
	highcolor = {0.62f, 0.80f, 0.41f};
	candlew = 10.0f;

	pointsenabled = false;
	isfilledarea = false;
	isautoscaled = false;
	linethickness = 1.0f;
	needsupdate = false;
	hascustomoffset = false;
	offsetleft = 0;
	offsetright = 0;
	offsettop = 0;
	offsetbottom = 0;

	linecolors.reserve(defaultlinecolornum);
	filledcolors.reserve(defaultlinecolornum);
	for (int i = 0; i < defaultlinecolornum; i++) {
		gColor lc = defaultLineColor(i);
		linecolors.push_back(lc);
		gColor fc = lc;
		fc.a = 0.15f;
		filledcolors.push_back(fc);
	}
}

gGUICandleStickChart::~gGUICandleStickChart() {
}

gColor gGUICandleStickChart::defaultLineColor(int lineIndex) {
	static const gColor palette[defaultlinecolornum] = {
		{0.20f, 0.20f, 0.96f},
		{0.96f, 0.46f, 0.55f},
		{0.62f, 0.80f, 0.41f},
		{0.47f, 0.63f, 0.96f},
		{0.87f, 0.68f, 0.40f},
		{0.73f, 0.60f, 0.96f}
	};
	int safeIndex = lineIndex % defaultlinecolornum;
	if (safeIndex < 0) safeIndex += defaultlinecolornum;
	return palette[safeIndex];
}

void gGUICandleStickChart::ensureLineColorSlot(int lineIndex) {
	if (lineIndex < 0) return;
	while (static_cast<int>(linecolors.size()) <= lineIndex) {
		int idx = static_cast<int>(linecolors.size());
		gColor lc = defaultLineColor(idx);
		linecolors.push_back(lc);
		gColor fc = lc;
		fc.a = 0.15f;
		filledcolors.push_back(fc);
	}
}

void gGUICandleStickChart::clearLineGeometryCache() {
	for (auto& line : cachedlines) {
		line.clear();
	}
	cachedlines.clear();
	linesmap.clear();

	for (auto& circle : cachedcircles) {
		circle.clear();
	}
	cachedcircles.clear();
	circlesmap.clear();
}

void gGUICandleStickChart::set(gBaseApp* root, gBaseGUIObject* topParentGUIObject, gBaseGUIObject* parentGUIObject, int parentSlotLineNo, int parentSlotColumnNo, int x, int y, int w, int h) {
	gGUIGraph::set(root, topParentGUIObject, parentGUIObject, parentSlotLineNo, parentSlotColumnNo, x, y, w, h);
	if (hascustomoffset) {
		updateOffset();
	}
	needsupdate = true;
}

void gGUICandleStickChart::setMaxX(float maxX) {
	gGUIGraph::setMaxX(maxX);
	gGUIGraph::setLabelCountX(getLabelCountX());
	needsupdate = true;
}

void gGUICandleStickChart::setMinX(float minX) {
	gGUIGraph::setMinX(minX);
	gGUIGraph::setLabelCountX(getLabelCountX());
	needsupdate = true;
}

void gGUICandleStickChart::setMaxY(float maxY) {
	gGUIGraph::setMaxY(maxY);
	gGUIGraph::setLabelCountY(getLabelCountY());
	needsupdate = true;
}

void gGUICandleStickChart::setMinY(float minY) {
	gGUIGraph::setMinY(minY);
	gGUIGraph::setLabelCountY(getLabelCountY());
	needsupdate = true;
}

void gGUICandleStickChart::setLabelCountX(int labelCount) {
	gGUIGraph::setLabelCountX(labelCount);
	needsupdate = true;
}

void gGUICandleStickChart::setLabelCountY(int labelCount) {
	gGUIGraph::setLabelCountY(labelCount);
	needsupdate = true;
}

void gGUICandleStickChart::setHighColor(gColor highColor) {
	highcolor = highColor;
}

gColor gGUICandleStickChart::getHighColor() {
	return highcolor;
}

void gGUICandleStickChart::setLowColor(gColor lowColor) {
	lowcolor = lowColor;
}

gColor gGUICandleStickChart::getLowColor() {
	return lowcolor;
}

void gGUICandleStickChart::addPointToLine(float x, float high, float low, float open, float close) {
	if (x < smallestvaluex) setMinX(x);
	if (x > largestvaluex || maxx < minx) setMaxX(x);

	float candlemin = std::min(low, std::min(high, std::min(open, close)));
	float candlemax = std::max(high, std::max(low, std::max(open, close)));
	if (candlemin < smallestvaluey) setMinY(candlemin);
	if (candlemax > largestvaluey || maxy < miny) setMaxY(candlemax);

	float lengthx = maxx - minx;
	if (std::abs(lengthx) < 1e-6f) lengthx = 1.0f;
	float lengthy = maxy - miny;
	if (std::abs(lengthy) < 1e-6f) lengthy = 1.0f;

	float pointx = axisx1 + axisxw * (x - minx) / lengthx;
	float highy = axisy2 - (axisyh * (high - miny) / lengthy);
	float lowy = axisy2 - axisyh * (low - miny) / lengthy;
	float openy = axisy2 - axisyh * (open - miny) / lengthy;
	float closey = axisy2 - axisyh * (close - miny) / lengthy;

	int pointcount = static_cast<int>(graphline.size());
	if (pointcount == 0) {
		graphline.push_back({x, high, low, open, close, pointx, highy, lowy, openy, closey});
		needsupdate = true;
		return;
	}
	int index = 0;
	while (index < pointcount) {
		if (graphline[index++][0] < x) continue;
		index--;
		break;
	}
	graphline.insert(graphline.begin() + index, {x, high, low, open, close, pointx, highy, lowy, openy, closey});
	needsupdate = true;
}

void gGUICandleStickChart::setCandleWidth(float candleWidth) {
	candlew = candleWidth;
}

float gGUICandleStickChart::getCandleWidth() {
	return candlew;
}

void gGUICandleStickChart::enablePoints(bool arePointsEnabled) {
	pointsenabled = arePointsEnabled;
	needsupdate = true;
}

bool gGUICandleStickChart::isPointsEnabled() const {
	return pointsenabled;
}

void gGUICandleStickChart::setLineColor(int lineIndex, gColor lineColor) {
	if (lineIndex < 0) return;
	ensureLineColorSlot(lineIndex);
	linecolors[lineIndex] = lineColor;
}

gColor gGUICandleStickChart::getLineColor(int lineIndex) {
	if (lineIndex < 0) return gColor(0.0f, 0.0f, 0.0f, 0.0f);
	if (lineIndex < static_cast<int>(linecolors.size())) {
		return linecolors[lineIndex];
	}
	return defaultLineColor(lineIndex);
}

void gGUICandleStickChart::setFilledAreaColor(int lineIndex, gColor fillColor) {
	if (lineIndex < 0) return;
	ensureLineColorSlot(lineIndex);
	filledcolors[lineIndex] = fillColor;
}

gColor gGUICandleStickChart::getFilledAreaColor(int lineIndex) {
	if (lineIndex < 0) return gColor(0.0f, 0.0f, 0.0f, 0.0f);
	if (lineIndex < static_cast<int>(filledcolors.size())) {
		return filledcolors[lineIndex];
	}
	gColor fc = defaultLineColor(lineIndex);
	fc.a = 0.15f;
	return fc;
}

void gGUICandleStickChart::addLine() {
	int newIndex = static_cast<int>(graphlines.size());
	ensureLineColorSlot(newIndex);
	std::deque<std::array<float, 4>> newline;
	graphlines.push_back(newline);
	needsupdate = true;
}

void gGUICandleStickChart::addData(int lineIndex, std::deque<std::array<float, 2>> dataToAdd) {
	if (lineIndex < 0 || lineIndex >= static_cast<int>(graphlines.size())) return;
	int datasize = static_cast<int>(dataToAdd.size());
	for (int i = 0; i < datasize; i++) {
		addPointToLine(lineIndex, dataToAdd[i][0], dataToAdd[i][1]);
	}
}

void gGUICandleStickChart::addPointToLine(int lineIndex, float x, float y) {
	if (lineIndex < 0 || lineIndex >= static_cast<int>(graphlines.size())) return;
	if (x < smallestvaluex) setMinX(x);
	if (x > largestvaluex || maxx < minx) setMaxX(x);
	if (y < smallestvaluey) setMinY(y);
	if (y > largestvaluey || maxy < miny) setMaxY(y);

	float lengthx = maxx - minx;
	if (std::abs(lengthx) < 1e-6f) lengthx = 1.0f;
	float lengthy = maxy - miny;
	if (std::abs(lengthy) < 1e-6f) lengthy = 1.0f;

	float px = axisx1 + axisxw * (x - minx) / lengthx;
	float py = axisy2 - axisyh * (y - miny) / lengthy;

	int pointcount = static_cast<int>(graphlines[lineIndex].size());
	if (pointcount == 0) {
		graphlines[lineIndex].push_back({x, y, px, py});
		needsupdate = true;
		return;
	}
	int index = 0;
	while (index < pointcount) {
		if (graphlines[lineIndex][index++][0] < x) continue;
		index--;
		break;
	}
	graphlines[lineIndex].insert(graphlines[lineIndex].begin() + index, {x, y, px, py});
	needsupdate = true;
}

void gGUICandleStickChart::setPointValues(int lineIndex, int pointIndex, float newX, float newY) {
	if (lineIndex < 0 || lineIndex >= static_cast<int>(graphlines.size())) return;
	if (pointIndex < 0 || pointIndex >= static_cast<int>(graphlines[lineIndex].size())) return;

	if (newX < smallestvaluex) setMinX(newX);
	if (newX > largestvaluex || maxx < minx) setMaxX(newX);
	if (newY < smallestvaluey) setMinY(newY);
	if (newY > largestvaluey || maxy < miny) setMaxY(newY);

	float lengthx = maxx - minx;
	if (std::abs(lengthx) < 1e-6f) lengthx = 1.0f;
	float lengthy = maxy - miny;
	if (std::abs(lengthy) < 1e-6f) lengthy = 1.0f;

	float px = axisx1 + axisxw * (newX - minx) / lengthx;
	float py = axisy2 - axisyh * (newY - miny) / lengthy;

	auto& series = graphlines[lineIndex];
	series.erase(series.begin() + pointIndex);

	int index = 0;
	int pointcount = static_cast<int>(series.size());
	while (index < pointcount && series[index][0] < newX) {
		index++;
	}
	series.insert(series.begin() + index, {newX, newY, px, py});
	needsupdate = true;
}

void gGUICandleStickChart::setPointValues(int lineIndex, float oldX, float oldY, float newX, float newY) {
	if (lineIndex < 0 || lineIndex >= static_cast<int>(graphlines.size())) return;

	auto& series = graphlines[lineIndex];
	bool updated = false;
	for (auto& pt : series) {
		if (pt[0] == oldX && pt[1] == oldY) {
			pt[0] = newX;
			pt[1] = newY;
			updated = true;
		}
	}
	if (!updated) return;

	if (newX < smallestvaluex) setMinX(newX);
	if (newX > largestvaluex || maxx < minx) setMaxX(newX);
	if (newY < smallestvaluey) setMinY(newY);
	if (newY > largestvaluey || maxy < miny) setMaxY(newY);

	std::stable_sort(series.begin(), series.end(), [](const std::array<float, 4>& a, const std::array<float, 4>& b) {
		return a[0] < b[0];
	});
	needsupdate = true;
}

void gGUICandleStickChart::removeFirstPointsFromLine(int lineIndex, int pointNumLimit) {
	if (lineIndex < 0 || lineIndex >= static_cast<int>(graphlines.size())) return;
	if (pointNumLimit <= 0) return;
	if (static_cast<int>(graphlines[lineIndex].size()) >= pointNumLimit) {
		for (int i = 0; i < pointNumLimit; i++) {
			graphlines[lineIndex].pop_front();
		}
		needsupdate = true;
	}
}

int gGUICandleStickChart::getLineNum() const {
	return static_cast<int>(graphlines.size());
}

int gGUICandleStickChart::getPointNum(int lineIndex) {
	if (lineIndex < 0 || lineIndex >= static_cast<int>(graphlines.size())) return 0;
	return static_cast<int>(graphlines[lineIndex].size());
}

float gGUICandleStickChart::getPointXValue(int lineIndex, int pointIndex) {
	if (lineIndex < 0 || lineIndex >= static_cast<int>(graphlines.size())) return 0.0f;
	if (pointIndex < 0 || pointIndex >= static_cast<int>(graphlines[lineIndex].size())) return 0.0f;
	return graphlines[lineIndex][pointIndex][0];
}

float gGUICandleStickChart::getPointYValue(int lineIndex, int pointIndex) {
	if (lineIndex < 0 || lineIndex >= static_cast<int>(graphlines.size())) return 0.0f;
	if (pointIndex < 0 || pointIndex >= static_cast<int>(graphlines[lineIndex].size())) return 0.0f;
	return graphlines[lineIndex][pointIndex][1];
}

void gGUICandleStickChart::clearLines() {
	graphlines.clear();
	clearLineGeometryCache();
	needsupdate = true;
}

void gGUICandleStickChart::clear() {
	graphline.clear();
	graphlines.clear();
	clearLineGeometryCache();

	smallestvaluex = 0.0f;
	largestvaluex = 0.0f;
	smallestvaluey = 0.0f;
	largestvaluey = 0.0f;

	minx = 0.0f;
	maxx = -2147483647.0f;
	miny = 0.0f;
	maxy = -2147483647.0f;

	needsupdate = true;
}

void gGUICandleStickChart::setOffset(int leftOffset, int rightOffset, int topOffset, int bottomOffset) {
	offsetleft = leftOffset;
	offsetright = rightOffset;
	offsettop = topOffset;
	offsetbottom = bottomOffset;
	hascustomoffset = true;
	updateOffset();
}

void gGUICandleStickChart::enableFilledArea(bool isEnabled) {
	isfilledarea = isEnabled;
}

void gGUICandleStickChart::setLineThickness(float thickness) {
	if (thickness > 0.0f) {
		linethickness = thickness;
		needsupdate = true;
	}
}

float gGUICandleStickChart::getLineThickness() const {
	return linethickness;
}

void gGUICandleStickChart::enableAutoScale(bool isEnabled) {
	isautoscaled = isEnabled;
	if (isautoscaled) {
		floatlabelsyenabled = true;
	}
	needsupdate = true;
}

void gGUICandleStickChart::drawGraph() {
	if (needsupdate) {
		updatePoints();
		needsupdate = false;
	}
	if (graphline.empty() && graphlines.empty()) return;

	gColor oldcolor = *renderer->getColor();
	drawFilledAreas();
	drawCandles();
	drawOverlayLines();
	renderer->setColor(oldcolor);
}

void gGUICandleStickChart::drawFilledAreas() {
	if (!isfilledarea || graphlines.empty()) return;

	int linecount = static_cast<int>(graphlines.size());
	float bottomY = axisy2;
	for (int i = 0; i < linecount; i++) {
		int pointcount = static_cast<int>(graphlines[i].size());
		if (pointcount <= 1) continue;

		renderer->setColor(getFilledAreaColor(i));
		for (int j = 1; j < pointcount; j++) {
			if (rangeenabled) {
				if (graphlines[i][j - 1][0] < rangestart || graphlines[i][j - 1][0] > rangeend ||
					graphlines[i][j][0] < rangestart || graphlines[i][j][0] > rangeend) {
					continue;
				}
			}

			float x1 = graphlines[i][j - 1][2];
			float y1 = graphlines[i][j - 1][3];
			float x2 = graphlines[i][j][2];
			float y2 = graphlines[i][j][3];

			gDrawTriangle(x1, y1, x2, y2, x1, bottomY, true);
			gDrawTriangle(x2, y2, x2, bottomY, x1, bottomY, true);
		}
	}
}

void gGUICandleStickChart::drawCandles() {
	if (graphline.empty()) return;

	int pointcount = static_cast<int>(graphline.size());
	for (int i = 0; i < pointcount; i++) {
		if (rangeenabled && (graphline[i][0] < rangestart || graphline[i][0] > rangeend)) continue;
		if (graphline[i][3] - graphline[i][4] < 0) renderer->setColor(lowcolor);
		else renderer->setColor(highcolor);
		gDrawLine(graphline[i][5], graphline[i][6], graphline[i][5], graphline[i][7]);
		gDrawRectangle(graphline[i][5] - candlew, graphline[i][8], candlew * 2, graphline[i][9] - graphline[i][8], true);
	}
}

void gGUICandleStickChart::drawOverlayLines() {
	if (graphlines.empty()) return;

	int linecount = static_cast<int>(graphlines.size());
	for (int i = 0; i < linecount; i++) {
		int pointcount = static_cast<int>(graphlines[i].size());
		if (pointcount == 0) continue;

		renderer->setColor(getLineColor(i));
		bool skipped = true;
		for (int j = 0; j < pointcount; j++) {
			if (rangeenabled) {
				if (graphlines[i][j][0] < rangestart || graphlines[i][j][0] > rangeend) {
					skipped = true;
					continue;
				}
			}
			if (pointsenabled) {
				auto cit = circlesmap.find(hash(i, j));
				if (cit != circlesmap.end() && cit->second < cachedcircles.size()) {
					cachedcircles[cit->second].draw();
				}
			}
			if (skipped) {
				skipped = false;
				continue;
			}

			auto lit = linesmap.find(hash(i, j));
			if (lit != linesmap.end() && lit->second < cachedlines.size()) {
				cachedlines[lit->second].draw();
			}
		}
	}
}

void gGUICandleStickChart::updatePoints() {
	if (isautoscaled) {
		bool hasAnyY = false;
		float rawMin = std::numeric_limits<float>::max();
		float rawMax = std::numeric_limits<float>::lowest();

		for (const auto& candle : graphline) {
			float cmin = std::min(candle[2], std::min(candle[1], std::min(candle[3], candle[4])));
			float cmax = std::max(candle[1], std::max(candle[2], std::max(candle[3], candle[4])));
			if (cmin < rawMin) rawMin = cmin;
			if (cmax > rawMax) rawMax = cmax;
			hasAnyY = true;
		}
		for (const auto& series : graphlines) {
			for (const auto& pt : series) {
				if (pt[1] < rawMin) rawMin = pt[1];
				if (pt[1] > rawMax) rawMax = pt[1];
				hasAnyY = true;
			}
		}

		if (hasAnyY) {
			if (rawMin > rawMax) std::swap(rawMin, rawMax);
			float range = rawMax - rawMin;
			if (range < 1e-6f) range = 1.0f;
			float pad = std::max(range * 0.05f, 0.001f);
			floatlabelsyenabled = true;
			setLabelsYForRange(rawMin - pad, rawMax + pad);
		}
	}

	float lengthx = maxx - minx;
	if (std::abs(lengthx) < 1e-6f) lengthx = 1.0f;
	float lengthy = maxy - miny;
	if (std::abs(lengthy) < 1e-6f) lengthy = 1.0f;

	int candlecount = static_cast<int>(graphline.size());
	for (int i = 0; i < candlecount; i++) {
		graphline[i][5] = axisx1 + axisxw * (graphline[i][0] - minx) / lengthx;
		graphline[i][6] = axisy2 - axisyh * (graphline[i][1] - miny) / lengthy;
		graphline[i][7] = axisy2 - axisyh * (graphline[i][2] - miny) / lengthy;
		graphline[i][8] = axisy2 - axisyh * (graphline[i][3] - miny) / lengthy;
		graphline[i][9] = axisy2 - axisyh * (graphline[i][4] - miny) / lengthy;
	}

	int linecount = static_cast<int>(graphlines.size());
	int totalpoints = 0;
	int totalsegments = 0;
	for (int i = 0; i < linecount; i++) {
		int pointcount = static_cast<int>(graphlines[i].size());
		totalpoints += pointcount;
		if (pointcount > 1) totalsegments += pointcount - 1;
		for (int j = 0; j < pointcount; j++) {
			graphlines[i][j][2] = axisx1 + axisxw * (graphlines[i][j][0] - minx) / lengthx;
			graphlines[i][j][3] = axisy2 - axisyh * (graphlines[i][j][1] - miny) / lengthy;
		}
	}

	// Resize cached line primitives to match total segment count
	int linediff = static_cast<int>(cachedlines.size()) - totalsegments;
	if (linediff > 0) {
		for (int i = 0; i < linediff; i++) {
			gLine& line = cachedlines.back();
			line.clear();
			cachedlines.pop_back();
		}
	} else {
		for (int i = 0; i < -linediff; i++) {
			cachedlines.push_back(gLine());
		}
	}

	linesmap.clear();
	size_t segIndex = 0;
	for (int i = 0; i < linecount; i++) {
		int pointcount = static_cast<int>(graphlines[i].size());
		for (int j = 1; j < pointcount; j++) {
			if (segIndex >= cachedlines.size()) break;
			float x1 = graphlines[i][j - 1][2];
			float y1 = graphlines[i][j - 1][3];
			float x2 = graphlines[i][j][2];
			float y2 = graphlines[i][j][3];
			if (std::abs(x2 - x1) < 1e-6f && std::abs(y2 - y1) < 1e-6f) continue;

			cachedlines[segIndex].setThickness(linethickness);
			cachedlines[segIndex].setPoints(x1, y1, x2, y2);
			linesmap[hash(i, j)] = segIndex;
			segIndex++;
		}
	}

	if (!pointsenabled) {
		for (auto& item : cachedcircles) {
			item.clear();
		}
		cachedcircles.clear();
		circlesmap.clear();
		return;
	}

	int circlediff = static_cast<int>(cachedcircles.size()) - totalpoints;
	if (circlediff > 0) {
		for (int i = 0; i < circlediff; i++) {
			gCircle& circle = cachedcircles.back();
			circle.clear();
			cachedcircles.pop_back();
		}
	} else {
		for (int i = 0; i < -circlediff; i++) {
			cachedcircles.push_back(gCircle());
		}
	}

	circlesmap.clear();
	size_t circleIndex = 0;
	for (int i = 0; i < linecount; i++) {
		int pointcount = static_cast<int>(graphlines[i].size());
		for (int j = 0; j < pointcount; j++) {
			if (circleIndex >= cachedcircles.size()) break;
			cachedcircles[circleIndex].setPoints(graphlines[i][j][2], graphlines[i][j][3], 5, true, 64);
			circlesmap[hash(i, j)] = circleIndex;
			circleIndex++;
		}
	}
}

void gGUICandleStickChart::updateOffset() {
	axisx1 = left + offsetleft;
	axisy1 = top + offsettop;

	axisxw = width - (offsetleft + offsetright);
	axisyh = height - (offsettop + offsetbottom);
	axisx2 = axisx1 + axisxw;
	axisy2 = axisy1 + axisyh;

	float lengthx = maxx - minx;
	float lengthy = maxy - miny;
	axisxstart = axisy2;
	if (miny < 0 && std::abs(lengthy) > 1e-6f) axisxstart -= axisyh * ((-miny) / lengthy);
	axisystart = axisx1;
	if (minx < 0 && std::abs(lengthx) > 1e-6f) axisystart += axisxw * ((-minx) / lengthx);

	setLabelCountX(labelcountx);
	setLabelCountY(labelcounty);
	needsupdate = true;
}
