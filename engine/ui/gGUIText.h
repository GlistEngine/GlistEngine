/*
 * gGUIText.h
 *
 *  Created on: Sep 1, 2021
 *      Author: noyan
 */

#ifndef UI_GGUITEXT_H_
#define UI_GGUITEXT_H_

#include "gGUIControl.h"


class gGUIText: public gGUIControl {
public:

	static const int TEXTALIGNMENT_LEFT = 0, TEXTALIGNMENT_CENTER = 1, TEXTALIGNMENT_RIGHT = 2;

	gGUIText();
	virtual ~gGUIText();

	void set(gBaseApp* root, gBaseGUIObject* parentGUIObject, int parentSlotLineNo, int parentSlotColumnNo, int x, int y, int w, int h);

	void setText(std::string text);

	void setTextAlignment(int textAligment);

	void draw();

private:
	static const int maxlinenum = 50;

	std::string text;
	std::string line[maxlinenum];
	int linenum;
	int textalignment;
	int linefirstx[maxlinenum];
	int fontsize, lineh;

	void resetText();
	void resetAlignment();

	std::vector<std::string> splitString(const std::string& textToSplit, gFont* font, int lineWidth);
};

#endif /* UI_GGUITEXT_H_ */
