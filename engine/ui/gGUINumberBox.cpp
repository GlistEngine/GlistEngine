/*
 * gGUINumberBox.cpp
 *
 *  Created on: 27 Jul 2022
 *      Author: Aliv
 */

#include "gGUINumberBox.h"
#include "gBaseCanvas.h"
#include <sstream>
#include <iomanip>

gGUINumberBox::gGUINumberBox() {
	b1ispressed = false;
	b1isdisabled = false;
	b2ispressed = false;
	b2isdisabled = false;
	isdisabled = false;
	disabledbcolor = *disabledbuttoncolor;
	disabledfcolor = *disabledbuttonfontcolor;
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
	boxtoph = istitleshown * getFont()->getSize();

	boxsizer.setSize(1, 2);
	float columnprops[2] = {0.70f, 0.30f};
	boxsizer.setColumnProportions(columnprops);
	boxsizer.enableBorders(false);
	boxsizer.setAlignContentVertically(true);
	textbox.setEditable(true);
	textbox.setNumeric(true);
	textbox.enableVerticalMargin(false);
	boxsizer.setControl(0, 0, &textbox);
	setSizer(&boxsizer);
	maxvalue = std::numeric_limits<int>::max();
	minvalue = std::numeric_limits<int>::min();
	maxvaluef = std::numeric_limits<float>::max();
	minvaluef = std::numeric_limits<float>::min();
	floatprecision = 2;
	increment = 1;
	incrementf = 0.1f;
}

gGUINumberBox::~gGUINumberBox() {
}

void gGUINumberBox::set(gBaseApp* root, gBaseGUIObject* topParentGUIObject, gBaseGUIObject* parentGUIObject, int parentSlotLineNo, int parentSlotColumnNo, int x, int y, int w, int h) {
	gGUIContainer::set(root, topParentGUIObject, parentGUIObject, parentSlotLineNo, parentSlotColumnNo, x, y, w, h);
	smalboxheight = (textbox.height - 2) / 2;
	smalltriangleheight = smalboxheight * 0.4f;
	if (smalltriangleheight < 3) smalltriangleheight = 3;

	incboxposx = textbox.left + textbox.width + 5;
	incboxposy = textbox.top;
	decboxposx = incboxposx;
	decboxposy = incboxposy + smalboxheight + 2;

	inctrilcorpx = incboxposx + smalboxwidth * 0.25f;
	inctrilcorpy = incboxposy + smalboxheight - (smalboxheight - smalltriangleheight) / 2;
	inctrircorpx = incboxposx + smalboxwidth * 0.75f;
	inctrircorpy = inctrilcorpy;
	inctriucorpx = incboxposx + smalboxwidth * 0.5f;
	inctriucorpy = inctrilcorpy - smalltriangleheight;

	dectrilcorpx = decboxposx + smalboxwidth * 0.25f;
	dectrilcorpy = decboxposy + (smalboxheight - smalltriangleheight) / 2;
	dectrircorpx = decboxposx + smalboxwidth * 0.75f;
	dectrircorpy = dectrilcorpy;
	dectriucorpx = decboxposx + smalboxwidth * 0.5f;
	dectriucorpy = dectrilcorpy + smalltriangleheight;
}

