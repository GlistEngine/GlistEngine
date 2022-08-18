/*
 * gGUIColorPicker.h
 *
 *  Created on: 8 A�u 2022
 *      Author: Hp
 */

#ifndef UI_GGUICOLORPICKER_H_
#define UI_GGUICOLORPICKER_H_

#include "gGUIImageButton.h"
#include "gImage.h"
#include "gBaseCanvas.h"

class gGUIColorPicker: public gGUIControl {
public:
	gGUIColorPicker();
	virtual ~gGUIColorPicker();

	void draw();
	void mousePressed(int x, int y, int button);
	void countColors(int column, int line);

private:
	int cx, cy;
	int colorpickerw, colorpickerh;
	int linenum;
	int pixel1, pixel2;
	int startcolorpixel;
	int lastdata;
	int columnnum;
	int column, line;
	bool lastcolorforline;
	bool ispressed;
	bool isdisabled;


	gImage colorpicker;
	unsigned char *data;
};

#endif /* UI_GGUICOLORPICKER_H_ */
