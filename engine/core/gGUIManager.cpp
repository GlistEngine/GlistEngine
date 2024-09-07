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

gFont* gGUIManager::getFont(int fontNo, int fontType) {
	return &themefonts[fontNo][fontType];
}

void gGUIManager::setCurrentFrame(gGUIFrame* currentFrame) {
	setCurrentFrame(currentFrame,
			currentFrame->getScreenWidth(),
			currentFrame->getScreenHeight());
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
	dialogue->width = appmanager->getCurrentCanvas()->getScreenWidth() * 0.39f;
	dialogue->height = appmanager->getCurrentCanvas()->getScreenHeight() * 0.21f;
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
	dialogue->resetMessageBar();

	dialogues.push_back(dialogue);
}

gGUIFrame* gGUIManager::getCurrentFrame() {
	return currentframe;
}

bool gGUIManager::showDialogue(gGUIDialogue* dialogue) {
	dialoguesshown.push_back(dialogue);
	return true;
}

bool gGUIManager::hideDialogue(gGUIDialogue* dialogue) {
	if(dialoguesshown.empty()) return false;
	if(dialoguesshown[dialoguesshown.size() - 1] == dialogue) {
		dialoguesshown.pop_back();
		return true;
	}
	return false;
}

void gGUIManager::keyPressed(int key) {
	if(!dialoguesshown.empty()) {
		dialoguesshown[dialoguesshown.size() - 1]->keyPressed(key);
		return;
	}
	currentframe->keyPressed(key);
}

void gGUIManager::keyReleased(int key) {
	if(!dialoguesshown.empty()) {
		dialoguesshown[dialoguesshown.size() - 1]->keyReleased(key);
		return;
	}
	currentframe->keyReleased(key);
}

void gGUIManager::charPressed(unsigned int key) {
	if(!dialoguesshown.empty()) {
		dialoguesshown[dialoguesshown.size() - 1]->charPressed(key);
		return;
	}
	currentframe->charPressed(key);
}

void gGUIManager::mouseMoved(int x, int y) {
	if(!dialoguesshown.empty()) {
		appmanager->setCursor(dialoguesshown[dialoguesshown.size() - 1]->getCursor(x, y));
		dialoguesshown[dialoguesshown.size() - 1]->mouseMoved(x, y);
		return;
	}
	appmanager->setCursor(currentframe->getCursor(x, y));
	currentframe->mouseMoved(x, y);
}

void gGUIManager::mousePressed(int x, int y, int button) {
	if(!dialoguesshown.empty()) {
		dialoguesshown[dialoguesshown.size() - 1]->mousePressed(x, y, button);
		return;
	}
	currentframe->mousePressed(x, y, button);
}

void gGUIManager::mouseDragged(int x, int y, int button) {
	if(!dialoguesshown.empty()) {
		dialoguesshown[dialoguesshown.size() - 1]->mouseDragged(x, y, button);
		return;
	}
	currentframe->mouseDragged(x, y, button);
}

void gGUIManager::mouseReleased(int x, int y, int button) {
	if(!dialoguesshown.empty()) {
		dialoguesshown[dialoguesshown.size() - 1]->mouseReleased(x, y, button);
		return;
	}
	currentframe->mouseReleased(x, y, button);
}

void gGUIManager::mouseScrolled(int x, int y) {
	if(!dialoguesshown.empty()) {
		dialoguesshown[dialoguesshown.size() - 1]->mouseScrolled(x, y);
		return;
	}
	currentframe->mouseScrolled(x, y);
}

void gGUIManager::mouseEntered() {
	if(!dialoguesshown.empty()) {
		dialoguesshown[dialoguesshown.size() - 1]->mouseEntered();
		return;
	}
	currentframe->mouseEntered();
}

void gGUIManager::mouseExited() {
	if(!dialoguesshown.empty()) {
		dialoguesshown[dialoguesshown.size() - 1]->mouseExited();
		return;
	}
	currentframe->mouseExited();
}

void gGUIManager::windowResized(int w, int h) {
	currentframe->width = w;
	currentframe->height = h;
	currentframe->windowResized(w, h);
}

void gGUIManager::update() {
	if(!isframeset) return;

	currentframe->update();

	selecteddialogue = nullptr;
	if(!dialoguesshown.empty()) selecteddialogue = dialoguesshown[dialoguesshown.size() - 1];

	if (selecteddialogue != nullptr) {
		selecteddialogue->update();
		if (selecteddialogue->getButtonEvent() == gGUIDialogue::EVENT_EXIT) {
			selecteddialogue->hide();
			selecteddialogue->setButtonEvent(gGUIDialogue::EVENT_NONE);
		}
		if (selecteddialogue->getButtonEvent() == gGUIDialogue::EVENT_MINIMIZE) {
			selecteddialogue->hide();
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
			int twidth = appmanager->getCurrentCanvas()->getScreenWidth() / 1 * 0.39f;
			int theight = appmanager->getCurrentCanvas()->getScreenHeight() / 1 * 0.21f;
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
	if(!dialoguesshown.empty()) {
		for (int i = 0; i < dialoguesshown.size(); i++) {
			dialoguesshown[i]->draw();
		}
	}
}

void gGUIManager::resetTheme(int guiTheme) {
	guitheme = guiTheme;
	gBaseGUIObject::setBackgroundColor(&themebackgroundcolor[guitheme]);
	gBaseGUIObject::setMiddlegroundColor(&thememiddlegroundcolor[guitheme]);
	gBaseGUIObject::setForegroundColor(&themeforegroundcolor[guitheme]);
	gBaseGUIObject::setTextBackgroundColor(&themetextbackgroundcolor[guitheme]);
	gBaseGUIObject::setNavigationBackgroundColor(&themenavigationbackgroundcolor[guitheme]);
	gBaseGUIObject::setFont(&themefonts[FONT_FREESANS][FONTTYPE_REGULAR]);
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

	themefonts[FONT_FREESANS][FONTTYPE_REGULAR].loadFont("FreeSans.ttf", 11);
	themefonts[FONT_FREESANS][FONTTYPE_BOLD].loadFont("FreeSansBold.ttf", 11);
	themefonts[FONT_FREESANS][FONTTYPE_ITALIC].loadFont("FreeSansOblique.ttf", 11);
	themefonts[FONT_FREESANS][FONTTYPE_BOLDITALIC].loadFont("FreeSansBoldOblique.ttf", 11);

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
