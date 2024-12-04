/*
 * gGUITopContainer.h
 *
 *  Created on: Aug 20, 2021
 *      Author: noyan
 */

#ifndef UI_GGUIFORM_H_
#define UI_GGUIFORM_H_

#include "gBaseGUIObject.h"
class gGUISizer;
class gGUIMenubar;
class gGUIToolbar;
class gGUIContextMenu;
class gGUIStatusBar;
class gGUITreelist;
class gGUITooltipText;


class gGUIForm: public gBaseGUIObject {
public:
	gGUIForm();
	virtual ~gGUIForm();

	enum {
		RESIZE_MENUBAR,
		RESIZE_STATUSBAR,
		RESIZE_TOOLBAR_HORIZONTAL,
		RESIZE_TOOLBAR_VERTICAL
	};

	void setMenuBar(gGUIMenubar* menuBar);
	void resizeMenuBar();
	void addToolBar(gGUIToolbar* toolBar);
	void resizeToolbars();
	void resizeVerticalToolbars();
	void setStatusBar(gGUIStatusBar* statusBar);
	void resizeStatusBar();
	void addContextMenu(gGUIContextMenu* contextMenu);
	void addTreelist(gGUITreelist* treeList, int treeListx, int treeListy, int treeListw);
	void resizeAll(int resizeCode);

	void setToolbarHeight(int toolbarHeight);
	int getToolbarHeight();
	void setVerticalToolbarWidth(int verticalToolbarWidth);
	int getVerticalToolbarWidth();

	void setSizer(gGUISizer* guiSizer);
	void updateSizer();
	gGUISizer* getSizer();

	virtual int getCursor(int x, int y);
	int getFocusId();
	int getPreviousFocusId();

	void show();
	void hide();

	void keyPressed(int key);
	void keyReleased(int key);
	void charPressed(unsigned int codepoint);
	void mouseMoved(int x, int y);
	void mousePressed(int x, int y, int button);
	void mouseDragged(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void mouseScrolled(int x, int y);
	void mouseEntered();
	void mouseExited();
	void windowResized(int w, int h);

	void setTooltipText(gGUITooltipText* tooltiptext);

	void showProgressBar(int style = 0, gColor col = gColor(1.0f, 1.0f, 1.0f), float speed = 1.0f);
	void hideProgressBar();
	bool isProgressShown();

protected:
	static const int maxtoolbarnum = 9;
	gGUISizer* guisizer;
	bool isshown;
	gGUIMenubar* menubar;
	gGUIToolbar *toolbars[maxtoolbarnum];
	gGUIToolbar *verticaltoolbars[maxtoolbarnum];
	gGUIStatusBar* statusbar;
	int toolbarnum, verticaltoolbarnum;
	int menuh;
	int toolbarh, verticaltoolbarw;
	int statusw, statush;
	int sizerh;
	gGUIContextMenu* contextmenu;
	int contextmenuw, contextmenuh;
	int contextmenux, contextmenuy;
	gGUITreelist* treelist;
	int treelistw, treelisth;
	std::vector<gGUITooltipText*> vectooltiptext;
	bool isprogressshown;
	float cprdeg;
	int cpx, cpy, cpw, cph;
	gColor cpcolor;
	float cpspeed;
};

#endif /* UI_GGUIFORM_H_ */
