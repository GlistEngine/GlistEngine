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

class gGUILineGraph: public gGUIGraph  {

public:
	gGUILineGraph();
	virtual ~gGUILineGraph();

	void set(gBaseApp* root, gBaseGUIObject* topParentGUIObject, gBaseGUIObject* parentGUIObject, int parentSlotLineNo, int parentSlotColumnNo, int x, int y, int w, int h);

	void setMaxX(int maxX);
	void setMinX(int minX);
	void setMaxY(int maxY);
	void setMinY(int minY);

	void setLabelCountX(int labelCount);
	void setLabelCountY(int labelCount);

	void addLine();
	void addPointToLine(int lineindex, float x, float y);


private:
	void drawGraph();
	void updatePoints();

	std::vector<std::vector<std::array<float, 4>>> graphlines;
	gColor linecolors[5];
};


#endif /* UI_GGUILINEGRAPH_H_ */
