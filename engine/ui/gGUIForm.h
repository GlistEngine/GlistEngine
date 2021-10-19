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


class gGUIForm: public gBaseGUIObject {
public:
	gGUIForm();
	virtual ~gGUIForm();

	void setMenuBar(gGUIMenubar* menuBar);
	void addToolBar(gGUIToolbar* toolBar);

	void setSizer(gGUISizer* guiSizer);
	gGUISizer* getSizer();

	virtual int getCursor(int x, int y);

	void show();
	void hide();

	void keyPressed(int key);
	void keyReleased(int key);
	void charPressed(unsigned int codepoint);
	void mouseMoved(int x, int y);
	void mousePressed(int x, int y, int button);
	void mouseDragged(int x, int y, int button);
	void mouseReleased(int x, int y, int button);

protected:
	static const int maxtoolbarnum = 9;
	gGUISizer* guisizer;
	bool isshown;
	gGUIMenubar* menubar;
	gGUIToolbar *toolbars[maxtoolbarnum];
	int toolbarnum;
	int menuh;
	int toolbarh;
};

#endif /* UI_GGUIFORM_H_ */
