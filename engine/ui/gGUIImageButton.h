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
 * Author: Medine, Yasin 2022-on                                             *
 ****************************************************************************/
/*
/*
 * gGUIImageButton.h
 *
 *  Created on: 21 Tem 2022
 *      Author: Medine, Yasin
 */

#ifndef UI_GGUIIMAGEBUTTONH
#define UI_GGUIIMAGEBUTTONH

#include "gGUIButton.h"
#include "gImage.h"

/**
 * Uploaded image acts as a button. The Image adding both as an image and file
 * path. Stretches the image at the button size or placed the image by keeping
 * its proportions. Gets the image with its file location, width and height
 * of the button.
 */
class gGUIImageButton: public gGUIButton {
public:
    gGUIImageButton();
    virtual ~gGUIImageButton();

    void draw();

/**
 * Loads the image to the button when mouse released
 *
 * @param imagePath Adds the image with its file location.
 */
    void loadButtonImages(const std::string& imagePath);

/**
 * Loads the image to the button when mouse pressed
 *
 * @param imagePath Adds the image with its file location. If the image isn't
 * uploaded to the previous section, there will be blank area at button when
 * mouse pressed
 *
 */
    void loadPressedButtonImages(const std::string& imagePath);

/**
 * Sets the image
 * @param setImage adds the preloaded image with using gImage gGUI
 *  without file location.
 */
    void setButtonImage(gImage setImage);

/**
 * Sets the image to the button when mouse pressed
 * @param setImage adds the preloaded image with using gImage gGUI
 *  without file location. If the image isn't uploaded to the
 *  previous section, there will be blank area at button when mouse pressed
 */
    void setPressedButtonImage(gImage setImage);

/**
 * Strethes the image in button.
 * @param stretch stretches the image if the value is true. If the value is
 * false the image will placed keeping the proportion.
 */
    void stretche(bool stretch);

/**
 *  @param getButtonImagePath Gets the image with its file location. If set
 *  command used instead of load command code line returns null result .
 */
    std::string getButtonImagePath();

/**
 *  @param getPressedButtonImagePath Gets the button pressed image with its
 *  file location. If set command used instead of load command code line
 *  returns null result.
 */
    std::string getPressedButtonImagePath();

/**
 * Gets the width of the button.
 */
    int getButtonWidth();

/**
 * Gets the height of the button.
 */
    int getButtonHeight();


private:
        float imagew, imageh, proportion;
        bool stretch;
        std::string buttonimagepath;
        std::string pressedbuttonimagepath;
        gImage buttonimage;
        gImage pressedbuttonimage;
};

#endif /* UI_GGUIIMAGEBUTTONH */

