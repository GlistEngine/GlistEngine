/*
 * gGUISlider.h
 *
 *  Created on: 19 Tem 2022
 *      Author: murat
 */

#ifndef UI_GGUISLIDER_H_
#define UI_GGUISLIDER_H_

#include "gGUIControl.h"

class gGUISlider: public gGUIControl {
public:
	gGUISlider();
	virtual ~gGUISlider();

	void draw();
	void drawtick();
	void drawSlider();
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void mouseDragged(int x, int y, int button);

private:
	int sliderbarh, sliderbarw;
	float sliderh, sliderw;
	bool isdisabled;
	int sliderx;
	bool ispressedslider;
	float ticknum;
	int direction;
	int counter, counterlimit;
	float unitlength;

};

#endif /* UI_GGUISLIDER_H_ */
