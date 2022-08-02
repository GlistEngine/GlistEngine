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
 * Author: Noyan Culum, 2021-09-02                                     		*
 ****************************************************************************//*
 * gTextbox.h
 *
 *  Created on: Sep 2, 2021
 *      Author: noyan
 */

#ifndef UI_GGUITEXTBOX_H_
#define UI_GGUITEXTBOX_H_

#include "gGUIControl.h"


class gGUITextbox: public gGUIControl {
public:

	gGUITextbox();
	virtual ~gGUITextbox();

	virtual void set(gBaseApp* root, gBaseGUIObject* topParentGUIObject, gBaseGUIObject* parentGUIObject, int parentSlotLineNo, int parentSlotColumnNo, int x, int y, int w, int h);

	void setText(const std::string& text);
	std::string getText();

	void setEditable(bool isEditable);
	bool isEditable();

	void setLineCount(int linecount);
	int getLineCount();

	int getCursor(int x, int y);

	void update();
	void draw();

	void keyPressed(int key);
	void keyReleased(int key);
	void charPressed(unsigned int codepoint);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void mouseDragged(int x, int y, int button);

private:
	static const int KEY_NONE = 0, KEY_BACKSPACE = 1, KEY_LEFT = 2, KEY_RIGHT = 4, KEY_DELETE = 8, KEY_ENTER = 16;

	int boxw, boxh;
	int cursorposx, cursorposchar, cursorposutf;
	std::string text;
	int cursorshowcounter, cursorshowcounterlimit, cursorshowlimit;
	bool editmode;
	std::vector<short> letterlength;
	std::vector<int> letterpos;
	int keystate;
	int keypresstime, keypresstimelimit1, keypresstimelimit2;
	int initx;
	int firstchar, firstutf, firstposx;
	int lastutf, lutf, futf;
	void handleKeys();
	void pressKey();
	std::vector<int> clickTextbox(int x, int y);
	std::vector<int> calculateClickPosition(int x, int y);
	std::vector<int> calculateLetterPosition(int letterCharNo);
	std::vector<int> calculateAllLetterPositions();
	int calculateLastUtf();
	int calculateFirstUtf();
	int calculateCharNoFromUtf(int letterUtfNo);
	int findFirstSpace(int lineend);
	void startSelection();
	std::vector<short> readString(const std::string& str);
	bool isLetter(char c);
	bool isNumber(char c);
	bool selectionmode;
	int selectionposchar1, selectionposchar2;
	int selectionposx1, selectionposx2;
	int selectionposutf1, selectionposutf2;
	int selectionboxx1, selectionboxx2, selectionboxw;
	bool shiftpressed, ctrlpressed;
	bool ctrlcpressed, ctrlvpressed, ctrlxpressed, ctrlapressed;
	bool isdragging;
	long clicktime, previousclicktime, firstclicktime, clicktimediff;
	bool isdoubleclicked, istripleclicked;
	bool iseditable;
	bool isselectedall;
	int linecount;
	int lineheight;
	bool ismultiline;
};

#endif /* UI_GGUITEXTBOX_H_ */
