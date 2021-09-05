/*
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

	static const int GUIEVENT_TEXTBOX_ENTRY = 0;

	gGUITextbox();
	virtual ~gGUITextbox();

	int getCursor(int x, int y);

	void update();
	void draw();

	void keyPressed(int key);
	void charPressed(unsigned int codepoint);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);

private:
	int boxw, boxh;
	int cursorposx, cursorposchar;
	std::string text;
	int cursorshowcounter, cursorshowcounterlimit, cursorshowlimit;
	bool editmode;

	char toChars(unsigned int codePoint);
	bool isBmpCodePoint(unsigned int codePoint);
	char toSurrogates(unsigned int codePoint);
	static const int MIN_HIGH_SURROGATE;
	static const int MIN_SUPPLEMENTARY_CODE_POINT;
	static const int MIN_LOW_SURROGATE;
	char highSurrogate(unsigned int codePoint);
	char lowSurrogate(unsigned int codePoint);
};

#endif /* UI_GGUITEXTBOX_H_ */
