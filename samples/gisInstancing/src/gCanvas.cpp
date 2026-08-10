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
	root->setTargetFramerate(100000);
	font.loadFont("FreeSansBold.ttf", 16);

	// Position the camera so the instance grid is visible.
	camera.setPosition(0.0f, 8.0f, 18.0f);
	camera.lookAt(glm::vec3(0.0f, 0.0f, -12.0f));

	// Light the instanced boxes.
	light = gLight(gLight::LIGHTTYPE_POINT);
	light.setPosition(0.0f, 10.0f, 5.0f);
	light.setAmbientColor(20, 20, 20);
	light.setDiffuseColor(120, 120, 120);
	light.setSpecularColor(20, 20, 20);

	// Store one transform for each box instance.
	instanceTransformations.reserve(400);

	for (int z = 0; z < 20; ++z) {
		for (int x = 0; x < 20; ++x) {
			glm::mat4 transformation(1.0f);

			// Place the boxes in a 20x20 grid.
			transformation = glm::translate(
				transformation,
				glm::vec3((x - 10) * 4.0f, 0.0f, -z * 4.0f)
			);
			instanceTransformations.push_back(transformation);
		}
	}
}

void gCanvas::update() {
}

void gCanvas::draw() {
    enableDepthTest();

    camera.begin();
    light.enable();

    // Draw all boxes using one instanced draw call.
    box.drawInstanced(instanceTransformations);

    light.disable();
    camera.end();

    disableDepthTest();

    // Draw FPS
    font.drawText("FPS: " + gToStr(root->getFramerate()), 30, 30);
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

