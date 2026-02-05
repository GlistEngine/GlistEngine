/*
 * gEventHook.h
 *
 *  Created on: Feb 6, 2026
 *      Author: Metehan Gezer
 */

#ifndef ENGINE_EVENTS_GEVENTHOOK_H_
#define ENGINE_EVENTS_GEVENTHOOK_H_

#include "gEvent.h"
#include <list>

class gEventHook {
public:
	gEventHook();
	virtual ~gEventHook();

	virtual void onEvent(gEvent& event) = 0;

	static std::list<gEventHook*> hooks;
};

#endif /* ENGINE_EVENTS_GEVENTHOOK_H_ */
