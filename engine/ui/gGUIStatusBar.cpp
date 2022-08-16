/*
 * gGUIStatusBar.cpp
 *
 *  Created on: 27 Tem 2022
 *      Author: batuh
 */

#include "gGUIStatusBar.h"
#include "gBaseCanvas.h"
#include "gBaseApp.h"

gGUIStatusBar::gGUIStatusBar() {

	sizerrescaling = true;
	statush = 30;
	setSizer(&statussizer);
}
gGUIStatusBar::~gGUIStatusBar() {

}

void gGUIStatusBar::draw() {
		statusw = getScreenWidth();
		//gLogi("w: ") << statusw;
		gColor oldcolor = *renderer->getColor();
		renderer->setColor(middlegroundcolor);
		//renderer->setColor(navigationbackgroundcolor); //
		//gDrawRectangle(left, top, statusw, statush, true);
		gDrawRectangle(left, top, statusw, statush, true);
		renderer->setColor(middlegroundcolor->r, middlegroundcolor->g, middlegroundcolor->b);
		renderer->setColor(&oldcolor);
		if(guisizer) guisizer->draw();
}

void gGUIStatusBar::windowResized(int w, int h) {
 gLogi("width: ") << w;
 gLogi("height ") << h;
 top = h - statush;
}

