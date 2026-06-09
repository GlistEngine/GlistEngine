#include "gCanvas.h"

gCanvas::gCanvas(gApp* root) : gBaseCanvas(root) {
	this->root = root;
}

gCanvas::~gCanvas() {
}

void gCanvas::setup() {
	bothAngle = 0.0f;
	pictureAngle = 0.0f;
	maskAngle = 0.0f;
	freeImageAngle = 0.0f;
	freeMaskAngle = 0.0f;

	imageBoth.loadImage("4colour.png");
	imageBoth.loadMaskImage("starmask.png");
	imageBoth.setAlphaMasking(true);

	imagePicture.loadImage("4colour.png");
	imagePicture.loadMaskImage("starmask.png");
	imagePicture.setAlphaMasking(true);

	imageMask.loadImage("4colour.png");
	imageMask.loadMaskImage("starmask.png");
	imageMask.setAlphaMasking(true);

	imageFree.loadImage("4colour.png");
	imageFree.loadMaskImage("starmask.png");
	imageFree.setAlphaMasking(true);
}

void gCanvas::update() {
	bothAngle += 1.0f;
	pictureAngle += 1.5f;
	maskAngle += 1.5f;
	freeImageAngle += 3.0f;
	freeMaskAngle += 1.0f;

	if(bothAngle >= 360.0f) bothAngle -= 360.0f;
	if(pictureAngle >= 360.0f) pictureAngle -= 360.0f;
	if(maskAngle >= 360.0f) maskAngle -= 360.0f;
	if(freeImageAngle >= 360.0f) freeImageAngle -= 360.0f;
	if(freeMaskAngle >= 360.0f) freeMaskAngle -= 360.0f;
}

void gCanvas::draw() {
	int size = 220;
	int gap = 40;
	int totalW = size * 4 + gap * 3;
	int startX = (getWidth() - totalW) / 2;
	int y = (getHeight() - size) / 2;

	int x1 = startX;
	int x2 = x1 + size + gap;
	int x3 = x2 + size + gap;
	int x4 = x3 + size + gap;

	imageBoth.setMaskMode(gTexture::MASKMODE_BOTH);
	imageBoth.setImageRotation(0.0f);
	imageBoth.setMaskRotation(0.0f);
	imageBoth.draw(x1, y, size, size, size / 2, size / 2, bothAngle);

	imagePicture.setMaskMode(gTexture::MASKMODE_PICTURE);
	imagePicture.setImageRotation(pictureAngle);
	imagePicture.setMaskRotation(0.0f);
	imagePicture.draw(x2, y, size, size, size / 2, size / 2, 0.0f);

	imageMask.setMaskMode(gTexture::MASKMODE_MASK);
	imageMask.setImageRotation(0.0f);
	imageMask.setMaskRotation(maskAngle);
	imageMask.draw(x3, y, size, size, size / 2, size / 2, 0.0f);

	imageFree.setMaskMode(gTexture::MASKMODE_BOTH_FREE_ROTATE);
	imageFree.setImageRotation(freeImageAngle);
	imageFree.setMaskRotation(freeMaskAngle);
	imageFree.draw(x4, y, size, size, size / 2, size / 2, 0.0f);
}

void gCanvas::keyPressed(int key) {
}

void gCanvas::keyReleased(int key) {
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
