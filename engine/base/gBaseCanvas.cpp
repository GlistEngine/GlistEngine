/*
 * gBaseCanvas.cpp
 *
 *  Created on: May 6, 2020
 *      Author: noyan
 */

#include "gBaseCanvas.h"

const int gBaseCanvas::MOUSEBUTTON_LEFT = 2;
const int gBaseCanvas::MOUSEBUTTON_RIGHT = 4;
const int gBaseCanvas::MOUSEBUTTON_MIDDLE = 8;


gBaseCanvas::gBaseCanvas(gBaseApp *root) {
	this->root = root;
}

gBaseCanvas::~gBaseCanvas() {
}

void gBaseCanvas::setup() {

}

void gBaseCanvas::update() {

}

void gBaseCanvas::draw() {

}

void gBaseCanvas::keyPressed(int key) {

}

void gBaseCanvas::keyReleased(int key) {

}

void gBaseCanvas::charPressed(unsigned int codepoint) {

}

void gBaseCanvas::mouseMoved(int x, int y ) {

}

void gBaseCanvas::mouseDragged(int x, int y, int button) {

}

void gBaseCanvas::mousePressed(int x, int y, int button) {

}

void gBaseCanvas::mouseReleased(int x, int y, int button) {

}

void gBaseCanvas::mouseScrolled(int x, int y) {

}

void gBaseCanvas::mouseEntered() {

}

void gBaseCanvas::mouseExited() {

}

void gBaseCanvas::windowResized(int w, int h) {

}

void gBaseCanvas::onGuiEvent(int guiObjectId, int eventType, std::string value1, std::string value2) {

}


void gBaseCanvas::showNotify() {

}

void gBaseCanvas::hideNotify() {

}

int gBaseCanvas::getWidth() {
	return renderer->getWidth();
}

int gBaseCanvas::getHeight() {
	return renderer->getHeight();
}


void gBaseCanvas::clearBackground() {
	// Clear the background
	clearColor(0, 0, 30, 0);
	setColor(255, 255, 255);
}

void gBaseCanvas::setColor(int r, int g, int b, int a) {
	renderer->setColor(r, g, b, a);
}

void gBaseCanvas::setColor(gColor color) {
	renderer->setColor(color);
}

gColor gBaseCanvas::getColor() {
	return renderer->getColor();
}

void gBaseCanvas::clear() {
	renderer->clear();
}

void gBaseCanvas::clearColor(int r, int g, int b, int a) {
	renderer->clearColor(r, g, b, a);
}

void gBaseCanvas::clearColor(gColor color) {
	renderer->clearColor(color);
}

void gBaseCanvas::enableLighting() {
	renderer->enableLighting();
}

void gBaseCanvas::disableLighting() {
	renderer->disableLighting();
}

bool gBaseCanvas::isLightingEnabled() {
	return renderer->isLightingEnabled();
}

void gBaseCanvas::setLightingColor(int r, int g, int b, int a) {
	renderer->setLightingColor(r, g, b, a);
}

void gBaseCanvas::setLightingColor(gColor* color) {
	renderer->setLightingColor(color);
}

gColor* gBaseCanvas::getLightingColor() {
	return renderer->getLightingColor();
}

void gBaseCanvas::setLightingPosition(glm::vec3 lightingPosition) {
	renderer->setLightingPosition(lightingPosition);
}

glm::vec3 gBaseCanvas::getLightingPosition() {
	return renderer->getLightingPosition();
}

void gBaseCanvas::setGlobalAmbientColor(int r, int g, int b, int a) {
	renderer->setGlobalAmbientColor(r, g, b, a);
}

void gBaseCanvas::setGlobalAmbientColor(gColor color) {
	renderer->setGlobalAmbientColor(color);
}

gColor* gBaseCanvas::getGlobalAmbientColor() {
	return renderer->getGlobalAmbientColor();
}

void gBaseCanvas::addSceneLight(gLight* light) {
	renderer->addSceneLight(light);
}

gLight* gBaseCanvas::getSceneLight(int lightNo) {
	return renderer->getSceneLight(lightNo);
}

int gBaseCanvas::getSceneLightNum() {
	return renderer->getSceneLightNum();
}

void gBaseCanvas::removeSceneLight(gLight* light)  {
	renderer->removeSceneLight(light);
}

void gBaseCanvas::removeAllSceneLights() {
	renderer->removeAllSceneLights();
}

void gBaseCanvas::enableDepthTest() {
	renderer->enableDepthTest();
}

void gBaseCanvas::enableDepthTest(int depthTestType) {
	renderer->enableDepthTest(depthTestType);
}

void gBaseCanvas::setDepthTestFunc(int depthTestType) {
	renderer->setDepthTestFunc(depthTestType);
}

void gBaseCanvas::disableDepthTest() {
	renderer->disableDepthTest();
}

bool gBaseCanvas::isDepthTestEnabled() {
	return renderer->isDepthTestEnabled();
}

int gBaseCanvas::getDepthTestType() {
	return renderer->getDepthTestType();
}


void gBaseCanvas::enableAlphaBlending() {
	renderer->enableAlphaBlending();
}

void gBaseCanvas::disableAlphaBlending() {
	renderer->disableAlphaBlending();
}

void gBaseCanvas::enableAlphaTest() {
	renderer->enableAlphaTest();
}

void gBaseCanvas::disableAlphaTest() {
	renderer->disableAlphaTest();
}

void gBaseCanvas::enabledGrid() {
	renderer->enabledGrid();
}

void gBaseCanvas::disabledGrid() {
	renderer->disabledGrid();
}

//set which Grid axis to show or close with xy,yz,xz boolean
void gBaseCanvas::setGridShowedAxis(bool xy, bool yz, bool xz) {
	renderer->setGridShowedAxis(xy, yz, xz);
}

// set Grid XY axis enable or not with xy boolean
void gBaseCanvas::setGridEnabledXY(bool xy) {
	renderer->setGridEnabledXY(xy);
}

// set Grid XZ axis enable or not with xy boolean
void gBaseCanvas::setGridEnabledXZ(bool xz) {
	renderer->setGridEnabledXZ(xz);
}

// set Grid YZ axis enable or not with yz boolean
void gBaseCanvas::setGridEnabledYZ(bool yz) {
	renderer->setGridEnabledYZ(yz);
}

bool gBaseCanvas::isGridEnabled() {
	return renderer->isGridEnabled();
}

bool gBaseCanvas::isGridXYEnabled() {
	return renderer->isGridXYEnabled();
}

bool gBaseCanvas::isGridXZEnabled() {
	return renderer->isGridXZEnabled();
}

bool gBaseCanvas::isGridYZEnabled() {
	return renderer->isGridYZEnabled();
}