void gGUINumberBox::set(int x, int y, int w, int h) {
	gGUIContainer::set(x, y, w, h);
	smalboxheight = (textbox.height - 2) / 2;
	smalltriangleheight = smalboxheight * 0.4f;
	if (smalltriangleheight < 3) smalltriangleheight = 3;

	incboxposx = textbox.left + textbox.width + 5;
	incboxposy = textbox.top;
	decboxposx = incboxposx;
	decboxposy = incboxposy + smalboxheight + 2;

	inctrilcorpx = incboxposx + smalboxwidth * 0.25f;
	inctrilcorpy = incboxposy + smalboxheight - (smalboxheight - smalltriangleheight) / 2;
	inctrircorpx = incboxposx + smalboxwidth * 0.75f;
	inctrircorpy = inctrilcorpy;
	inctriucorpx = incboxposx + smalboxwidth * 0.5f;
	inctriucorpy = inctrilcorpy - smalltriangleheight;

	dectrilcorpx = decboxposx + smalboxwidth * 0.25f;
	dectrilcorpy = decboxposy + (smalboxheight - smalltriangleheight) / 2;
	dectrircorpx = decboxposx + smalboxwidth * 0.75f;
	dectrircorpy = dectrilcorpy;
	dectriucorpx = decboxposx + smalboxwidth * 0.5f;
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
	boxtoph = istitleshown * getFont()->getSize();
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
	std::stringstream ss;
	ss << std::fixed << std::setprecision(floatprecision) << value;
	textbox.setText(ss.str());
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

void gGUINumberBox::setFloatIncrement(float value) {
	incrementf = value;
}

float gGUINumberBox::getFloatIncrement() {
	return incrementf;
}

void gGUINumberBox::setFloatPrecision(int precision) {
	if(precision < 0) precision = 0;
	floatprecision = precision;
}

int gGUINumberBox::getFloatPrecision() const {
	return floatprecision;
}

void gGUINumberBox::mousePressed(int x, int y, int button) {
	if(isdisabled) return;
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
	if(isdisabled) return;
	gGUIContainer::mouseReleased(x, y, button);
	textbox.mouseReleased(x, y, button);
	if(x >= incboxposx && x < incboxposx + smalboxwidth && y >= incboxposy && y < incboxposy + smalboxheight) {
		if(b1isdisabled) return;
		b1ispressed = false;
		root->getCurrentCanvas()->onGuiEvent(id, G_GUIEVENT_BUTTONRELEASED);

		std::string oldvaluestr = textbox.getText();
		if(isinteger){
			castcurrtexttoint = gToInt(textbox.getText());
			castcurrtexttoint = castcurrtexttoint + increment;
			if (castcurrtexttoint >= maxvalue) {
			    castcurrtexttoint = maxvalue;
			    b1isdisabled = true;
			}
			setText(gToStr(castcurrtexttoint));
			if (castcurrtexttoint > minvalue) b2isdisabled = false;
			if(oldvaluestr != getText()) root->getCurrentCanvas()->onGuiEvent(id, G_GUIEVENT_VALUECHANGED, getText(), oldvaluestr);
		} else {
			castcurrtexttofloat = gToFloat(textbox.getText());
			castcurrtexttofloat = castcurrtexttofloat + incrementf;
			if (castcurrtexttofloat >= maxvaluef) {
				castcurrtexttofloat = maxvaluef;
			    b1isdisabled = true;
			}
			setFloat(castcurrtexttofloat);
			if (castcurrtexttofloat > minvaluef) b2isdisabled = false;
			if(oldvaluestr != getText()) root->getCurrentCanvas()->onGuiEvent(id, G_GUIEVENT_VALUECHANGED, getText(), oldvaluestr);
		}
	} else {
		b1ispressed = false;
	}

	if(x >= decboxposx && x < decboxposx + smalboxwidth && y >= decboxposy && y < decboxposy + smalboxheight) {
		if(b2isdisabled) return;
		b2ispressed = false;
		root->getCurrentCanvas()->onGuiEvent(id, G_GUIEVENT_BUTTONRELEASED);

		std::string oldvaluestr = textbox.getText();
		if(isinteger){
			castcurrtexttoint = gToInt(textbox.getText());
			castcurrtexttoint = castcurrtexttoint - increment;
			if (castcurrtexttoint <= minvalue) {
			    castcurrtexttoint = minvalue;
			    b2isdisabled = true;
			}
			setText(gToStr(castcurrtexttoint));
			if (castcurrtexttoint < maxvalue) b1isdisabled = false;
			if(oldvaluestr != getText()) root->getCurrentCanvas()->onGuiEvent(id, G_GUIEVENT_VALUECHANGED, getText(), oldvaluestr);
		} else {
			castcurrtexttofloat = gToFloat(textbox.getText());
			castcurrtexttofloat = castcurrtexttofloat - incrementf;
			if (castcurrtexttofloat <= minvaluef) {
				castcurrtexttofloat = minvaluef;
			    b2isdisabled = true;
			}
			setFloat(castcurrtexttofloat);
			if (castcurrtexttofloat < maxvaluef) b1isdisabled = false;
			if(oldvaluestr != getText()) root->getCurrentCanvas()->onGuiEvent(id, G_GUIEVENT_VALUECHANGED, getText(), oldvaluestr);
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
	gColor oldcolor = renderer->getColor();
	gColor buttonColor = gColor(0.1f, 0.45f, 0.87f, 1.0f);
	gColor pressedButtonColor = gColor(0.08f, 0.36f, 0.71f, 1.0f);
	if(isdisabled) {
		renderer->setColor(&disabledbcolor);
	}
	else {
		if(b1ispressed) renderer->setColor(pressedButtonColor);
		else renderer->setColor(buttonColor);
	}
	gDrawRectangle(incboxposx, incboxposy + b1ispressed, smalboxwidth, smalboxheight, true);

	if(b1ispressed) renderer->setColor(middlegroundcolor);
	else renderer->setColor(textbackgroundcolor);
	gDrawTriangle(inctriucorpx, inctriucorpy, inctrilcorpx, inctrilcorpy, inctrircorpx, inctrircorpy, true);

	if(isdisabled) {
		renderer->setColor(&disabledbcolor);
	}
	else {
		if(b2ispressed) renderer->setColor(pressedButtonColor);
		else renderer->setColor(buttonColor);
	}
	gDrawRectangle(decboxposx, decboxposy + b2ispressed, smalboxwidth, smalboxheight, true);

	if(b2ispressed) renderer->setColor(middlegroundcolor);
	else renderer->setColor(textbackgroundcolor);
	gDrawTriangle(dectriucorpx, dectriucorpy, dectrilcorpx, dectrilcorpy, dectrircorpx, dectrircorpy, true);

	if(istitleshown) {
		if(isdisabled) renderer->setColor(&disabledfcolor);
		renderer->setColor(fontcolor);
		getFont()->drawText(title, left, top + getFont()->getSize());
	}
	renderer->setColor(oldcolor);
	if(guisizer) guisizer->draw();
}

void gGUINumberBox::setDisabled(bool isDisabled) {
	isdisabled = isDisabled;
	textbox.setDisabled(isdisabled);
}

int gGUINumberBox::calculateContentHeight() {
	return textbox.calculateContentHeight();
}
