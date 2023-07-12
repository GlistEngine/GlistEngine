/*
 * gGUIManager.cpp
 *
 *  Created on: Aug 20, 2021
 *      Author: noyan
 */

#include "gGUIManager.h"
#include "gBaseApp.h"
#include "gBaseCanvas.h"
#include "gAppManager.h"


gGUIManager::gGUIManager(gBaseApp* root, int width, int height) {
	this->root = root;
	isframeset = false;
	selecteddialogue = nullptr;
	loadThemes();
	resetTheme(GUITHEME_LIGHT);
	if(appmanager->getWindowMode() == G_WINDOWMODE_GUIAPP || appmanager->getWindowMode() == G_WINDOWMODE_FULLSCREENGUIAPP) {
		setCurrentFrame(&emptyframe, width, height);
		emptyframe.setSizer(&emptysizer);
		emptysizer.enableBackgroundFill(true);
	}
}

gGUIManager::~gGUIManager() {
}

void gGUIManager::setTheme(int guiTheme) {
	resetTheme(guiTheme);
}

int gGUIManager::getTheme() {
	return guitheme;
}

gFont* gGUIManager::getFont(int fontNum) {
	return &themefonts[fontNum];
}

void gGUIManager::setCurrentFrame(gGUIFrame* currentFrame) {
	setCurrentFrame(currentFrame,
                    appmanager->getCurrentCanvas()->getScreenWidth(),
                    appmanager->getCurrentCanvas()->getScreenHeight());
}

void gGUIManager::setCurrentFrame(gGUIFrame* currentFrame, int width, int height) {
	currentframe = currentFrame;
	currentframe->setParentSlotNo(0, 0);
	currentframe->left = 0;
	currentframe->top = 0;
	currentframe->right = width;
	currentframe->bottom = height;
	currentframe->width = currentframe->right - currentframe->left;
	currentframe->height = currentframe->bottom - currentframe->top;
	currentframe->setRootApp(root);
	isframeset = true;
}

void gGUIManager::setupDialogue(gGUIDialogue* dialogue) {
	dialogue->setParentSlotNo(0, 0);
	dialogue->width = appmanager->getCurrentCanvas()->getScreenWidth() / 1 * 0.84f;
	dialogue->height = appmanager->getCurrentCanvas()->getScreenHeight() / 1 * 0.84f;
	dialogue->left = (appmanager->getCurrentCanvas()->getScreenWidth() - dialogue->width) / 2;
	dialogue->top = (appmanager->getCurrentCanvas()->getScreenHeight() - dialogue->height) / 2;
	dialogue->right = dialogue->left + dialogue->width;
	dialogue->bottom = dialogue->top + dialogue->height;
	dialogue->setRootApp(root);

	defdialoguesizer.setSize(1, 1);
	defdialoguesizer.enableBorders(false);
	dialogue->setSizer(&defdialoguesizer);

	dialogue->getSizer()->enableBackgroundFill(false);
	dialogue->resetTitleBar();
	dialogue->resetButtonsBar();

	dialogues.push_back(dialogue);
}

gGUIFrame* gGUIManager::getCurrentFrame() {
	return currentframe;
}

void gGUIManager::keyPressed(int key) {
	currentframe->keyPressed(key);
	for (int i = 0; i < dialogues.size(); i++) {
		if (dialogues[i]->getIsDialogueActive()) dialogues[i]->keyPressed(key);
	}
}

void gGUIManager::keyReleased(int key) {
	currentframe->keyReleased(key);
	for (int i = 0; i < dialogues.size(); i++) {
		if (dialogues[i]->getIsDialogueActive()) dialogues[i]->keyReleased(key);
	}
}

void gGUIManager::charPressed(unsigned int key) {
	currentframe->charPressed(key);
	for (int i = 0; i < dialogues.size(); i++) {
		if (dialogues[i]->getIsDialogueActive()) dialogues[i]->charPressed(key);
	}
}

void gGUIManager::mouseMoved(int x, int y) {
	appmanager->setCursor(currentframe->getCursor(x, y));
	currentframe->mouseMoved(x, y);

	for (int i = 0; i < dialogues.size(); i++) {
		if (dialogues[i]->getIsDialogueActive()) {
			appmanager->setCursor(dialogues[i]->getCursor(x, y));
			dialogues[i]->mouseMoved(x, y);
		}
	}
}

void gGUIManager::mousePressed(int x, int y, int button) {
	if (selecteddialogue == nullptr) currentframe->mousePressed(x, y, button);
	else selecteddialogue->mousePressed(x, y, button);
}

void gGUIManager::mouseDragged(int x, int y, int button) {
	if (selecteddialogue == nullptr) currentframe->mouseDragged(x, y, button);
	else selecteddialogue->mouseDragged(x, y, button);
}

