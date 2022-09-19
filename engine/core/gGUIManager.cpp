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
	if(root->getAppManager()->getWindowMode() == G_WINDOWMODE_GUIAPP) {
		setCurrentFrame(&emptyframe);
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

void gGUIManager::setupDialogue(gGUIDialogue* dialogue) {
	dialogue->setParentSlotNo(0, 0);
	dialogue->width = root->getAppManager()->getCurrentCanvas()->getScreenWidth() / 1 * 0.84f;
	dialogue->height = root->getAppManager()->getCurrentCanvas()->getScreenHeight() / 1 * 0.84f;
	dialogue->left = (root->getAppManager()->getCurrentCanvas()->getScreenWidth() - dialogue->width) / 2;
	dialogue->top = (root->getAppManager()->getCurrentCanvas()->getScreenHeight() - dialogue->height) / 2;
	dialogue->right = dialogue->left + dialogue->width;
	dialogue->bottom = dialogue->top + dialogue->height;
	dialogue->setRootApp(root);

	defdialoguesizer.setSize(1, 1);
	defdialoguesizer.enableBorders(false);
	dialogue->setSizer(&defdialoguesizer);

	dialogue->getSizer()->enableBackgroundFill(false);
	dialogue->resetTitleBar();
	dialogue->resetButtonsBar();

	dialogue->setIsDialogueActive(false);
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
	root->getAppManager()->setCursor(currentframe->getCursor(x, y));
	currentframe->mouseMoved(x, y);

	for (int i = 0; i < dialogues.size(); i++) {
		if (dialogues[i]->getIsDialogueActive()) {
			root->getAppManager()->setCursor(dialogues[i]->getCursor(x, y));
			dialogues[i]->mouseMoved(x, y);
		}
	}
}

void gGUIManager::mousePressed(int x, int y, int button) {
	for (int i = 0; i < dialogues.size(); i++) {
		if (!dialogues[i]->getIsDialogueActive()) currentframe->mousePressed(x, y, button);
		if (dialogues[i]->getIsDialogueActive()) dialogues[i]->mousePressed(x, y, button);
	}
}

void gGUIManager::mouseDragged(int x, int y, int button) {
	for (int i = 0; i < dialogues.size(); i++) {
		if (!dialogues[i]->getIsDialogueActive()) currentframe->mouseDragged(x, y, button);
		if (dialogues[i]->getIsDialogueActive()) selecteddialogue = dialogues[i];
	}
	if (selecteddialogue) selecteddialogue->mouseDragged(x, y, button);
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
	currentframe->update();
	for (int i = 0; i < dialogues.size(); i++) {
		if (dialogues[i]->getIsDialogueActive()) {
			dialogues[i]->update();
			if (dialogues[i]->getButtonEvent() == gGUIDialogue::EVENT_EXIT) {
				dialogues[i]->setIsDialogueActive(false);
				dialogues[i]->setButtonEvent(gGUIDialogue::EVENT_NONE);
			}
			if (dialogues[i]->getButtonEvent() == gGUIDialogue::EVENT_MINIMIZE) {
				dialogues[i]->setIsDialogueActive(false);
				dialogues[i]->setButtonEvent(gGUIDialogue::EVENT_NONE);
			}
			if (dialogues[i]->getButtonEvent() == gGUIDialogue::EVENT_MAXIMIZE) {
				int titlebarheight = dialogues[i]->getTitleBar()->height;
				int buttonsbarheight = dialogues[i]->getButtonsBar()->height;
				int twidth = root->getAppManager()->getCurrentCanvas()->getScreenWidth();
				int theight = root->getAppManager()->getCurrentCanvas()->getScreenHeight() - titlebarheight - buttonsbarheight;
				int tleft = 0;
				int ttop = titlebarheight;
				dialogues[i]->transformDialogue(tleft, ttop, twidth, theight);

				dialogues[i]->setIsMaximized(true);

				dialogues[i]->resetTitleBar();
				dialogues[i]->resetButtonsBar();
				dialogues[i]->setButtonEvent(gGUIDialogue::EVENT_NONE);
			}
			if (dialogues[i]->getButtonEvent() == gGUIDialogue::EVENT_RESTORE) {
				int twidth = root->getAppManager()->getCurrentCanvas()->getScreenWidth() / 1 * 0.84f;
				int theight = root->getAppManager()->getCurrentCanvas()->getScreenHeight() / 1 * 0.84f;
				int tleft = (root->getAppManager()->getCurrentCanvas()->getScreenWidth() - twidth) / 2;
				int ttop = (root->getAppManager()->getCurrentCanvas()->getScreenHeight() - theight) / 2;
				dialogues[i]->transformDialogue(tleft, ttop, twidth, theight);

				dialogues[i]->setIsMaximized(false);

				dialogues[i]->resetTitleBar();
				dialogues[i]->resetButtonsBar();
				dialogues[i]->setButtonEvent(gGUIDialogue::EVENT_NONE);
			}
		}
	}
}

void gGUIManager::draw() {
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
	gBaseGUIObject::setFont(&themefont);
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

	themefont.loadFont("FreeSans.ttf", 11);

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
