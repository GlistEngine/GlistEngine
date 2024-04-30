/*
 * gFrame.h
 *
 *  Created on: Aug 11, 2021
 *      Author: noyan
 */

#ifndef UI_GGUIFRAME_H_
#define UI_GGUIFRAME_H_

#include "gGUIForm.h"


class gGUIFrame : public gGUIForm {
public:
	gGUIFrame();
	gGUIFrame(gBaseApp* root);
	virtual ~gGUIFrame();

	void update();
	void draw();

	int getCursor(int x, int y) override;
private:
};

#endif /* UI_GGUIFRAME_H_ */
