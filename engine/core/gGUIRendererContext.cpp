#include "gGUIRendererContext.h"

gGUIRendererContext* ctx = nullptr;

void gGUIRendererContext::push(EventType type, std::function<bool(gEvent&)> fn) {
#ifdef DEBUG
	assert(allow); // You should not be pushing events in this stage!
#endif
	map[type].push_back(std::move(fn));
}


void gGUIRendererContext::begin() {
	map.clear();
	allow = true;
}

void gGUIRendererContext::end() {
	allow = false;
}


void gGUIRendererContext::dispatch(gEvent& event) {
	if (event.ishandled) {
		return;
	}

	auto list = map[event.getEventType()];
	for (auto it = list.rbegin(); it != list.rend(); it++) {
		if ((*it)(event)) {
			event.ishandled = true;
			return;
		}
	}
}
