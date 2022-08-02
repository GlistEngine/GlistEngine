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
 * Author: Noyan Culum, 2022-07-18                                             *
 ****************************************************************************/
/*
 * gGUISwitchButton.h
 *
 *  Created on: 18 Tem 2022
 *      Author: noyan
 */

#ifndef UI_GGUISWITCHBUTTON_H_
#define UI_GGUISWITCHBUTTON_H_

#include "gGUIControl.h"

/**
 * Sends the information about buttons open or closed status to the gGUIEvent.
 * If the button is off, it sends the value G_GUIEVENT_SWITCHBUTTON_ON (7),
 * and if it is on, it sends the value G_GUIEVENT_SWITCHBUTTON_OFF (8).
 */
class gGUISwitchButton: public gGUIControl {
public:
	gGUISwitchButton();
	virtual ~gGUISwitchButton();

	void draw();
	void mousePressed(int x, int y, int button);

	/**
	 * Returns the button status.
	 *
	 * @return TRUE if the button is on, FALSE if the button is off
	 */
	bool isOn();

private:
	int togglew, toggleh;
	bool ison;
	bool isdisabled;
};

#endif /* UI_GGUISWITCHBUTTON_H_ */
