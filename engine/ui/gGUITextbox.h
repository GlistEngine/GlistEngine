/****************************************************************************
 * Copyright (c) 2014 Nitra Games Ltd., Istanbul, Turkey                    *
 *                                                                          *
 * Permission is hereby granted, free of charge, to any person obtaining a  *
 * copy of this software and associated documentation files (the            *
 * "Software"), to deal in the Software without restriction, including      *
 * without limitation the rights to use, copy, modify, merge, publish,      *
 * distribute, distribute with modifications, sublicense, and/or sell       *
 * copies of the Software, and to permit persons to whom the Software is    *
 * furnished to do so, subject to the following conditions:                 *
 *                                                                          *
 * The above copyright notice and this permission notice should not be      *
 * deleted from the source form of the Software.                            *
 *                                                                          *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS  *
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF               *
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.   *
 * IN NO EVENT SHALL THE ABOVE COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,   *
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR    *
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR    *
 * THE USE OR OTHER DEALINGS IN THE SOFTWARE.                               *
 *                                                                          *
 * Except as contained in this notice, the name(s) of the above copyright   *
 * holders shall not be used in advertising or otherwise to promote the     *
 * sale, use or other dealings in this Software without prior written       *
 * authorization.                                                           *
 ****************************************************************************/

/****************************************************************************
 * Author: Noyan Culum, 2021-09-02, Utku Sarialan, 2022-on                 	*
 ****************************************************************************//*
 * gTextbox.h
 *
 *  Created on: Sep 2, 2021
 *      Author: noyan
 */

#ifndef UI_GGUITEXTBOX_H_
#define UI_GGUITEXTBOX_H_

#include "gGUIControl.h"
#include <stack>
#include "gGUIManager.h"


class gGUITextbox: public gGUIControl {
public:

	gGUITextbox();
	virtual ~gGUITextbox();

	virtual void set(gBaseApp* root, gBaseGUIObject* topParentGUIObject, gBaseGUIObject* parentGUIObject, int parentSlotLineNo, int parentSlotColumnNo, int x, int y, int w, int h);

	/**
	 * Changes the text displayed in the textbox.
	 *
	 * @param text Text you want to put in the textbox.
	 */
	void setText(const std::string& text);

	/**
	 * Returns the text that is currently inside the textbox.
	 *
	 * @return Text inside the textbox.
	 */
	std::string getText();

	/**
	 * Enables/disables the textbox for editing.
	 *
	 * @param isEditable Boolean value to enable/disable the textbox for editing.
	 * If it is 'true', textbox becomes editable. If it is 'false', textbox
	 * becomes static.
	 */
	void setEditable(bool isEditable);

	/**
	 * Returns the bool value that indicates if textbox is editable or not.
	 *
	 * @return Bool value that indicates if textbox is editable or not.
	 */
	bool isEditable();

	/**
	 * Sets the line count of the textbox.
	 *
	 * NOTE: Currently if line count is set to a higher value than 1, editing
	 * is not functioning properly. So, if you want to use multilines in your
	 * textbox, use the setText() function.
	 *
	 * @param linecount Line count of the textbox. By default it is set to 1.
	 * It must be an integer value bigger than 1.
	 */
	void setLineCount(int linecount);

	/**
	 * Returns the line count of the textbox.
	 *
	 * @return Line count of the textbox.
	 */
	int getLineCount();

	/**
	 * Returns the current position of the cursor.
	 */
	int getCursor(int x, int y);

	/**
	 * Changes the textbox's measurements. Makes sure that textbox's size is
	 * within the panel's limits.
	 *
	 * @param width Width of the textbox.
	 *
	 * @param height Height of the textbox.
	 */
	void setSize(int width, int height);

	/**
	 * Enables/disables the numeric value only mode of the textbox.
	 *
	 * @param isNumeric Boolean value to enable/disable entering numeric values
	 * only. If it is 'true', you can only input numeric values into the textbox,
	 * if it is 'false', it works normally.
	 */
	void setNumeric(bool isNumeric);

	/**
	 * Check if the textbox only allows numeric value entry.
	 *
	 * @return Bool value that indicates if textbox allows only numbers to be
	 * input into the textbox.
	 */
	bool isNumeric();

	/**
	 * Shifts the left and right coordinates of the textbox by given value. Makes
	 * sure that both left and right coordinates are within the limits of the
	 * panel.
	 *
	 * @param left Desired shift value for the left coordinate of the textbox. It
	 * can be a negative integer value.
	 */
	void addLeftMargin(int left);

	/**
	 * Shifts the top and bottom coordinates of the textbox by given value. Makes
	 * sure that both top and bottom coordinates are within the limits of the
	 * panel.
	 *
	 * @param top Desired shift value for the top coordinate of the textbox. It
	 * can be a negative integer value.
	 */
	void addTopMargin(int top);

