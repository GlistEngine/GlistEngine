/*
 * gGUIButton.h
 *
 *  Created on: Aug 23, 2021
 *      Author: noyan
 */

#ifndef UI_GGUIBUTTON_H_
#define UI_GGUIBUTTON_H_

#include "gGUIControl.h"


class gGUIButton: public gGUIControl {
public:
	static const int TEXTTYPE_DEFAULT = 0;
	static const int TEXTTYPE_VERTICALLY_FLIPPED = 1;
	static const int TEXTTYPE_HORIZONTALLY_FLIPPED = 2;

	static const int TEXTSIDE_LEFT = 0;
	static const int TEXTSIDE_CENTER = 1;
	static const int TEXTSIDE_RIGHT = 2;

	gGUIButton(int texttype = TEXTTYPE_DEFAULT, int textside = TEXTSIDE_CENTER);
	virtual ~gGUIButton();

	void set(gBaseApp* root, gBaseGUIObject* topParentGUIObject, gBaseGUIObject* parentGUIObject, int parentSlotLineNo, int parentSlotColumnNo, int x, int y, int w, int h);

	void setTitle(std::string title);

	void setSize(int width, int height);

	/**
	 * @brief Sets whether the button fills the screen horizontally or vertically
	 *
	 * @param isWidthFit true if the text is scaled to fill the button horizontally, false otherwise
	 * @param isHeigthFit true if the text is scaled to fill the button vertically, false otherwise
	 */
	void setFitInArea(bool isHorizontalFit, bool isVerticalFit);

	/**
	 * @brief Changes whether the button works in toggle mode
	 *
	 * Toggle mode allows the button to switch between two states
	 *
	 * (e.g. active and inactive) each time it is clicked
	 *
	 * @param isToggle if true to enable toggle mode, false to disable it
	 */
	void setToggle(bool isToggle);

	/**
	 * @brief Changes whether the button is disabled
	 *
	 * When disabled, the button appears inactive and does not respond to user interactions
	 *
	 * @param isDisabled if true to disable the button, false to enable it
	 */
	void setDisabled(bool isDisabled);

	void setTextVisibility(bool isVisible);

	/**
	 * @brief Changing text type
	 *
	 * This function sets the orientation of the text to default, vertically flipped, or horizontally flipped.
	 *
	 * Possible values for `texttype` parameter:
	 * TEXTTYPE_DEFAULT
	 * TEXTTYPE_VERTICALLY_FLIPPED
	 * TEXTTYPE_HORIZONTALLY_FLIPPED
	 *
	 * @param texttype value of type of text
	 */
	void setTextType(int texttype);

	/**
	 * @brief Sets the alignment of the text
	 *
	 * This function sets the alignment of the text to either left, center, or right
	 *
	 * Possible values for `textside` parameter:
	 * TEXTSIDE_LEFT
	 * TEXTSIDE_CENTER
	 * TEXTSIDE_RIGHT
	 *
	 * @param textside The value of the side where the text is located
	 */
	void setTextSide(int textside);

	void setButtonw(int buttonw);

	void setButtonh(int buttonh);

	bool isPressed();

	bool isToggle();

	bool isDisabled();

	bool isTextVisible();

	void setButtonColor(gColor color);

	void setPressedButtonColor(gColor color);

	void setDisabledButtonColor(gColor color);

	void setButtonFontColor(gColor color);

	void setPressedButtonFontColor(gColor color);

	void setDisabledButtonFontColor(gColor color);

	/**
	 * @brief Gives button color RGBA value.
	 *
	 * This function can be use two type. Using directly giving memory address.
	 * Other way, Typing like function.r, function.g, function.b, function.a
	 * giving value of color component.
	 *
	 * @return bcolor a pointer to the button's background color
	 */
	gColor* getButtonColor();

	/**
	 * @brief Gives pressed button color RGBA value.
	 *
	 * This function can be use two type. Using directly giving memory address.
	 * Other way, Typing like function.r, function.g, function.b, function.a
	 * giving value of color component.
	 *
	 * @return pressedbcolor a pointer to the button's pressed background color
	 */
	gColor* getPressedButtonColor();

	/**
	 * @brief Gives disabled button color RGBA value.
	 *
	 * This function can be use two type. Using directly giving memory address.
	 * Other way, Typing like function.r, function.g, function.b, function.a
	 * giving value of color component.
	 *
	 * @return disabledbcolor a pointer to the button's disabled background color
	 */
	gColor* getDisabledButtonColor();

