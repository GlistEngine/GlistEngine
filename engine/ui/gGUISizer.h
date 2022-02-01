/*
 * gGUISizer.h
 *
 *  Created on: Aug 20, 2021
 *      Author: noyan
 */

#ifndef UI_GGUISIZER_H_
#define UI_GGUISIZER_H_

#include "gGUIControl.h"


class gGUISizer: public gGUIControl {
public:
	static const int SIZERTYPE_HORIZONTAL = 0, SIZERTYPE_VERTICAL = 1, SIZERTYPE_GRID = 2;

	gGUISizer();
	virtual ~gGUISizer();

	void set(int x, int y, int w, int h);
	int getSizerType();

	void setSize(int lineNum, int columnNum);
	int getLineNum();
	int getColumnNum();
	void setLineProportions(float* proportions);
	void setColumnProportions(float* proportions);
	void setSlotPadding(int padding);

	void setControl(int lineNo, int columnNo, gGUIControl* guiControl);
	gGUIControl* getControl(int lineNo, int columnNo);

	void update();
	void draw();

	void enableBorders(bool isEnabled);
	void enableResizing(bool isEnabled);
	void enableRescaling(bool isEnabled);
	void enableBackgroundFill(bool isEnabled);

	int getCursor(int x, int y);
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

private:
	int sizertype;
	int linenum, columnnum;
	gGUIControl*** guicontrol;
	bool** iscontrolset;
	bool bordersenabled;
	float* lineprs;
	float* columnprs;
	float* linetprs;
	float* columntprs;
	bool resizable;
	int resizecolumn, resizeline;
	int resizex, resizey;
	bool fillbackground;
	int slotpadding;
	bool rescaling;

	int detectSizerType();
};

#endif /* UI_GGUISIZER_H_ */
