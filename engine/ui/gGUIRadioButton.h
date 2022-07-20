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
 * Author: Utku Sarýalan, 2022-on                                           *
 ****************************************************************************/

/*
 * gGUIRadioButton.h
 *
 *  Created on: 19 Tem 2022
 *      Author: Utku Sarýalan
 */

#ifndef UI_GGUIRADIOBUTTON_H_
#define UI_GGUIRADIOBUTTON_H_

#include "gGUIControl.h"

class gGUIRadioButton: public gGUIControl {
public:
	gGUIRadioButton();
	virtual ~gGUIRadioButton();

	/**
	 * Changes the title of the radio button group.
	 *
	 * @param title New title value.
	 */
	void setTitle(std::string title);

	/**
	 * Returns the title of the radio button group.
	 *
	 * @return Current title of the radio button group.
	 */
	std::string getTitle();

	/**
	 * Shows/hides the title of the radio button group.
	 *
	 * @param command Boolean value to set the value that controls title's
	 * visibility. If it is false, thetitle becomes invisible. If it is true,
	 * the title becomes visible.
	 */
	void showTitle(bool command);

	/**
	 * Selects a radio button from the group.
	 *
	 * @param index Index value of the designated radio button to select. When
	 * the class is initialized, the value is set to 0. If index is bigger or
	 * equal to the button count, or is smaller than 0, selected button is set
	 * to 0.
	 */
	void setSelectedButton(int index);

	/**
	 * Returns the currently selected radio button's index value.
	 *
	 * @return Index of the selected button.
	 */
	int getSelectedButton();

	/**
	 * Sets the radius of the radio buttons.
	 *
	 * @param radius Float value for radio button radius. When the class is
	 * initialized, the value is set to 6.5.
	 */
	void setButtonRadius(float radius);

	/**
	 * Returns the current radius of the radio buttons.
	 *
	 * @return Radius of the radio buttons.
	 */
	int getButtonRadius();

	/**
	 * Changes the count of radio buttons to the designated value. Deletes
	 * titles of the previous radio buttons and creates new ones with default
	 * radio title "Radio Button".
	 *
	 * @param buttoncount Value that specifies how many radio buttons to have in
	 * the radio button group. If the value is smaller than 0, button count
	 * value becomes 1.
	 */
	void setButtonCount(int buttoncount);

	/**
	 * Returns the count of the radio buttons in the radio button group.
	 *
	 * @return Button count.
	 */
	int getButtonCount();

	/**
	 * Changes the title of a desired radio button. Updates the maximum radio
	 * button title length if the title to be set is longer that it.
	 *
	 * @param index Index value of the radio button with the title change. If
	 * index is bigger or equal to the button count, or is smaller than 0,
	 * nothing happens.
	 *
	 * @param title Title to be assigned to the desired radio button.
	 */
	void setRadioTitle(int index, std::string title);

	/**
	 * Returns the title of a desired radio button.
	 *
	 * @param index Index value of the radio button to find. If index is bigger
	 * or equal to the button count, or is smaller than 0, returns an error
	 * message.
	 *
	 * @return Title of the radio button with given index.
	 */
	std::string getRadioTitle(int index);

	/**
	 * Changes the horizontal distance to put between the radio buttons if there
	 * are more than just one column.
	 *
	 * @param Desired pixel distance to put between radio buttons.
	 */
	void setHorizontalDistance(int distance);

	/**
	 * Returns the horizontal distance between the radio buttons.
	 *
	 * @return Horizontal distance between the radio buttons.
	 */
	int getHorizontalDistance();

	/**
	 * Changes the vertical distance between the radio buttons.
	 *
	 * @param Desired pixel distance to put between radio buttons.
	 */
	void setVerticalDistance(int distance);

	/**
	 * Returns the vertical distance between the radio buttons.
	 *
	 * @return Vertical distance between the radio buttons.
	 */
	int getVerticalDistance();

	/**
	 * Changes the color of the selected radio button.
	 *
	 * @param selectedcolor Desired color value to set in gColor formation.
	 */
	void setSelectedButtonColor(gColor selectedcolor);

	/**
	 * Returns the the color of the selected radio button.
	 *
	 * @return Selected radio button color
	 */
	gColor* getSelectedButtonColor();

	/**
	 * Changes the column count of the radio button group.
	 *
	 * @param columncount Indicates the desired amount of columns when drawing
	 * the radio buttons. If the value is smaller than 0, column count value
	 * becomes 1.
	 */
	void setColumnCount(int columncount);

	/**
	 * Returns the column count of the radio button group.
	 *
	 * @return Column count.
	 */
	int getColumnCount();

	bool isTextVisible();
	bool isDisabled();
	void setDisabled(bool command);

	void update();
	void draw();
	void updateHeightLimit();

	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);

private:
	bool istextvisible;
	bool istitlevisible;
	bool isdisabled;

	float buttonradius;
	int titlew, titleh;
	int buttoncount;
	int lineheightlimit;
	int selectedbutton;
	int linemidpoint;
	int textmargin;
	int buttonmargin;
	int maxradiowidth;
	int horizontaldistance;
	int verticaldistance;
	int columncount;

	std::string *titles;

	gColor titlecolor, selectedcolor;
};

#endif /* UI_GGUIRADIOBUTTON_H_ */
