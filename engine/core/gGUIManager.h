/*
 * Copyright (C) 2016 Nitra Games Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef CORE_GGUIMANAGER_H_
#define CORE_GGUIMANAGER_H_

#include "gGUIFrame.h"
#include "gGUISizer.h"
#include "gGUIDialogue.h"
class gBaseApp;

/**
 * GUI Manager for GlistEngine
 *
 * GUI is still in progress. Do not use in production.
 *
 * In order to use gui, you should create a full window frame and set it in the guimanager.
 * Then create a full window sizer and set it into the frame.
 * Finally put your controls into the sizer.
 * guimanager->frame->sizer->controls
 */
class gGUIManager {
public:
	static const int GUITHEME_LIGHT = 0, GUITHEME_DARK = 1, GUITHEME_LIGHTBLUE = 2, GUITHEME_DARKBLUE = 3;

	gGUIManager(gBaseApp* root);
	virtual ~gGUIManager();

	void setTheme(int guiTheme);
	int getTheme();

	void setCurrentFrame(gGUIFrame* currentFrame);
	void setActiveDialogue(gGUIDialogue* activeDialogue);
	gGUIFrame* getCurrentFrame();
	gGUIDialogue* getActiveDialogue();

	void setIsDialogueActive(bool isactive);

	void keyPressed(int key);
	void keyReleased(int key);
	void charPressed(unsigned int key);
	void mouseMoved(int x, int y);
	void mousePressed(int x, int y, int button);
	void mouseDragged(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void mouseScrolled(int x, int y);
	void mouseEntered();
	void mouseExited();
	void windowResized(int w, int h);

	void update();
	void draw();

	bool isframeset;
	bool isdialogueactive;

private:
	gBaseApp* root;
	gGUIFrame* currentframe;
	gGUIDialogue* activedialogue;
	gGUIFrame emptyframe;
	gGUISizer emptysizer;
	int guitheme;

	static const int themenum = 4;
	gColor themebackgroundcolor[themenum];
	gColor thememiddlegroundcolor[themenum];
	gColor themeforegroundcolor[themenum];
	gColor themetextbackgroundcolor[themenum];
	gColor themenavigationbackgroundcolor[themenum];
	gColor themefontcolor[themenum];
	gColor themenavigationfontcolor[themenum];
	gColor themebuttoncolor[themenum];
	gColor themepressedbuttoncolor[themenum];
	gColor themedisabledbuttoncolor[themenum];
	gColor themebuttonfontcolor[themenum];
	gColor themepressedbuttonfontcolor[themenum];
	gColor themedisabledbuttonfontcolor[themenum];
	gFont themefont;
	void loadThemes();
	void resetTheme(int guiTheme);
};

#endif /* CORE_GGUIMANAGER_H_ */
