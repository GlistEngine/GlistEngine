/*
 * gGUIDivider.cpp
 *
 *  Created on: Sep 29, 2021
 *      Author: noyan
 */

#include "gGUIDivider.h"


gGUIDivider::gGUIDivider() {
	direction = DIR_HORIZONTAL;
}

gGUIDivider::~gGUIDivider() {
}

void gGUIDivider::setDirection(int direction) {
	this->direction = direction;
}

int gGUIDivider::getDirection() {
	return direction;
}

