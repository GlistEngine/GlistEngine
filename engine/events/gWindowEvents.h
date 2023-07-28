/*
* gEvent.h
*
*  Created on: June 24, 2023
*      Author: Metehan Gezer
*/

#ifndef GWINDOWEVENTS_H_
#define GWINDOWEVENTS_H_

#include "gEvent.h"

class gKeyEvent : public gEvent {
public:
	int getKeyCode() const { return keycode; }

	G_EVENT_CLASS_CATEGORY(EventCategoryKeyboard | EventCategoryInput)
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

	G_EVENT_CLASS_TYPE(EventTypeCharTyped)
	G_EVENT_CLASS_CATEGORY(EventCategoryKeyboard | EventCategoryInput)
private:
	unsigned int keycode;
};

class gKeyPressedEvent : public gKeyEvent {
public:
	gKeyPressedEvent(const int keycode, bool isrepeat = false)
		: gKeyEvent(keycode),
		  isrepeat(isrepeat) {}

	bool isRepeat() const { return isrepeat; }

	G_EVENT_CLASS_TYPE(EventTypeKeyPressed)
private:
	bool isrepeat;
};

class gKeyReleasedEvent : public gKeyEvent {
public:
	gKeyReleasedEvent(const int keycode)
		: gKeyEvent(keycode) {}

	G_EVENT_CLASS_TYPE(EventTypeKeyReleased)
};


class gMouseMovedEvent : public gEvent {
public:
	gMouseMovedEvent(const int x, const int y)
		: mousex(x),
		  mousey(y) {}

	int getX() const { return mousex; }
	int getY() const { return mousey; }

	G_EVENT_CLASS_TYPE(EventTypeMouseMoved)
	G_EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)
private:
	int mousex, mousey;
};

class gMouseScrolledEvent : public gEvent {
public:
	gMouseScrolledEvent(const int offsetx, const int offsety)
		: offsetx(offsetx),
		  offsety(offsety) {}

	int getOffsetX() const { return offsetx; }
	int getOffsetY() const { return offsety; }


	G_EVENT_CLASS_TYPE(EventTypeMouseScrolled)
	G_EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)
private:
	int offsetx, offsety;
};

class gMouseButtonEvent : public gEvent {
public:
	int getMouseButton() const { return button; }
	int getX() const { return x; }
	int getY() const { return y; }

	G_EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput | EventCategoryMouseButton)
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

	G_EVENT_CLASS_TYPE(EventTypeMouseButtonPressed)
};

class gMouseButtonReleasedEvent : public gMouseButtonEvent {
public:
	gMouseButtonReleasedEvent(const int button, const int x, const int y)
		: gMouseButtonEvent(button, x, y) {}

	G_EVENT_CLASS_TYPE(EventTypeMouseButtonReleased)
};

class gWindowResizeEvent : public gEvent {
public:
	gWindowResizeEvent(int width, int height)
		: width(width), height(height) {}

	int getWidth() const { return width; }
	int getHeight() const { return height; }

	G_EVENT_CLASS_TYPE(EventTypeWindowResize)
	G_EVENT_CLASS_CATEGORY(EventCategoryApp)
private:
	int width, height;
};

class gWindowFocusEvent : public gEvent {
public:
	gWindowFocusEvent() {}

	G_EVENT_CLASS_TYPE(EventTypeWindowFocus)
	G_EVENT_CLASS_CATEGORY(EventCategoryApp)
};

class gWindowLoseFocusEvent : public gEvent {
public:
	gWindowLoseFocusEvent() {}

	G_EVENT_CLASS_TYPE(EventTypeWindowLoseFocus)
	G_EVENT_CLASS_CATEGORY(EventCategoryApp)
};

class gWindowMouseEnterEvent : public gEvent {
public:
	gWindowMouseEnterEvent() {}

	G_EVENT_CLASS_TYPE(EventTypeWindowMouseEnter)
	G_EVENT_CLASS_CATEGORY(EventCategoryApp)
};

class gWindowMouseExitEvent : public gEvent {
public:
	gWindowMouseExitEvent() {}

	G_EVENT_CLASS_TYPE(EventTypeWindowMouseExit)
	G_EVENT_CLASS_CATEGORY(EventCategoryApp)
};

class gJoystickConnectEvent : public gEvent {
public:
	gJoystickConnectEvent(int joystickId, bool isgamepad)
		: joystickid(joystickId), isgamepad(isgamepad) {}

	int getJoystickId() const { return joystickid; }
	bool isGamepad() const { return isgamepad; }

	G_EVENT_CLASS_TYPE(EventTypeJoystickConnect)
	G_EVENT_CLASS_CATEGORY(EventCategoryInput | EventCategoryJoystick)
private:
	int joystickid;
	bool isgamepad;
};


class gJoystickDisconnectEvent : public gEvent {
public:
	gJoystickDisconnectEvent(int joystickId)
		: joystickid(joystickId) {}

	int getJoystickId() const { return joystickid; }

	G_EVENT_CLASS_TYPE(EventTypeJoystickDisconnect)
	G_EVENT_CLASS_CATEGORY(EventCategoryInput | EventCategoryJoystick)
private:
	int joystickid;
};

class gReallocateRenderDataEvent : public gEvent {
public:
    gReallocateRenderDataEvent() {}

    G_EVENT_CLASS_TYPE(EventTypeReallocateRenderData)
    G_EVENT_CLASS_CATEGORY(EventCategoryRenderer | EventCategoryApp)
private:
};


#ifdef ANDROID
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

	G_EVENT_CLASS_TYPE(EventTypeTouch)
	G_EVENT_CLASS_CATEGORY(EventCategoryInput | EventCategoryTouchscreen)
private:
	int inputcount;
	TouchInput* inputs;
	int actionindex;
	ActionType action;
};

class gAppPauseEvent : public gEvent {
public:
	gAppPauseEvent() {}

	G_EVENT_CLASS_TYPE(EventTypeAppPause)
	G_EVENT_CLASS_CATEGORY(EventCategoryApp)
private:
};

class gAppResumeEvent : public gEvent {
public:
	gAppResumeEvent() {}

	G_EVENT_CLASS_TYPE(EventTypeAppResume)
	G_EVENT_CLASS_CATEGORY(EventCategoryApp)
private:
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

	G_EVENT_CLASS_TYPE(EventTypeDeviceOrientationChanged)
	G_EVENT_CLASS_CATEGORY(EventCategoryApp)
private:
	DeviceOrientation orientation;
};

#endif /* ANDROID */

#endif /* GWINDOWEVENTS_H_ */
