/*
* gEvent.h
*
*  Created on: June 24, 2023
*      Author: Metehan Gezer
*/

#ifndef GEVENT_H_
#define GEVENT_H_

#include <functional>

#define BIT(x) (1 << x)
enum EventCategory {
	EVENTCATEGORY_APP = BIT(0),
	EVENTCATEGORY_INPUT = BIT(1),
	EVENTCATEGORY_KEYBOARD = BIT(2),
	EVENTCATEGORY_MOUSE = BIT(3),
	EVENTCATEGORY_MOUSE_BUTTON = BIT(4),
	EVENTCATEGORY_JOYSTICK = BIT(5),
	EVENTCATEGORY_TOUCHSCREEN = BIT(6),
	EVENTCATEGORY_RENDERER = BIT(7),
	EVENTCATEGORY_CUSTOM = BIT(8),
};
#undef BIT

/*
 * Events are more future-proof, in case anything new is added to an event
 * developers doesn't have to update their function parameters.
 */

class gEvent {
public:
	virtual ~gEvent() = default;

	bool ishandled = false;

	virtual const char* getEventName() const = 0;
	virtual std::size_t getEventType() const = 0;
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
			event.ishandled |= func(*static_cast<T*>(&event));
			return true;
		}
		return false;
	}

private:
	gEvent& event;
};

#define G_EVENT_CLASS_TYPE(type)                                                \
  static size_t getStaticType() { return typeid(type).hash_code(); }                \
  virtual size_t getEventType() const override { return getStaticType(); } \
  virtual const char* getEventName() const override { return #type; }

#define G_EVENT_CLASS_CATEGORY(category) \
  virtual int getCategoryFlags() const override { return category; }


// This macro can be used instead of std::bind function call.
// G_BIND_FUNCTION can be used to bind class methods. While G_BIND_GLOBAL_FUNCTION can be used to bind global functions without a class.
#define G_BIND_FUNCTION(fn) [this](auto&&... args) -> decltype(auto) { return this->fn(std::forward<decltype(args)>(args)...); }
#define G_BIND_GLOBAL_FUNCTION(fn) [](auto&&... args) -> decltype(auto) { return fn(std::forward<decltype(args)>(args)...); }

#endif /* GEVENT_H_ */
