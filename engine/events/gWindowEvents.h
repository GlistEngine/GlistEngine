/*
* gEvent.h
*
*  Created on: June 24, 2023
*      Author: Metehan Gezer
*/

#ifndef GWINDOWEVENTS_H_
#define GWINDOWEVENTS_H_

#include "gEvent.h"
#include "gUtils.h"

enum gCursorMode : uint8_t {
	CURSORMODE_NORMAL,
	CURSORMODE_HIDDEN,
	CURSORMODE_DISABLED,
	CURSORMODE_RELATIVE
};

class gKeyEvent : public gEvent {
public:
	int getKeyCode() const { return keycode; }

	G_EVENT_CLASS_CATEGORY(EVENTCATEGORY_KEYBOARD | EVENTCATEGORY_INPUT)
protected:
	gKeyEvent(const int keycode)
		: keycode(keycode) {}

	int keycode;
};

class gCharTypedEvent : public gEvent {
public:
	gCharTypedEvent(const unsigned int keycode)
		: keycode(keycode) {}

	unsigned int getCharacter() const { return keycode; }

	G_EVENT_CLASS_TYPE(gCharTypedEvent)
	G_EVENT_CLASS_CATEGORY(EVENTCATEGORY_INPUT | EVENTCATEGORY_KEYBOARD)
private:
	unsigned int keycode;
};

class gKeyPressedEvent : public gKeyEvent {
public:
	gKeyPressedEvent(const int keycode, bool isrepeat = false)
		: gKeyEvent(keycode),
		  isrepeat(isrepeat) {}

	bool isRepeat() const { return isrepeat; }

	G_EVENT_CLASS_TYPE(gKeyPressedEvent)
private:
	bool isrepeat;
};

class gKeyReleasedEvent : public gKeyEvent {
public:
	gKeyReleasedEvent(const int keycode)
		: gKeyEvent(keycode) {}

	G_EVENT_CLASS_TYPE(gKeyReleasedEvent)
};


class gMouseMovedEvent : public gEvent {
public:
	gMouseMovedEvent(float x, float y, gCursorMode cursormode)
		: mousex(x),
		  mousey(y),
		  cursormode(cursormode) {}

	float getX() const { return mousex; }
	float getY() const { return mousey; }
	gCursorMode getCursorMode() const { return cursormode; }

	G_EVENT_CLASS_TYPE(gMouseMovedEvent)
	G_EVENT_CLASS_CATEGORY(EVENTCATEGORY_MOUSE | EVENTCATEGORY_INPUT)
private:
	float mousex, mousey;
	gCursorMode cursormode;
};

class gMouseScrolledEvent : public gEvent {
public:
	gMouseScrolledEvent(const int offsetx, const int offsety)
		: offsetx(offsetx),
		  offsety(offsety) {}

	int getOffsetX() const { return offsetx; }
	int getOffsetY() const { return offsety; }


	G_EVENT_CLASS_TYPE(gMouseScrolledEvent)
	G_EVENT_CLASS_CATEGORY(EVENTCATEGORY_MOUSE | EVENTCATEGORY_INPUT)
private:
	int offsetx, offsety;
};

class gMouseButtonEvent : public gEvent {
public:
	int getMouseButton() const { return button; }
	int getX() const { return x; }
	int getY() const { return y; }

	G_EVENT_CLASS_CATEGORY(EVENTCATEGORY_MOUSE | EVENTCATEGORY_INPUT | EVENTCATEGORY_MOUSE_BUTTON)
protected:
	gMouseButtonEvent(const int button, const int x, const int y)
		: button(button), x(x), y(y) {}

	int button;
	int x, y;
};

class gMouseButtonPressedEvent : public gMouseButtonEvent {
public:
	gMouseButtonPressedEvent(const int button, const int x, const int y)
		: gMouseButtonEvent(button, x, y) {}

	G_EVENT_CLASS_TYPE(gMouseButtonPressedEvent)
};

class gMouseButtonReleasedEvent : public gMouseButtonEvent {
public:
	gMouseButtonReleasedEvent(const int button, const int x, const int y)
		: gMouseButtonEvent(button, x, y) {}

	G_EVENT_CLASS_TYPE(gMouseButtonReleasedEvent)
};

class gWindowResizeEvent : public gEvent {
public:
	gWindowResizeEvent(int width, int height)
		: width(width), height(height) {}

	int getWidth() const { return width; }
	int getHeight() const { return height; }

	G_EVENT_CLASS_TYPE(gWindowResizeEvent)
	G_EVENT_CLASS_CATEGORY(EVENTCATEGORY_APP)
private:
	int width, height;
};

class gWindowScaleChangedEvent : public gEvent {
public:
	gWindowScaleChangedEvent(int width, int height, float scalex, float scaley)
		: width(width), height(height), scalex(scalex), scaley(scaley) {}

	int getWidth() const { return width; }
	int getHeight() const { return height; }
	float getScaleX() const { return scalex; }
	float getScaleY() const { return scaley; }

	G_EVENT_CLASS_TYPE(gWindowScaleChangedEvent)
	G_EVENT_CLASS_CATEGORY(EVENTCATEGORY_APP)
private:
	int width, height;
	float scalex, scaley;
};

class gWindowFocusEvent : public gEvent {
public:
	gWindowFocusEvent() {}

