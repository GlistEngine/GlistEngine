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
	void addData(std::string lineData);
	void drawContent();

	void setSelected(int lineNo);
	int getSelected();
	std::string getData(int lineNo);

	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);

private:
	int listboxh;
	int lineh, linenum;
	int minlinenum, minboxh;
	int maxlinenum;
	std::vector<std::string> data;
	int firstlineno, flno;
	int selectedno;
	bool mousepressedonlist;
};

#endif /* UI_GGUILISTBOX_H_ */
