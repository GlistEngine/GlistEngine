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
 * Author: Medine, 2022-on                                             *
 ****************************************************************************/
/*
/*
 * gGUIColorbox.h
 *
 *  Created on: 25 Tem 2022
 *      Author: Medine
 */

#ifndef UI_GGUICOLORBOXH
#define UI_GGUICOLORBOXH

#include "gGUIControl.h"

/**
 * Developers can choose whichever color they want with using setColorboxColor
 * and loadColorboxColor commands. This class can get the box's widht, height
 * and color values.
 */
class gGUIColorbox: public gGUIControl {
public:
    gGUIColorbox();
    virtual ~gGUIColorbox();
    void draw();

/**
 * Sets the boxsize dimensions
 *
 * @param colorboxwidth sets the colorbox's width value.
 * @param colowboxheight sets the colorbox's height value.
 */
    void setColorboxsize(int colorboxwidth, int colorboxheight);

/**
 * Sets the colorbox's color first time with using the float values.
 *
 * @param r sets the r(red color) value using float parameter
 * @param g sets the g(green color) value using float parameter
 * @param b sets the b(blue) value using float parameter
 */
    void setColorboxColor(float r, float g, float b);

/**
 * sets the colorbox's color
 *
 * @param setcolor sets the preused color with using setcolor command.
 */
    void setColorboxColor(gColor newColor);

/**
 * Gets the box's width value.
 */
    int getWidth();

/**
 * Gets the box's height value.
 */
    int getHeight();

/**
 * Gets the box's color value in gColor format.
 */
    gColor getcolor();

private:
    int colorboxw, colorboxh;
    bool setsize;
    gColor color;


};

#endif /* UI_GGUICOLORBOXH */
