/*
 * gGUIButton.h
 *
 *  Created on: Aug 23, 2021
 *      Author: noyan
 */

#ifndef UI_GGUIBUTTON_H_
#define UI_GGUIBUTTON_H_

#include "gGUIControl.h"


class gGUIButton: public gGUIControl {
public:

	static const int GUIEVENT_BUTTONPRESSED = 0, GUIEVENT_BUTTONRELEASED = 1;

	gGUIButton();
	virtual ~gGUIButton();

	void setTitle(std::string title);

	void update();
	void draw();

	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);

private:
	bool ispressed;
	int buttonw, buttonh;
	int tx, ty;

	void resetTitlePosition();
};

#endif /* UI_GGUIBUTTON_H_ */
