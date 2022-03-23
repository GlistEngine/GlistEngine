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

void gGUIManager::mouseEntered() {
	currentframe->mouseEntered();
}

void gGUIManager::mouseExited() {
	currentframe->mouseExited();
}

void gGUIManager::windowResized(int w, int h) {
	currentframe->width = w;
	currentframe->height = h;
	currentframe->windowResized(w, h);
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
	gBaseGUIObject::setMiddlegroundColor(&thememiddlegroundcolor[guitheme]);
	gBaseGUIObject::setForegroundColor(&themeforegroundcolor[guitheme]);
	gBaseGUIObject::setTextBackgroundColor(&themetextbackgroundcolor[guitheme]);
	gBaseGUIObject::setFont(&themefont);
	gBaseGUIObject::setFontColor(&themefontcolor[guitheme]);
	gBaseGUIObject::setButtonColor(&themebuttoncolor[guitheme]);
	gBaseGUIObject::setPressedButtonColor(&themepressedbuttoncolor[guitheme]);
	gBaseGUIObject::setDisabledButtonColor(&themedisabledbuttoncolor[guitheme]);
	gBaseGUIObject::setButtonFontColor(&themebuttonfontcolor[guitheme]);
	gBaseGUIObject::setPressedButtonFontColor(&themepressedbuttonfontcolor[guitheme]);
	gBaseGUIObject::setDisabledButtonFontColor(&themedisabledbuttonfontcolor[guitheme]);
}

