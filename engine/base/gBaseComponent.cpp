/*
 * gBaseComponent.cpp
 *
 *  Created on: Jan 28, 2023
 *      Author: noyan
 */

#include "gBaseComponent.h"

std::deque<gBaseComponent*> gBaseComponent::usedcomponents;


gBaseComponent::gBaseComponent() {
	usedcomponents.push_back(this);
}

gBaseComponent::~gBaseComponent() {
}

void gBaseComponent::setup() {

}

void gBaseComponent::update() {

}

