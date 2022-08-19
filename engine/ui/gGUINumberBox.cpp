/*
 * gGUINumberBox.cpp
 *
 *  Created on: 27 Jul 2022
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
	typo = true;
	lineno = 5;
	columno = 3;
	boxwidth = 117;
	boxheight = 64;
	numboxwidth = 80;
	numboxheight = 23;
	smalboxwidth = 11;
	smalboxheight = 11;
	castcurrtexttoint = 0;
	castcurrtexttofloat = 0.0f;
	incboxposx = 90;
	incboxposy = 52;
	decboxposx = 90;
	decboxposy = 64;
	inctriucorpx = incboxposx + 5;
	inctriucorpy = incboxposy + 3;
	inctrilcorpx = incboxposx + 1;
	inctrilcorpy = incboxposy + 8;
	inctrircorpx = incboxposx + 11;
	inctrircorpy = incboxposy + 8;
	dectriucorpx = decboxposx + 5;
	dectriucorpy = decboxposy + 9;
	dectrilcorpx = decboxposx + 1;
	dectrilcorpy = decboxposy + 4;
	dectrircorpx = decboxposx + 11;
	dectrircorpy = decboxposy + 4;
	headertext = "Number Box";
	currenttext = "";
	defintvalue = "0";
	deffloatvalue = "0.0";

	boxsizer.setSize(lineno, columno);
	float lineprops[] = {0.03f, 0.06f, 0.2f, 0.2f};
	float columnprops[] = {0.014f, 0.14f};
	boxsizer.enableBorders(false);
	boxsizer.setColumnProportions(columnprops);
	boxsizer.setLineProportions(lineprops);
	textbox.setEditable(true);
	textbox.setNumeric(true);
	boxsizer.setControl(2, 1, &textbox);
	setSizer(&boxsizer);
}

gGUINumberBox::~gGUINumberBox() {
}

void gGUINumberBox::setText(const std::string& text) {
	textbox.setText(text);
}

void gGUINumberBox::setSize(int width, int height) {
	textbox.setSize(75, 23);
}

bool gGUINumberBox::setType(bool texttype) {
	typo = texttype;
	return typo;
}

void gGUINumberBox::keyPressed(int key) {
	textbox.keyPressed(key);
}

void gGUINumberBox::keyReleased(int key) {
	textbox.keyReleased(key);
}

void gGUINumberBox::charPressed(unsigned int codepoint) {
	textbox.charPressed(codepoint);
}

void gGUINumberBox::mousePressed(int x, int y, int button) {
	gGUIContainer::mousePressed(x, y, button);
	textbox.mousePressed(x, y, button);
	if(b1isdisabled) return;
	if(x >= incboxposx && x < incboxposx + smalboxwidth && y >= incboxposy && y < incboxposy + smalboxheight) {
		if(!b1istoggle) b1ispressed = true;
		else {
			if(!b1ispressed) {
				b1ispressed = true;
				b1ispressednow = true;
			}
		}
		root->getCurrentCanvas()->onGuiEvent(id, G_GUIEVENT_BUTTONPRESSED);
//		gLogi("Increase Button: ") << "Pressed";

		if(typo){
			castcurrtexttoint = gToInt(textbox.getText());
			castcurrtexttoint = castcurrtexttoint + 1;
			gLogi("current value: ") << castcurrtexttoint;
			setText(gToStr(castcurrtexttoint));
		}
		if(!typo) {
			castcurrtexttofloat = gToFloat(textbox.getText());
			castcurrtexttofloat = castcurrtexttofloat + 0.1;
			gLogi("current value: ") << castcurrtexttofloat;
			setText(gToStr(castcurrtexttofloat));
		}
	}
	if(x >= decboxposx && x < decboxposx + smalboxwidth && y >= decboxposy && y < decboxposy + smalboxheight) {
		if(!b2istoggle) b2ispressed = true;
			else {
				if(!b2ispressed) {
					b2ispressed = true;
					b2ispressednow = true;
				}
			}
		root->getCurrentCanvas()->onGuiEvent(id, G_GUIEVENT_BUTTONPRESSED);
//		gLogi("Decrease Button: ") << "Pressed";

		if(typo){
			castcurrtexttoint = gToInt(textbox.getText());
			castcurrtexttoint = castcurrtexttoint - 1;
			gLogi("current value: ") << castcurrtexttoint;
			setText(gToStr(castcurrtexttoint));
		}
		if(!typo) {
			castcurrtexttofloat = gToFloat(textbox.getText());
			castcurrtexttofloat = castcurrtexttofloat - 0.1;
			gLogi("current value: ") << castcurrtexttofloat;
			setText(gToStr(castcurrtexttofloat));
		}
	}
}

void gGUINumberBox::mouseReleased(int x, int y, int button) {
	gGUIContainer::mouseReleased(x, y, button);
	textbox.mouseReleased(x, y, button);
	if(b1isdisabled) return;
	if(x >= left + 150 && x < left + 150 + smalboxwidth && y >= top + 30 && y < top + 30 + smalboxheight) {
		if(!b1istoggle) b1ispressed = false;
		else {
			if(!b1ispressednow) b1ispressed = false;
			}
			root->getCurrentCanvas()->onGuiEvent(id, G_GUIEVENT_BUTTONRELEASED);
//			gLogi("Increase Button: ") << "Released";
		} else {
			if(!b1istoggle) b1ispressed = false;
	}
	b1ispressednow = false;

	if(x >= left + 150 && x < left + 150 + smalboxwidth && y >= top + 62 && y < top + 62 + smalboxheight) {
		if(!b2istoggle) b2ispressed = false;
		else {
			if(!b2ispressednow) b2ispressed = false;
			}
			root->getCurrentCanvas()->onGuiEvent(id, G_GUIEVENT_BUTTONRELEASED);
//			gLogi("Decrease Button: ") << "Released";;
		} else {
			if(!b2istoggle) b2ispressed = false;
	}
	b2ispressednow = false;
}

void gGUINumberBox::mouseDragged(int x, int y, int button) {
	textbox.mouseDragged(x, y, button);
}

void gGUINumberBox::update() {
	textbox.update();
}

void gGUINumberBox::draw() {
	gColor defColor = renderer->getColor();
	gColor frameColor = gColor(0.9f, 0.9f, 0.9f, 1.0f);
	gColor black = gColor(0.0f, 0.0f, 0.0f, 1.0f);
	gColor white = gColor(1.0f, 1.0f, 1.0f, 1.0f);
	gColor buttonColor = gColor(0.1f, 0.45f, 0.87f, 1.0f);
	gColor pressedButtonColor = gColor(0.08f, 0.36f, 0.71f, 1.0f);

	renderer->setColor(frameColor);
	gDrawRectangle(left, top, boxwidth, boxheight, true);

	if(b1ispressed) renderer->setColor(pressedButtonColor);
	if(!b1ispressed) renderer->setColor(buttonColor);
	gDrawRectangle(incboxposx, incboxposy + 1 * b1ispressed, smalboxwidth, smalboxheight, true);

	if(b1ispressed) renderer->setColor(middlegroundcolor);
	if(!b1ispressed) renderer->setColor(white);
	gDrawTriangle(inctriucorpx, inctriucorpy, inctrilcorpx, inctrilcorpy, inctrircorpx, inctrircorpy, true);

	if(b2ispressed) renderer->setColor(pressedButtonColor);
	if(!b2ispressed) renderer->setColor(buttonColor);
	gDrawRectangle(decboxposx, decboxposy+1 + 1 * b2ispressed, smalboxwidth, smalboxheight, true);

	if(b2ispressed) renderer->setColor(middlegroundcolor);
	if(!b2ispressed) renderer->setColor(white);
	gDrawTriangle(dectriucorpx, dectriucorpy, dectrilcorpx, dectrilcorpy, dectrircorpx, dectrircorpy, true);

	renderer->setColor(black);
	font->drawText(headertext, left + 10, top + 13);

	renderer->setColor(defColor);
	if(guisizer) guisizer->draw();
}
