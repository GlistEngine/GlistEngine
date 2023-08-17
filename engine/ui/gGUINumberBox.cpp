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
	b1isdisabled = false;
	b2ispressed = false;
	b2isdisabled = false;
	isinteger = true;
	lineno = 4;
	columno = 3;
	boxwidth = 117;
	boxheight = 64;
	numboxwidth = 80;
	numboxheight = 23;
	smalboxwidth = 16;
	smalboxheight = 16;
	smalltriangleheight = 6;
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
	title = "Number Box";
	currenttext = "";
	defintvalue = "0";
	deffloatvalue = "0.0";
	istitleshown = false;
	boxtoph = istitleshown * font->getSize();

	boxsizer.setSize(lineno, columno);
	float lineprops[] = {0.43f, 0.06f, 0.2f, 0.01f};
	float columnprops[] = {0.20f, 0.80f};
	boxsizer.enableBorders(false);
	boxsizer.setColumnProportions(columnprops);
	boxsizer.setLineProportions(lineprops);
	textbox.setEditable(true);
	textbox.setNumeric(true);
	boxsizer.setControl(0, 0, &textbox);
	setSizer(&boxsizer);
	maxvalue = std::numeric_limits<int>::max();
	minvalue = std::numeric_limits<int>::min();
	maxvaluef = std::numeric_limits<float>::max();
	minvaluef = std::numeric_limits<float>::min();
	increment = 1;
}

gGUINumberBox::~gGUINumberBox() {
}

void gGUINumberBox::set(gBaseApp* root, gBaseGUIObject* topParentGUIObject, gBaseGUIObject* parentGUIObject, int parentSlotLineNo, int parentSlotColumnNo, int x, int y, int w, int h) {
	gGUIContainer::set(root, topParentGUIObject, parentGUIObject, parentSlotLineNo, parentSlotColumnNo, x, y, w, h);
	incboxposx = textbox.left + textbox.width + 5;
	incboxposy = textbox.top;
	decboxposx = incboxposx;
	decboxposy = incboxposy + smalboxheight + 2;
	inctrilcorpx = incboxposx + 3;
	inctrilcorpy = incboxposy + smalboxheight - 1 - (smalboxheight - smalltriangleheight) / 2;
	inctrircorpx = incboxposx + smalboxwidth - 2;
	inctrircorpy = inctrilcorpy;
	inctriucorpx = (inctrilcorpx + inctrircorpx) / 2;
	inctriucorpy = inctrilcorpy - smalltriangleheight;
	dectrilcorpx = decboxposx + 3;
	dectrilcorpy = decboxposy + 1 + (smalboxheight - smalltriangleheight) / 2;
	dectrircorpx = decboxposx + smalboxwidth - 2;
	dectrircorpy = dectrilcorpy;
	dectriucorpx = (dectrilcorpx + dectrircorpx) / 2;
	dectriucorpy = dectrilcorpy + smalltriangleheight;
}

void gGUINumberBox::set(int x, int y, int w, int h) {
	gGUIContainer::set(x, y, w, h);
	incboxposx = textbox.left + textbox.width + 5;
	incboxposy = textbox.top;
	decboxposx = incboxposx;
	decboxposy = incboxposy + smalboxheight + 2;
	inctrilcorpx = incboxposx + 3;
	inctrilcorpy = incboxposy + smalboxheight - 1 - (smalboxheight - smalltriangleheight) / 2;
	inctrircorpx = incboxposx + smalboxwidth - 2;
	inctrircorpy = inctrilcorpy;
	inctriucorpx = (inctrilcorpx + inctrircorpx) / 2;
	inctriucorpy = inctrilcorpy - smalltriangleheight;
	dectrilcorpx = decboxposx + 3;
	dectrilcorpy = decboxposy + 1 + (smalboxheight - smalltriangleheight) / 2;
	dectrircorpx = decboxposx + smalboxwidth - 2;
	dectrircorpy = dectrilcorpy;
	dectriucorpx = (dectrilcorpx + dectrircorpx) / 2;
	dectriucorpy = dectrilcorpy + smalltriangleheight;
}

void gGUINumberBox::setText(const std::string& text) {
	textbox.setText(text);
}

std::string gGUINumberBox::getText() {
	 return textbox.getText();
}

void gGUINumberBox::setSize(int width, int height) {
	textbox.setSize(width, height);
}

void gGUINumberBox::showTitle(bool isShown) {
	istitleshown = isShown;
	boxtoph = istitleshown * font->getSize();
}

bool gGUINumberBox::setType(bool isInteger) {
	isinteger = isInteger;
	return isinteger;
}

void gGUINumberBox::setMaxValue(int maxValue) {
	maxvalue = maxValue;

}

void gGUINumberBox::setMinValue(int minValue) {
	minvalue = minValue;
}

void gGUINumberBox::setMaxValue(float maxValuef) {
	maxvaluef = maxValuef;
}

