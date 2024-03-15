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
#include "gGUIButton.h"

class gGUINavigation;


class gGUIPane: public gGUIContainer {
public:
	gGUIPane();
	virtual ~gGUIPane();

	virtual void set(gBaseApp* root, gBaseGUIObject* topParentGUIObject, gBaseGUIObject* parentGUIObject, int parentSlotLineNo, int parentSlotColumnNo, int x, int y, int w, int h);

	void setNavigation(gGUINavigation* nav);
	void setNavigationOrder(int orderNo);
	int getNavigationOrder();

	virtual void draw();

	virtual void mouseMoved(int x, int y);
	virtual void mousePressed(int x, int y, int button);
	virtual void mouseReleased(int x, int y, int button);
	virtual void onGUIEvent(int guiObjectId, int eventType, int sourceEventType, std::string value1 = "", std::string value2 = "");

	void setPreviousPane(gGUIPane* previousPane);
	void setNextPane(gGUIPane* nextPane);
	void enablePreviousButton(bool isEnabled);
	void enableNextButton(bool isEnabled);

	gGUIButton getNextButton();

private:
	gGUINavigation* navigation;
	int navorder;
	gGUISizer panesizer;
	gFont titlefont;
	gColor titlecolor;
	int titlefontsize;
	bool navbuttonsenabled;
	gGUISizer buttonsizer;
	gGUIPane *previouspane, *nextpane;
	gGUIButton previousbutton, nextbutton;
	bool previousbuttonenabled, nextbuttonenabled;
};

#endif /* UI_GGUIPANE_H_ */
