/*
 * gGUIToolbar.h
 *
 *  Created on: Sep 25, 2021
 *      Author: noyan
 */

#ifndef UI_GGUITOOLBAR_H_
#define UI_GGUITOOLBAR_H_

#include "gGUIContainer.h"
#include "gGUIToolbarButton.h"


class gGUIToolbar: public gGUIContainer {
public:
	gGUIToolbar();
	virtual ~gGUIToolbar();

	void draw();

private:
};

#endif /* UI_GGUITOOLBAR_H_ */
