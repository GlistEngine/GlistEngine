/*
 * gBasePlugin.cpp
 *
 *  Created on: May 6, 2020
 *      Author: noyan
 */

#include "gBasePlugin.h"

std::deque<gBasePlugin*> gBasePlugin::usedplugins;


gBasePlugin::gBasePlugin() {
	usedplugins.push_back(this);
}

gBasePlugin::~gBasePlugin() {
}

void gBasePlugin::setup() {

}

void gBasePlugin::update() {

}

void gBasePlugin::charPressed(char key) {

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

void gBasePlugin::mouseEntered() {

}

void gBasePlugin::mouseExited() {

}