void gGUIManager::mouseReleased(int x, int y, int button) {
	currentframe->mouseReleased(x, y, button);
	for (int i = 0; i < dialogues.size(); i++) {
		if (dialogues[i]->getIsDialogueActive()) dialogues[i]->mouseReleased(x, y, button);
	}
}

void gGUIManager::mouseScrolled(int x, int y) {
	currentframe->mouseScrolled(x, y);
	for (int i = 0; i < dialogues.size(); i++) {
		if (dialogues[i]->getIsDialogueActive()) dialogues[i]->mouseScrolled(x, y);
	}
}

void gGUIManager::mouseEntered() {
	currentframe->mouseEntered();
	for (int i = 0; i < dialogues.size(); i++) {
		if (dialogues[i]->getIsDialogueActive()) dialogues[i]->mouseEntered();
	}
}

void gGUIManager::mouseExited() {
	currentframe->mouseExited();
	for (int i = 0; i < dialogues.size(); i++) {
		if (dialogues[i]->getIsDialogueActive()) dialogues[i]->mouseEntered();
	}
}

void gGUIManager::windowResized(int w, int h) {
	currentframe->width = w;
	currentframe->height = h;
	currentframe->windowResized(w, h);
}

void gGUIManager::update() {
	if(!isframeset) return;

	currentframe->update();

	for (int i = dialogues.size() - 1; i >= 0; i--) {
		if (dialogues[i]->getIsDialogueActive()) {selecteddialogue = dialogues[i]; break;}
		selecteddialogue = nullptr;
	}

	if (selecteddialogue != nullptr) {
		selecteddialogue->update();
		if (selecteddialogue->getButtonEvent() == gGUIDialogue::EVENT_EXIT) {
			selecteddialogue->setIsDialogueActive(false);
			selecteddialogue->setButtonEvent(gGUIDialogue::EVENT_NONE);
		}
		if (selecteddialogue->getButtonEvent() == gGUIDialogue::EVENT_MINIMIZE) {
			selecteddialogue->setIsDialogueActive(false);
			selecteddialogue->setButtonEvent(gGUIDialogue::EVENT_NONE);
		}
		if (selecteddialogue->getButtonEvent() == gGUIDialogue::EVENT_MAXIMIZE) {
			int titlebarheight = selecteddialogue->getTitleBar()->height;
			int buttonsbarheight = selecteddialogue->getButtonsBar()->height;
			int twidth = appmanager->getCurrentCanvas()->getScreenWidth();
			int theight = appmanager->getCurrentCanvas()->getScreenHeight() - titlebarheight - buttonsbarheight;
			int tleft = 0;
			int ttop = titlebarheight;
			selecteddialogue->transformDialogue(tleft, ttop, twidth, theight);

			selecteddialogue->setIsMaximized(true);

			selecteddialogue->resetTitleBar();
			selecteddialogue->resetButtonsBar();
			selecteddialogue->setButtonEvent(gGUIDialogue::EVENT_NONE);
		}
		if (selecteddialogue->getButtonEvent() == gGUIDialogue::EVENT_RESTORE) {
			int twidth = appmanager->getCurrentCanvas()->getScreenWidth() / 1 * 0.84f;
			int theight = appmanager->getCurrentCanvas()->getScreenHeight() / 1 * 0.84f;
			int tleft = (appmanager->getCurrentCanvas()->getScreenWidth() - twidth) / 2;
			int ttop = (appmanager->getCurrentCanvas()->getScreenHeight() - theight) / 2;
			selecteddialogue->transformDialogue(tleft, ttop, twidth, theight);

			selecteddialogue->setIsMaximized(false);

			selecteddialogue->resetTitleBar();
			selecteddialogue->resetButtonsBar();
			selecteddialogue->setButtonEvent(gGUIDialogue::EVENT_NONE);
		}
	}
}

void gGUIManager::draw() {
	if(!isframeset) return;

	currentframe->draw();
	for (int i = 0; i < dialogues.size(); i++) {
		if (dialogues[i]->getIsDialogueActive()) dialogues[i]->draw();
	}
}

