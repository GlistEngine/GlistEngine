/*
* gCanvas.cpp
*
*  Created on: May 6, 2020
*      Author: Noyan Culum
*/


#include "gCanvas.h"


gCanvas::gCanvas(gApp* root) : gBaseCanvas(root) {
	this->root = root;
}

gCanvas::~gCanvas() {
}

void gCanvas::setup() {
	// Load the logo image from assets
	logo.loadImage("gamelab-istanbul_logo_128.png");

	// Initial x and y values to draw the image (screen center)
	x = (getWidth() - logo.getWidth()) / 2;
	y = (getHeight() - logo.getHeight()) / 2;

	// Delta values for image animation
	dx = 1;
	dy = 1;
}

void gCanvas::update() {
	// Increase x until reaching left or right side of the screen
	x += dx;
	if (x <= 0 || x >= getWidth() - logo.getWidth()) dx = -dx;

	// Increase y until reaching top or bottom side of the screen
	y+= dy;
	if (y <= 0 || y >= getHeight() - logo.getHeight()) dy = -dy;
}

void gCanvas::draw() {
	// Clear the background
	clearColor(0, 0, 30);

	// Set drawing color
	setColor(255, 255, 255);

	// Draw the logo
	logo.draw(x, y);
}

void gCanvas::keyPressed(int key) {
//	gLogi("gCanvas") << "keyPressed:" << key;
}

void gCanvas::keyReleased(int key) {
//	gLogi("gCanvas") << "keyReleased:" << key;
}

void gCanvas::charPressed(unsigned int codepoint) {
//	gLogi("gCanvas") << "charPressed:" << gCodepointToStr(codepoint);
}

void gCanvas::mouseMoved(int x, int y) {
//	gLogi("gCanvas") << "mouseMoved" << ", x:" << x << ", y:" << y;
}

void gCanvas::mouseDragged(int x, int y, int button) {
//	gLogi("gCanvas") << "mouseDragged" << ", x:" << x << ", y:" << y << ", b:" << button;
}

void gCanvas::mousePressed(int x, int y, int button) {
//	gLogi("gCanvas") << "mousePressed" << ", x:" << x << ", y:" << y << ", b:" << button;
}

void gCanvas::mouseReleased(int x, int y, int button) {
//	gLogi("gCanvas") << "mouseReleased" << ", button:" << button;
}

void gCanvas::mouseScrolled(int x, int y) {
//	gLogi("gCanvas") << "mouseScrolled" << ", x:" << x << ", y:" << y;
}

void gCanvas::mouseEntered() {

}

void gCanvas::mouseExited() {

}

void gCanvas::windowResized(int w, int h) {

}

void gCanvas::showNotify() {

}

void gCanvas::hideNotify() {

}

