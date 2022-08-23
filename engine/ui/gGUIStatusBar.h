/*
 * gGUIStatusBar.h
 *
 *  Created on: 27 Jul 2022
 *      Author: Batuhan Sarihan
 */

#ifndef UI_GGUISTATUSBAR_H_
#define UI_GGUISTATUSBAR_H_

#include "gGUIContainer.h"


class gGUIStatusBar: public gGUIContainer {
public:
	gGUIStatusBar();
	virtual ~gGUIStatusBar();

	void draw();

	void windowResized(int w, int h);

private:
	int statush, statusw;
	gGUISizer statussizer;

};

#endif /* UI_GGUISTATUSBAR_H_ */

