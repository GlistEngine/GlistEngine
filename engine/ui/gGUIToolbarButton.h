/*
 * gGUIToolbarButton.h
 *
 *  Created on: Sep 25, 2021
 *      Author: noyan
 */

#ifndef UI_GGUITOOLBARBUTTON_H_
#define UI_GGUITOOLBARBUTTON_H_

#include "gGUIButton.h"


class gGUIToolbarButton: public gGUIButton {
public:
	gGUIToolbarButton();
	virtual ~gGUIToolbarButton();

	void set(gBaseApp* root, gBaseGUIObject* topParentGUIObject, gBaseGUIObject* parentGUIObject, int parentSlotLineNo, int parentSlotColumnNo, int x, int y, int w, int h);

	void setImage(int iconId);

	void draw();

//	void mousePressed(int x, int y, int button);
//	void mouseReleased(int x, int y, int button);

private:
	int iconid;
	int iw, ih, ix, iy;
};

#endif /* UI_GGUITOOLBARBUTTON_H_ */
