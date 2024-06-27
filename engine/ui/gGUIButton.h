/*
 * gGUIButton.h
 *
 *  Created on: Aug 23, 2021
 *      Author: noyan
 */

#ifndef UI_GGUIBUTTON_H_
#define UI_GGUIBUTTON_H_

#include "gGUIControl.h"
#include "gGUIScrollable.h"
#include "gFont.h"


class gGUIButton: public gGUIControl {
public:

	gGUIButton();
	virtual ~gGUIButton();

	//void set(gBaseApp* root, gBaseGUIObject* topParentGUIObject, gBaseGUIObject* parentGUIObject, int parentSlotLineNo, int parentSlotColumnNo, int x, int y, int w, int h);

	void setTitle(std::string title);
	void setSize(int width, int height);
	void setToggle(bool isToggle);
	void setStaticness(bool isStatic); //must be set to false if button is used for static elements such as dropdownlist
	void setButtonMargin(int margin);  //given margin would compress the button in the panel by the given amount
	void setDisabled(bool isDisabled);
	void setTextVisibility(bool isVisible);

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
	gColor* getButtonColor();
	gColor* getPressedButtonColor();
	gColor* getDisabledButtonColor();
	gColor* getButtonFontColor();
	gColor* getPressedButtonFontColor();
	gColor* getDisabledButtonFontColor();

	void enableBackgroundFill(bool isEnabled);

	int getButtonWidth();
	int getButtonHeight();

	virtual void update();
	virtual void draw();

	virtual void mousePressed(int x, int y, int button);
	virtual void mouseReleased(int x, int y, int button);
	virtual void mouseMoved(int x, int y);
	virtual void mouseDragged(int x, int y, int button);
	virtual void mouseEntered();
	virtual void mouseExited();
	void setButtonh(int buttonh);
	void setButtonw(int buttonw);

protected:
	bool ispressed;
	bool ishover;
	int buttonx, buttony, buttonw, buttonh;
	int tx, ty;
	bool istextvisible;
	bool istoggle;
	bool ispressednow;
	bool isdisabled;
	gColor bcolor, pressedbcolor, disabledbcolor;
	gColor fcolor, pressedfcolor, disabledfcolor;
	gColor hcolor;
	bool fillbackground;

	void resetTitlePosition();

private:
	int fontsize;
	int buttonmargin;
	bool isstatic;
	bool issetupped;
	gFont freesans12;
	gFont freesans18;
	gFont freesans24;
	gFont freesans36;
	gFont freesans48;
	gFont freesans60;
};

#endif /* UI_GGUIBUTTON_H_ */