	G_EVENT_CLASS_TYPE(gWindowFocusEvent)
	G_EVENT_CLASS_CATEGORY(EVENTCATEGORY_APP)
};

class gWindowLoseFocusEvent : public gEvent {
public:
	gWindowLoseFocusEvent() {}

	G_EVENT_CLASS_TYPE(gWindowLoseFocusEvent)
	G_EVENT_CLASS_CATEGORY(EVENTCATEGORY_APP)
};

class gWindowMouseEnterEvent : public gEvent {
public:
	gWindowMouseEnterEvent() {}

	G_EVENT_CLASS_TYPE(gWindowMouseEnterEvent)
	G_EVENT_CLASS_CATEGORY(EVENTCATEGORY_APP)
};

class gWindowMouseExitEvent : public gEvent {
public:
	gWindowMouseExitEvent() {}

	G_EVENT_CLASS_TYPE(gWindowMouseExitEvent)
	G_EVENT_CLASS_CATEGORY(EVENTCATEGORY_APP)
};

class gJoystickConnectEvent : public gEvent {
public:
	gJoystickConnectEvent(int joystickId, bool isgamepad)
		: joystickid(joystickId), isgamepad(isgamepad) {}

	int getJoystickId() const { return joystickid; }
	bool isGamepad() const { return isgamepad; }

	G_EVENT_CLASS_TYPE(gJoystickConnectEvent)
	G_EVENT_CLASS_CATEGORY(EVENTCATEGORY_INPUT | EVENTCATEGORY_JOYSTICK)
private:
	int joystickid;
	bool isgamepad;
};

class gJoystickDisconnectEvent : public gEvent {
public:
	gJoystickDisconnectEvent(int joystickId)
		: joystickid(joystickId) {}

	int getJoystickId() const { return joystickid; }

	G_EVENT_CLASS_TYPE(gJoystickDisconnectEvent)
	G_EVENT_CLASS_CATEGORY(EVENTCATEGORY_INPUT | EVENTCATEGORY_JOYSTICK)
private:
	int joystickid;
};

#if GLIST_ANDROID || GLIST_IOS
enum InputType {
	INPUTTYPE_UNKNOWN = 0,
	INPUTTYPE_FINGER = 1,
	INPUTTYPE_STYLUS = 2,
	INPUTTYPE_MOUSE = 3
};

enum ActionType {
	ACTIONTYPE_DOWN = 0,
	ACTIONTYPE_UP = 1,
	ACTIONTYPE_MOVE = 2,
	ACTIONTYPE_CANCEL = 3,
	ACTIONTYPE_OUTSIDE = 4,
	ACTIONTYPE_POINTER_DOWN = 5,
	ACTIONTYPE_POINTER_UP = 6,
	ACTIONTYPE_HOVER_MOVE = 7,
	ACTIONTYPE_SCROLL = 8,
	ACTIONTYPE_HOVER_ENTER = 9,
	ACTIONTYPE_HOVER_EXIT = 10
};

struct TouchInput {
	InputType type;
	int fingerid;
	int pointerindex;
	int x, y;
};

class gTouchEvent : public gEvent {
public:
	gTouchEvent(int inputCount, TouchInput* inputs, int actionIndex, ActionType action) : inputs(inputs), inputcount(inputCount), action(action), actionindex(actionIndex) {}

	int getInputCount() const { return inputcount; }
	TouchInput* getInputs() const { return inputs; }
	int getActionIndex() const { return actionindex; }
	ActionType getAction() const { return action; }

	G_EVENT_CLASS_TYPE(gTouchEvent)
	G_EVENT_CLASS_CATEGORY(EVENTCATEGORY_INPUT | EVENTCATEGORY_TOUCHSCREEN)
private:
	int inputcount;
	TouchInput* inputs;
	int actionindex;
	ActionType action;
};

enum DeviceOrientation {
	DEVICEORIENTATION_UNSPECIFIED = -1,
	DEVICEORIENTATION_LANDSCAPE = 0,
	DEVICEORIENTATION_PORTRAIT = 1,
	DEVICEORIENTATION_REVERSE_LANDSCAPE = 8,
	DEVICEORIENTATION_REVERSE_PORTRAIT = 9
};

class gDeviceOrientationChangedEvent : public gEvent {
public:
	gDeviceOrientationChangedEvent(DeviceOrientation orientation) : orientation(orientation) {}

	DeviceOrientation getOrientation() const { return orientation; }

	G_EVENT_CLASS_TYPE(gDeviceOrientationChangedEvent)
	G_EVENT_CLASS_CATEGORY(EVENTCATEGORY_APP)
private:
	DeviceOrientation orientation;
};

#endif

class gAppPauseEvent : public gEvent {
public:
	gAppPauseEvent() {}

	G_EVENT_CLASS_TYPE(gAppPauseEvent)
	G_EVENT_CLASS_CATEGORY(EVENTCATEGORY_APP)
private:
};

class gAppResumeEvent : public gEvent {
public:
	gAppResumeEvent() {}

	G_EVENT_CLASS_TYPE(gAppResumeEvent)
	G_EVENT_CLASS_CATEGORY(EVENTCATEGORY_APP)
private:
};

#endif /* GWINDOWEVENTS_H_ */
