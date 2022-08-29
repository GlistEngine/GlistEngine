/*
 * gGUINotebookPanel.h
 *
 *  Created on: 10 Aug 2022
 *      Author: burakmeydan
 */

#ifndef UI_GGUINOTEBOOKPANEL_H_
#define UI_GGUINOTEBOOKPANEL_H_

#include "gGUIContainer.h"
#include "gGUIPanel.h"

class gGUINotebookPanel: public gGUIContainer {
public:
	gGUINotebookPanel();
	virtual ~gGUINotebookPanel();
	void draw();
	void set(gBaseApp* root, gBaseGUIObject* topParentGUIObject, gBaseGUIObject* parentGUIObject, int parentSlotLineNo, int parentSlotColumnNo, int x, int y, int w, int h);
	void setTitleVisibility(bool isVisible);

private:
	void loadFont();
	void mousePressed(int x, int y, int button);
	void mouseMoved(int x, int y);
	void slideButtonPressed(int x, int y);
	void drawTabs(int x, int tabSize, int index, bool drawFromLeftToRight);
	void drawSlideButtons(bool isRightButton);
	void closeTab(int index);
	gColor colorReductionOnCursor(gColor color);
	int getWidthOfTabLabels();

	bool istitlevisible = false, drawfromlefttoright = true, isslidingleft = false, isslidingright = false, colorreductionl = false, colorreductionr = false;
	int tablinetop = 0, titleh = 20, tabsizersh = 22, tablabelssizew, tabfontsize = 11;
	int spaceforleft = 20, spaceforright = 30, slidebuttonw = 11;
	int indexleft = 0, indexright, indexcursoroncross;
	float oncursorcolorreduction = - 0.1f;

	gGUISizer notebookpanelsizer;
	gColor color;
	gColor tablinecolor = color.LIGHT_GRAY;
	gColor slidebuttoncolor = color.GRAY;
	gColor crossoutlinecolor = color.DARK_GRAY;
	gColor cursoroncrosscolor = color.RED;
	gFont fontfortabs;
	gFbo fbo;
};

#endif /* UI_GGUINOTEBOOKPANEL_H_ */
