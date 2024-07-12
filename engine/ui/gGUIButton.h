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

	gGUIButton();
	virtual ~gGUIButton();

	void setTitle(std::string title);
	void setSize(int width, int height);
	void setToggle(bool isToggle);
	void setDisabled(bool isDisabled);
	void setTextVisibility(bool isVisible);

	bool isPressed();
	bool& isPressedRef();
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
	int buttonw, buttonh;
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
};

#endif /* UI_GGUIBUTTON_H_ */
