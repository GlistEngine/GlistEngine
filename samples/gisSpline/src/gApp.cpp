/*
 * gApp.cpp
 *
 * Created on: Aug 21, 2026
 * Authors: Bahar Kucukozer, Mehmet Sefa Ciftci
 */

#include "gApp.h"
#include "gCanvas.h"


gApp::gApp() {
}

gApp::gApp(int argc, char** argv) : gBaseApp(argc, argv) {
}

gApp::~gApp() {
}

void gApp::setup() {
	gCanvas* cnv = new gCanvas(this);
	appmanager->setCurrentCanvas(cnv);
}

void gApp::update() {
}
