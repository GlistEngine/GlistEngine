/*
 * gGUICheckbox.h
 *
 *  Created on: Nov 5, 2021
 *      Author: Ahmet Melih
 */

#ifndef UI_GGUICHECKBOX_H_
#define UI_GGUICHECKBOX_H_

#include "gGUIButton.h"

class gGUICheckbox: public gGUIButton {
public:
	gGUICheckbox();
	virtual ~gGUICheckbox();

	void setTitle(std::string title);
	void setSize(int width, int height);
	void setPosition(int left, int top);
	void setChecked(bool isChecked);

	bool isChecked();
	bool isTextVisible();

	void setBgColor(gColor color);
	void setTickColor(gColor color);
	gColor* getBgColor();
	gColor* getTickColor();

	void update() override;
	void draw() override;

	void mousePressed(int x, int y, int button) override;
	void mouseReleased(int x, int y, int button) override;

	void setDisabled(bool isDisabled);

	int calculateContentHeight() override;

protected:
	friend class gGUIToolbar;

	bool ischecked;

	int buttonw, buttonh;
	int titlew, titleh;

	gColor tickcolor, titlecolor;
	bool isdisabled;
};

#endif /* UI_GGUICHECKBOX_H_ */
