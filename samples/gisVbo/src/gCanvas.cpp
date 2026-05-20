/*
* gCanvas.cpp
*
*  Created on: May 6, 2020
*      Author: Noyan Culum
*/


#include "gCanvas.h"
#include <glm/glm.hpp>


gCanvas::gCanvas(gApp* root) : gBaseCanvas(root) {
	this->root = root;
}

gCanvas::~gCanvas() {
}

void gCanvas::setup() {
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

void gCanvas::update() {
	logi("gisVbo update");
}

void gCanvas::draw() {
	logi("gisVbo draw");
	light.enable();
	setColor(255, 50, 0);
	vbo1.draw();
	setColor(0, 255, 50);
	vbo2.draw();
	light.disable();
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

