/*
 * gGUINumberBox.h
 *
 *  Created on: 27 Jul 2022
 *      Author: Aliv
 */

#ifndef UI_GGUINUMBERBOX_H_
#define UI_GGUINUMBERBOX_H_

#include "gGUIContainer.h"

class gGUINumberBox: public gGUIContainer {
public:
	gGUINumberBox();
	virtual ~gGUINumberBox();

	void draw();
	/*
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	*/

private:
	int boxwidth, boxheight, numboxwidth, numboxheight, smalboxwidth, smalboxheight;

	bool b1ispressed, b2ispressed;
	bool b1istoggle, b2istoggle;
	bool b1ispressednow, b2ispressednow;
	bool b1isdisabled, b2isdisabled;
};

#endif /* UI_GGUINUMBERBOX_H_ */
