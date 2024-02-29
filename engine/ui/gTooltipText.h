/*
 * gTooltipText.h
 *
 *  Created on: 03 Jan 2024
 *  Author    : Furkan
 */

#ifndef SRC_GTOOLTIPTEXT_H_
#define SRC_GTOOLTIPTEXT_H_

#include "gGUISizer.h"
#include "gFont.h"
#include "gGUIControl.h"
#include "gColor.h"


class gTooltipText: public gGUIControl {
public:

	/*
	 * To activate these class needed 2 step as follows;
	 * 	First: void setTooltipText(tooltipText* tooltiptext);
	 * above method needed to be evoked by current frame object
	 * 	Second: void tooltipActivate(gGUISizer* sizer, int guiobject, int lineNo, int columnNo, std::string text);
	 * below method should be called with the necessary parameters
	 */

	gTooltipText();
	virtual ~gTooltipText();

	enum objects {
		Button,
		Textbox,
		Numberbox,
		Panel
	};

	/*
	 * The set method works for setting the tooltip text which the user desires
	 * The get method works for getting the tooltip text which the user defined
	 * @param text, Text the user set it
	 */

	void setText(std::string text);
	std::string getText();

	/*
	 *	tooltipActivate method works for the activation of the tooltip text massage by the parameters of user
	 *	and assigning the coordinates to draw the massage
	 *	This method evokes 2 other functions
	 *	First it evokes setText method
	 *	second it evokes setTitlePosition method
	 *	@param sizer which is the guiobject included
	 *	@param guiobject which is the type of the object such as Button, textbox or text
	 *	@param lineNo which is the line number refers the desired guiobject
	 *	@param lineNo which is the column number refers the desired guiobject
	 *	@param text Text the user set it
	 */

	void gTooltipActivate(gGUISizer* sizer, objects guiobject, int lineNo, int columnNo, std::string text);

	/*
	 * setTitleLength method manage the length of the massage to draw
	 * @param text Text the user set it
	 */

	void setTitleLength(std::string text);

	void update();
	void draw();
	void mouseMoved(int x, int y);

private:
	gColor color;
	std::string text;
	std::deque<std::string> textlist;
	int sizerx, sizery, sizerw, sizerh;
	int cursorx, cursory;
	int objecth, margin;
	float texth, textw, tx;
};

#endif /* SRC_GTOOLTIPTEXT_H_ */
