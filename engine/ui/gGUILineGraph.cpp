/*
 * gGUILineGraph.cpp
 *
 *  Created on: 28 July 2022
 *      Author: Berke Adil
 */


#include "gGUILineGraph.h"
#include "gGUISlider.h"

gGUILineGraph::gGUILineGraph() {
	title = "Graph";
	pointsenabled = true;

	linecolors[0] = {0.20f, 0.20f, 0.96f};
	linecolors[1] = {0.96f, 0.46f, 0.55f};
	linecolors[2] = {0.62f, 0.80f, 0.41f};
	linecolors[3] = {0.47f, 0.63f, 0.96f};
	linecolors[4] = {0.87f, 0.68f, 0.40f};
	linecolors[5] = {0.73f, 0.60f, 0.96f};
}

gGUILineGraph::~gGUILineGraph() {

}

void gGUILineGraph::set(gBaseApp* root, gBaseGUIObject* topParentGUIObject, gBaseGUIObject* parentGUIObject, int parentSlotLineNo, int parentSlotColumnNo, int x, int y, int w, int h) {
	gGUIGraph::set(root, topParentGUIObject, parentGUIObject, parentSlotLineNo, parentSlotColumnNo, x, y, w, h);
	updatePoints();
}

void gGUILineGraph::setMaxX(int maxX){
	gGUIGraph::setMaxX(maxX);
	updatePoints();
}


void gGUILineGraph::setMinX(int minX) {
	gGUIGraph::setMinX(minX);
	updatePoints();
}

void gGUILineGraph::setMaxY(int maxY) {
	gGUIGraph::setMaxY(maxY - 1);
	updatePoints();
}


void gGUILineGraph::setMinY(int minY) {
	gGUIGraph::setMinY(minY);
	updatePoints();
}

void gGUILineGraph::setLabelCountX(int labelCount){
	gGUIGraph::setLabelCountX(labelCount);
	updatePoints();
}

void gGUILineGraph::setLabelCountY(int labelCount) {
	gGUIGraph::setLabelCountY(labelCount);
	updatePoints();
}

void gGUILineGraph::enablePoints(bool arePointsEnabled) {
	pointsenabled = arePointsEnabled;
}

void gGUILineGraph::setLineColor(int lineIndex, gColor lineColor) {
	linecolors[lineIndex] = lineColor;
}

gColor gGUILineGraph::getLineColor(int lineIndex) {
	return linecolors[lineIndex];
}

void gGUILineGraph::addLine() {
	std::deque<std::array<float, 4>> newline;
	graphlines.push_back(newline);
}

void gGUILineGraph::addData(int lineIndex, std::deque<std::array<float, 2>> dataToAdd) {
	int datasize = dataToAdd.size();
	for(int i = 0; i < datasize; i++) addPointToLine(lineIndex, dataToAdd[i][0], dataToAdd[i][1]);
}

void gGUILineGraph::addPointToLine(int lineIndex, float x, float y) {
	if(graphlines.size() - 1 < lineIndex) return;
	if(x < smallestvaluex) setMinX(x);
	else if(x > largestvaluex) setMaxX(x);
	if(y < smallestvaluey) setMinY(y);
	else if(y > largestvaluey) setMaxY(y);
	setLabelCountX(getLabelCountX());
	setLabelCountY(getLabelCountY());
	int pointcount = graphlines[lineIndex].size();
	if(pointcount == 0) {
		graphlines[lineIndex].push_back({x, y, axisx1 + axisxw * (x - minx) / (maxx - minx), axisy2 - axisyh * (y - miny) / (maxy - miny)});
		return;
	}
	int index = 0;
	while(index < pointcount) {
		if(graphlines[lineIndex][index++][0] < x) continue;
		index--;
		break;
	}
	graphlines[lineIndex].insert(graphlines[lineIndex].begin() + index, {x, y, axisx1 + axisxw * (x - minx) / (maxx - minx), axisy2 - axisyh * (y - miny) / (maxy - miny)});
}

void gGUILineGraph::setPointValues(int lineIndex, int pointIndex, float newX, float newY) {
	if(graphlines.size() - 1 < lineIndex) return;
		if(newX < smallestvaluex) setMinX(newX);
		else if(newX > largestvaluex) setMaxX(newX);
		if(newY < smallestvaluey) setMinY(newY);
		else if(newY > largestvaluey) setMaxY(newY);
		setLabelCountX(getLabelCountX());
		setLabelCountY(getLabelCountY());
		int pointcount = graphlines[lineIndex].size();
		int index = 0;
		while(index < pointcount) {
			if(graphlines[lineIndex][index++][0] < newX) continue;
			index--;
			break;
		}
		graphlines[lineIndex][pointIndex - 1] = {newX,newY, axisx1 + axisxw * (newX - minx) / (maxx - minx), axisy2 - axisyh * (newY - miny) / (maxy - miny)};
}

void gGUILineGraph::setPointValues(int lineIndex, float oldX, float oldY, float newX, float newY) {
    if(graphlines.size() - 1 < lineIndex) return;
    if(newX < smallestvaluex) {
		setMinX(newX);
	} else if(newX > largestvaluex) {
		setMaxX(newX);
	}
	if(newY < smallestvaluey) {
		setMinY(newY);
	} else if(newY > largestvaluey) {
		setMaxY(newY);
	}
    setLabelCountX(getLabelCountX());
    setLabelCountY(getLabelCountY());
    int pointcount = graphlines[lineIndex].size();
    int index = 0;
    while(index < pointcount) {
        if(graphlines[lineIndex][index++][0] < newX) continue;
        index--;
        break;
    }
    for (int i = 0; i < pointcount; i++) {
        if (graphlines[lineIndex][i][0] == oldX && graphlines[lineIndex][i][1] == oldY) {
            graphlines[lineIndex][i][0] = newX;
            graphlines[lineIndex][i][1] = newY;
            graphlines[lineIndex][i][2] = axisx1 + axisxw * (newX - minx) / (maxx - minx);
            graphlines[lineIndex][i][3] = axisy2 - axisyh * (newY - miny) / (maxy - miny);
        }
    }
	needsupdate = true;
}