void gGUIManager::loadThemes() {
	themebackgroundcolor[GUITHEME_LIGHT] = gColor(170.0f / 255.0f, 170.0f / 255.0f, 170.0f / 255.0f);
	themebackgroundcolor[GUITHEME_DARK] = gColor(39.0f / 255.0f, 39.0f / 255.0f, 39.0f / 255.0f);
	themebackgroundcolor[GUITHEME_LIGHTBLUE] = gColor(195.0f / 255.0f, 224.0f / 255.0f, 235.0f / 255.0f);
	themebackgroundcolor[GUITHEME_DARKBLUE] = gColor(28.0f / 255.0f, 40.0f / 255.0f, 53.0f / 255.0f);

	thememiddlegroundcolor[GUITHEME_LIGHT] = gColor(205.0f / 255.0f, 205.0f / 255.0f, 205.0f / 255.0f);
	thememiddlegroundcolor[GUITHEME_DARK] = gColor(49.0f / 255.0f, 49.0f / 255.0f, 49.0f / 255.0f);
	thememiddlegroundcolor[GUITHEME_LIGHTBLUE] = gColor(195.0f / 255.0f, 224.0f / 255.0f, 235.0f / 255.0f);
	thememiddlegroundcolor[GUITHEME_DARKBLUE] = gColor(28.0f / 255.0f, 40.0f / 255.0f, 53.0f / 255.0f);

	themeforegroundcolor[GUITHEME_LIGHT] = gColor(240.0f / 255.0f, 240.0f / 255.0f, 240.0f / 255.0f);
	themeforegroundcolor[GUITHEME_DARK] = gColor(59.0f / 255.0f, 59.0f / 255.0f, 59.0f / 255.0f);
	themeforegroundcolor[GUITHEME_LIGHTBLUE] = gColor(195.0f / 255.0f, 224.0f / 255.0f, 235.0f / 255.0f);
	themeforegroundcolor[GUITHEME_DARKBLUE] = gColor(28.0f / 255.0f, 40.0f / 255.0f, 53.0f / 255.0f);

	themetextbackgroundcolor[GUITHEME_LIGHT] = gColor(255.0f / 255.0f, 255.0f / 255.0f, 255.0f / 255.0f);
	themetextbackgroundcolor[GUITHEME_DARK] = gColor(39.0f / 255.0f, 39.0f / 255.0f, 39.0f / 255.0f);
	themetextbackgroundcolor[GUITHEME_LIGHTBLUE] = gColor(255.0f / 255.0f, 255.0f / 255.0f, 255.0f / 255.0f);
	themetextbackgroundcolor[GUITHEME_DARKBLUE] = gColor(0.0f / 255.0f, 0.0f / 255.0f, 0.0f / 255.0f);

	themefont.loadFont("FreeSans.ttf", 11);

	themefontcolor[GUITHEME_LIGHT] = gColor(5.0f / 255.0f, 5.0f / 255.0f, 5.0f / 255.0f);
	themefontcolor[GUITHEME_DARK] = gColor(220.0f / 255.0f, 220.0f / 255.0f, 220.0f / 255.0f);
	themefontcolor[GUITHEME_LIGHTBLUE] = gColor(5.0f / 255.0f, 5.0f / 255.0f, 5.0f / 255.0f);
	themefontcolor[GUITHEME_DARKBLUE] = gColor(28.0f / 255.0f, 40.0f / 255.0f, 53.0f / 255.0f);

	themebuttoncolor[GUITHEME_LIGHT] = gColor(209.0f / 255.0f, 209.0f / 255.0f, 209.0f / 255.0f);
	themebuttoncolor[GUITHEME_DARK] = gColor(109.0f / 255.0f, 109.0f / 255.0f, 109.0f / 255.0f);
	themebuttoncolor[GUITHEME_LIGHTBLUE] = gColor(5.0f / 255.0f, 5.0f / 255.0f, 5.0f / 255.0f);
	themebuttoncolor[GUITHEME_DARKBLUE] = gColor(28.0f / 255.0f, 40.0f / 255.0f, 53.0f / 255.0f);

	themepressedbuttoncolor[GUITHEME_LIGHT] = gColor(189.0f / 255.0f, 189.0f / 255.0f, 189.0f / 255.0f);
	themepressedbuttoncolor[GUITHEME_DARK] = gColor(89.0f / 255.0f, 89.0f / 255.0f, 89.0f / 255.0f);
	themepressedbuttoncolor[GUITHEME_LIGHTBLUE] = gColor(5.0f / 255.0f, 5.0f / 255.0f, 5.0f / 255.0f);
	themepressedbuttoncolor[GUITHEME_DARKBLUE] = gColor(28.0f / 255.0f, 40.0f / 255.0f, 53.0f / 255.0f);

	themedisabledbuttoncolor[GUITHEME_LIGHT] = gColor(5.0f / 255.0f, 5.0f / 255.0f, 5.0f / 255.0f);
	themedisabledbuttoncolor[GUITHEME_DARK] = gColor(109.0f / 255.0f, 109.0f / 255.0f, 109.0f / 255.0f);
	themedisabledbuttoncolor[GUITHEME_LIGHTBLUE] = gColor(5.0f / 255.0f, 5.0f / 255.0f, 5.0f / 255.0f);
	themedisabledbuttoncolor[GUITHEME_DARKBLUE] = gColor(28.0f / 255.0f, 40.0f / 255.0f, 53.0f / 255.0f);

	themebuttonfontcolor[GUITHEME_LIGHT] = gColor(5.0f / 255.0f, 5.0f / 255.0f, 5.0f / 255.0f);
	themebuttonfontcolor[GUITHEME_DARK] = gColor(220.0f / 255.0f, 220.0f / 255.0f, 220.0f / 255.0f);
	themebuttonfontcolor[GUITHEME_LIGHTBLUE] = gColor(5.0f / 255.0f, 5.0f / 255.0f, 5.0f / 255.0f);
	themebuttonfontcolor[GUITHEME_DARKBLUE] = gColor(28.0f / 255.0f, 40.0f / 255.0f, 53.0f / 255.0f);

	themepressedbuttonfontcolor[GUITHEME_LIGHT] = gColor(5.0f / 255.0f, 5.0f / 255.0f, 5.0f / 255.0f);
	themepressedbuttonfontcolor[GUITHEME_DARK] = gColor(220.0f / 255.0f, 220.0f / 255.0f, 220.0f / 255.0f);
	themepressedbuttonfontcolor[GUITHEME_LIGHTBLUE] = gColor(5.0f / 255.0f, 5.0f / 255.0f, 5.0f / 255.0f);
	themepressedbuttonfontcolor[GUITHEME_DARKBLUE] = gColor(28.0f / 255.0f, 40.0f / 255.0f, 53.0f / 255.0f);

	themedisabledbuttonfontcolor[GUITHEME_LIGHT] = gColor(5.0f / 255.0f, 5.0f / 255.0f, 5.0f / 255.0f);
	themedisabledbuttonfontcolor[GUITHEME_DARK] = gColor(160.0f / 255.0f, 160.0f / 255.0f, 160.0f / 255.0f);
	themedisabledbuttonfontcolor[GUITHEME_LIGHTBLUE] = gColor(5.0f / 255.0f, 5.0f / 255.0f, 5.0f / 255.0f);
	themedisabledbuttonfontcolor[GUITHEME_DARKBLUE] = gColor(28.0f / 255.0f, 40.0f / 255.0f, 53.0f / 255.0f);

}

