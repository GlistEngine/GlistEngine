/*
 * gBaseGUIObject.h
 *
 *  Created on: Aug 11, 2021
 *      Author: noyan
 */

#ifndef BASE_GBASEGUIOBJECT_H_
#define BASE_GBASEGUIOBJECT_H_

#include "gRenderObject.h"
#include "gFont.h"
class gBaseApp;
#include "gGUIActionManager.h"
#include "gGUIResources.h"


class gBaseGUIObject : public gRenderObject {
public:
	static gGUIActionManager actionmanager;

	static const int GUIEVENT_FOCUSED = -1, GUIEVENT_UNFOCUSED = -2;

	static const int CURSOR_ARROW = 0, CURSOR_IBEAM = 1, CURSOR_CROSSHAIR = 2, CURSOR_HAND = 3, CURSOR_HRESIZE = 4, CURSOR_VRESIZE = 5;

	enum {
		TEXTALIGNMENT_LEFT,
		TEXTALIGNMENT_MIDDLE,
		TEXTALIGNMENT_RIGHT
	};

	gBaseGUIObject();
	virtual ~gBaseGUIObject();

	int getId();
	int getType();

	void setEnabled(bool isEnabled);
	bool isEnabled();

	void setTitleOn(bool isTitleOn);
	bool isTitleOn();

	virtual void setTopParent(gBaseGUIObject* parentGUIObject);
	gBaseGUIObject* getTopParent();

	virtual void setParent(gBaseGUIObject* parentGUIObject);
	gBaseGUIObject* getParent();

	void setParentSlotNo(int parentSlotLineNo, int parentSlotColumnNo);
	int getParentSlotLineNo();
	int getParentSlotColumnNo();

	void setTitle(std::string title);
	std::string getTitle();

	virtual void update() = 0;
	virtual void draw() = 0;
//	virtual void resetSize() = 0;

	static void setBackgroundColor(gColor* backgroundColor);
	static gColor* getBackgroundColor();

	static void setMiddlegroundColor(gColor* middlegroundColor);
	static gColor* getMiddlegroundColor();

	static void setForegroundColor(gColor* foregroundColor);
	static gColor* getForegroundColor();

	static void setTextBackgroundColor(gColor* textBackgroundColor);
	static gColor* getTextBackgroundColor();

	static void setNavigationBackgroundColor(gColor* navigationBackgroundColor);
	static gColor* getNavigationBackgroundColor();

	static void setFont(gFont* font);
	static gFont* getFont();

	static void setFontColor(gColor* fontColor);
	static gColor* getFontColor();

	static void setNavigationFontColor(gColor* navigationFontColor);
	static gColor* getNavigationFontColor();

	static void setButtonColor(gColor* color);
	static gColor* getButtonColor();

	static void setPressedButtonColor(gColor* color);
	static gColor* getPressedButtonColor();

	static void setDisabledButtonColor(gColor* color);
	static gColor* getDisabledButtonColor();

	static void setButtonFontColor(gColor* color);
	static gColor* getButtonFontColor();

	static void setPressedButtonFontColor(gColor* color);
	static gColor* getPressedButtonFontColor();

	static void setDisabledButtonFontColor(gColor* color);
	static gColor* getDisabledButtonFontColor();

	static void setToolBarButtonColor(gColor* toolbarbuttoncolor);
	static gColor* getToolBarButtonColor();

	static void setToolBarBottomLineColor(gColor* toolbarbottomlinecolor);
	static gColor* getToolBarBottomLineColor();

	virtual int getCursor(int x, int y);
	virtual void keyPressed(int key);
	virtual void keyReleased(int key);
	virtual void charPressed(unsigned int codepoint);
	virtual void mouseMoved(int x, int y);
	virtual void mousePressed(int x, int y, int button);
	virtual void mouseDragged(int x, int y, int button);
	virtual void mouseReleased(int x, int y, int button);
	virtual void mouseScrolled(int x, int y);
	virtual void mouseEntered();
	virtual void mouseExited();
	virtual void windowResized(int w, int h);
	virtual void onGUIEvent(int guiObjectId, int eventType, int sourceEventType, std::string value1 = "", std::string value2 = "");

	void setRootApp(gBaseApp* root);

	virtual void setCursorOn(bool isOn);


	int id, type;
	int left, top, right, bottom, width, height;
	int titlex, titley, titleheight;
	bool isfocused, iscursoron;
	bool issizer, iscontainer;
	static int focusid, previousfocusid;
	bool isresizable;

protected:
	static gGUIResources res;
	static gColor* backgroundcolor;
	static gColor* middlegroundcolor;
	static gColor* foregroundcolor;
	static gColor* textbackgroundcolor;
	static gColor* navigationbackgroundcolor;
	static gFont* font;
	static gColor* fontcolor;
	static gColor* navigationfontcolor;
	static gColor* buttoncolor;
	static gColor* pressedbuttoncolor;
	static gColor* disabledbuttoncolor;
	static gColor* buttonfontcolor;
	static gColor* pressedbuttonfontcolor;
	static gColor* disabledbuttonfontcolor;
	static gColor* toolbarbuttoncolor1;
	static gColor* toolbarbottomlinecolor1;

	std::string title;

	gBaseApp* root;
	gBaseGUIObject* topparent;
	gBaseGUIObject* parent;
	bool isenabled;
	bool istitleon;
	int parentslotlineno, parentslotcolumnno;

private:
	static int lastid;
};

#endif /* BASE_GBASEGUIOBJECT_H_ */
