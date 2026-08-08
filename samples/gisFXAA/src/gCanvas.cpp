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
	enableAlphaBlending();
	// Instantiate FXAA with the High Quality preset
	FXAA = new gFXAA(0.75f, 0.0312f, 0.0625f);

	/*
	 * 1. PARAMETERS:
     * - subpixelblend   : Amount of sub-pixel aliasing removal (0.0 - 1.0). Higher values soften edges.
     * - edgethresholdmin: Minimum local contrast to be considered an edge (reduces GPU load on flat areas).
     * - edgethresholdmax: Maximum contrast threshold trading performance for quality.
	 *
	 * 2. RECOMMENDED PRESETS (subpixelblend, edgethresholdmin, edgethresholdmax):
	 * - Extreme Quality : (1.0f,  0.02f,   0.0312f) -> Maximum edge smoothing and sensitive edge detection.
	 * - High Quality    : (0.75f, 0.0312f, 0.0625f) -> High quality with balanced sharpness.
	 * - Default         : (0.75f, 0.0312f, 0.125f)  -> Standard balanced mode.
	 * - Fast / Low      : (0.25f, 0.0833f, 0.25f)    -> High performance, minimal blur.
	 */

	ppm.setDimensions(getWidth(), getHeight());

	// MUST BE ADDED AS THE FIRST EFFECT TO THE PIPELINE:
	ppm.addEffect(FXAA);

	// Additional effects (Bloom, Vignette, etc.) must be added AFTER FXAA.

	isRotationPaused = false;
	camera.setPosition(0.0f, 0.0f, 170.0f);
	camera.setFov(20.0f);
	orbitAngle = 0.0f;

	cubeMesh.setAllVertexColor(glm::vec3(1.0f, 0.0f, 1.0f));
	sphereMesh.setAllVertexColor(glm::vec3(1.0f, 1.0f, 0.0f));
	coneMesh.setAllVertexColor(glm::vec3(0.0f, 1.0f, 1.0f));

	cubeMesh.scale(4.8f);
	sphereMesh.scale(4.8f);
	coneMesh.scale(4.8f);

	cubeMesh.panDeg(45.0f);
	cubeMesh.tiltDeg(45.0f);
	coneMesh.tiltDeg(30.0f);

	font.loadFont("FreeSans.ttf", 16);
}

void gCanvas::update() {
	if (!isRotationPaused) {
		float rotationSpeed = 0.004f;
		orbitAngle += rotationSpeed;

		if (orbitAngle >= 2.0f * PI) {
			orbitAngle -= 2.0f * PI;
		}
	}
}

void gCanvas::draw() {
	camera.begin();

	enableDepthTest();

	float radius = 10.0f;
	float tiltAngle = gDegToRad(40.0f);
	float yOffset = -8.0f;
	float p;
	float rawX, rawZ;

	// Post Process Shaders Enabled
	ppm.enable();

	float rightOffsetX = 32.0f;
	p = orbitAngle + ((4.0f * PI) / 3.0f);
	rawX = cos(p) * radius;
	rawZ = sin(p) * radius;
	cubeMesh.setPosition(rawX + rightOffsetX, (rawZ * sin(tiltAngle)) + yOffset, rawZ * cos(tiltAngle));
	cubeMesh.draw();

	p = orbitAngle;
	rawX = cos(p) * radius;
	rawZ = sin(p) * radius;
	sphereMesh.setPosition(rawX + rightOffsetX, (rawZ * sin(tiltAngle)) + yOffset, rawZ * cos(tiltAngle));
	sphereMesh.draw();

	p = orbitAngle + ((2.0f * PI) / 3.0f);
	rawX = cos(p) * radius;
	rawZ = sin(p) * radius;
	coneMesh.setPosition(rawX + rightOffsetX, (rawZ * sin(tiltAngle)) + yOffset, rawZ * cos(tiltAngle));
	coneMesh.draw();

	ppm.disable();

	float leftOffsetX = -32.0f;
	p = orbitAngle + ((4.0f * PI) / 3.0f);
	rawX = cos(p) * radius;
	rawZ = sin(p) * radius;
	cubeMesh.setPosition(rawX + leftOffsetX, (rawZ * sin(tiltAngle)) + yOffset, rawZ * cos(tiltAngle));
	cubeMesh.draw();

	p = orbitAngle;
	rawX = cos(p) * radius;
	rawZ = sin(p) * radius;
	sphereMesh.setPosition(rawX + leftOffsetX, (rawZ * sin(tiltAngle)) + yOffset, rawZ * cos(tiltAngle));
	sphereMesh.draw();

	p = orbitAngle + ((2.0f * PI) / 3.0f);
	rawX = cos(p) * radius;
	rawZ = sin(p) * radius;
	coneMesh.setPosition(rawX + leftOffsetX, (rawZ * sin(tiltAngle)) + yOffset, rawZ * cos(tiltAngle));
	coneMesh.draw();

	camera.end();

	disableDepthTest();

	setColor(255, 255, 255);
	gDrawRectangle(getWidth() / 2 - 2, 0, 4, getHeight() - 40, true);

	std::string leftTitle = "NATIVE (NO AA)";
	std::string rightTitle = "FXAA";
	std::string info = "Press Space to Pause rotation";
	int leftTitleWidth = font.getStringWidth(leftTitle);
	int rightTitleWidth = font.getStringWidth(rightTitle);

	font.drawText(leftTitle, (getWidth() / 4) - (leftTitleWidth / 2), 40);
	font.drawText(rightTitle, ((getWidth() / 4) * 3) - (rightTitleWidth / 2), 40);
	std::string spaceInfo = "Press SPACE to Pause/Resume";

	int spaceInfoWidth = font.getStringWidth(spaceInfo);
	setColor(255, 255, 255);
	font.drawText(spaceInfo, (getWidth() / 2) - (spaceInfoWidth / 2), getHeight() - 20);
}
void gCanvas::keyPressed(int key) {
}

void gCanvas::keyReleased(int key) {
	if(key == G_KEY_SPACE) {
		isRotationPaused = !isRotationPaused;
	}
}

void gCanvas::charPressed(unsigned int codepoint) {
}

void gCanvas::mouseMoved(int x, int y) {
}

void gCanvas::mouseDragged(int x, int y, int button) {
}

void gCanvas::mousePressed(int x, int y, int button) {
}

void gCanvas::mouseReleased(int x, int y, int button) {
}

void gCanvas::mouseScrolled(int x, int y) {
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
