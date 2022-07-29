/*
 * gGUINumberBox.h
 *
 *  Created on: 27 Tem 2022
 *      Author: Aliv
 */

#ifndef UI_GGUINUMBERBOX_H_
#define UI_GGUINUMBERBOX_H_

#include "gGUIControl.h"

class gGUINumberBox: public gGUIControl {
public:
	gGUINumberBox();
	virtual ~gGUINumberBox();

	void draw();

private:
	int boxwidth, boxheight, numboxwidth, numboxheight, smalboxwidth, smalboxheight;

};

#endif /* UI_GGUINUMBERBOX_H_ */
