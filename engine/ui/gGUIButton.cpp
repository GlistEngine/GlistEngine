/*
 * gGUIButton.cpp
 *
 *  Created on: Aug 23, 2021
 *      Author: noyan
 */

#include "gGUIButton.h"
#include "gBaseApp.h"
#include "gBaseCanvas.h"


gGUIButton::gGUIButton(int texttype, int textside) {
	ispressed = false;
	ishover = false;
	buttonh = 50; // default value
	buttonw = 120; // default value
	istoggle = false;
	title = "Button";
	istextvisible = true;
	ispressednow = false;
	isdisabled = false;
	bcolor = *buttoncolor;
//	gLogi("Button") << "fgr:" << foregroundcolor->r << ", fgg:" << foregroundcolor->g << ", fgb:" << foregroundcolor->b;
//	gLogi("Button") << "br:" << buttoncolor->r << ", bg:" << buttoncolor->g << ", bb:" << buttoncolor->b;
//	gLogi("Button") << "r:" << bcolor.r << ", g:" << bcolor.g << ", b:" << bcolor.b;
	pressedbcolor = *pressedbuttoncolor;
	hcolor.set((pressedbcolor.r + bcolor.r) / 2, (pressedbcolor.g + bcolor.g) / 2, (pressedbcolor.b + bcolor.b) / 2);
	disabledbcolor = *disabledbuttoncolor;
	fcolor = *buttonfontcolor;
	pressedfcolor = *pressedbuttonfontcolor;
	disabledfcolor = *disabledbuttonfontcolor;
	fillbackground = true;
	this->textside = textside;
	this->texttype = texttype;
	scalefactor = 1.0f;
	punto = 20;
	newpunto = 0;
	fontpath = "FreeSans.ttf"; // default font

	buttonfont = new gFont();
	buttonfont->loadFont(fontpath, punto);

	resetTitlePosition();
}

gGUIButton::~gGUIButton() {
}

void gGUIButton::set(gBaseApp* root, gBaseGUIObject* topParentGUIObject, gBaseGUIObject* parentGUIObject, int parentSlotLineNo, int parentSlotColumnNo, int x, int y, int w, int h) {
	gGUIControl::set(root, topParentGUIObject, parentGUIObject, parentSlotLineNo, parentSlotColumnNo, x, y, w, h);

	//If ishorizontalfit true, it transfers the width of the current parent to buttonw. If false, it assigns the default width value.
	buttonw = ishorizontalfit ? width : buttonw;

	//If isverticalfit true, it transfers the height of the current parent to buttonh. If false, it assigns the default height value.
	buttonh = isverticalfit ? height : buttonh;
}

void gGUIButton::setTitle(std::string title) {
	gBaseGUIObject::setTitle(title);
	resetTitlePosition();
}

void gGUIButton::setSize(int width, int height) {
	buttonw = width;
	buttonh = height;
	resetTitlePosition();
}

void gGUIButton::setTextVisibility(bool isVisible) {
	istextvisible = isVisible;
}

void gGUIButton::setToggle(bool isToggle) {
	istoggle = isToggle;
}

void gGUIButton::setDisabled(bool isDisabled) {
	isdisabled = isDisabled;
}

bool gGUIButton::isDisabled() {
	return isdisabled;
}

bool gGUIButton::isToggle() {
	return istoggle;
}

bool gGUIButton::isTextVisible() {
	return istextvisible;
}

bool gGUIButton::isPressed() {
	return ispressed;
}

void gGUIButton::update() {
//	gLogi("gGUIButton") << "update";
}

void gGUIButton::draw() {

	// Get the current color and save the old color
	gColor* oldcolor = renderer->getColor();

	// Draw button body
	drawBody();

	// Draw button text
	writeText();

	// Restore the color to the old color
	renderer->setColor(oldcolor);
}

void gGUIButton::drawBody() {
	// If fillbackground is true, it fills the background with the given color.
	if(fillbackground) {
		// If it is button disabled, the disabled button color assigned.
		if(isdisabled) renderer->setColor(&disabledbcolor);
		else {
			if(ispressed) renderer->setColor(&pressedbcolor);
			else if(ishover) renderer->setColor(&hcolor);
			else renderer->setColor(&bcolor);
		}

		// Calls the gDrawRectangle function to draw a rectangle.
		gDrawRectangle(left, top + ispressed, buttonw, buttonh, true);
	}
}

