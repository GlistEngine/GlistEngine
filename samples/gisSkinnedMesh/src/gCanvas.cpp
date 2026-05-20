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
		model.loadModel("astroboy/astroBoy_walk.dae");
	model.setAnimationFrameNum(model.getAnimationFrameNum() * 2);
	model.makeVertexAnimated();
	model.scale(6);

	camera.setPosition(-0.8, 1.3, 0.8);
	camera.rotate(-44.85, 0, 1, 0);
	camera.rotate(-44.5, 1, 0, 0);

	light = gLight(gLight::LIGHTTYPE_POINT);
	light.setPosition(camera.getPosition());
//	light.rotate(75, 1, 0, 0);
	light.setAmbientColor(128, 128, 128);
	light.setDiffuseColor(255, 255, 255);
	light.setSpecularColor(16, 16, 16);
}

void gCanvas::update() {
	model.nextAnimationFrame();
}

void gCanvas::draw() {
	setColor(255, 255, 255);
	camera.begin();
	enableDepthTest();
//	light.enable();
	model.draw();
//	light.disable();
	disableDepthTest();
	camera.end();
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

