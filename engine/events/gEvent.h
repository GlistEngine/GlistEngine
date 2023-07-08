/*
* gEvent.h
*
*  Created on: June 24, 2023
*      Author: Metehan Gezer
*/

#ifndef GEVENT_H_
#define GEVENT_H_

#include <functional>

// todo documentation
enum EventType {
	EventTypeCharTyped,
	EventTypeKeyPressed,
	EventTypeKeyReleased,
	EventTypeMouseMoved,
	EventTypeMouseScrolled,
	EventTypeMouseButtonPressed,
	EventTypeMouseButtonReleased,
	EventTypeWindowResize,
	EventTypeWindowFocus,
	EventTypeWindowLoseFocus,
	EventTypeWindowMouseEnter,
	EventTypeWindowMouseExit,
	EventTypeJoystickConnect,
	EventTypeJoystickDisconnect,
	EventTypeReallocateRenderData,
#ifdef ANDROID
	EventTypeAppPause,
	EventTypeAppRestart,
	EventTypeAppStop,
	EventTypeTouch,
#endif
};

#define BIT(x) (1 << x)
enum EventCategory {
	EventCategoryApp = BIT(0),
	EventCategoryInput = BIT(1),
	EventCategoryKeyboard = BIT(2),
	EventCategoryMouse = BIT(3),
	EventCategoryMouseButton = BIT(4),
	EventCategoryJoystick = BIT(5),
	EventCategoryTouchscreen = BIT(6),
	EventCategoryRenderer = BIT(7),
};
#undef BIT

/*
 * Events are more future proof, in case anything new is added to an event
 * developers doesn't have to update their function parameters.
 */

class gEvent {
  public:
	virtual ~gEvent() = default;

	bool ishandled = false;

	virtual const char* getEventName() const = 0;
	virtual EventType getEventType() const = 0;
	virtual int getCategoryFlags() const = 0;

	bool isInCategory(EventCategory category) {
		return getCategoryFlags() & category;
	}
};

typedef std::function<void(gEvent&)> EventHandlerFn;

/**
 * Example usage:
 *
 * \code{.cpp}
 * void gCanvas::onEvent(gEvent& event) {
 *    gEventDispatcher dispatcher(event);
 *    dispatcher.dispatch<gSomethingEvent>(G_BIND_FUNCTION(onSomethingEvent)); // to bind functions inside the current class
 *    dispatcher.dispatch<gSomethingEvent>(G_BIND_GLOBAL_FUNCTION(onSomethingEvent)); // to bind global functions without a class
 * }
 *
 * bool gCanvas::onSomethingEvent(gSomethingEvent& event) {
 *    gLogi("gCanvas") << "My something event is called!";
 *    // Return true if event is handled, meaning that no other event function will be called.
 *    return false;
 * }
 * \endcode
 */
class gEventDispatcher {
  public:
	gEventDispatcher(gEvent& event)
		: event(event) {
	}

	// F will be deduced by the compiler
	template<typename T, typename F>
	bool dispatch(const F& func) {
		if (event.getEventType() == T::getStaticType()) {
			event.ishandled |= func(static_cast<T&>(event));
			return true;
		}
		return false;
	}

  private:
	gEvent& event;
};

#define G_EVENT_CLASS_TYPE(type)                                                \
  static EventType getStaticType() { return EventType::type; }                \
  virtual EventType getEventType() const override { return getStaticType(); } \
  virtual const char* getEventName() const override { return #type; }

#define G_EVENT_CLASS_CATEGORY(category) \
  virtual int getCategoryFlags() const override { return category; }
#define G_BIND_FUNCTION(fn) [this](auto&&... args) -> decltype(auto) { return this->fn(std::forward<decltype(args)>(args)...); }
#define G_BIND_GLOBAL_FUNCTION(fn) [](auto&&... args) -> decltype(auto) { return fn(std::forward<decltype(args)>(args)...); }

#endif /* GEVENT_H_ */
