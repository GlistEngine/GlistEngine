/*
 * gGUINotebook.h
 *
 *  Created on: 10 Aug 2022
 *      Author: burakmeydan
 */

#ifndef UI_GGUINOTEBOOK_H_
#define UI_GGUINOTEBOOK_H_

#include "gGUIContainer.h"
#include "gGUIPanel.h"
#include "gRenderer.h"
#include "gFont.h"
#include <deque>

/**
 * gGUINotebook provides displaying multiple sizers for one frame.
 * gGUINotebook is set to framesizer by using "setControl" function.
 * For each gGUISizer which you want to display on gGUINotebook, the
 * addSizerToDeque(&gGUISizer, "sizerNameThatShownOnTab") function is
 * called (gGUISizers must be arranged as you like before calling
 * addSizerToDeque).
 */
class gGUINotebook: public gGUIContainer {
public:
	gGUINotebook();
	virtual ~gGUINotebook();

	void setActiveTab(int index);

	void draw();

	/**
	 * Sets the visibility of the title.
	 *
	 * @param is title visible or not.
	 */
	void setTitleVisibility(bool isVisible);

	/**
	 * Sets the closable property of tabs.
	 *
	 * @param is closable or not.
	 */
	void setClosableTab(bool isClosableTab);

	/**
	 * Adds gGUISizer and store it.
	 *
	 * @param is gGUISizer that is stored in "guisizers" deque.
	 *
	 * @param is gGUISizer name that is stored in "quisizerlabels" vector
	 * and the label of tab.
	 */
	void addSizerToDeque(gGUISizer* guiSizer, std::string sizerLabel = "");

    /**
     * Opens a new tab with the specified gGUISizer.
     *
     * @param guiSizer gGUISizer to be displayed in the new tab.
     * @param sizerLabel Label of the new tab.
     */
    void openTab(gGUISizer* guiSizer, const std::string& sizerLabel);

    /**
     * Closes the tab at the specified index.
     *
     * @param index Index of the tab to be closed.
     */
    void closeTab(int index);
    void drawTabs(int x, int tabSize, int index, bool drawFromLeftToRight);

    /*
     * Sets the size of the sizer with the given name.
     *
     * @param lineNum Number of lines to set to the sizer.
     * @param columnNum Number of columns to set to the sizer.
     * @param sizerLabel Label of the sizer to be sized.
     */
    void setSizerSize(int lineNum, int columnNum, std::string sizerLabel);

    /*
	 * Sets the size of the sizer at the given index.
	 *
	 * @param linenNum Number of lines to set to the sizer.
	 * @param columnNum Number of columns to set to the sizer.
	 * @param sizerIndex Index of the sizer to be sized.
	 */
    void setSizerSize(int lineNum, int columnNum, int sizerIndex);

    /*
     * Returns the sizer at the given index.
     *
     * @param sizerIndex Index of the sizer to be returned.
     */
    gGUISizer* getSizer(int sizerIndex);

    /*
	 * Returns the sizer with the given name.
	 *
	 * @param sizerLabel Label of the sizer to be returned.
	 */
	gGUISizer* getSizer(std::string sizerLabel);

private:
	void loadFont();
	void mousePressed(int x, int y, int button);
	void mouseMoved(int x, int y);
	void slideButtonPressed(int x, int y);

	void drawSlideButtons(bool isRightButton);
	gColor colorReductionOnCursor(gColor color);
	int getWidthOfTabLabels();
	void setSizerFromDeque(int guiSizersIndex);
	void set(gBaseApp* root, gBaseGUIObject* topParentGUIObject, gBaseGUIObject* parentGUIObject, int parentSlotLineNo, int parentSlotColumnNo, int x, int y, int w, int h);

	std::deque<gGUISizer*> guisizers;
	std::vector<std::string> quisizerlabels;
	bool istitlevisible = false, drawfromlefttoright = true, isslidingleft = false, isslidingright = false, colorreductionl = false, colorreductionr = false, isclosabletab = true;
	int tablinetop = 0, titleh = 20, tabsizersh = 22, tablabelssizew, tabfontsize = 11;
	int spaceforleft = 4, spaceforright = 4, slidebuttonw = 11;
	int indexleft = 0, indexright, indexcursoroncross, activesizerindex = -1;
	float oncursorcolorreduction = - 0.1f;
	int tabSize = 20;


	gGUISizer notebooksizer;
	gColor color;
	gColor tablinecolor = color.LIGHT_GRAY;
	gColor slidebuttoncolor = color.GRAY;
	gColor crossoutlinecolor = color.DARK_GRAY;
	gColor cursoroncrosscolor = color.RED;
	gFont fontfortabs;
	gFbo fbo;
};

#endif /* UI_GGUINOTEBOOK_H_ */
