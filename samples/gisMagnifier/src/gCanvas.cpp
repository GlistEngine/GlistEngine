/*
 * gCanvas.cpp
 *
 * Created on: Aug 26, 2026
 * Authors: Bahar Kucukozer, Mehmet Sefa Ciftci
 */

#include "gCanvas.h"


gCanvas::gCanvas(gApp* root)
		: gBaseCanvas(root), root(root), magnifier(nullptr), magnifierx(0), magnifiery(0) {
}

gCanvas::~gCanvas() {
	delete magnifier;
}

void gCanvas::setup() {
	background.loadImage("images/magnifier_background.png");
	logo.loadImage("images/glistengine_logo.png");
	magnifierframe.loadImage("images/magnifier_frame.png");

	postprocessmanager.setDimensions(getWidth(), getHeight());

	magnifier = new gMagnifier(
			160.0f, // Radius in pixels
			2.0f,   // Zoom amount
			4.0f    // Edge softness
	);

	magnifierx = getWidth() / 2;
	magnifiery = getHeight() / 2;
	magnifier->setScreenSize(getWidth(), getHeight());
	magnifier->setMousePosition(magnifierx, magnifiery);
	postprocessmanager.addEffect(magnifier);
}

void gCanvas::update() {
}

void gCanvas::draw() {
	postprocessmanager.enable();

	background.draw(0, 0, getWidth(), getHeight());
	logo.draw(
			(getWidth() - logo.getWidth()) / 2,
			(getHeight() - logo.getHeight()) / 2
	);

	postprocessmanager.disable();

	// The lens center is located at (160, 160) in the frame image.
	magnifierframe.draw(magnifierx - 160, magnifiery - 160);
}

void gCanvas::keyPressed(int key) {
}

void gCanvas::keyReleased(int key) {
}

void gCanvas::charPressed(unsigned int codepoint) {
}

void gCanvas::mouseMoved(int x, int y) {
	magnifierx = x;
	magnifiery = y;
	if(magnifier != nullptr) {
		magnifier->setMousePosition(x, y);
	}
}

void gCanvas::mouseDragged(int x, int y, int button) {
	magnifierx = x;
	magnifiery = y;
	if(magnifier != nullptr) {
		magnifier->setMousePosition(x, y);
	}
}

void gCanvas::mousePressed(int x, int y, int button) {
}

void gCanvas::mouseReleased(int x, int y, int button) {
}

void gCanvas::mouseScrolled(int x, int y) {
}

void gCanvas::mouseEntered() {
}

void gCanvas::mouseExited() {
}

void gCanvas::windowResized(int w, int h) {
	if(magnifier != nullptr) {
		magnifier->setScreenSize(w, h);
	}
}

void gCanvas::deviceOrientationChanged(DeviceOrientation deviceorientation) {
}

void gCanvas::touchMoved(int x, int y, int fingerId) {
	magnifierx = x;
	magnifiery = y;
	if(magnifier != nullptr) {
		magnifier->setMousePosition(x, y);
	}
}

void gCanvas::touchPressed(int x, int y, int fingerId) {
	magnifierx = x;
	magnifiery = y;
	if(magnifier != nullptr) {
		magnifier->setMousePosition(x, y);
	}
}

void gCanvas::touchReleased(int x, int y, int fingerId) {
}

void gCanvas::pause() {
}

void gCanvas::resume() {
}

void gCanvas::showNotify() {
}

void gCanvas::hideNotify() {
}