void gGUIManager::resetTheme(int guiTheme) {
	guitheme = guiTheme;
	gBaseGUIObject::setBackgroundColor(&themebackgroundcolor[guitheme]);
	gBaseGUIObject::setMiddlegroundColor(&thememiddlegroundcolor[guitheme]);
	gBaseGUIObject::setForegroundColor(&themeforegroundcolor[guitheme]);
	gBaseGUIObject::setTextBackgroundColor(&themetextbackgroundcolor[guitheme]);
	gBaseGUIObject::setNavigationBackgroundColor(&themenavigationbackgroundcolor[guitheme]);
	gBaseGUIObject::setFont(&themefonts[FONT_REGULAR]);
	gBaseGUIObject::setFontColor(&themefontcolor[guitheme]);
	gBaseGUIObject::setNavigationFontColor(&themenavigationfontcolor[guitheme]);
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

	themenavigationbackgroundcolor[GUITHEME_LIGHT] = gColor(0.0f / 255.0f, 40.0f / 255.0f, 80.0f / 255.0f);
	themenavigationbackgroundcolor[GUITHEME_DARK] = gColor(195.0f / 255.0f, 224.0f / 255.0f, 235.0f / 255.0f);
	themenavigationbackgroundcolor[GUITHEME_LIGHTBLUE] = gColor(195.0f / 255.0f, 224.0f / 255.0f, 235.0f / 255.0f);
	themenavigationbackgroundcolor[GUITHEME_DARKBLUE] = gColor(195.0f / 255.0f, 224.0f / 255.0f, 235.0f / 255.0f);

	themefonts[FONT_REGULAR].loadFont("FreeSans.ttf", 11);
	themefonts[FONT_BOLD].loadFont("FreeSansBold.ttf", 11);
	themefonts[FONT_ITALIC].loadFont("FreeSansOblique.ttf", 11);
	themefonts[FONT_BOLDITALIC].loadFont("FreeSansBoldOblique.ttf", 11);

	themefontcolor[GUITHEME_LIGHT] = gColor(5.0f / 255.0f, 5.0f / 255.0f, 5.0f / 255.0f);
	themefontcolor[GUITHEME_DARK] = gColor(220.0f / 255.0f, 220.0f / 255.0f, 220.0f / 255.0f);
	themefontcolor[GUITHEME_LIGHTBLUE] = gColor(5.0f / 255.0f, 5.0f / 255.0f, 5.0f / 255.0f);
	themefontcolor[GUITHEME_DARKBLUE] = gColor(28.0f / 255.0f, 40.0f / 255.0f, 53.0f / 255.0f);

	themenavigationfontcolor[GUITHEME_LIGHT] = gColor(255.0f / 255.0f, 255.0f / 255.0f, 255.0f / 255.0f);
	themenavigationfontcolor[GUITHEME_DARK] = gColor(255.0f / 255.0f, 255.0f / 255.0f, 255.0f / 255.0f);
	themenavigationfontcolor[GUITHEME_LIGHTBLUE] = gColor(255.0f / 255.0f, 255.0f / 255.0f, 255.0f / 255.0f);
	themenavigationfontcolor[GUITHEME_DARKBLUE] = gColor(255.0f / 255.0f, 255.0f / 255.0f, 255.0f / 255.0f);

	themebuttoncolor[GUITHEME_LIGHT] = gColor(209.0f / 255.0f, 209.0f / 255.0f, 209.0f / 255.0f);
	themebuttoncolor[GUITHEME_DARK] = gColor(109.0f / 255.0f, 109.0f / 255.0f, 109.0f / 255.0f);
	themebuttoncolor[GUITHEME_LIGHTBLUE] = gColor(5.0f / 255.0f, 5.0f / 255.0f, 5.0f / 255.0f);
	themebuttoncolor[GUITHEME_DARKBLUE] = gColor(28.0f / 255.0f, 40.0f / 255.0f, 53.0f / 255.0f);

	themepressedbuttoncolor[GUITHEME_LIGHT] = gColor(189.0f / 255.0f, 189.0f / 255.0f, 189.0f / 255.0f);
	themepressedbuttoncolor[GUITHEME_DARK] = gColor(89.0f / 255.0f, 89.0f / 255.0f, 89.0f / 255.0f);
	themepressedbuttoncolor[GUITHEME_LIGHTBLUE] = gColor(5.0f / 255.0f, 5.0f / 255.0f, 5.0f / 255.0f);
	themepressedbuttoncolor[GUITHEME_DARKBLUE] = gColor(28.0f / 255.0f, 40.0f / 255.0f, 53.0f / 255.0f);

	themedisabledbuttoncolor[GUITHEME_LIGHT] = gColor(209.0f / 255.0f, 209.0f / 255.0f, 209.0f / 255.0f);
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

	themedisabledbuttonfontcolor[GUITHEME_LIGHT] = gColor(160.0f / 255.0f, 160.0f / 255.0f, 160.0f / 255.0f);
	themedisabledbuttonfontcolor[GUITHEME_DARK] = gColor(160.0f / 255.0f, 160.0f / 255.0f, 160.0f / 255.0f);
	themedisabledbuttonfontcolor[GUITHEME_LIGHTBLUE] = gColor(5.0f / 255.0f, 5.0f / 255.0f, 5.0f / 255.0f);
	themedisabledbuttonfontcolor[GUITHEME_DARKBLUE] = gColor(28.0f / 255.0f, 40.0f / 255.0f, 53.0f / 255.0f);

}
