/*
 * gGUIPanel.h
 *
 *  Created on: Aug 20, 2021
 *      Author: noyan
 */

#ifndef UI_GGUIPANEL_H_
#define UI_GGUIPANEL_H_

#include "gGUIContainer.h"


class gGUIPanel: public gGUIContainer {
public:
	gGUIPanel();
	virtual ~gGUIPanel();

	void showTitle(bool isShown);

	void draw();

private:
	int toplineh;
	gGUISizer panelsizer;
	bool istitleshown;
};

#endif /* UI_GGUIPANEL_H_ */
