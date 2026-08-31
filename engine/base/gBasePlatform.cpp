/*
 * gBasePlatform.cpp
 *
 *  Created on: Aug 14, 2026
 */

#include "gBasePlatform.h"

gBasePlatform* gBasePlatform::current = nullptr;


gBasePlatform::gBasePlatform() {
	current = this;
}

gBasePlatform::~gBasePlatform() {
	if (current == this) current = nullptr;
}

gBasePlatform* gBasePlatform::getCurrent() {
	return current;
}