	/**
	 * Changes the top margin between lines.
	 *
	 * @param linetopmargin Desired margin value between lines.
	 */
	void setLineTopMargin(int linetopmargin);

	/**
	 * Converts textbox into a password box or converts password box back into a
	 * textbox.
	 *
	 * @param isPassword Boolean value to select between password box and textbox.
	 * If it is true, password box is active. If it is false, textbox is active.
	 */
	void setPassword(bool isPassword);

	/**
	 * Check if the textbox is a password box.
	 *
	 * @return Bool value that indicates if textbox is a password box
	 */
	bool isPassword();

	/**
	 * Enables/disable drawing a background for textbox.
	 *
	 * @param isEnabled Boolean value to enable/disable drawing a background.
	 */
	void enableBackground(bool isEnabled);
	/*
	 * Enables/disables the vertical margin between textbox and its background
	 *
	 */
	void enableVerticalMargin(bool isAligned);
	bool isBackgroundEnabled();

	void update();
	void draw();

	void keyPressed(int key);
	void keyReleased(int key);
	void charPressed(unsigned int codepoint);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void mouseDragged(int x, int y, int button);

	int getTextboxh();
	void cleanText();

	void setFirstX(int firstx);
	void setFirstY(int firsty);

	int getInitX();

	void setTextFont(gFont* textFont);

	void setTextAlignment(int textAlignment, float cellW, int initX);
	float getTextMoveAmount();

	int getCursorPosX();
	void setCursorPosX(int cursorPosX, int length);

private:
	static const int KEY_NONE = 0, KEY_BACKSPACE = 1, KEY_LEFT = 2, KEY_RIGHT = 4, KEY_DELETE = 8, KEY_ENTER = 16, KEY_UP = 32, KEY_DOWN = 64;

	int boxw, boxh;
	int cursorposx, cursorposy, cursorposchar, cursorposutf;
	std::string text;
	int cursorshowcounter, cursorshowcounterlimit, cursorshowlimit;
	bool editmode;
	std::vector<short> letterlength;
	std::vector<int> letterpos;
	int keystate;
	int keypresstime, keypresstimelimit1, keypresstimelimit2;
	int initx;
	int firstchar, firstutf, firstposx;
	int lastutf;
	void handleKeys();
	void pressKey();
	std::vector<int> clickTextbox(int x, int y);
	std::vector<int> calculateClickPosition(int x, int y);
	std::vector<int> calculateClickPositionMultiline(int x, int y);
	std::vector<int> calculateLetterPosition(int letterCharNo);
	std::vector<int> calculateAllLetterPositions();
	void calculateLines();
	int calculateLastUtf();
	int calculateFirstUtf();
	int calculateCharNoFromUtf(int letterUtfNo);
	int findFirstSpace(int lineend);
	void startSelection();
	std::vector<short> readString(const std::string& str);
	bool isLetter(char c);
	bool isNumber(char c);
	void findCursorPosition();
	void findCursorPositionPassword();
	bool selectionmode;
	int selectionposchar1, selectionposchar2;
	int selectionposx1, selectionposx2;
	int selectionposutf1, selectionposutf2;
	int selectionboxx1, selectionboxx2, selectionboxw;
	bool shiftpressed, ctrlpressed;
	bool ctrlcpressed, ctrlvpressed, ctrlxpressed, ctrlapressed, ctrlzpressed;
	bool isdragging;
	long clicktime, previousclicktime, firstclicktime, clicktimediff;
	bool isdoubleclicked, istripleclicked;
	bool iseditable;
	bool isselectedall;
	int linecount;
	int lineheight;
	bool ismultiline;
	int currentline;
	int linetopmargin;
	int hdiff;
	std::vector<std::string> lines;
	std::vector<int> lineendchar;
	int leftlimit, rightlimit, toplimit, bottomlimit;
	bool isnumeric;
	bool ispassword;
	int dotradius;
	bool isbackgroundenabled;
	int totalh;
	int firstx, firsty;
	bool widthchanged;
	bool arrowkeypressed;
	int arrowamount;
	gFont* textfont;
	gGUIManager* manager;
	int textalignment;
	int textalignmentamount;
	float cursormoveamount;
	float textmoveamount;

	//undo stacks
	std::stack<std::string> undostack;
	std::stack<int> cursorposxstack, cursorposystack, cursorposcharstack, cursorposutfstack;
	std::stack<int> firstutfstack, firstcharstack, firstposxstack;

	void pushToStack();
};

#endif /* UI_GGUITEXTBOX_H_ */
