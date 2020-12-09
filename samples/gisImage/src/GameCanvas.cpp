/*
 * GameCanvas.cpp
 *
 *  Created on: May 6, 2020
 *      Author: noyan
 */


#include "GameCanvas.h"


GameCanvas::GameCanvas(gBaseApp *root) : gBaseCanvas(root) {
	x = 0;
	y = 0;
	dx = 0;
	dy = 0;
}

GameCanvas::~GameCanvas() {
}

void GameCanvas::setup() {
	// Load the logo image from assets
	logo.loadImage("gamelab-istanbul_logo_128.png");

	// Initial x and y values to draw the image (screen center)
	x = (getWidth() - logo.getWidth()) / 2;
	y = (getHeight() - logo.getHeight()) / 2;

	// Delta values for image animation
	dx = 1;
	dy = 1;
}

void GameCanvas::update() {
	// Increase x until reaching left or right side of the screen
	x += dx;
	if (x <= 0 || x >= getWidth() - logo.getWidth()) dx = -dx;

	// Increase y until reaching top or bottom side of the screen
	y+= dy;
	if (y <= 0 || y >= getHeight() - logo.getHeight()) dy = -dy;
}

void GameCanvas::draw() {
	// Clear the background
	clearColor(0, 0, 30);

	// Set drawing color
	setColor(255, 255, 255);

	// Draw the logo
	logo.draw(x, y);
}

void GameCanvas::keyPressed(int key) {
}

void GameCanvas::keyReleased(int key) {
}

void GameCanvas::mouseMoved(int x, int y) {
}

void GameCanvas::mouseDragged(int x, int y, int button) {
}

void GameCanvas::mousePressed(int x, int y, int button) {
}

void GameCanvas::mouseReleased(int x, int y, int button) {
}

void GameCanvas::mouseEntered() {
}

void GameCanvas::mouseExited() {
}

void GameCanvas::showNotify() {
}

void GameCanvas::hideNotify() {
}

