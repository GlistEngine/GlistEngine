/*
 * gGUINavigation.h
 *
 *  Created on: Apr 3, 2022
 *      Author: noyan
 */

#ifndef UI_GGUINAVIGATION_H_
#define UI_GGUINAVIGATION_H_

#include "gGUIScrollable.h"
#include "gGUIPane.h"
#include "gGUIToolbar.h"
#include <deque>


class gGUINavigation: public gGUIScrollable {
public:
	gGUINavigation();
	virtual ~gGUINavigation();

	virtual void set(gBaseApp* root, gBaseGUIObject* topParentGUIObject, gBaseGUIObject* parentGUIObject, int parentSlotLineNo, int parentSlotColumnNo, int x, int y, int w, int h);

	virtual void update();
	virtual void draw();

	void addPane(gGUIPane* newPane, bool isEnabled = true);
	void setPane(int paneNo, gGUIPane* newPane, bool isEnabled = true);
	void removePane(int paneNo);
	gGUIPane* getPane(int paneNo);
	int getPaneNum();
	void setPaneEnabled(int paneNo, bool isEnabled);
	bool isPaneEnabled(int paneNo);
	void clear();

	void setSelectedPaneNo(int paneNo);
	int getSelectedPaneNo();
	gGUIPane* getSelectedPane();

	void showPane(gGUIPane* paneToShow);

	virtual void mousePressed(int x, int y, int button);
	virtual void mouseReleased(int x, int y, int button);
	virtual void mouseMoved(int x, int y);
	virtual void mouseDragged(int x, int y, int button);
	virtual void mouseEntered();
	virtual void mouseExited();

	void enableToolbar();
	gGUISizer* getToolbarSizer();

private:
	std::deque<gGUIPane*> panes;
	std::deque<bool> paneenabled;
	int panetoph;
	int panelineh;
	int panelinepad;
	int selectedpane;

	gGUISizer maintoolbarsizer;
	gGUIToolbar toolbar;
	gGUISizer toolbarsizer;
	bool toolbarenabled;
};

#endif /* UI_GGUINAVIGATION_H_ */
