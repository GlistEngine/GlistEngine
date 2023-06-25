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

	G_EVENT_CLASS_CATEGORY(EventCategory::Keyboard | EventCategory::Input)
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

	G_EVENT_CLASS_TYPE(CharTyped)
	G_EVENT_CLASS_CATEGORY(EventCategory::Keyboard | EventCategory::Input)
  private:
	unsigned int keycode;
};

class gKeyPressedEvent : public gKeyEvent {
  public:
	gKeyPressedEvent(const int keycode, bool isrepeat = false)
		: gKeyEvent(keycode),
		  isrepeat(isrepeat) {}

	bool isRepeat() const { return isrepeat; }

	G_EVENT_CLASS_TYPE(KeyPressed)
  private:
	bool isrepeat;
};

class gKeyReleasedEvent : public gKeyEvent {
  public:
	gKeyReleasedEvent(const int keycode)
		: gKeyEvent(keycode) {}

	G_EVENT_CLASS_TYPE(KeyReleased)
};


class gMouseMovedEvent : public gEvent {
  public:
	gMouseMovedEvent(const int x, const int y)
		: mousex(x),
		  mousey(y) {}

	int getX() const { return mousex; }
	int getY() const { return mousey; }

	G_EVENT_CLASS_TYPE(MouseMoved)
	G_EVENT_CLASS_CATEGORY(EventCategory::Mouse | EventCategory::Input)
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


	G_EVENT_CLASS_TYPE(MouseScrolled)
	G_EVENT_CLASS_CATEGORY(EventCategory::Mouse | EventCategory::Input)
  private:
	int offsetx, offsety;
};

class gMouseButtonEvent : public gEvent {
  public:
	int getMouseButton() const { return button; }
	int getX() const { return x; }
	int getY() const { return y; }

	G_EVENT_CLASS_CATEGORY(EventCategory::Mouse | EventCategory::Input | EventCategory::MouseButton)
  protected:
	gMouseButtonEvent(const int button, const int x, const int y)
		: button(button), x(x), y(y) {}

	int button;
	int x;
	int y;
};

class gMouseButtonPressedEvent : public gMouseButtonEvent {
  public:
	gMouseButtonPressedEvent(const int button, const int x, const int y)
		: gMouseButtonEvent(button, x, y) {}

	G_EVENT_CLASS_TYPE(MouseButtonPressed)
};

class gMouseButtonReleasedEvent : public gMouseButtonEvent {
  public:
	gMouseButtonReleasedEvent(const int button, const int x, const int y)
		: gMouseButtonEvent(button, x, y) {}

	G_EVENT_CLASS_TYPE(MouseButtonReleased)
};

class gWindowResizeEvent : public gEvent {
  public:
	gWindowResizeEvent(int width, int height)
		: width(width), height(height) {}

	const int getWidth() { return width; }
	const int getHeight() { return height; }

	G_EVENT_CLASS_TYPE(WindowResize)
	G_EVENT_CLASS_CATEGORY(EventCategory::App)
  private:
	int width, height;
};

class gWindowFocusEvent : public gEvent {
  public:
	gWindowFocusEvent() {}

	G_EVENT_CLASS_TYPE(WindowFocus)
	G_EVENT_CLASS_CATEGORY(EventCategory::App)
};

class gWindowLoseFocusEvent : public gEvent {
  public:
	gWindowLoseFocusEvent() {}

	G_EVENT_CLASS_TYPE(WindowLoseFocus)
	G_EVENT_CLASS_CATEGORY(EventCategory::App)
};

class gWindowMouseEnterEvent : public gEvent {
  public:
	gWindowMouseEnterEvent() {}

	G_EVENT_CLASS_TYPE(WindowMouseEnter)
	G_EVENT_CLASS_CATEGORY(EventCategory::App)
};

class gWindowMouseExitEvent : public gEvent {
  public:
	gWindowMouseExitEvent() {}

	G_EVENT_CLASS_TYPE(WindowMouseExit)
	G_EVENT_CLASS_CATEGORY(EventCategory::App)
};

#endif /* GWINDOWEVENTS_H_ */