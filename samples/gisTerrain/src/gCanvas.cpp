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
	// Generate terrain from a heightmap image.
	// Parameters: (imagePath, width, depth, heightScale, maxHeight)
	// heightScale controls vertical exaggeration, maxHeight defines the height range.
	ter.generateTerrain("heightmap1.png", 900, 512, 1.0f, 100.0f);

	// Alternative heightmaps can be used for different terrain variations.
	//ter.generateTerrain("heightmap2.png", 900, 512, 1.0f, 100.0f);
	//ter.generateTerrain("heightmap3.png", 900, 512, 1.0f, 100.0f);

	// Load terrain texture and assign it to the material as a diffuse map.
	tertex.loadTexture("maptexture.png");
	termaterial.setDiffuseMap(&tertex);
	ter.setMaterial(&termaterial);

	// Configure initial camera position and orientation.
	cam.setPosition(100, 120, 200);
	cam.tilt(-0.8f);

	// Set terrain transform (position and orientation in world space).
	ter.setPosition(-328, -229, -463);
	ter.tilt(0.2f);

	// Configure lighting parameters.
	sun.setSpecularColor(8, 8, 8);
	sun.setAmbientColor(255, 255, 255);
	sun.setDiffuseColor(255, 255, 245);
	sun.setPosition(0.0f, 50.0f, 20.0f);

	// Initialize shadow mapping (resolution affects quality and performance).
	shadow.allocate(&sun, &cam, 4096, 4096);

	// Define light projection frustum for shadow calculations.
	shadow.setLightProjection(-20.0f, 20.0f, 20.0f, -20.0f, 0.01f, 60.0f);

	// Enable shadow system.
	shadow.activate();
}

void gCanvas::update() {
}

void gCanvas::draw() {
	// Enable rendering states: lighting, camera, shadow mapping, depth testing.
	cam.begin();
	sun.enable();
	shadow.enable();
	enableDepthTest();

	// Render terrain geometry.
	ter.draw();

	// Restore rendering states.
	disableDepthTest();
	shadow.disable();
	sun.disable();
	cam.end();;
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

