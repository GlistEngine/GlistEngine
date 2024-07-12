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

	/**
	 * @brief Changing text title.
	 *
	 * @param title text inside the button
	 */
	void setTitle(std::string title);

	/**
	 * @brief Changing box size using width and height
	 *
	 * @param width width of button
	 * @param height height of button
	 */
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

	/**
	 * @brief Changes the visibility of the button's text
	 *
	 * @param isVisible if true, the button text is visible, if false, it is not visible
	 */
	void setTextVisibility(bool isVisible);

	/**
	 * @brief Changing text type
	 *
	 * @param texttype value of type of text
	 */
	void setTextType(int texttype);

	/**
	 * @brief Changing text side
	 *
	 * @param textside The value of the side where the text is located
	 */
	void setTextSide(int textside);

	/**
	 * @brief Checks if the button is currently pressed
	 *
	 * @return ispressed true if the button is pressed, false otherwise
	 */
	bool isPressed();

	/**
	 * @brief Checks if the button is in toggle mode
	 *
	 * @return istoggle true if the button is in toggle mode, false otherwise
	 */
	bool isToggle();

	/**
	 * @brief Checks if the button is disabled
	 *
	 * @return isdisabled true if the button is disabled, false otherwise
	 */
	bool isDisabled();

	/**
	 * @brief Checks if the text on the button is visible
	 *
	 * @return istextvisible true if the text on the button is visible, false otherwise
	 */
	bool isTextVisible();


	/**
	 * @brief Sets the button's background color
	 *
	 * @param color new background color
	 */
	void setButtonColor(gColor color);

	/**
	 * @brief Sets the pressed button background color
	 *
	 * @param color new pressed background color
	 */
	void setPressedButtonColor(gColor color);

	/**
	 * @brief Sets the disabled button background color
	 *
	 * @param color new disabled background color
	 */
	void setDisabledButtonColor(gColor color);

	/**
	 * @brief Sets the button's font color
	 *
	 * @param color new font color
	 */
	void setButtonFontColor(gColor color);

	/**
	 * @brief Sets the pressed button font color
	 *
	 * @param color new pressed font color
	 */
	void setPressedButtonFontColor(gColor color);

	/**
	 * @brief Sets the disabled button font color
	 *
	 * @param color new disabled font color
	 */
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

	/**
	 * @brief Enables or disables the filling of the background
	 *
	 * @param isEnabled If true, the container's background will be filled; if false, the background will not be filled
	 */
	void enableBackgroundFill(bool isEnabled);

	/**
 	 * @brief Gives the width of the button
 	 *
	 * @return buttonw button width
	 */
	int getButtonWidth();

	/**
 	 * @brief Gives the height of the button
 	 *
	 * @return buttonh button height
	 */
	int getButtonHeight();

	/**
	 * @brief Changing the width of the button using buttonh
	 *
	 * @param buttonw button width
	 */
	void setButtonw(int buttonw);

	/**
	 * @brief Changing the height of the button using buttonh
	 *
	 * @param buttonh button height
	 */
	void setButtonh(int buttonh);


	/**
	 * @brief Draws the body of the button
	 *
	 * This function sets the color based on the button's state (disabled, pressed, hovered, or normal)
	 * and then draws the button's rectangle if the background should be filled
	 */
	void drawBody();

	/*
	 * @brief Called once per frame
	 *
	 */
	virtual void update();

	/*
	 * @brief Renders the component on the screen
	 *
	 * This function is called to draw the button. It saves the current color,
	 * renders the button's body and text, and then restores the old color
	 */
	virtual void draw();

	/**
	 * @brief Handles mouse press events for a button
	 *
	 * @param x The x-coordinate of the mouse press
	 * @param y The y-coordinate of the mouse press
	 * @param button The button ID of the mouse press
	 */
	virtual void mousePressed(int x, int y, int button);

	/**
	 * @brief Handles mouse release events for a button.
	 *
	 * @param x The x-coordinate of the mouse release.
	 * @param y The y-coordinate of the mouse release.
	 * @param button The button ID of the mouse release.
	 */
	virtual void mouseReleased(int x, int y, int button);

	/**
	 * @brief Handles mouse move events for the button to detect hovering.
	 *
	 * @param x The x-coordinate of the mouse move.
	 * @param y The y-coordinate of the mouse move.
	 */
	virtual void mouseMoved(int x, int y);

	/**
	 * @brief Handles mouse drag events for the button.
	 *
	 * @param x The x-coordinate of the mouse drag.
	 * @param y The y-coordinate of the mouse drag.
	 * @param button The button ID of the mouse drag.
	 */
	virtual void mouseDragged(int x, int y, int button);

	/**
	 * @brief Handles mouse exit events for the button to reset hover state.
	 */
	virtual void mouseExited();

	/*
	 * @brief Handles mouse enter events for the button to set hover state
	 */
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
	float scaledTextWidth, scaledTextHeight, scalefactor;
	bool isHorizontalFit, isVerticalFit;
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
