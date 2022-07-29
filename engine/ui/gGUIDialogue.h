/*
 * gGUIDialogue.h
 *
 *  Created on: 27 Tem 2022
 *      Author: Umut Can
 */

#ifndef UI_GGUIDIALOGUE_H_
#define UI_GGUIDIALOGUE_H_

#include "gGUIForm.h"

class gGUIDialogue: public gGUIForm {
public:
	gGUIDialogue();
	virtual ~gGUIDialogue();

	void update();
	void draw();

	void showDialogue();
};

#endif /* UI_GGUIDIALOGUE_H_ */
