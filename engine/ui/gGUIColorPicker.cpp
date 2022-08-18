/*
 * gGUIColorPicker.cpp
 *
 *  Created on: 8 Aðu 2022
 *      Author: Hp
 */

#include "gGUIColorPicker.h"

gGUIColorPicker::gGUIColorPicker() {
	lastcolorforline = false;
	isdisabled = false;
	ispressed = false;
	colorpicker.loadImage("ColorPicker2.png");
	colorpickerw = colorpicker.getWidth();
	colorpickerh = colorpicker.getHeight();
	columnnum = 1;
	data = colorpicker.getImageData();

}

gGUIColorPicker::~gGUIColorPicker() {

}
void gGUIColorPicker::draw() {
	colorpicker.draw(left, top);
	if(ispressed) {
		gColor oldcolor = renderer->getColor();
		renderer->setColor(0, 0, 0, 255);
		gDrawRectangle(cx, cy, 16, 16, false);
		renderer->setColor(oldcolor);
	}
}

void gGUIColorPicker::mousePressed(int x, int y, int button) {
	if(isdisabled) return;
	if(x >= left && x < left + colorpicker.getWidth() && y >= top && y < top + colorpicker.getHeight() ) {
		if(x - left > 0 && x - left <= 227 && y - top > 0 && y - top <= 227) {
			ispressed = true;
			cx = (x - left) / 16;
			column = cx + 1; // 1-14
			cx = cx * 16 + left + 3;
			cy = (y - top) / 16;
			line = cy + 1; // 1-14
			cy = cy * 16 + top + 1;
			if(cx > 212) cx = 214;
			if(cy > 230) cy = 241;
		//	gLogi("GameCanvas") << "mousePressed" << ", cx:" << cx << ", cy:" << cy;
			//gLogi("GameCanvas") << "mousePressed" << ", column:" << column << ", line:" << line;
			countColors(column, line);
		}
		if(x - left > 0 && x - left <= 227 && y - top > 254 && y - top <= colorpicker.getHeight()) {
			ispressed = true;
			cx = (x - left) / 16;
			column = cx + 1;
			cx = cx * 16 + left + 3;
			cy = 283;
			line = 15;
			gLogi("GameCanvas") << "mousePressed" << ", cx:" << column << ", cy:" << line;
			gLogi("GameCanvas") << "mousePressed" << ", cx:" << cx << ", cy:" << cy;
			countColors(column, line);
		}
		root->getCurrentCanvas()->onGuiEvent(id, G_GUIEVENT_COLORSELECTED);
	}
}

void gGUIColorPicker::countColors(int column, int line) {
	startcolorpixel = colorpicker.getWidth() * 16 * (line) * 4 + 32 + 64 * (column - 1);
	if(line > 14) startcolorpixel = colorpicker.getWidth() * 16 * (line + 1) * 4 + 32 + 64 * (column - 1);
	lastdata = startcolorpixel + 4; // 227-269
		for(int i = startcolorpixel; i <= lastdata; i++) {
			gLogi("GameCanvas") << "----" << ", column:" << column << ", line:" << line;
			gLogi("--------") << " R " << (short)data[i] << " G " << (short)data[i + 1] << " B "<< (short)data[i + 2] << " A "<< (short)data[i + 3];
			i = i + 3; //next pixel data (i + 4)
			i = i + 60; // next color data (i + 64)
	}
}
