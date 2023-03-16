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
 * Author: Sevval Bulburu, Aynur Dogan 2022-07-20                           *
 ****************************************************************************/

/*
 * gGUIProgressBar.h
 *
 *  Created on: July 20, 2022
 *      Author: sevval, aynur
 *      Revised: 16 March 2022 Umutcan Turkmen
 */

#ifndef UI_GGUIPROGRESSBAR_H_
#define UI_GGUIPROGRESSBAR_H_

#include "gGUIControl.h"

/**
 * This class creates and controls a progress bar. Progress bars show the current
 * value in the given range. They basically used for everything that can be
 * increased or decreased.
 *
 * Developers can change the current value of the progress bar according to their
 * calculations. They can change the color of the progress bar.
 *
 * (value * 180) / (valuemax - valuemin)
 * This formula used for calculating the progress bar's current value.
 */
class gGUIProgressBar: public gGUIControl {
public:
	gGUIProgressBar();
	virtual ~gGUIProgressBar();
	void draw();

	/**
	 * Sets the shown value of the progress bar. Developers can determine the shown
	 * value of the progress bar according to the minimum value and the maximum
	 * value.If the value is not between the minimum value and the maximum value,
	 * value is silently ignored.
	 *
	 * @param value The value should be a float number between the minimum value
	 * and the maximum value.
	 */
	void setValue(float value);

	/**
	 * Returns the shown value of the progress bar.
	 */
	float getValue();

	/**
	 * Sets the maximum value of the progress bar. The maximum value can be any
	 * float value between C++ float limits.
	 * The maximum value should be bigger than the minimum value. If the maximum
	 * value is lower than minimum value, the given maximum value is silently
	 * ignored.
	 *
	 * @param value A float number bigger than the minimum value
	 */
	void setMaxValue(float value);

	/**
	 * Sets the minimum value of the progress bar. The minimum value can be any
	 * float value between C++ float limits.
	 * The minimum value should be lower than the maximum value.If the minimum
	 * value is bigger than the maximum value, the minimum value is silently
	 * ignored.
	 *
	 * @param value A float number lower than the maximum value
	 */
	void setMinValue(float value);

	/**
	 * Returns the maximum value of the progress bar.
	 */
	float getMaxValue();

	/**
	 * Returns the maximum value of the progress bar.
	 */
	float getMinValue();

	/**
	 * Sets the color of the progress bar with the given color. Colors consist
	 * of RGB values float between 0.0f-1.0f.
	 *
	 * @param color The given color consist of (r, g, b) float values
	 */
	void setProgressBarColor(float r, float g, float b);

	/**
	 * Sets the color of the background of the progressbar with the given color. Colors consist
	 * of RGB values float between 0.0f-1.0f.
	 *
	 * @param color The given color consist of (r, g, b) float values
	 */
	void setBackgroundColor(float r, float g, float b);

	/**
	 * Returns the color of the progress bar.
	 */
	gColor* getProgressBarColor();

	/**
	 * Sets the thickness of progress bar's borders. Borders can be between 0.0f
	 * and 25.0f.
	 *
	 * @param thickness Should be given float type. If the given value is bigger
	 * than 25.0f than the value is silently ignored. Any value smaller and equal
	 * than 0.0f makes the borders disabled.
	 */
	void setBorderThickness(float thickness);

	/**
	 * Returns the thickness of progress bar's borders.
	 */
	float getBorderThickness();

private:
	int progressbarw, progressbarh;
	float valuemax, valuemin;
	float value;
	float thickness;

	gColor progressbarcolor;
	gColor backgroundcolor;
};

#endif /* UI_GGUIPROGRESSBAR_H_ */