void gGUIButton::writeText() {


	buttonfont->loadFont(fontpath, punto);

 // Check istextvisible, if true, draw the text, if false, do not draw the text.
	if(istextvisible) {
		// Check if the button is disabled, if true, assign the disabledcolor to the renderer, if false, assign the set color to the renderer.
		if(isdisabled) renderer->setColor(&disabledfcolor);
		else {
			// Check whether the button is clicked and assign the color accordingly.
			if(ispressed) renderer->setColor(&pressedfcolor);
			else renderer->setColor(&fcolor);
		}

		// Adjusts the location of the text to the desired side.
		switch(textside) {
			case TEXTSIDE_LEFT:
				calculateCenterLeft();
				break;
			case TEXTSIDE_CENTER:
				calculateCenter();
				break;
			case TEXTSIDE_RIGHT:
				calculateCenterRight();
				break;
			default:
				calculateCenter();
				break;
		}

		// Sets the text type and draws the text.
		switch(texttype) {
			case TEXTTYPE_DEFAULT:
				buttonfont->drawText(title, centerx, centery);
				break;
			case TEXTTYPE_VERTICALLY_FLIPPED:
				buttonfont->drawTextVerticallyFlipped(title, centerx, centery);
				break;
			case TEXTTYPE_HORIZONTALLY_FLIPPED:
				buttonfont->drawTextHorizontallyFlipped(title, centerx, centery);
				break;
			default:
				buttonfont->drawText(title, centerx, centery);
				break;
		}
	}
}

int gGUIButton::getNearestPunto(float value) {
	// Calculate the nearest value within the desired font size range
	float roundedvalue = std::round(value / PUNTO_STEP) * PUNTO_STEP;

	// Check if the font size is within the specified range
	if (roundedvalue < PUNTO_MIN) return static_cast<int>(PUNTO_MIN);
	if (roundedvalue > PUNTO_MAX) return static_cast<int>(PUNTO_MAX);
	return static_cast<int>(roundedvalue);
}

void gGUIButton::scaletext() {
	// Calculate the initial text dimensions
	scaledtextwidth = tw * 1.0f;
	scaledtextheight = th * 1.0f;

	// Calculate the scaling factors
	widthfactor = buttonw / scaledtextwidth;
	heightfactor = buttonh / scaledtextheight;

	// Determine the most suitable scaling factor to ensure the text fits within the buttons boundaries
	scalefactor = std::min(widthfactor, heightfactor);

	// Calculate the scaled font size
	scaledpunto = DEFAULT_FONT_SIZE * scalefactor;

	// Find the nearest font size value
	punto = getNearestPunto(scaledpunto);

	// Calculate the updated text width
	scaledtextwidth = (tw * scalefactor) / 1.75;

	// Calculate scaled text height
	scaledtextheight = th * scalefactor - scaledpunto;
}

void gGUIButton::calculateCenter() {
	scaletext();

	// Position the text in the center of the button
    centerx = left + (buttonw - scaledtextwidth) / 2.0f;

    // Calculate the appropriate average for text height
    centery = top + (buttonh + scaledtextheight) / 2.0f;
}

void gGUIButton::calculateCenterLeft() {
	scaletext();

	// Position the text to the left of the button
    centerx = left + (buttonw - scaledtextwidth) / 8.0f;

    // Calculate the appropriate average for text height
    centery = top + (buttonh + scaledtextheight) / 2.0f;
}

void gGUIButton::calculateCenterRight() {
	scaletext();

	// Position the text to the right of the button
    centerx = left + 3 * (buttonw - scaledtextwidth) / 4.0f;

    // Calculate the appropriate average for text height
    centery = top + (buttonh + scaledtextheight) / 2.0f;
}

