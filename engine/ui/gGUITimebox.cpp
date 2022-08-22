/*
 * gGUITimebox.cpp
 *
 *  Created on: 4 Aug 2022
 *      Author: Baris Karakaya
 */

#include "gGUITimebox.h"
#include "gGUISlider.h"
#include "gBaseCanvas.h"
#include "gBaseApp.h"


gGUITimebox::gGUITimebox() {
	//Boxes starting and finishing coordination
	timeboxw = 170;
	timeboxh = 60;
	buttoncoverw = 25;
	buttoncoverh = 30;
	buttoncoverx = timeboxw - buttoncoverw - 2;
	buttoncovery = top + 24;
	//Triangles x coordinations
	triangleax = timeboxw - (buttoncoverw / 2) - 7;
	trianglebx = timeboxw - (buttoncoverw / 2) - 12;
	trianglecx = timeboxw - (buttoncoverw / 2) - 2;
	//Triangles y coordinations
	triangleay1 = buttoncovery + 4;
	triangleby1 = buttoncovery + 14;
	trianglecy1 = buttoncovery + 14;
	triangleay2 = buttoncovery + 27;
	triangleby2 = buttoncovery + 17;
	trianglecy2 = buttoncovery + 17;

	bluecolor = gColor(0.1f, 0.45f, 0.87f);
	whitecolor = gColor(1.0f, 1.0f, 1.0f);

	//Begining values..
	hour = 10;
	minute = 45;
	second = 15;

	isdisabled = false;
	ispressedb1 = false;
	ispressedb2 = false;
	ispressednow = false;
	istoggle = false;
	ishour = true;
	isminute = false;
	issecond = false;

	buttontype = 0;
	//Sizer & Controller Options...
	lineno = 3;
	columno = 6;
	timeboxsizer.setSize(lineno, columno);
	float lineprops[] = {0.27f, 0.25f, 0.2f};
	float columnprops[] = {0.040f,0.1f, 0.1f,0.1f,0.025f};
	timeboxsizer.setColumnProportions(columnprops);
	timeboxsizer.setLineProportions(lineprops);
	timeboxsizer.enableBorders(false);
	hourbox.setNumeric(true);
	minutebox.setNumeric(true);
	secondbox.setNumeric(true);
	timeboxsizer.setControl(1, 1, &hourbox);
	timeboxsizer.setControl(1, 2, &minutebox);
	timeboxsizer.setControl(1, 3, &secondbox);
	setSizer(&timeboxsizer);
}

gGUITimebox::~gGUITimebox() {

}

void gGUITimebox::set(gBaseApp* root, gBaseGUIObject* topParentGUIObject, gBaseGUIObject* parentGUIObject, int parentSlotLineNo,int parentSlotColumnNo, int x, int y, int w, int h) {
	totalh = h;
	gGUIScrollable::set(root, topParentGUIObject, parentGUIObject, parentSlotLineNo, parentSlotColumnNo, x, y, w, h);
	gGUIScrollable::setDimensions(w, h);
	guisizer->set(root, topParentGUIObject, parentGUIObject, parentSlotLineNo, parentSlotColumnNo, x, y + topbarh, w, h - topbarh);
}

void gGUITimebox::set(int x, int y, int w, int h) {
	left = x;
	top = y;
	right = x + w;
	bottom = y + h;
	width = w;
	height = h;
	guisizer->set(x, y + topbarh, w, h - topbarh);
}

void gGUITimebox::keyPressed(int key) {
	hourbox.keyPressed(key);
	minutebox.keyPressed(key);
	secondbox.keyPressed(key);
}

void gGUITimebox::keyReleased(int key) {
	hourbox.keyReleased(key);
	minutebox.keyReleased(key);
	secondbox.keyReleased(key);
}

void gGUITimebox::charPressed(unsigned int codepoint) {
	hourbox.charPressed(codepoint);
	minutebox.charPressed(codepoint);
	secondbox.charPressed(codepoint);
}

void gGUITimebox::mousePressed(int x, int y, int button) {
	gGUIContainer::mousePressed(x, y, button);
	hourbox.mousePressed(x, y, button);
	minutebox.mousePressed(x, y, button);
	secondbox.mousePressed(x, y, button);
	if(isdisabled) return;

	if(x >= left + 20 && x < left + 50 && y >= top + buttoncovery && y < top + buttoncovery + 25){
		ishour = true;
		isminute = false;
		issecond = false;
	}

	if(x >= left + 60 && x < left + 90 && y >= top + buttoncovery && y < top + buttoncovery + 25){
		ishour = false;
		isminute = true;
		issecond = false;
	}

	if(x >= left + 100 && x < left + 130 && y >= top + buttoncovery && y < top + buttoncovery + 25){
		ishour = false;
		isminute = false;
		issecond = true;
	}


	if(x >= left + trianglebx && x < left + trianglecx && y >= top + buttoncovery + 4 && y < top + buttoncovery + 14) {
		buttontype = 1;
			if(!ispressedb1 && ishour) {
				ispressedb1 = true;
				ispressednow = true;
				setHour();
			}

			if(!ispressedb1 && isminute) {
				ispressedb1 = true;
				ispressednow = true;
				setMinute();
			}

			if(!ispressedb1 && issecond) {
				ispressedb1 = true;
				ispressednow = true;
				setSecond();
			}
		root->getCurrentCanvas()->onGuiEvent(id, G_GUIEVENT_BUTTONPRESSED);
	}

	if(x >= left + trianglebx && x < left + trianglecx && y >= top + buttoncovery + 17 && y < top + buttoncovery + 27) {
		buttontype = 2;
			if(!ispressedb2 && ishour) {
				ispressedb2 = true;
				ispressednow = true;
				setHour();
			}
			if(!ispressedb2 && isminute) {
				ispressedb2 = true;
				ispressednow = true;
				setMinute();
			}
			if(!ispressedb2 && issecond) {
				ispressedb2 = true;
				ispressednow = true;
				setSecond();
			}
		root->getCurrentCanvas()->onGuiEvent(id, G_GUIEVENT_BUTTONPRESSED);
	}
}

