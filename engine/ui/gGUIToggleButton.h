/*
 * gGUIToggleButton.h
 *
 *  Created on: 18 Tem 2022
 *      Author: utkus
 */

#ifndef UI_GGUITOGGLEBUTTON_H_
#define UI_GGUITOGGLEBUTTON_H_

#include "gGUIControl.h"

class gGUIToggleButton: public gGUIControl {
public:
	gGUIToggleButton();
	virtual ~gGUIToggleButton();

	void draw();
	void mousePressed(int x, int y, int button);
	bool isOn();

private:
	int togglew, toggleh;
	bool ison;
	bool isdisabled;
};

#endif /* UI_GGUITOGGLEBUTTON_H_ */
