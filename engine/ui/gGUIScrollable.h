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
 * Author: Noyan Culum, Sevval Bulburu, Aynur Dogan 2022-08-19              *
 ****************************************************************************/

/*
 * gGUIScrollable.h
 *
 *  Created on: Feb 5, 2022
 *      Author: noyan
 */

#ifndef UI_GGUISCROLLABLE_H_
#define UI_GGUISCROLLABLE_H_

#include "gGUIControl.h"
#include "gFbo.h"

/*
 * This class is a child class of gGUIControl. Scrollable class provide scrollable
 * function on objects. When the box object's width and height is smaller than
 * it's content, developers can use this class.
 *
 * For using this class, firstly a set function must be wrote in the class that
 * used Scrollalble class. In this set function, set function must be called
 * from gGUIControl class. Secondly, setDimensions() function must be called
 * from Scrollable class. setDimensions() function updates some variables for
 * using draw window and scroll bars. Because of that parameters should be width
 * and height. Except for this two function, developers can call any function
 * which should be setted when added object to panel.
 *
 * After written set function, enableScrollbars function must be called. This
 * function makes scroll bars visible. For using scrollable function at least one
 * of the parameters must be active.
 *
 * If a class is generated from Scrollable class with inheritance, new class must
 * overrided drawContent() function.
 *
 */

class gGUIScrollable: public gGUIControl {
public:
	gGUIScrollable();
	virtual ~gGUIScrollable();

	/*
	 * Makes scrollbars visible or invisible. For using scrollable function in
	 * other gui objects, this function must be used. First parameter is for the
	 * verticle scroll bar and the second one is horizontal scroll bar. Developer
	 * can use both of the equalities or one of them. bool 'true' makes the bars
	 * visible. When a bar is visible, that means that scrollable function is
	 * active for the given direction.
	 *
	 * @param isVerticalEnabled is a bool value that makes vertical scroll bar
	 * active or inactive. 'true ' for activation, 'false ' for inactivation.
	 *
	 * @param isHorizontalEnabled is a bool value that makes horizontal scroll bar
	 * active or inactive. 'true ' for activation, 'false ' for inactivation.
	 */
	void enableScrollbars(bool isVerticalEnabled, bool isHorizontalEnabled);

	void draw();
	/*
	 * Draws contents of class. But it is empty for now, must be overrided in
	 * child classes from this parent class.
	 */
	virtual void drawContent();

	/*
	 *	Draws scroll bars according to if vertical or horizontal movement is
	 *	enabled.
	 */
	virtual void drawScrollbars();

	/*
	 *	Updates the width and height of the window with given parameters. This
	 *	function is overrided from gGUIControl class.
	 *
	 *	@param w is the new width value.
	 *
	 *	@param h is the new height value.
	 */
	virtual void windowResized(int w, int h);

	virtual void mouseMoved(int x, int y);
	virtual void mousePressed(int x, int y, int button);
	virtual void mouseDragged(int x, int y, int button);
	virtual void mouseReleased(int x, int y, int button);

	/*
	 * Makes the scroll movement according to coordinate of the mouse. When mouse
	 * used for scroll, it changed some values of box that we draw.
	 *
	 */
	virtual void mouseScrolled(int x, int y);


	gFbo* getFbo();

	int getTitleTop();

protected:

	/*
	 *	Updates the values of variables that we use to draw window and scrollbars
	 *	with the given parameters.
	 *
	 *	@param w is the new width value.
	 *
	 *	@param h is the new height value.
	 */
	void setDimensions(int width, int height);
	int boxw, boxh;
	int totalh;
	int firstx, firsty;
	int vsbx, vsby, vsbw, vsbh;
	int hsbx, hsby, hsbw, hsbh;
	int titlex, titley, titledy;
	int scrolldiff;

private:
	gFbo* boxfbo;

	bool vsbenabled, hsbenabled, sbenabled;
	bool vsbactive, hsbactive;
	bool iscursoronvsb, iscursoronhsb;
	gColor sbbgcolor, sbfgcolor, sbdragcolor;
	float vsbalpha, hsbalpha;
	float sbalphaboxlimit, sbalphasblimit;
	bool alphablending;
	int vrx, vry, vrw, vrh;
	int hrx, hry, hrw, hrh;
	int vsbmy;
};

#endif /* UI_GGUISCROLLABLE_H_ */
