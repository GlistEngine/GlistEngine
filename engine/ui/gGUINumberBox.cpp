/*
 * gGUINumberBox.cpp
 *
 *  Created on: 27 Tem 2022
 *      Author: Aliv
 */

#include "gGUINumberBox.h"

gGUINumberBox::gGUINumberBox() {
	boxwidth = 256;
	boxheight = 128;
	numboxwidth = 160;
	numboxheight = 64;
	smalboxwidth = 48;
	smalboxheight = 32;
}

gGUINumberBox::~gGUINumberBox() {

}
// buttonlarý hallet ilk önce
void gGUINumberBox::draw() {
	gColor defColor = renderer->getColor();
	gColor barColor = gColor(0.3f, 0.3f, 0.3f, 1.0f);
	gColor incdecColor = gColor(0.1f, 0.9f, 0.5f, 1.0f);
	gColor frameColor = gColor(0.1f, 0.1f, 0.6f, 1.0f);
	renderer->setColor(frameColor);
	gDrawRectangle(left, top, boxwidth, boxheight, true);								// frame
	renderer->setColor(barColor);
	gDrawRectangle(left, top, boxwidth, 34, true);										// top bar
	gDrawRectangle(left + 16, top + 50, numboxwidth, numboxheight, true);				// number box
	renderer->setColor(incdecColor);
	gDrawRectangle(left + 176, top + 50, smalboxwidth, smalboxheight, true);			// increment box
	gDrawRectangle(left + 176, top + 82, smalboxwidth, smalboxheight, true);			// decrement box
}

