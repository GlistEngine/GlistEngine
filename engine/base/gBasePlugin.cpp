/*
 * gBasePlugin.cpp
 *
 *  Created on: May 6, 2020
 *      Author: noyan
 */

#include "gBasePlugin.h"

std::list<gBasePlugin*> gBasePlugin::usedplugins;


gBasePlugin::gBasePlugin() {
	usedplugins.push_back(this);
}

gBasePlugin::~gBasePlugin() {
	// This has O(n) complexity.
	usedplugins.remove_if([this](gBasePlugin* ptr) {
		return ptr == this;
	});
}

void gBasePlugin::setup() {

}

void gBasePlugin::update() {

}

void gBasePlugin::charPressed(unsigned int key) {

}

void gBasePlugin::keyPressed(int key) {

}

void gBasePlugin::keyReleased(int key) {

}

void gBasePlugin::mouseMoved(int x, int y ) {

}

void gBasePlugin::mouseDragged(int x, int y, int button) {

}

void gBasePlugin::mousePressed(int x, int y, int button) {

}

void gBasePlugin::mouseReleased(int x, int y, int button) {

}

void gBasePlugin::mouseScrolled(int x, int y) {

}

void gBasePlugin::mouseEntered() {

}

void gBasePlugin::mouseExited() {

}

void gBasePlugin::onEvent(gEvent& event) {

}


