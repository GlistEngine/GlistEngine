/****************************************************************************
 * Copyright (c) 2014 Nitra Games Ltd., Istanbul, Turkey                    *
 *                                                                          *
 * Permission is hereby granted, free of charge, to any person obtaining a  *
 * copy of this software and associated documentation files (the            *
 * "Software"), to deal in the Software without restriction, including      *
 * without limitation the rights to use, copy, modify, merge, publish,      *
 * distribute, distribute with modifications, sublicense, and/or sell       *
 * copies of the Software, and to permit persons to whom the Software is    *
 * furnished to do so, subject to the following conditions:                 *
 *                                                                          *
 * The above copyright notice and this permission notice should not be      *
 * deleted from the source form of the Software.                            *
 *                                                                          *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS  *
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF               *
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.   *
 * IN NO EVENT SHALL THE ABOVE COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,   *
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR    *
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR    *
 * THE USE OR OTHER DEALINGS IN THE SOFTWARE.                               *
 *                                                                          *
 * Except as contained in this notice, the name(s) of the above copyright   *
 * holders shall not be used in advertising or otherwise to promote the     *
 * sale, use or other dealings in this Software without prior written       *
 * authorization.                                                           *
 ****************************************************************************/

/****************************************************************************
 * Author: Noyan Culum, 2014-on                                             *
 ****************************************************************************/
/*
 * gGUISlider.h
 *
 *  Created on: 27 Tem 2022
 *      Author: yaren
 */
#ifndef UI_GGUISLIDER_H_
#define UI_GGUISLIDER_H_

#include "gGUIControl.h"

class gGUISlider: public gGUIControl {
public:

	/**
	 * Constructor of gGUISlider class. Constructor is automatically called when
	 * object(instance of class) is created. A constructor will have exact same name as
	 * the class and it does not have any return type at all, not even void.
	 * Constructors can be very useful for setting initial values for certain member
	 * variables. In gGUISlider constructor, inital values are set.
	 */
	gGUISlider();
	virtual ~gGUISlider();


	void update();
	void draw();
	void drawSliderBar();
	void drawSlider();
	void drawTick();
	void drawText();
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void mouseDragged(int x, int y, int button);

	/**
	 * Returns the current width of the sliderbar.
	 *
	 * @return width of the sliderbar.
	 */
	float getSliderbarWidth();

	/**
	 * Returns the current height of the sliderbar.
	 *
	 * @return height of the sliderbar.
	 */
	float getSliderbarHeight();

	/**
	 * Sets the visibility of the ticks.
	 *
	 * @param visibility information of the ticks.
	 */
	void setTickVisibility(bool isVisible);

	/**
	 * Sets the visibility of the numbers.
	 *
	 * @param visibility information of the numbers.
	 */
	void setTextVisibility(bool isTextVisible);

	void setDisabled(bool isDisabled);

	/**
	 * Returns the current minimum value of the sliderbar gap.
	 *
	 * @return current minimum value of the sliderbar gap..
	 */
	float getMinValue();

	/**
	 * Sets the minimum value of the sliderbar gap.
	 *
	 * @param minimum value of the sliderbar gap.
	 */
	void setMinValue(float minValue);

	/**
	 * Returns the current maximum value of the sliderbar gap.
	 *
	 * @return current maximum value of the sliderbar gap..
	 */
	float getMaxValue();

	/**
	 * Sets the maximum value of the sliderbar gap.
	 *
	 * @param maximum value of the sliderbar gap.
	 */
	void setMaxValue(float maxValue);

	/**
	 * Returns the value of the current position of the slider.
	 *
	 * @return value of the current position of the slider.
	 */
	float getCurrentValue();

	/**
	 * Sets the tick number.
	 *
	 * @param tick count. Sliderbarw / ticknum must be bigger than sliderw
	 *
	 */
	void setTicknum(float tickNum);

	/**
	 * Sets the color of the disabled slider.
	 *
	 * @param color of the disabled slider. The values of the color must be between 0 and 1.
	 */
	void setDisabledColor(gColor color);

	/**
	 * Returns current color of the disabled slider.
	 *
	 * @return current color of the disabled slider.
	 */
	gColor* getDisabledColor();

	/**
	 * Sets the color of the sliderbar.
	 *
	 * @param color of the sliderbar. The values of the color must be between 0 and 1.
	 */
	void setSliderbarColor(gColor color);

	/**
	 * Returns current color of the sliderbar.
	 *
	 * @return current color of the sliderbar.
	 */
	gColor* getSliderbarColor();

	/**
	 * Sets the color of the slider.
	 *
	 * @param color of the slider. The values of the color must be between 0 and 1.
	 */
	void setSliderColor(gColor color);

	/**
	 * Returns current color of the slider.
	 *
	 * @return current color of the slider.
	 */
	gColor* getSliderColor();

	/**
	 * Sets the color of the pressed slider.
	 *
	 * @param color of the pressed slider. The values of the color must be between 0 and 1.
	 */
	void setPressedSliderColor(gColor color);

	/**
	 * Returns the current color of the pressed slider.
	 *
	 * @return current color of the pressed slider.
	 */
	gColor* getPressedSliderColor();

	/**
	 * Sets the color of the ticks.
	 *
	 * @param color of the ticks. The values of the color must be between 0 and 1.
	 */
	void setTickColor(gColor color);

	/**
	 * Returns the current color of the ticks.
	 *
	 * @return current color of the ticks.
	 */
	gColor* getTickColor();


	bool isTickVisible();
	bool isDisabled();
	bool isPressed();
	bool isSliderPressed();

	virtual void set(gBaseApp* root, gBaseGUIObject* topParentGUIObject, gBaseGUIObject* parentGUIObject, int parentSlotLineNo, int parentSlotColumnNo, int x, int y, int w, int h);

private:
	float sliderbarh, sliderbarw;
	float sliderh, sliderw;
	float sliderx, slidery;
	float sliderbarx, sliderbary;
	float textx, texty;
	float tickx, ticky;
	float currentvaluex, currentvaluey;
	float slidercenterx;
	std::string ticknumbers;
	std::string currentvaluetext;

	bool isdisabled;
	bool ispressedslider;
	bool istickvisible;
	bool ispressed;
	bool istextvisible;

	float ticknum;
	float spacelength;
	float minvalue;
	float maxvalue;
	float currentvalue;
	float direction;
	float counter, counterlimit;
	float mousex;
	float counterpressed, counterpressedlimit;

	gColor slidercolor, pressedslidercolor, disabledcolor;
	gColor sliderbarcolor;
	gColor tickcolor;
};

#endif /* UI_GGUISLIDER_H_ */
