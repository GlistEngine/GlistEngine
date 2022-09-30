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
 * Author: Noyan Culum, Utku Sarialan, 2022                                 *
 ****************************************************************************/

/*
 * gGUIGraph.h
 *
 *  Created on: Sep 23, 2022
 *      Author: noyan
 */

#ifndef UI_GGUIGRAPH_H_
#define UI_GGUIGRAPH_H_

#include "gGUIControl.h"

/**
 * This is a base class for all Graph classes. Developers can use the child classes
 * of this class for their graph needs.
 */
class gGUIGraph: public gGUIControl {
public:
	gGUIGraph();
	virtual ~gGUIGraph();

	virtual void set(gBaseApp* root, gBaseGUIObject* topParentGUIObject, gBaseGUIObject* parentGUIObject, int parentSlotLineNo, int parentSlotColumnNo, int x, int y, int w, int h);

	virtual void setMaxX(int maxX);
	int getMaxX();
	virtual void setMinX(int minX);
	int getMinX();
	virtual void setMaxY(int maxY);
	int getMaxY();
	virtual void setMinY(int minY);
	int getMinY();

	void enableGridlinesX(bool isEnabled);
	void enableGridlinesY(bool isEnabled);

	void setTitleX(std::string titleX);
	std::string getTitleX();
	void setTitleY(std::string titleY);
	std::string getTitleY();

	virtual void setLabelCountX(int labelCount);
	int getLabelCountX();
	virtual void setLabelCountY(int labelCount);
	int getLabelCountY();

	virtual void enableRange(bool isRangeEnabled);
	virtual void setRange(float rangeStart, float rangeEnd);
	int getRangeStart();
	int getRangeEnd();

	void draw();

protected:
	float axisx1, axisy1, axisx2, axisy2, axisxstart, axisystart;
	float axisxw, axisyh;
	float maxy, miny, maxx, minx;
	float largestvaluex, largestvaluey, smallestvaluex, smallestvaluey;
	bool gridlinesxenabled, gridlinesyenabled;
	bool floatlabelsenabled;
	int labelcountx, labelcounty;
	int rangestart, rangeend;
	bool rangeenabled;

private:
	void drawBackground();
	void drawLabels();
	virtual void drawGraph();
	void updateLabelsX();
	void updateLabelsY();
	int countDigits(int number);
	float labelwidthx, labelwidthy;
	std::string axisytitle, axisxtitle;
	std::vector<int> labelsx;
	std::vector<int> labelsy;
	std::vector<float> floatlabelsx;
	std::vector<float> floatlabelsy;
	std::vector<int> labelsteps;
};

#endif /* UI_GGUIGRAPH_H_ */
