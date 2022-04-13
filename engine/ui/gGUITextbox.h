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

	gGUITextbox();
	virtual ~gGUITextbox();

	void setText(const std::string& text);
	std::string getText();

	void setEditable(bool isEditable);
	bool isEditable();

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
	int keystate;
	int keypresstime, keypresstimelimit1, keypresstimelimit2;
	void handleKeys();
	void pressKey();
	std::vector<int> clickTextbox(int x, int y);
	std::vector<int> calculateClickPosition(int x, int y);
	std::vector<int> calculateLetterPosition(int letterNo);
	void startSelection();
	std::vector<short> readString(const std::string& str);
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
};

#endif /* UI_GGUITEXTBOX_H_ */
