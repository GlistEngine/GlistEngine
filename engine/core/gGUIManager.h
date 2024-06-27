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
#include <deque>

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
	static const int GUITHEME_LIGHT = 0, GUITHEME_DARK = 1, GUITHEME_LIGHTBLUE = 2, GUITHEME_DARKBLUE = 3,
	GUITHEME_UMUTCANTHEME = 4;
	static const int fontnum = 1;
	static const int fonttypenum = 5;

	enum {
		FONTTYPE_REGULAR,
		FONTTYPE_BOLD,
		FONTTYPE_ITALIC,
		FONTTYPE_BOLDITALIC
	};

	enum {
		FONT_FREESANS
	};

	gGUIManager(gBaseApp* root, int width, int height);
	virtual ~gGUIManager();

	void setTheme(int guiTheme);
	int getTheme();

	gFont* getFont(int fontNo, int fontType = FONTTYPE_REGULAR);

	void setCurrentFrame(gGUIFrame* currentFrame);
	void setCurrentFrame(gGUIFrame* currentFrame, int width, int height);
	void setupDialogue(gGUIDialogue* dialogue);
	gGUIFrame* getCurrentFrame();

	bool showDialogue(gGUIDialogue* dialogue);
	bool hideDialogue(gGUIDialogue* dialogue);

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

private:
	gBaseApp* root;
	gGUIFrame* currentframe;
	gGUIFrame emptyframe;
	gGUISizer emptysizer;
	int guitheme;

	static const int themenum = 5;
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
	gFont themefonts[fontnum][fonttypenum];
	void loadThemes();
	void resetTheme(int guiTheme);

	gGUISizer defdialoguesizer;

	std::deque<gGUIDialogue*> dialogues;
	std::deque<gGUIDialogue*> dialoguesshown;
	gGUIDialogue* selecteddialogue;

};

#endif /* CORE_GGUIMANAGER_H_ */
