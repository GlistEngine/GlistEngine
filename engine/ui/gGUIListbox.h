/*
 * gGUIListbox.h
 *
 *  Created on: Feb 5, 2022
 *      Author: noyan
 */

#ifndef UI_GGUILISTBOX_H_
#define UI_GGUILISTBOX_H_

#include "gGUIScrollable.h"


class gGUIListbox: public gGUIScrollable {
public:
	gGUIListbox();
	virtual ~gGUIListbox();

	void set(gBaseApp* root, gBaseGUIObject* parentGUIObject, int parentSlotLineNo, int parentSlotColumnNo, int x, int y, int w, int h);

	void drawContent();

	void addData(std::string lineData);

private:
	int listboxh;
	int lineh, linenum;
	int minlinenum, minboxh;
	std::vector<std::string> data;
};

#endif /* UI_GGUILISTBOX_H_ */
