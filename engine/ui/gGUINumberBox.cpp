/*
 * gGUINumberBox.cpp
 *
 *  Created on: 27 Tem 2022
 *      Author: Aliv
 */

#include "gGUINumberBox.h"
#include "gBaseCanvas.h"

gGUINumberBox::gGUINumberBox() {

	b1ispressed = false;
	b1istoggle = false;
	b1ispressednow = false;
	b1isdisabled = false;
	b2ispressed = false;
	b2istoggle = false;
	b2ispressednow = false;
	b2isdisabled = false;
	boxwidth = 256;
	boxheight = 128;
	numboxwidth = 80;
	numboxheight = 24;
	smalboxwidth = 24;
	smalboxheight = 24;

}

gGUINumberBox::~gGUINumberBox() {

}

void gGUINumberBox::draw() {

	gColor defColor = renderer->getColor();
	gColor barColor = gColor(0.68f, 0.81f, 0.94f, 1.0f);
	gColor pressedColor = gColor(0.25f, 0.96f, 0.958f, 1.0f);
	gColor incdecColor = gColor(0.47f, 0.96f, 0.958f, 1.0f);
	gColor frameColor = gColor(0.9f, 0.9f, 0.9f, 1.0f);
	gColor numboxColor = gColor(0.98f, 0.90f, 0.88f, 1.0f);
	gColor triColor = gColor(0.0f, 0.0f, 0.66f, 1.0f);
	gColor pressedTriColor = gColor(0.375f, 0.375f, 0.66f, 1.0f);

	// frame
	renderer->setColor(frameColor);
	gDrawRectangle(left, top, boxwidth, boxheight, true);

	// top bar
	renderer->setColor(barColor);
	gDrawRectangle(left, top, boxwidth, 34, true);

	// number box
	renderer->setColor(numboxColor);
	gDrawRectangle(left + 64, top + 64, numboxwidth, numboxheight, true);

	// increment box
	if(b1ispressed) renderer->setColor(incdecColor);
	if(!b1ispressed) renderer->setColor(pressedColor);
	gDrawRectangle(left + 150, top + 48 + 1  * b1ispressed, smalboxwidth, smalboxheight, true);

	if(b1ispressed) renderer->setColor(triColor);
	if(!b1ispressed) renderer->setColor(pressedTriColor);
	gDrawTriangle(left + 162, top + 55, left + 153, top + 65, left + 172, top + 65, true);

	// decrement box
	if(b2ispressed) renderer->setColor(incdecColor);
	if(!b2ispressed) renderer->setColor(pressedColor);
	gDrawRectangle(left + 150, top + 80 + 1 * b2ispressed, smalboxwidth, smalboxheight, true);

	if(b2ispressed) renderer->setColor(triColor);
	if(!b2ispressed) renderer->setColor(pressedTriColor);
	gDrawTriangle(left + 162, top + 97, left + 153, top + 87, left + 172, top + 87, true);

	// default renderer color
	renderer->setColor(defColor);
}
/*
void gGUINumberBox::mousePressed(int x, int y, int button) {
	if(b1isdisabled) return;
	if(x >= left + 150 && x < left + 150 + smalboxwidth && y >= top + 48 && y < top + 48 + smalboxheight) {
		if(!b1istoggle) b1ispressed = true;
		else {
			if(!b1ispressed) {
				b1ispressed = true;
				b1ispressednow = true;
			}
		}
		root->getCurrentCanvas()->onGuiEvent(id, G_GUIEVENT_BUTTONPRESSED);
	}
	if(x >= left + 150 && x < left + 150 + smalboxwidth && y >= top + 80 && y < top + 80 + smalboxheight) {
		if(!b2istoggle) b2ispressed = true;
			else {
				if(!b2ispressed) {
					b2ispressed = true;
					b2ispressednow = true;
				}
			}
			root->getCurrentCanvas()->onGuiEvent(id, G_GUIEVENT_BUTTONPRESSED);
	}
}

void gGUINumberBox::mouseReleased(int x, int y, int button) {
	if(b1isdisabled) return;
	if(x >= left + 150 && x < left + 150 + smalboxwidth && y >= top + 48 && y < top + 48 + smalboxheight) {
		if(!b1istoggle) b1ispressed = false;
		else {
			if(!b1ispressednow) b1ispressed = false;
			}
			root->getCurrentCanvas()->onGuiEvent(id, G_GUIEVENT_BUTTONRELEASED);
		} else {
			if(!b1istoggle) b1ispressed = false;
	}
	b1ispressednow = false;

	if(x >= left + 150 && x < left + 150 + smalboxwidth && y >= top + 80 && y < top + 80 + smalboxheight) {
		if(!b2istoggle) b2ispressed = false;
		else {
			if(!b2ispressednow) b2ispressed = false;
			}
			root->getCurrentCanvas()->onGuiEvent(id, G_GUIEVENT_BUTTONRELEASED);
		} else {
			if(!b2istoggle) b2ispressed = false;
	}
	b2ispressednow = false;
}
*/
