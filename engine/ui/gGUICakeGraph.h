/*
 * gGUICakeGraph.h
 *
 *  Created on: 28 Tem 2022
 *      Author: buraks
 */

#ifndef UI_GGUICAKEGRAPH_H_
#define UI_GGUICAKEGRAPH_H_

#include "gGUIControl.h"

class gGUICakeGraph: public gGUIControl {
public:
	gGUICakeGraph();
	virtual ~gGUICakeGraph();

	void draw();
	void addVariable(std::string variableName, float variableValue);

private:
	std::vector<std::string> variablenames;
	std::vector<float> variablevalues;
	float radius, numberOfSides;
	bool isFilled;
	gColor color;
};

#endif /* UI_GGUICAKEGRAPH_H_ */
