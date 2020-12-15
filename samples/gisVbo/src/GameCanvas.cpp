/*
 * GameCanvas.cpp
 *
 *  Created on: May 6, 2020
 *      Author: noyan
 */


#include "GameCanvas.h"
#include <glm/glm.hpp>


GameCanvas::GameCanvas(gBaseApp *root) : gBaseCanvas(root) {
}

GameCanvas::~GameCanvas() {
}

void GameCanvas::setup() {
//	logi("setup");
	vertices[0] = -0.75f;
	vertices[1] = -0.5f;
	vertices[2] = 0.0f;
	vertices[3] = 0.25f;
	vertices[4] = -0.5f;
	vertices[5] = 0.0f;
	vertices[6] = -0.25f;
	vertices[7] = 0.5f;
	vertices[8] = 0.0f;
	vbo1.setVertexData(&vertices[0], 3, 3, 0x88E4);

	indices[0] = 0;
	indices[1] = 1;
	indices[2] = 2;
//	vbo.setIndexData(&indices[0], 3);

	verts2[0].position= glm::vec3(0.25f, -0.5f, 0.0f);
	verts2[1].position= glm::vec3(0.75f, 0.5f, 0.0f);
	verts2[2].position= glm::vec3(-0.25f, 0.5f, 0.0f);
	vbo2.setVertexData(&verts2[0], 3, 3);

	light = gLight(gLight::LIGHTTYPE_AMBIENT);
	light.setAmbientColor(255, 255, 255, 255);
}

void GameCanvas::update() {
	logi("gisVbo update");
}

void GameCanvas::draw() {
	logi("gisVbo draw");
	light.enable();
	setColor(255, 50, 0);
	vbo1.draw();
	setColor(0, 255, 50);
	vbo2.draw();
	light.disable();
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

