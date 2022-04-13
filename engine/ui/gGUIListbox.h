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

	void set(gBaseApp* root, gBaseGUIObject* topParentGUIObject, gBaseGUIObject* parentGUIObject, int parentSlotLineNo, int parentSlotColumnNo, int x, int y, int w, int h);
	void addData(std::string lineData);
	void insertData(int lineNo, std::string lineData);
	void removeData(int lineNo);
	void removeSelected();
	void clear();
	void drawContent();

	void setSelected(int lineNo);
	int getSelected();
	std::string getSelectedData();
	std::string getData(int lineNo);
	int getDataNum();

	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);

private:
	int listboxh;
	int lineh, linenum;
	int minlinenum, minboxh;
	int maxlinenum;
	std::vector<std::string> data;
	float datady;
	int firstlineno, flno, fldy;
	int selectedno;
	bool mousepressedonlist;
};

#endif /* UI_GGUILISTBOX_H_ */
