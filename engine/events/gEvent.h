/*
* gEvent.h
*
*  Created on: June 24, 2023
*      Author: Metehan Gezer
*/

#ifndef GEVENT_H_
#define GEVENT_H_

enum EventType {
	CharTyped,
	KeyPressed,
	KeyReleased,
	MouseMoved,
	MouseScrolled,
	MouseButtonPressed,
	MouseButtonReleased,
	WindowResize,
	WindowFocus,
	WindowLoseFocus,
	WindowMouseEnter,
	WindowMouseExit,
#ifdef ANDROID
	AppPause,
	AppRestart,
	AppStop,
	Touch
#endif
};

#define BIT(x) (1 << x)
enum EventCategory {
	App = BIT(0),
	Input = BIT(1),
	Keyboard = BIT(2),
	Mouse = BIT(3),
	MouseButton = BIT(4)
};
#undef BIT

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

using EventHandlerFn = std::function<void(gEvent&)>;

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
