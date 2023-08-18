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
 * gGUIPicturebox.h
 *
 *  Created on: 20 Tem 2022
 *      Author: Medine, Yasin
 */

#ifndef UI_GGUIBITMAP_H_
#define UI_GGUIBITMAP_H_

#include "gGUIControl.h"
#include "gImage.h"

class gGUIBitmap: public gGUIControl {
public:
	gGUIBitmap();
	virtual ~gGUIBitmap();
	void draw();

/**
 * Loads the specific picture file
 * @param imagePath specifies the pictures file location
 * @param isPropotional if the value of parameter is given true the picture
 *  will stretch in picturebox.
 */
	void loadImage(const std::string& imagePath, bool isProportional = true);
	void load(const std::string& fullPath, bool isProportional = true);

/**
 * Sets the specific picture file
 * @param setImage adds the preloaded image using the variable name
 * @param isPropotional if the value of parameter is given true the picture
 *  will stretch in picturebox.
 */
	void setImage(gImage setImage, bool isProportional = false);

/**
 * Sets the loaded picture's size
 * @param x changes the position of the image on the x-axis up to the added
 * pixel value.
 * @param y changes the position of the image on the y-axis up to the added
 * pixel value.
 * @param scalex changes the size of the image on the x- axis by proportioning
 *  it to the given float value.
 * @param scaley changes the size of the image on the y- axis by proportioning
 * it to the given float value.
 */
	void setImageSize(int x, int y, float scalex, float scaley);

/**
 * Specifies the path of the image
 */
	std::string getImagePath();

/**
 * @param getImageWidth specifies the width of the image
 * @param getImageHeight specifies the height of the image
 */
	int getImageWidth();
	int getImageHeight();

private:
	float imagew, imageh, proportion;
	int setsizex, setsizey;
	float setsizew, setsizeh;
	bool stretch;
	std::string imagepath;
	gImage image;
};

#endif /* UI_GGUIBITMAP_H_ */
