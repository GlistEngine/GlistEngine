/*
 * gEventHook.cpp
 *
 *  Created on: Feb 6, 2026
 *      Author: Metehan Gezer
 */

#include "gEventHook.h"

std::list<gEventHook*> gEventHook::hooks;

gEventHook::gEventHook() {
	hooks.push_back(this);
}

gEventHook::~gEventHook() {
	hooks.remove_if([this](gEventHook* ptr) {
		return ptr == this;
	});
}