	/**
	 * @brief Gives button's font color RGBA value.
	 *
	 * This function can be use two type. Using directly giving memory address.
	 * Other way, Typing like function.r, function.g, function.b, function.a
	 * giving value of color component.
	 *
	 * @return fcolor a pointer to the button's font color
	 */
	gColor* getButtonFontColor();

	/**
	 * @brief Gives pressed button button's font color RGBA value.
	 *
	 * This function can be use two type. Using directly giving memory address.
	 * Other way, Typing like function.r, function.g, function.b, function.a
	 * giving value of color component.
	 *
	 * @return pressedfcolor a pointer to the button's pressed font color
	 */
	gColor* getPressedButtonFontColor();

	/**
	 * @brief Gives button color RGBA value.
	 *
	 * This function can be use two type. Using directly giving memory address.
	 * Other way, Typing like function.r, function.g, function.b, function.a
	 * giving value of color component.
	 *
	 * @return disabledfcolor a pointer to the button's disabled font color
	 */
	gColor* getDisabledButtonFontColor();

	void enableBackgroundFill(bool isEnabled);

	int getButtonWidth();

	int getButtonHeight();

	/**
	 * @brief Draws the body of the button
	 *
	 * This function sets the color based on the button's state (disabled, pressed, hovered, or normal)
	 * and then draws the button's rectangle if the background should be filled
	 */
	void drawBody();

	virtual void update();

	/*
	 * @brief Renders the component on the screen
	 *
	 * This function is called to draw the button. It saves the current color,
	 * renders the button's body and text, and then restores the old color
	 */
	virtual void draw();

	virtual void mousePressed(int x, int y, int button);

	virtual void mouseReleased(int x, int y, int button);

	virtual void mouseMoved(int x, int y);

	virtual void mouseDragged(int x, int y, int button);

	virtual void mouseExited();

	virtual void mouseEntered();

protected:
	const int DEFAULT_FONT_SIZE = 10;
	bool ispressed;
	bool ishover;
	int buttonw, buttonh;
	int tx, ty, tw, th;
	int punto;
	int texttype, textside;
	float centerx, centery;
	float scaledtextwidth, scaledtextheight, scalefactor;
	bool ishorizontalfit, isverticalfit;
	bool textsizecontrol;
	bool istextvisible;
	bool istoggle;
	bool ispressednow;
	bool isdisabled;
	gColor bcolor, pressedbcolor, disabledbcolor;
	gColor fcolor, pressedfcolor, disabledfcolor;
	gColor hcolor;
	bool fillbackground;

	/**
	 * @brief Resets the position and size of the title text on the button
	 *
	 * This function recalculates the position (tx, ty) and size (tw, th) of the title text
	 * based on the current button dimensions and the dimensions of the rendered text
	 */
	void resetTitlePosition();

private:
	/**
	 * @brief Function used to write text on a Button
	 *
	 * Adjusts the position of the text to the desired side
	 * Sets the text type and draws the text
	 *
	 * @param textside variable determining the side where the text will be placed
	 * @param texttype variable determining the type of text
	 */
	void writeText();

	/**
	 * @brief Calculates the center position of the text within a button and scales the text if necessary
	 *
	 * @param tw the original width of the text
	 * @param th the original height of the text
	 * @param buttonw the width of the button
	 * @param buttonh the height of the button
	 * @param left the left position of the button
	 * @param top the top position of the button
	 * @param ispressed offset value indicating if the button is pressed
	 * @param DEFAULT_FONT_SIZE the default font size of the text
	 * @param scaledTextWidth the scaled width of the text
	 * @param scaledTextHeight the scaled height of the text
	 * @param scalefactor the factor by which the text is scaled to fit within the button
	 * @param punto the resulting font size after scaling
	 * @param centerx the x-coordinate of the text center
	 * @param centery the y-coordinate of the text center
	 * @return true if the text was scaled to fit within the button, false otherwise
	 */
	bool calculateCenter();

	/**
	 * @brief Calculates the center-left position of the text within a button and scales the text if necessary
	 *
	 * @see calculateCenter
	 */

	bool calculateCenterLeft();

	/**
	 * @brief Calculates the center-right position of the text within a button and scales the text if necessary
	 *
	 * @see calculateCenter
	 */
	bool calculateCenterRight();
};

#endif /* UI_GGUIBUTTON_H_ */
