/*
 * gApp.cpp
 *
 * Created on: May 6, 2020
 * Original Author: Noyan Culum
 *
 * gisSpline Sample
 * Created on: Aug 21, 2026
 * Sample Authors: Bahar Kucukozer, Mehmet Sefa Ciftci
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
