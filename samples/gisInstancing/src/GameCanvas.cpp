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

void GameCanvas::update() {
}

void GameCanvas::draw() {
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

