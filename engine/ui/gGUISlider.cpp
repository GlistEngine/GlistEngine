/*
 * gGUISlider.cpp
 *
 *  Created on: 19 Tem 2022
 *      Author: murat
 */

#include "gGUISlider.h"
#include "gBaseApp.h"
#include "gBaseCanvas.h"

gGUISlider::gGUISlider() {
	sliderbarh = 3;
	sliderbarw = 150;
	sliderh = 30;
	sliderw = 10;
	isdisabled = false;
	sliderx = left + (sliderbarw / 2) - (sliderw / 2);
	ispressedslider = false;
	ticknum = 10;
	direction = -1;
	counter = 0;
	counterlimit = 6;
	unitlength = sliderbarw / ticknum;
}

gGUISlider::~gGUISlider() {

}

void gGUISlider::draw() {
	gColor oldcolor = renderer->getColor();
	renderer->setColor(pressedbuttonfontcolor);
	gDrawRectangle(left + sliderw / 2, top + sliderh / 2, sliderbarw, sliderbarh, true);
	renderer->setColor(255, 0, 0, 1);
	drawtick();
	renderer->setColor(buttoncolor);
	drawSlider();
	renderer->setColor(oldcolor);

}

void gGUISlider::drawtick() {
	for(int i = 0; i <= ticknum; i++) {
		gDrawLine(left + sliderw / 2 + unitlength * i, top + sliderh + 12, left + sliderw / 2 + unitlength * i, top + sliderh + 2);
	}
}

void gGUISlider::drawSlider() {
	if(std::fmod(sliderx,unitlength) == 0) {
		gDrawRectangle(sliderx, top + sliderbarh / 2, sliderw, sliderh, true);
	}else if(std::fmod(sliderx,unitlength) >= unitlength / 2) {
		gDrawRectangle(sliderx + unitlength - std::fmod(sliderx,unitlength), top + sliderbarh / 2, sliderw, sliderh, true);
	}else if(std::fmod(sliderx,unitlength) < unitlength / 2) {
		gDrawRectangle(sliderx - std::fmod(sliderx,unitlength), top + sliderbarh / 2, sliderw, sliderh, true);
	}

}

void gGUISlider::mousePressed(int x, int y, int button) {
	if(isdisabled) return;

	if((x >= sliderx && x < (sliderx + sliderw * 2)) && y >= top - (sliderh / 2) && y < top + sliderbarh + (sliderh / 2)) {
		ispressedslider = true;
	}else if(x >= left && x < left + sliderbarw && y >= top && y < top + sliderh && !(x >= sliderx && x < (sliderx + sliderw * 2))) {
		if(x > sliderx) direction = 1;
		sliderx = (sliderx + unitlength * direction);
		direction = -1;
	}



	/*
	if((!(x >= sliderx && x < (sliderx + sliderw)) && x >= left && x < left + sliderbarw) && y >= top && y < top + sliderh) {
		gLogi("gGUISlider") << "ifin içine girdim za";
			if(x > sliderx) direction = 1;
			sliderx = (sliderx + unitlength * direction);
			direction = -1;
		}
	*/



}

void gGUISlider::mouseReleased(int x, int y, int button) {
	if(isdisabled) return;
	ispressedslider = false;



}

void gGUISlider::mouseDragged(int x, int y, int button) {
	if(isdisabled) return;

	if(ispressedslider) {
		if(x <= left) {
			sliderx = left;
		} else if(x > left + sliderbarw) {
			sliderx = left + sliderbarw;
		} else {
			sliderx = x - (sliderw / 2);
		}
	}else if(!ispressedslider && x >= left && x < left + sliderbarw && y >= top && y < top + sliderh && !(x >= sliderx && x < (sliderx + sliderw * 2))) {
		counter++;
		if(counter >= counterlimit) {
			if(x > sliderx) direction = 1;
			sliderx = (sliderx + unitlength * direction);
			direction = -1;
			counter = 0;
		}
	}
}

