/*
 * gBaseComponent.cpp
 *
 *  Created on: Jan 28, 2023
 *      Author: noyan
 */

#include "gBaseComponent.h"

std::list<gBaseComponent*> gBaseComponent::usedcomponents;


gBaseComponent::gBaseComponent() {
	usedcomponents.push_back(this);
}

gBaseComponent::~gBaseComponent() {
	// This has O(n) complexity.
	usedcomponents.remove_if([this](gBaseComponent* ptr) {
		return ptr == this;
	});
}

void gBaseComponent::setup() {

}

void gBaseComponent::update() {

}

