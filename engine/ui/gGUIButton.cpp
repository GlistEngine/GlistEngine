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
	resetTitlePosition();
}

gGUIButton::~gGUIButton() {
}

void gGUIButton::set(gBaseApp* root, gBaseGUIObject* topParentGUIObject, gBaseGUIObject* parentGUIObject, int parentSlotLineNo, int parentSlotColumnNo, int x, int y, int w, int h) {
	gGUIControl::set(root, topParentGUIObject, parentGUIObject, parentSlotLineNo, parentSlotColumnNo, x, y, w, h);

	//If isHorizontalFit true, it transfers the width of the current parent to buttonw. If false, it assigns the default width value.
	buttonw = isHorizontalFit ? width : buttonw;

	//If isVerticalFit true, it transfers the height of the current parent to buttonh. If false, it assigns the default height value.
	buttonh = isVerticalFit ? height : buttonh;
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
				textsizecontrol = calculateCenterLeft();
				break;
			case TEXTSIDE_CENTER:
				textsizecontrol = calculateCenter();
				break;
			case TEXTSIDE_RIGHT:
				textsizecontrol = calculateCenterRight();
				break;
			default:
				textsizecontrol = calculateCenter();
				break;
		}

		// Sets the text type and draws the text.
		switch(texttype) {
			case TEXTTYPE_DEFAULT:
				if(textsizecontrol) font->drawText(title, centerx, centery, punto);
				else font->drawText(title, centerx, centery);
				break;
			case TEXTTYPE_VERTICALLY_FLIPPED:
				if(textsizecontrol) font->drawTextVerticallyFlipped(title, centerx, centery, punto);
				else font->drawTextVerticallyFlipped(title, centerx, centery);
				break;
			case TEXTTYPE_HORIZONTALLY_FLIPPED:
				if(textsizecontrol) font->drawTextHorizontallyFlipped(title, centerx, centery, punto);
				else font->drawTextHorizontallyFlipped(title, centerx, centery);
				break;
			default:
				if(textsizecontrol) font->drawText(title, centerx, centery, punto);
				else font->drawText(title, centerx, centery);
				break;
		}
	}
}

bool gGUIButton::calculateCenter() {
	// The width and height of the text are scaled based on text size.
	scaledTextWidth = tw * 1.0f;
	scaledTextHeight = th * 1.0f;

	// Check if scaling is necessary.
	if (scaledTextWidth > buttonw || scaledTextHeight > buttonh) {
		// Calculate the scale factor to make the text fit the button size.
		scalefactor = std::min(buttonw / scaledTextWidth, buttonh / scaledTextHeight);

		// Calculate the position where the text is aligned within the button with scaling applied.
		punto = static_cast<int>(DEFAULT_FONT_SIZE * scalefactor);
		centerx = left + (buttonw - scaledTextWidth * scalefactor) / 2;
		centery = top + ispressed + (buttonh - scaledTextHeight * scalefactor) / 2;

		return true;
	}
	else {
		// Calculate text position.
		centerx = left + (buttonw - tw) / 2;
		centery = top + ispressed + (buttonh - th) / 2;

		return false;
	}
	return false;
}

bool gGUIButton::calculateCenterLeft() {
    // The width and height of the text are scaled based on text size.
    scaledTextWidth = tw * 1.0f;
    scaledTextHeight = th * 1.0f;

    // Check if scaling is necessary.
    if (scaledTextWidth > buttonw || scaledTextHeight > buttonh) {
        // Calculate the scale factor to make the text fit the button size.
        scalefactor = std::min(buttonw / scaledTextWidth, buttonh / scaledTextHeight);

        // Calculate the position where the text is aligned within the button with scaling applied.
        punto = static_cast<int>(DEFAULT_FONT_SIZE * scalefactor);
        centerx = left;
        centery = top + ispressed + (buttonh - scaledTextHeight * scalefactor) / 2;

        return true;
    }
    else {
        // Calculate text position.
        centerx = left;
        centery = top + ispressed + (buttonh - th) / 2;

        return false;
    }
    return false;
}

bool gGUIButton::calculateCenterRight() {
    // The width and height of the text are scaled based on text size.
    scaledTextWidth = tw * 1.0f;
    scaledTextHeight = th * 1.0f;

    // Check if scaling is necessary.
    if (scaledTextWidth > buttonw || scaledTextHeight > buttonh) {
        // Calculate the scale factor to make the text fit the button size.
        scalefactor = std::min(buttonw / scaledTextWidth, buttonh / scaledTextHeight);

        // Calculate the position where the text is aligned within the button with scaling applied.
        punto = static_cast<int>(DEFAULT_FONT_SIZE * scalefactor);
        centerx = left + buttonw - scaledTextWidth * scalefactor;
        centery = top + ispressed + (buttonh - scaledTextHeight * scalefactor) / 2;

        return true;
    }
    else {
        // Calculate text position.
        centerx = left + buttonw - tw - 5;
        centery = top + ispressed + (buttonh - th) / 2;

        return false;
    }
    return false;
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
	tx = (buttonw - font->getStringWidth(title)) / 2 - 1;
	ty = (buttonh - font->getStringHeight("a")) / 2;
	tw = font->getStringWidth(title);
	th = font->getStringHeight(title);
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
	this->isHorizontalFit = isHorizontalFit;
	this->isVerticalFit = isVerticalFit;
}

void gGUIButton::setTextType(int texttype) {
	this->texttype = texttype;
}

void gGUIButton::setTextSide(int textside) {
	this->textside = textside;
}
