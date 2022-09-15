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

	enum {
		TOOLBAR_HORIZONTAL,
		TOOLBAR_VERTICAL
	};

	void setToolbarType(int toolbarType);
	int getToolbarType();

	void draw();

private:
	int toolbartype;
};

#endif /* UI_GGUITOOLBAR_H_ */
