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


class gBaseGUIObject : public gRenderObject {
public:

	static const int GUIEVENT_FOCUSED = -1, GUIEVENT_UNFOCUSED = -2;

	static const int CURSOR_ARROW = 0, CURSOR_IBEAM = 1, CURSOR_CROSSHAIR = 2, CURSOR_HAND = 3, CURSOR_HRESIZE = 4, CURSOR_VRESIZE = 5;

	gBaseGUIObject();
	virtual ~gBaseGUIObject();

	int getId();
	int getType();

	void setEnabled(bool isEnabled);
	bool isEnabled();

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

	static void setForegroundColor(gColor* foregroundColor);
	static gColor* getForegroundColor();

	static void setFont(gFont* font);
	static gFont* getFont();

	static void setFontColor(gColor* fontColor);
	static gColor* getFontColor();

	virtual int getCursor(int x, int y);
	virtual void keyPressed(int key);
	virtual void keyReleased(int key);
	virtual void charPressed(unsigned int key);
	virtual void mouseMoved(int x, int y);
	virtual void mousePressed(int x, int y, int button);
	virtual void mouseDragged(int x, int y, int button);
	virtual void mouseReleased(int x, int y, int button);
	virtual void mouseScrolled(int x, int y);
	virtual void mouseEntered();
	virtual void mouseExited();

	void setRootApp(gBaseApp* root);

	int id, type;
	int left, top, right, bottom, width, height;
	bool isfocused, iscursoron;

protected:
	static gColor* backgroundcolor;
	static gColor* foregroundcolor;
	static gFont* font;
	static gColor* fontcolor;

	std::string title;

	gBaseApp* root;
	gBaseGUIObject* parent;
	bool isenabled;
	int parentslotlineno, parentslotcolumnno;

private:
	static int lastid;
};

#endif /* BASE_GBASEGUIOBJECT_H_ */
