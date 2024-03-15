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
#include <deque>


class gGUINavigation: public gGUIScrollable {
public:
	gGUINavigation();
	virtual ~gGUINavigation();

	virtual void set(gBaseApp* root, gBaseGUIObject* topParentGUIObject, gBaseGUIObject* parentGUIObject, int parentSlotLineNo, int parentSlotColumnNo, int x, int y, int w, int h);

	virtual void update();
	virtual void draw();

	void addPane(gGUIPane* newPane);
	void setPane(int paneNo, gGUIPane* newPane);
	void removePane(int paneNo);
	gGUIPane* getPane(int paneNo);
	int getPaneNum();
	void clear();

	void setSelectedPane(int paneNo);

	virtual void mousePressed(int x, int y, int button);
	virtual void mouseReleased(int x, int y, int button);

private:
	std::deque<gGUIPane*> panes;
	int panetoph;
	int panelineh;
	int panelinepad;
	int selectedpane;
};

#endif /* UI_GGUINAVIGATION_H_ */
