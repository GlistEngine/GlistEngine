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
	void slideButtonPressed(int x, int y);
	void drawTabs(int x, int tabSize, int index, bool drawFromLeftToRight);
	void drawSlideButtons(bool isRightButton);
	int getWidthOfTabLabels();

	bool istitlevisible = false, drawfromlefttoright = true, isslidingleft = false, isslidingright = false;
	int tablinetop = 0, titleh = 20, tabsizersh = 22, tablabelssizew, tabfontsize = 11;
	int spaceforleft = 23, spaceforright = 28;
	int indexleft = 0, indexright;

	gGUISizer notebookpanelsizer;

	gColor color;
	gColor tablinecolor = color.LIGHT_GRAY;
	gColor slidebuttoncolor = color.GRAY;
	gFont fontfortabs;

	gFbo fbo;
};

#endif /* UI_GGUINOTEBOOKPANEL_H_ */
