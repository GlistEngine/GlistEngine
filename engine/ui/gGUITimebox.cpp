/*
 * gGUITimebox.cpp
 *
 *  Created on: 4 Aug 2022
 *      Author: Baris Karakaya
 */

#include "gGUITimebox.h"
#include "gBaseCanvas.h"
#include "gBaseApp.h"


gGUITimebox::gGUITimebox() {
	timeboxw = 150;
	timeboxh = 60;
	textboxw = 80;
	textboxh = 30;
	buttoncoverw = 25;
	buttoncoverh = 50;
	buttoncoverx = timeboxw - buttoncoverw - 2;
	buttoncovery = top + 10;
	trianglea = timeboxw - (buttoncoverw / 2) - 4;
	triangleb = timeboxw - (buttoncoverw / 2) - 10;
	trianglec = timeboxw - (buttoncoverw / 2) + 2;
	pressedbluecolor = gColor(0.0f, 0.6f, 0.8f);
	bluecolor = gColor(0.0f, 0.7f, 1.0f);
	boxcolor = gColor(0.7f, 0.7f, 0.7f);
	isdisabled = false;
	ispressedb1 = false;
	ispressedb2 = false;
	ispressednow = false;
	istoggle = false;
	setSizer(&timeboxsizer);

}

gGUITimebox::~gGUITimebox() {

}

void gGUITimebox::draw() {
	gColor oldcolor = renderer->getColor();
	renderer->setColor(middlegroundcolor);
	gDrawRectangle(left + 5, top + 5, timeboxw, timeboxh,true);
	gDrawRectangle(left + 5, top + 5, timeboxw, timeboxh,true);
	renderer->setColor(boxcolor);
	gDrawRectangle(left + timeboxw - buttoncoverw - 3, top + 10, buttoncoverw, buttoncoverh,true);
	renderer->setColor(bluecolor);
	gDrawTriangle(left + trianglea, top + buttoncovery + 8 + ispressedb1, left + triangleb, top + buttoncovery + 20 + ispressedb1,
			left + trianglec, top + buttoncovery + 20 + ispressedb1, true);
	gDrawTriangle(left + trianglea, top + buttoncovery + 42 + ispressedb2, left + triangleb, top + buttoncovery + 30 + ispressedb2,
			left + trianglec, top + buttoncovery + 30 + ispressedb2, true);

	if(ispressednow){
		renderer->setColor(pressedbluecolor);
		if(buttontype == 1){
			gDrawTriangle(left + trianglea, top + buttoncovery + 8 + ispressedb1, left + triangleb, top + buttoncovery + 20 + ispressedb1,
					left + trianglec, top + buttoncovery + 20 + ispressedb1, true);
		}
		if(buttontype == 2){
			gDrawTriangle(left + trianglea, top + buttoncovery + 42 + ispressedb2, left + triangleb, top + buttoncovery + 30 + ispressedb2,
					left + trianglec, top + buttoncovery + 30 + ispressedb2, true);
		}
	}
	renderer->setColor(textbackgroundcolor);
	gDrawRectangle(left + 20, top + 20, textboxw / 2, textboxh, true);
	gDrawRectangle(left + 24 + textboxw / 2, top + 20, textboxw / 2, textboxh, true);

//	textcolon.setText(":");
}

void gGUITimebox::mousePressed(int x, int y, int button) {
	if(isdisabled) return;
		if(x >= left + triangleb && x < left + trianglec && y >= top + buttoncovery + 8 && y < top + buttoncovery + 20) {
			buttontype = 1;
				if(!ispressedb1) {
					ispressedb1 = true;
					ispressednow = true;
				}
			root->getCurrentCanvas()->onGuiEvent(id, G_GUIEVENT_BUTTONPRESSED);
		}

		if(x >= left + triangleb && x < left + trianglec && y >= top + buttoncovery + 30 && y < top + buttoncovery + 42) {
			buttontype = 2;
				if(!ispressedb2) {
					ispressedb2 = true;
					ispressednow = true;
				}
			root->getCurrentCanvas()->onGuiEvent(id, G_GUIEVENT_BUTTONPRESSED);
		}

}

void gGUITimebox::mouseReleased(int x, int y, int button) {
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

