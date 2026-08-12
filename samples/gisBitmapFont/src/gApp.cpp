#include "gApp.h"
#include "gCanvas.h"

gApp::gApp(int argc, char** argv) : gBaseApp(argc, argv) {
}

void gApp::setup() {
	appmanager->setCurrentCanvas(new gCanvas(this));
}