void gGUITimebox::mouseReleased(int x, int y, int button) {
	gGUIContainer::mouseReleased(x, y, button);
	hourbox.mouseReleased(x, y, button);
	minutebox.mouseReleased(x, y, button);
	secondbox.mouseReleased(x, y, button);
	if(isdisabled) return;
	if(x >= timeboxw / 2 - 14 && x < left + 12 && y >= top + 9 && y < top + 12) {
		if(!istoggle) {
			ispressedb1 = false;
			ispressedb2 = false;
		}
				else {
					if(!ispressednow) {
						ispressedb1 = false;
						ispressedb2 = false;
					}
				}
				root->getCurrentCanvas()->onGuiEvent(id, G_GUIEVENT_BUTTONRELEASED);
	} else {
		if(!istoggle) {
			ispressedb1 = false;
			ispressedb2 = false;
		}
	}
	ispressednow = false;
}

void gGUITimebox::mouseDragged(int x, int y, int button) {
	hourbox.mouseDragged(x, y, button);
	minutebox.mouseDragged(x, y, button);
	secondbox.mouseDragged(x, y, button);
}

void gGUITimebox::update() {
	hourbox.update();
	minutebox.update();
	secondbox.update();
}

void gGUITimebox::draw() {
	gColor oldcolor = renderer->getColor();
	renderer->setColor(middlegroundcolor);
	gDrawRectangle(left + 5, top + 10, timeboxw, timeboxh,true);
	renderer->setColor(bluecolor);
	gDrawRectangle(left + timeboxw - buttoncoverw - 6, top + buttoncovery, buttoncoverw, buttoncoverh,true);
	renderer->setColor(whitecolor);
	gDrawTriangle(left + triangleax, top + triangleay1 + ispressedb1, left + trianglebx, top + triangleby1 + ispressedb1, left + trianglecx,
			top + trianglecy1 + ispressedb1, true);
	gDrawTriangle(left + triangleax, top + triangleay2 + ispressedb2 ,left + trianglebx, top + triangleby2 + ispressedb2, left + trianglecx,
			top + trianglecy2 + ispressedb2, true);

	if(ispressednow){
		renderer->setColor(middlegroundcolor);
		if(buttontype == 1){
			gDrawTriangle(left + triangleax, top + triangleay1 + ispressedb1, left + trianglebx, top + triangleby1 + ispressedb1, left + trianglecx,
					top + trianglecy1 + ispressedb1, true);
		}
		if(buttontype == 2){
			gDrawTriangle(left + triangleax, top + triangleay2 + ispressedb2, left + trianglebx, top + triangleby2 + ispressedb2, left + trianglecx,
					top + trianglecy2 + ispressedb2, true);
		}
	}
	renderer->setColor(fontcolor);
	font->drawText(":", 51.2, 69);
	font->drawText(":", 89, 69);
	font->drawText(title, left + 2, top + 5);
	if(guisizer) guisizer->draw();
	renderer->setColor(oldcolor);
}

void gGUITimebox::setHour(){
	if(ispressednow && buttontype == 1){
		if(hour >= 23){
			hour = -1;
		}
		hour++;
		hourbox.setText(gToStr(hour));
	}

	if (ispressednow && buttontype == 2){
		if(hour <= 0){
			hour = 24;
		}
		hour--;
		hourbox.setText(gToStr(hour));
	}
}

void gGUITimebox::setMinute(){
	if(ispressednow && buttontype == 1){
		if(minute >= 59){
			minute = -1;
		}
		minute++;
		minutebox.setText(gToStr(minute));
	}

	if (ispressednow && buttontype == 2){
		if(minute <= 0){
			minute = 60;
		}
		minute--;
		minutebox.setText(gToStr(minute));
	}
}

void gGUITimebox::setSecond(){
	if(ispressednow && buttontype == 1){
		if(second >= 59){
			second = -1;
		}
		second++;
		secondbox.setText(gToStr(second));
	}

	if (ispressednow && buttontype == 2){
		if(second <= 0){
			second = 60;
		}
		second--;
		secondbox.setText(gToStr(second));
	}
}
