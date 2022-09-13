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

class gGUIForm: public gBaseGUIObject {
public:
	gGUIForm();
	virtual ~gGUIForm();

	void setMenuBar(gGUIMenubar* menuBar);
	void resizeMenuBar();
	void addToolBar(gGUIToolbar* toolBar);
	void resizeToolbars();
	void setStatusBar(gGUIStatusBar* statusBar);
	void resizeStatusBar();
	void addContextMenu(gGUIContextMenu* contextMenu);
	void addTreelist(gGUITreelist* treeList, int treeListx, int treeListy, int treeListw);

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

protected:
	static const int maxtoolbarnum = 9;
	gGUISizer* guisizer;
	bool isshown;
	gGUIMenubar* menubar;
	gGUIToolbar *toolbars[maxtoolbarnum];
	gGUIStatusBar* statusbar;
	int toolbarnum;
	int menuh;
	int toolbarh;
	int statusw, statush;
	int sizerh;
	gGUIContextMenu* contextmenu;
	int contextmenuw, contextmenuh;
	int contextmenux, contextmenuy;
	gGUITreelist* treelist;
	int treelistw, treelisth;
	bool issizerset;
};

#endif /* UI_GGUIFORM_H_ */
