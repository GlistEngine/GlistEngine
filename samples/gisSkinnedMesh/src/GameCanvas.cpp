/*
 * GameCanvas.cpp
 *
 *  Created on: May 6, 2020
 *      Author: noyan
 */


#include "GameCanvas.h"


GameCanvas::GameCanvas(gBaseApp *root) : gBaseCanvas(root) {
}

GameCanvas::~GameCanvas() {
}

void GameCanvas::setup() {
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

void GameCanvas::update() {
	model.nextAnimationFrame();
}

void GameCanvas::draw() {
	setColor(255, 255, 255);
	camera.begin();
	enableDepthTest();
//	light.enable();
	model.draw();
//	light.disable();
	disableDepthTest();
	camera.end();
}

void GameCanvas::keyPressed(int key) {
//	logi("keyPressed:" + gToStr(key));
}

void GameCanvas::keyReleased(int key) {
//	logi("keyReleased:" + gToStr(key));
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
//	logi("mouseReleased, button:" + gToStr(button));
}

void GameCanvas::mouseEntered() {
}

void GameCanvas::mouseExited() {
}

void GameCanvas::showNotify() {
}

void GameCanvas::hideNotify() {
}

