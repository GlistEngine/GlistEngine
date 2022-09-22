/*
 * gGUIStatusBar.h
 *
 *  Created on: 27 Jul 2022
 *      Author: Batuhan Sarihan
 */

#ifndef UI_GGUISTATUSBAR_H_
#define UI_GGUISTATUSBAR_H_

#include "gGUIContainer.h"
#include "gFont.h"


class gGUIStatusBar: public gGUIContainer {
public:
	gGUIStatusBar();
	virtual ~gGUIStatusBar();

	void setText(int index, std::string text);

	void draw();
	void windowResized(int w, int h);
	void mousePressed(int x, int y, int button);
	void mouseDragged(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void keyPressed(int key);
	void keyReleased(int key);

private:
	//statusbar
	int statusbarh, statusbarw, statusbarx, statusbary;
	gGUISizer statussizer;
	void statusbarDraw();
	//text
	static const int textobjectsize = 4;
	int selectedtext;
	std::string text[textobjectsize];
	int textx[textobjectsize], texty[textobjectsize];
	int textsliderx[textobjectsize], textslidery[textobjectsize], textsliderw[textobjectsize], textsliderh[textobjectsize];
	void statusbarAllTextDraw();
	void updateStatusBarCoordinate(int w, int h);
	void updateTextCoordinate(int w, int h);
};

#endif /* UI_GGUISTATUSBAR_H_ */

