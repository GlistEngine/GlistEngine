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

	void set(gBaseApp* root, gBaseGUIObject* topParentGUIObject, gBaseGUIObject* parentGUIObject, int parentSlotLineNo, int parentSlotColumnNo, int x, int y, int w, int h);
	void set(int x, int y, int w, int h);
	int getSizerType();

	void setSize(int lineNum, int columnNum);
	int getLineNum();
	int getColumnNum();
	void setLineProportions(float* proportions);
	void setColumnProportions(float* proportions);
	void setSlotPadding(int padding, int height = 0);

	void setControl(int lineNo, int columnNo, gGUIControl* guiControl);
	void removeControl(int lineNo, int columnNo);
	gGUIControl* getControl(int lineNo, int columnNo);

	void update();
	void draw();

	void enableBorders(bool isEnabled);
	void enableResizing(bool isEnabled);
	void enableBackgroundFill(bool isEnabled);

	void setAlignContentVertically(bool enabled);
	bool isAlignContentVertically();

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
	int getSlotWidth(int lineNo, int columnNo);
	int getSlotHeight(int lineNo, int columnNo);
	int getSlotX(int lineNo, int columnNo);
	int getSlotY(int lineNo, int columnNo);

	bool isControlSet(int lineNo, int columnNo);

private:
	struct Entry {
		gGUIControl* control = nullptr;
		bool isset = false;
	};

	int sizertype;
	int linenum, columnnum;
	std::vector<Entry> guicontrols;
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
	int slotheightpadding;
	bool alignvertically;

private:
	int detectSizerType();
	void reloadControls();
	void reloadControl(gGUIControl& control);
	void reloadControl(gGUIControl& control, int line, int column);

	int indexOf(int line, int column) const;

	void checkSpaces();
};

#endif /* UI_GGUISIZER_H_ */
