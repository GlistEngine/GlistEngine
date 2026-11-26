/*
 * gGUILineGraph.cpp
 *
 *  Created on: 28 July 2022
 *      Author: Berke Adil
 */


#include "gGUILineGraph.h"
#include "gGUISlider.h"
#include <algorithm>
#include <cmath>
#include <limits>

gGUILineGraph::gGUILineGraph() {
	title = "Graph";
	pointsenabled = true;
	isfilledarea = false;
	isautoscaled = false;
	roundedcorners = false;
	linethickness = 1.0f;
	floatlabelsyenabled = true;// Y values are frequently sub-integer (e.g. forex rates); whole-number-only labels would collapse a narrow range to one repeated number

	linecolors.reserve(linecolornum);
	filledcolors.reserve(linecolornum);
	for (int i = 0; i < linecolornum; i++) {
		gColor lc = defaultLineColor(i);
		linecolors.push_back(lc);
		gColor fc = lc;
		fc.a = 0.15f;
		filledcolors.push_back(fc);
	}

	offsetleft = 0;
	offsetright = 0;
	offsettop = 0;
	offsetbottom = 0;
}

gGUILineGraph::~gGUILineGraph() {
}

gColor gGUILineGraph::defaultLineColor(int lineIndex) {
	static const gColor palette[linecolornum] = {
		{0.20f, 0.20f, 0.96f},
		{0.96f, 0.46f, 0.55f},
		{0.62f, 0.80f, 0.41f},
		{0.47f, 0.63f, 0.96f},
		{0.87f, 0.68f, 0.40f},
		{0.73f, 0.60f, 0.96f}
	};
	int safeindex = lineIndex % linecolornum;
	if (safeindex < 0) safeindex += linecolornum;
	return palette[safeindex];
}

