/*
 * gGUIPane.h
 *
 *  Created on: Apr 3, 2022
 *      Author: noyan
 */

#ifndef UI_GGUIPANE_H_
#define UI_GGUIPANE_H_

#include "gGUIContainer.h"
#include "gFont.h"
#include "gColor.h"


class gGUIPane: public gGUIContainer {
public:
	gGUIPane();
	virtual ~gGUIPane();

	virtual void set(gBaseApp* root, gBaseGUIObject* topParentGUIObject, gBaseGUIObject* parentGUIObject, int parentSlotLineNo, int parentSlotColumnNo, int x, int y, int w, int h);

	virtual void draw();

private:
	gGUISizer panesizer;
	gFont titlefont;
	gColor titlecolor;
	int titlefontsize;
};

#endif /* UI_GGUIPANE_H_ */