void gGUIButton::mousePressed(int x, int y, int button) {
//	gLogi("Button") << "pressed, id:" << id;
	if(isdisabled) return;
	if(x >= left && x < left + buttonw && y >= top && y < top + buttonh) {
		if(!istoggle) ispressed = true;
		else {
			if(!ispressed) {
				ispressed = true;
				ispressednow = true;
			}
		}
		root->getCurrentCanvas()->onGuiEvent(id, G_GUIEVENT_BUTTONPRESSED);
	}
}

void gGUIButton::mouseReleased(int x, int y, int button) {
//	gLogi("Button") << "released, id:" << id;
	if(isdisabled) return;
	if(ispressed && x >= left && x < left + buttonw && y >= top && y < top + buttonh) {
		if(!istoggle) ispressed = false;
		else {
			if(!ispressednow) ispressed = false;
		}
		root->getCurrentCanvas()->onGuiEvent(id, G_GUIEVENT_BUTTONRELEASED);
		actionmanager.onGUIEvent(id, G_GUIEVENT_BUTTONRELEASED);
	} else {
		if(!istoggle) ispressed = false;
	}
	ispressednow = false;
}

void gGUIButton::mouseMoved(int x, int y) {
	if(iscursoron && x >= left && y >= top && x < left + buttonw && y < top + buttonh) ishover = true;
	else ishover = false;
}

void gGUIButton::mouseDragged(int x, int y, int button) {
	if(iscursoron && x >= left && y >= top && x < left + buttonw && y < top + buttonh) ishover = true;
	else ishover = false;
}

void gGUIButton::mouseEntered() {

}

void gGUIButton::mouseExited() {
	ishover = false;
}

void gGUIButton::resetTitlePosition() {
	tx = (buttonw - buttonfont->getStringWidth(title)) / 2 - 1;
	ty = (buttonh - buttonfont->getStringHeight("a")) / 2;
	tw = buttonfont->getStringWidth(title);
	th = buttonfont->getStringHeight(title);
}

void gGUIButton::setButtonColor(gColor color) {
	bcolor = color;
	hcolor.set((pressedbcolor.r + bcolor.r) / 2, (pressedbcolor.g + bcolor.g) / 2, (pressedbcolor.b + bcolor.b) / 2);
}

void gGUIButton::setPressedButtonColor(gColor color) {
	pressedbcolor = color;
	hcolor.set((pressedbcolor.r + bcolor.r) / 2, (pressedbcolor.g + bcolor.g) / 2, (pressedbcolor.b + bcolor.b) / 2);
}

void gGUIButton::setDisabledButtonColor(gColor color) {
	disabledbcolor = color;
}

void gGUIButton::setButtonFontColor(gColor color) {
	fcolor = color;
}

void gGUIButton::setPressedButtonFontColor(gColor color) {
	pressedfcolor = color;
}

void gGUIButton::setDisabledButtonFontColor(gColor color) {
	disabledfcolor = color;
}

gColor* gGUIButton::getButtonColor() {
	return &bcolor;
}

gColor* gGUIButton::getPressedButtonColor() {
	return &pressedbcolor;
}

gColor* gGUIButton::getDisabledButtonColor() {
	return &disabledbcolor;
}

gColor* gGUIButton::getButtonFontColor() {
	return &fcolor;
}

gColor* gGUIButton::getPressedButtonFontColor() {
	return &pressedfcolor;
}

gColor* gGUIButton::getDisabledButtonFontColor() {
	return &disabledfcolor;
}

void gGUIButton::enableBackgroundFill(bool isEnabled) {
	fillbackground = isEnabled;
}

int gGUIButton::getButtonWidth() {
	return buttonw;
}

int gGUIButton::getButtonHeight() {
	return buttonh;
}

void gGUIButton::setButtonh(int buttonh) {
	this->buttonh = buttonh;
}

void gGUIButton::setButtonw(int buttonw) {
	this->buttonw = buttonw;
}

void gGUIButton::setFitInArea(bool isHorizontalFit, bool isVerticalFit) {
	this->ishorizontalfit = isHorizontalFit;
	this->isverticalfit = isVerticalFit;
}

void gGUIButton::setTextType(int texttype) {
	this->texttype = texttype;
}

void gGUIButton::setTextSide(int textside) {
	this->textside = textside;
}

void gGUIButton::setFont(std::string fontpath) {
	this->fontpath = fontpath;
}
