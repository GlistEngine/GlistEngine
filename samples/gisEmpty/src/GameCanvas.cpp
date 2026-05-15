/*
* gCanvas.cpp
*
*  Created on: May 6, 2020
*      Author: Noyan Culum
*/

/*
 * GameCanvas.cpp
 *
 *  Created on: 15 May 2026
 *      Author: noyan
 */

#include <GameCanvas.h>

GameCanvas::GameCanvas(gBaseApp *root) : gBaseCanvas(root) {
}

GameCanvas::~GameCanvas() {
}

void GameCanvas::setup() {
//	logi("setup");
}

void GameCanvas::update() {
	logi("gisEmpty update");
}

void GameCanvas::draw() {
	logi("gisEmpty draw");
}

void GameCanvas::keyPressed(int key) {
//	logi("GC", "keyPressed:" + gToStr(key));
}

void GameCanvas::keyReleased(int key) {
//	logi("GC", "keyReleased:" + gToStr(key));
}

void GameCanvas::mouseMoved(int x, int y) {
//	logi("mouseMoved x:" + gToStr(x) + ", y:" + gToStr(y));
}

void GameCanvas::mouseDragged(int x, int y, int button) {
//	logi("mouseDragged x:" + gToStr(x) + ", y:" + gToStr(y) + ", b:" + gToStr(button));
}

void GameCanvas::mousePressed(int x, int y, int button) {
}

void GameCanvas::mouseReleased(int x, int y, int button) {
//	logi("GC", "mouseReleased, button:" + gToStr(button));
}

void GameCanvas::mouseEntered() {
}

void GameCanvas::mouseExited() {
}

void GameCanvas::showNotify() {

}

void GameCanvas::hideNotify() {

}



#include "gCanvas.h"


gCanvas::gCanvas(gApp* root) : gBaseCanvas(root) {
	this->root = root;
}

gCanvas::~gCanvas() {
}

void gCanvas::setup() {
	logo.loadImage("glistengine_logo.png");
}

void gCanvas::update() {
}

void gCanvas::draw() {
	logo.draw((getWidth() - logo.getWidth()) / 2, (getHeight() - logo.getHeight()) / 2);
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

