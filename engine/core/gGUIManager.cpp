/*
 * gGUIManager.cpp
 *
 *  Created on: Aug 20, 2021
 *      Author: noyan
 */

#include "gGUIManager.h"
#include "gBaseApp.h"
#include "gBaseCanvas.h"


gGUIManager::gGUIManager(gBaseApp* root) {
	this->root = root;
	isframeset = false;
	loadThemes();
	resetTheme(GUITHEME_LIGHT);
}

gGUIManager::~gGUIManager() {
}

void gGUIManager::setTheme(int guiTheme) {
	resetTheme(guiTheme);
}

int gGUIManager::getTheme() {
	return guitheme;
}

void gGUIManager::setCurrentFrame(gGUIFrame* currentFrame) {
	currentframe = currentFrame;
	currentframe->setParentSlotNo(0, 0);
	currentframe->left = 0;
	currentframe->top = 0;
	currentframe->right = root->getAppManager()->getCurrentCanvas()->getScreenWidth();
	currentframe->bottom = root->getAppManager()->getCurrentCanvas()->getScreenHeight();
	currentframe->width = currentframe->right - currentframe->left;
	currentframe->height = currentframe->bottom - currentframe->top;
	currentframe->setRootApp(root);
	isframeset = true;
}

gGUIFrame* gGUIManager::getCurrentFrame() {
	return currentframe;
}

void gGUIManager::keyPressed(int key) {
	currentframe->keyPressed(key);
}

void gGUIManager::keyReleased(int key) {
	currentframe->keyReleased(key);
}

void gGUIManager::charPressed(unsigned int key) {
	currentframe->charPressed(key);
}

void gGUIManager::mouseMoved(int x, int y) {
	root->getAppManager()->setCursor(currentframe->getCursor(x, y));
	currentframe->mouseMoved(x, y);
}

void gGUIManager::mousePressed(int x, int y, int button) {
	currentframe->mousePressed(x, y, button);
}

void gGUIManager::mouseDragged(int x, int y, int button) {
	currentframe->mouseDragged(x, y, button);
}

void gGUIManager::mouseReleased(int x, int y, int button) {
	currentframe->mouseReleased(x, y, button);
}

void gGUIManager::mouseScrolled(int x, int y) {
	currentframe->mouseScrolled(x, y);
}

void gGUIManager::update() {
	currentframe->update();
}

void gGUIManager::draw() {
	currentframe->draw();
}

void gGUIManager::resetTheme(int guiTheme) {
	guitheme = guiTheme;
	gBaseGUIObject::setBackgroundColor(&themebackgroundcolor[guitheme]);
	gBaseGUIObject::setForegroundColor(&themeforegroundcolor[guitheme]);
	gBaseGUIObject::setFont(&themefont);
	gBaseGUIObject::setFontColor(&themefontcolor[guitheme]);
}

void gGUIManager::loadThemes() {
	themebackgroundcolor[GUITHEME_LIGHT] = gColor(170.0f / 255.0f, 170.0f / 255.0f, 170.0f / 255.0f);
	themebackgroundcolor[GUITHEME_DARK] = gColor(39.0f / 255.0f, 39.0f / 255.0f, 39.0f / 255.0f);
	themebackgroundcolor[GUITHEME_LIGHTBLUE] = gColor(195.0f / 255.0f, 224.0f / 255.0f, 235.0f / 255.0f);
	themebackgroundcolor[GUITHEME_DARKBLUE] = gColor(28.0f / 255.0f, 40.0f / 255.0f, 53.0f / 255.0f);

	themeforegroundcolor[GUITHEME_LIGHT] = gColor(240.0f / 255.0f, 240.0f / 255.0f, 240.0f / 255.0f);
	themeforegroundcolor[GUITHEME_DARK] = gColor(59.0f / 255.0f, 59.0f / 255.0f, 59.0f / 255.0f);
	themeforegroundcolor[GUITHEME_LIGHTBLUE] = gColor(195.0f / 255.0f, 224.0f / 255.0f, 235.0f / 255.0f);
	themeforegroundcolor[GUITHEME_DARKBLUE] = gColor(28.0f / 255.0f, 40.0f / 255.0f, 53.0f / 255.0f);

	themefont.loadFont("FreeSans.ttf", 8);

	themefontcolor[GUITHEME_LIGHT] = gColor(5.0f / 255.0f, 5.0f / 255.0f, 5.0f / 255.0f);
	themefontcolor[GUITHEME_DARK] = gColor(220.0f / 255.0f, 220.0f / 255.0f, 220.0f / 255.0f);
	themefontcolor[GUITHEME_LIGHTBLUE] = gColor(5.0f / 255.0f, 5.0f / 255.0f, 5.0f / 255.0f);
	themefontcolor[GUITHEME_DARKBLUE] = gColor(28.0f / 255.0f, 40.0f / 255.0f, 53.0f / 255.0f);
}

