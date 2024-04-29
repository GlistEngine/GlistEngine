#pragma once

#include "gUtils.h"
#include "gEvent.h"
#include "gWindowEvents.h"
#include <unordered_map>
#include <list>
#include <functional>

#ifdef DEBUG
#include <assert.h>

#define BIND_GUI_EVENT(type, fn) 								\
assert(ctx);                                      					\
ctx->push(type::getStaticType(), [this](gEvent& event) -> bool { 	\
	if(auto* event2 = dynamic_cast<type*>(&event)) {   				\
		return this->fn(*event2); 									\
	} 																\
	return false; 													\
})
#else
#define BIND_GUI_EVENT(type, fn) 								\
ctx->push(type::getStaticType(), [this](gEvent& event) -> bool { 	\
	if(auto* event2 = dynamic_cast<type*>(&event)) {   				\
		return this->fn(*event2); 									\
	} 																\
	return false; 													\
})
#endif


class gGUIRendererContext {
public:
	void push(EventType type, std::function<bool(gEvent&)> fn);

	void begin();

	void end();

	void dispatch(gEvent& event);
private:
	std::unordered_map<EventType, std::list<std::function<bool(gEvent&)>>> map;
	bool allow = false;
};
