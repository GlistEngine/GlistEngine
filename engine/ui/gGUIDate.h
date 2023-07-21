/*
 * gGUIDate.h
 *
 *  Created on: 19 Tem 2023
 *      Author: halit
 */

#ifndef UI_GGUIDATE_H_
#define UI_GGUIDATE_H_

#include "gGUIControl.h"

class gGUIDate: public gGUIControl {
public:
	gGUIDate();
	virtual ~gGUIDate();
	virtual void setup();
	virtual void draw();

	void drawCalendar(int month, int year);
	void setSize(int w, int h);
	void setPosition(int x, int y);
	void setDaysName(std::vector<std::string> days);
private:
	int getStartDay(int month, int year);
	int getDaysInMonth(int month, int year);

	std::string days[7];
	int daysnameswidths[7];
	int daysnumderswidths[31];
	int textheight;
	int w, h, dx, dy, x, y;
	int leftmargin, topmargin;
};

#endif /* UI_GGUIDATE_H_ */
