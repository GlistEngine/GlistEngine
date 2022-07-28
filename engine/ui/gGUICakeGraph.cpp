/*
 * gGUICakeGraph.cpp
 *
 *  Created on: 28 Tem 2022
 *      Author: buraks
 */

#include "gGUICakeGraph.h"
#include "gBaseApp.h"
#include "gBaseCanvas.h"

gGUICakeGraph::gGUICakeGraph() {
	this->radius = 200;
	this->isFilled = false;
	this->numberOfSides = 60;
}

gGUICakeGraph::~gGUICakeGraph() {

}

void gGUICakeGraph::draw(){
	gDrawCircleSlice(
			(width/2) + left,
			(height/2) + top,
			radius,
			isFilled,
			numberOfSides,
			280);
	renderer->setColor(color.BLACK);
}

void gGUICakeGraph::addVariable(std::string variableName, float variableValue){
	variablenames.push_back(variableName);
	variablevalues.push_back(variableValue);
	gLogi("gGUICakeGraph.h") << "The added variable -> " << variableName << ", " << variableValue;
}