void gGUILineGraph::drawGraph() {
	if(needsupdate) {
		updatePoints();
		needsupdate = false;
	}
	if(graphlines.empty()) {
		return;
	}

	gColor oldcolor = *renderer->getColor();

	int linecount = graphlines.size();
	for(int i = 0; i < linecount; i++) {
		renderer->setColor(linecolors[i % linecolornum]);
		int pointcount = graphlines[i].size();
		bool skipped = true;
		for(int j = 0; j < pointcount; j++) {
			if(rangeenabled) {
				if(graphlines[i][j][0] < rangestart || graphlines[i][j][0] > rangeend) {
					skipped = true;
					continue;
				}
			}
            if(pointsenabled) gDrawCircle(graphlines[i][j][2], graphlines[i][j][3], 5, true);

            if(!skipped) gDrawLine(graphlines[i][j-1][2], graphlines[i][j-1][3], graphlines[i][j][2], graphlines[i][j][3]);
            skipped = false;
		}
	}

	renderer->setColor(oldcolor);
}

void gGUILineGraph::updatePoints() {
	int linecount = graphlines.size();
	int points = 0;
	for(int i = 0; i < linecount; i++) {
		int pointcount = graphlines[i].size();
		for(int j = 0; j < pointcount; j++) {
			graphlines[i][j][2] = axisx1 + axisxw * (graphlines[i][j][0] - minx) / (maxx - minx);
			graphlines[i][j][3] = axisy2 - axisyh * (graphlines[i][j][1] - miny) / (maxy - miny);
			points++;
		}
	}

	// resize the cached lines, create or destroy them as necessary
	int diff = cachedlines.size() - points;
	if (diff > 0) {
		// we have too many lines, so we destroy the last ones
		for (int i = 0; i < diff; i++) {
			gLine& line = cachedlines.back();
			cachedlines.pop_back();
			line.clear();
		}
	} else {
		// we need more lines, so we create them
		for (int i = 0; i < -diff; i++) {
			cachedlines.push_back(gLine());
		}
	}

	// update the lines
	linesmap.clear(); // clears the cached index to line pointer map

	auto lineit = cachedlines.begin();
	// iterate over all lines, and update the points
	for(int i = 0; i < linecount; i++) {
		int pointcount = graphlines[i].size();
		for(int j = 1; j < pointcount; j++) {
			if (lineit == cachedlines.end()) {
				// no cached lines left, this should not happen as we have already resized the cache
				// but kept here for safety, it would crash otherwise
				break;
			}
			lineit->setThickness(1.0f);
			lineit->setPoints(graphlines[i][j-1][2], graphlines[i][j-1][3], graphlines[i][j][2], graphlines[i][j][3]);
			linesmap[hash(i, j)] = &*lineit;

			lineit++; // get the next line from the list
		}
	}

	if (!pointsenabled) {
		// clear everything
		for (auto& item : cachedcircles) {
			item.clear();
		}
		cachedcircles.clear();
		circlesmap.clear();
		return;
	}

	// resize the cached circles, create or destroy them as necessary
	diff = cachedcircles.size() - points;
	if (diff > 0) {
		// we have too many lines, so we destroy the last ones
		for (int i = 0; i < diff; i++) {
			gCircle& circle = cachedcircles.back();
			cachedcircles.pop_back();
			circle.clear();
		}
	} else {
		// we need more circles, so we create them
		for (int i = 0; i < -diff; i++) {
			cachedcircles.push_back(gCircle());
		}
	}

	// update the circles
	circlesmap.clear(); // clears the cached index to line pointer map

	auto circleit = cachedcircles.begin();
	// iterate over all circles, and update the poitns
	for(int i = 0; i < linecount; i++) {
		int pointcount = graphlines[i].size();
		for(int j = 1; j < pointcount; j++) {
			if (circleit == cachedcircles.end()) {
				// no cached circles left, this should not happen as we have already resized the cache
				// but kept here for safety, it would crash otherwise
				break;
			}
			circleit->setPoints(graphlines[i][j][2], graphlines[i][j][3], 5, true, 64);
			circlesmap[hash(i, j)] = &*circleit;

			circleit++; // get the next circle from the list
		}
	}
}

void gGUILineGraph::removeFirstPointsFromLine(int lineIndex, int pointNumLimit) {
    if (!graphlines.empty()) {
        if (graphlines[lineIndex].size() >= pointNumLimit) {
            for(int i = 0; i < pointNumLimit; i++) graphlines[lineIndex].pop_front();
        }
    }
}

int gGUILineGraph::getPointNum(int lineIndex) {
	return graphlines[lineIndex].size();
}

float gGUILineGraph::getPointXValue(int lineIndex, int pointIndex) {
	return graphlines[lineIndex][pointIndex][0];
}

float gGUILineGraph::getPointYValue(int lineIndex, int pointIndex) {
	return graphlines[lineIndex][pointIndex][1];
}

void gGUILineGraph::clear() {
	graphlines.clear();
}