void gGUINumberBox::setMinValue(float minValuef) {
	minvaluef = minValuef;
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

void gGUINumberBox::setInteger(int value) {
	textbox.setText(gToStr(value));
}

int gGUINumberBox::getInteger() {
	return gToInt(textbox.getText());
}

void gGUINumberBox::setFloat(float value) {
	textbox.setText(gToStr(value));
}

float gGUINumberBox::getFloat() {
	return gToFloat(textbox.getText());

}

void gGUINumberBox::setIntegerIncrement(int value) {
	increment = value;
}

int gGUINumberBox::getIntegerIncrement() {
	return increment;
}

void gGUINumberBox::mousePressed(int x, int y, int button) {
	gGUIContainer::mousePressed(x, y, button);
	textbox.mousePressed(x, y, button);
	if(x >= incboxposx && x < incboxposx + smalboxwidth && y >= incboxposy && y < incboxposy + smalboxheight) {
		if(b1isdisabled) return;
		b1ispressed = true;
		root->getCurrentCanvas()->onGuiEvent(id, G_GUIEVENT_BUTTONPRESSED);
	}
	if(x >= decboxposx && x < decboxposx + smalboxwidth && y >= decboxposy && y < decboxposy + smalboxheight) {
		if(b2isdisabled) return;
		b2ispressed = true;
		root->getCurrentCanvas()->onGuiEvent(id, G_GUIEVENT_BUTTONPRESSED);
	}
}

void gGUINumberBox::mouseReleased(int x, int y, int button) {
	gGUIContainer::mouseReleased(x, y, button);
	textbox.mouseReleased(x, y, button);
	if(x >= incboxposx && x < incboxposx + smalboxwidth && y >= incboxposy && y < incboxposy + smalboxheight) {
		if(b1isdisabled) return;
		b1ispressed = false;
		root->getCurrentCanvas()->onGuiEvent(id, G_GUIEVENT_BUTTONRELEASED);

		if(isinteger){
			castcurrtexttoint = gToInt(textbox.getText());
			castcurrtexttoint = castcurrtexttoint + increment;
			setText(gToStr(castcurrtexttoint));
			if (castcurrtexttoint >= maxvalue) {
			    castcurrtexttoint = maxvalue;
			    b1isdisabled = true;
			}
			if (castcurrtexttoint > minvalue) b2isdisabled = false;
		} else {
			castcurrtexttofloat = gToFloat(textbox.getText());
			castcurrtexttofloat = castcurrtexttofloat + 0.1;
			setText(gToStr(castcurrtexttofloat));
			if (castcurrtexttofloat >= maxvaluef) {
				castcurrtexttofloat = maxvaluef;
			    b1isdisabled = true;
			}
			if (castcurrtexttofloat > minvaluef) b2isdisabled = false;
		}
	} else {
		b1ispressed = false;
	}

	if(x >= decboxposx && x < decboxposx + smalboxwidth && y >= decboxposy && y < decboxposy + smalboxheight) {
		if(b2isdisabled) return;
		b2ispressed = false;
		root->getCurrentCanvas()->onGuiEvent(id, G_GUIEVENT_BUTTONRELEASED);

		if(isinteger){
			castcurrtexttoint = gToInt(textbox.getText());
			castcurrtexttoint = castcurrtexttoint - increment;
			setText(gToStr(castcurrtexttoint));
			if (castcurrtexttoint <= minvalue) {
			    castcurrtexttoint = maxvalue;
			    b2isdisabled = true;
			}
			if (castcurrtexttoint < maxvalue) b1isdisabled = false;
		} else {
			castcurrtexttofloat = gToFloat(textbox.getText());
			castcurrtexttofloat = castcurrtexttofloat - 0.1;
			setText(gToStr(castcurrtexttofloat));
			if (castcurrtexttofloat <= minvaluef) {
				castcurrtexttofloat = maxvaluef;
			    b2isdisabled = true;
			}
			if (castcurrtexttofloat < maxvaluef) b1isdisabled = false;
		}
	} else {
		b2ispressed = false;
	}
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

//	renderer->setColor(frameColor);
//	gDrawRectangle(left, top, boxwidth, boxheight, true);

	if(b1ispressed) renderer->setColor(pressedButtonColor);
	else renderer->setColor(buttonColor);
	gDrawRectangle(incboxposx, incboxposy + b1ispressed, smalboxwidth, smalboxheight, true);

	if(b1ispressed) renderer->setColor(middlegroundcolor);
	else renderer->setColor(white);
	gDrawTriangle(inctriucorpx, inctriucorpy, inctrilcorpx, inctrilcorpy, inctrircorpx, inctrircorpy, true);

	if(b2ispressed) renderer->setColor(pressedButtonColor);
	else renderer->setColor(buttonColor);
	gDrawRectangle(decboxposx, decboxposy + b2ispressed, smalboxwidth, smalboxheight, true);

	if(b2ispressed) renderer->setColor(middlegroundcolor);
	else renderer->setColor(white);
	gDrawTriangle(dectriucorpx, dectriucorpy, dectrilcorpx, dectrilcorpy, dectrircorpx, dectrircorpy, true);

	if(istitleshown) {
		renderer->setColor(fontcolor);
		font->drawText(title, left, top + font->getSize());
	}
	renderer->setColor(defColor);
	if(guisizer) guisizer->draw();
}