void gGUILineGraph::ensureLineColorSlot(int lineIndex) {
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

void gGUILineGraph::clearGeometryCache() {
	for (auto& stroke : cachedstrokes) {
		stroke.clear();
	}
	cachedstrokes.clear();

	for (auto& circle : cachedcircles) {
		circle.clear();
	}
	cachedcircles.clear();
	circlesmap.clear();
}

void gGUILineGraph::enableFilledArea(bool isEnabled) {
	isfilledarea = isEnabled;
}

void gGUILineGraph::setLineThickness(float thickness) {
	if (thickness > 0.0f) {
		linethickness = thickness;
		needsupdate = true;
	}
}

float gGUILineGraph::getLineThickness() const {
	return linethickness;
}

void gGUILineGraph::setRoundedCorners(bool isRounded) {
	if (roundedcorners != isRounded) {
		roundedcorners = isRounded;
		needsupdate = true;
	}
}

bool gGUILineGraph::getRoundedCorners() const {
	return roundedcorners;
}

void gGUILineGraph::enableAutoScale(bool isEnabled) {
	isautoscaled = isEnabled;
	needsupdate = true;
}

void gGUILineGraph::set(gBaseApp* root, gBaseGUIObject* topParentGUIObject, gBaseGUIObject* parentGUIObject, int parentSlotLineNo, int parentSlotColumnNo, int x, int y, int w, int h) {
	gGUIGraph::set(root, topParentGUIObject, parentGUIObject, parentSlotLineNo, parentSlotColumnNo, x, y, w, h);
	needsupdate = true;

	updateOffset();
}

void gGUILineGraph::setMaxX(float maxX) {
	gGUIGraph::setMaxX(maxX);
	needsupdate = true;
}


void gGUILineGraph::setMinX(float minX) {
	gGUIGraph::setMinX(minX);
	needsupdate = true;
}

void gGUILineGraph::setMaxY(float maxY) {
	gGUIGraph::setMaxY(maxY);
	needsupdate = true;
}


void gGUILineGraph::setMinY(float minY) {
	gGUIGraph::setMinY(minY);
	needsupdate = true;
}

void gGUILineGraph::setLabelCountX(int labelCount) {
	gGUIGraph::setLabelCountX(labelCount);
	needsupdate = true;
}

void gGUILineGraph::setLabelCountY(int labelCount) {
	gGUIGraph::setLabelCountY(labelCount);
	needsupdate = true;
}

void gGUILineGraph::enableRange(bool isRangeEnabled) {
	gGUIGraph::enableRange(isRangeEnabled);
	needsupdate = true;
}

void gGUILineGraph::setRange(float rangeStart, float rangeEnd) {
	gGUIGraph::setRange(rangeStart, rangeEnd);
	needsupdate = true;
}

void gGUILineGraph::enablePoints(bool arePointsEnabled) {
	pointsenabled = arePointsEnabled;
	needsupdate = true;
}

void gGUILineGraph::setLineColor(int lineIndex, gColor lineColor) {
	if (lineIndex < 0) return;
	ensureLineColorSlot(lineIndex);
	linecolors[lineIndex] = lineColor;
}

gColor gGUILineGraph::getLineColor(int lineIndex) {
	if (lineIndex < 0) return gColor(0.0f, 0.0f, 0.0f, 0.0f);
	if (lineIndex < static_cast<int>(linecolors.size())) {
		return linecolors[lineIndex];
	}
	return defaultLineColor(lineIndex);
}

void gGUILineGraph::setFilledAreaColor(int lineIndex, gColor fillColor) {
	if (lineIndex < 0) return;
	ensureLineColorSlot(lineIndex);
	filledcolors[lineIndex] = fillColor;
}

gColor gGUILineGraph::getFilledAreaColor(int lineIndex) {
	if (lineIndex < 0) return gColor(0.0f, 0.0f, 0.0f, 0.0f);
	if (lineIndex < static_cast<int>(filledcolors.size())) {
		return filledcolors[lineIndex];
	}
	gColor fc = defaultLineColor(lineIndex);
	fc.a = 0.15f;
	return fc;
}

void gGUILineGraph::addLine() {
	int newindex = static_cast<int>(graphlines.size());
	ensureLineColorSlot(newindex);
	std::deque<std::array<float, 4>> newline;
	graphlines.push_back(newline);
	needsupdate = true;
}

void gGUILineGraph::addData(int lineIndex, std::deque<std::array<float, 2>> dataToAdd) {
	if (lineIndex < 0 || lineIndex >= static_cast<int>(graphlines.size())) return;
	int datasize = static_cast<int>(dataToAdd.size());
	for (int i = 0; i < datasize; i++) addPointToLine(lineIndex, dataToAdd[i][0], dataToAdd[i][1]);
}

void gGUILineGraph::addPointToLine(int lineIndex, float x, float y) {
	if (lineIndex < 0 || lineIndex >= static_cast<int>(graphlines.size())) return;
	if (x < smallestvaluex) setMinX(x);
	if (x > largestvaluex || maxx < minx) setMaxX(x);
	if (y < smallestvaluey) setMinY(y);
	if (y > largestvaluey || maxy < miny) setMaxY(y);
	setLabelCountX(getLabelCountX());
	setLabelCountY(getLabelCountY());

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

void gGUILineGraph::setPointValues(int lineIndex, int pointIndex, float newX, float newY) {
	if (lineIndex < 0 || lineIndex >= static_cast<int>(graphlines.size())) return;
	if (pointIndex <= 0 || pointIndex > static_cast<int>(graphlines[lineIndex].size())) return;
	if (newX < smallestvaluex) setMinX(newX);
	if (newX > largestvaluex || maxx < minx) setMaxX(newX);
	if (newY < smallestvaluey) setMinY(newY);
	if (newY > largestvaluey || maxy < miny) setMaxY(newY);
	setLabelCountX(getLabelCountX());
	setLabelCountY(getLabelCountY());

	float lengthx = maxx - minx;
	if (std::abs(lengthx) < 1e-6f) lengthx = 1.0f;
	float lengthy = maxy - miny;
	if (std::abs(lengthy) < 1e-6f) lengthy = 1.0f;

	graphlines[lineIndex][pointIndex - 1] = {newX, newY, axisx1 + axisxw * (newX - minx) / lengthx, axisy2 - axisyh * (newY - miny) / lengthy};
	needsupdate = true;
}

void gGUILineGraph::setPointValues(int lineIndex, float oldX, float oldY, float newX, float newY) {
	if (lineIndex < 0 || lineIndex >= static_cast<int>(graphlines.size())) return;
	if (newX < smallestvaluex) {
		setMinX(newX);
	}
	if (newX > largestvaluex || maxx < minx) {
		setMaxX(newX);
	}
	if (newY < smallestvaluey) {
		setMinY(newY);
	}
	if (newY > largestvaluey || maxy < miny) {
		setMaxY(newY);
	}
	setLabelCountX(getLabelCountX());
	setLabelCountY(getLabelCountY());

	float lengthx = maxx - minx;
	if (std::abs(lengthx) < 1e-6f) lengthx = 1.0f;
	float lengthy = maxy - miny;
	if (std::abs(lengthy) < 1e-6f) lengthy = 1.0f;

	int pointcount = static_cast<int>(graphlines[lineIndex].size());
	for (int i = 0; i < pointcount; i++) {
		if (graphlines[lineIndex][i][0] == oldX && graphlines[lineIndex][i][1] == oldY) {
			graphlines[lineIndex][i][0] = newX;
			graphlines[lineIndex][i][1] = newY;
			graphlines[lineIndex][i][2] = axisx1 + axisxw * (newX - minx) / lengthx;
			graphlines[lineIndex][i][3] = axisy2 - axisyh * (newY - miny) / lengthy;
		}
	}
	needsupdate = true;
}

void gGUILineGraph::drawGraph() {
	if (needsupdate) {
		updatePoints();
		needsupdate = false;
	}
	if (graphlines.empty()) {
		return;
	}

	gColor oldcolor = *renderer->getColor();

	int linecount = static_cast<int>(graphlines.size());
	for (int i = 0; i < linecount; i++) {
		int pointcount = static_cast<int>(graphlines[i].size());

		// Draw filled area if enabled
		if (isfilledarea && pointcount > 1) {
			renderer->setColor(getFilledAreaColor(i));

			float bottomY = axisy2;
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

		// Draw continuous joined polyline stroke for this series
		gColor linecolor = getLineColor(i);
		if (i < static_cast<int>(cachedstrokes.size())) {
			drawPolylineStroke(cachedstrokes[i], linecolor);
		}

		// Draw point markers if enabled
		if (pointsenabled && pointcount > 0) {
			renderer->setColor(linecolor);
			for (int j = 0; j < pointcount; j++) {
				if (rangeenabled) {
					if (graphlines[i][j][0] < rangestart || graphlines[i][j][0] > rangeend) {
						continue;
					}
				}
				auto cit = circlesmap.find(hash(i, j));
				if (cit != circlesmap.end() && cit->second < cachedcircles.size()) {
					cachedcircles[cit->second].draw();
				}
			}
		}
	}

	renderer->setColor(&oldcolor);
}

void gGUILineGraph::updatePoints() {
	int linecount = static_cast<int>(graphlines.size());
	int totalpoints = 0;
	for (int i = 0; i < linecount; i++) {
		totalpoints += static_cast<int>(graphlines[i].size());
	}

	if (totalpoints == 0) {
		clearGeometryCache();
		return;
	}

	float rawMin = smallestvaluey;
	float rawMax = largestvaluey;

	if (rawMin > rawMax) std::swap(rawMin, rawMax);

	float range = rawMax - rawMin;
	if (range < 1e-6f) range = 1.0f;// Prevent division by zero when the value range is too small

	float pad = std::max(range * 0.05f, 0.001f);

	float relativeSpan = range / std::max(std::abs(rawMax), 1e-6f);

	if (isautoscaled) {
		miny = rawMin - pad;
		maxy = rawMax + pad;
	} else if (rawMin >= 0.0f && relativeSpan > 0.20f) {
		miny = 0.0f;
		maxy = rawMax + pad;
	} else {
		miny = rawMin - pad;
		maxy = rawMax + pad;
	}

	setLabelsYForRange(miny, maxy);

	float lengthx = maxx - minx;
	if (std::abs(lengthx) < 1e-6f) lengthx = 1.0f;
	float lengthy = maxy - miny;
	if (std::abs(lengthy) < 1e-6f) lengthy = 1.0f;

	for (int i = 0; i < linecount; i++) {
		int pointcount = static_cast<int>(graphlines[i].size());
		for (int j = 0; j < pointcount; j++) {
			graphlines[i][j][2] = axisx1 + axisxw * (graphlines[i][j][0] - minx) / lengthx;
			graphlines[i][j][3] = axisy2 - axisyh * (graphlines[i][j][1] - miny) / lengthy;
		}
	}

	// Resize cached polyline strokes to match line count without copying gMesh instances
	while (static_cast<int>(cachedstrokes.size()) > linecount) {
		cachedstrokes.back().clear();
		cachedstrokes.pop_back();
	}
	while (static_cast<int>(cachedstrokes.size()) < linecount) {
		cachedstrokes.emplace_back();
	}

	for (int i = 0; i < linecount; i++) {
		int pointcount = static_cast<int>(graphlines[i].size());
		std::vector<std::vector<std::array<float, 2>>> runs;
		std::vector<std::array<float, 2>> currentrun;

		for (int j = 0; j < pointcount; j++) {
			if (rangeenabled && (graphlines[i][j][0] < rangestart || graphlines[i][j][0] > rangeend)) {
				if (currentrun.size() >= 2) {
					runs.push_back(std::move(currentrun));
				}
				currentrun.clear();
				continue;
			}
			currentrun.push_back({graphlines[i][j][2], graphlines[i][j][3]});
		}
		if (currentrun.size() >= 2) {
			runs.push_back(std::move(currentrun));
		}

		buildPolylineStroke(cachedstrokes[i], runs, linethickness, roundedcorners);
	}

	if (!pointsenabled) {
		for (auto& item : cachedcircles) {
			item.clear();
		}
		cachedcircles.clear();
		circlesmap.clear();
		return;
	}

	// Resize cached circles to match total point count
	int diff = static_cast<int>(cachedcircles.size()) - totalpoints;
	if (diff > 0) {
		for (int i = 0; i < diff; i++) {
			gCircle& circle = cachedcircles.back();
			circle.clear();
			cachedcircles.pop_back();
		}
	} else {
		for (int i = 0; i < -diff; i++) {
			cachedcircles.push_back(gCircle());
		}
	}

	circlesmap.clear();
	size_t circleindex = 0;
	for (int i = 0; i < linecount; i++) {
		int pointcount = static_cast<int>(graphlines[i].size());
		for (int j = 0; j < pointcount; j++) {
			if (circleindex >= cachedcircles.size()) break;
			cachedcircles[circleindex].setPoints(graphlines[i][j][2], graphlines[i][j][3], 5, true, 64);
			circlesmap[hash(i, j)] = circleindex;
			circleindex++;
		}
	}
}

void gGUILineGraph::removeFirstPointsFromLine(int lineIndex, int pointNumLimit) {
	if (lineIndex < 0 || lineIndex >= static_cast<int>(graphlines.size())) return;
	if (pointNumLimit <= 0) return;
	if (static_cast<int>(graphlines[lineIndex].size()) >= pointNumLimit) {
		for (int i = 0; i < pointNumLimit; i++) graphlines[lineIndex].pop_front();
		needsupdate = true;
	}
}

int gGUILineGraph::getPointNum(int lineIndex) {
	if (lineIndex < 0 || lineIndex >= static_cast<int>(graphlines.size())) return 0;
	return static_cast<int>(graphlines[lineIndex].size());
}

float gGUILineGraph::getPointXValue(int lineIndex, int pointIndex) {
	if (lineIndex < 0 || lineIndex >= static_cast<int>(graphlines.size())) return 0.0f;
	if (pointIndex < 0 || pointIndex >= static_cast<int>(graphlines[lineIndex].size())) return 0.0f;
	return graphlines[lineIndex][pointIndex][0];
}

float gGUILineGraph::getPointYValue(int lineIndex, int pointIndex) {
	if (lineIndex < 0 || lineIndex >= static_cast<int>(graphlines.size())) return 0.0f;
	if (pointIndex < 0 || pointIndex >= static_cast<int>(graphlines[lineIndex].size())) return 0.0f;
	return graphlines[lineIndex][pointIndex][1];
}

void gGUILineGraph::clear() {
	graphlines.clear();
	clearGeometryCache();

	smallestvaluex = std::numeric_limits<float>::max();
	largestvaluex = std::numeric_limits<float>::lowest();
	smallestvaluey = std::numeric_limits<float>::max();
	largestvaluey = std::numeric_limits<float>::lowest();

	minx = 0.0f;
	maxx = 0.0f;
	miny = 0.0f;
	maxy = 0.0f;

	needsupdate = true;
}

void gGUILineGraph::setOffset(int leftOffset, int rightOffset, int topOffset, int bottomOffset) {
	this->offsetleft = leftOffset;
	this->offsetright = rightOffset;
	this->offsettop = topOffset;
	this->offsetbottom = bottomOffset;
	updateOffset();
}

void gGUILineGraph::updateOffset() {
	axisx1 = left + offsetleft;
	axisy1 = top + offsettop;

	axisxw = width - (offsetleft + offsetright);
	axisyh = height - (offsettop + offsetbottom);
	axisx2 = axisx1 + axisxw;
	axisy2 = axisy1 + axisyh;

	axisxstart = axisy2;
	if (miny < 0 && (maxy - miny) != 0) axisxstart -= axisyh * ((-miny) / (maxy - miny));
	axisystart = axisx1;
	if (minx < 0 && (maxx - minx) != 0) axisystart += axisxw * ((-minx) / (maxx - minx));

	setLabelCountX(labelcountx);
	setLabelCountY(labelcounty);
	needsupdate = true;
}
