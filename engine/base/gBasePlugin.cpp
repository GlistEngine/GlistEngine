/*
 * gBasePlugin.cpp
 *
 *  Created on: May 6, 2020
 *      Author: noyan
 */

#include "gBasePlugin.h"

std::deque<gBasePlugin*> gBasePlugin::usedplugins;


gBasePlugin::gBasePlugin(gBaseApp *root) {
	this->root = root;
	usedplugins.push_back(this);
}

gBasePlugin::~gBasePlugin() {
}

void gBasePlugin::setup() {

}

void gBasePlugin::update() {

}
