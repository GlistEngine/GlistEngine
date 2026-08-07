/*
 * gGUILine.h
 *
 *  Created on: Aug 7, 2026
 *      Author: Antigravity
 */

#ifndef UI_GGUILINE_H_
#define UI_GGUILINE_H_

#include "gGUIControl.h"

class gGUILine : public gGUIControl {
public:
	gGUILine();
	virtual ~gGUILine();

	void setLineColor(gColor color);
	gColor getLineColor();

	void setThickness(float thickness);
	float getThickness();

	void setVertical(bool isVertical);
	bool isVertical();

	virtual void draw();

private:
	gColor linecolor;
	float thickness;
	bool isvertical;
};

#endif /* UI_GGUILINE_H_ */
