/*
 * gGUIDialogue.h
 *
 *  Created on: 27 Jul 2022
 *      Author: Umut Can
 */

#ifndef UI_GGUIDIALOGUE_H_
#define UI_GGUIDIALOGUE_H_

#include "gGUIForm.h"
#include "gGUIContainer.h"
#include "gGUIImageButton.h"
#include "gGUIBitmap.h"
#include "gGUIText.h"

class gGUIDialogue: public gGUIForm {
public:
	gGUIDialogue();
	virtual ~gGUIDialogue();

	void update();
	void draw();

	void setTitleBar(gGUIContainer* titleBar);
	gGUIContainer* getTitleBar();
	void setButtonsBar(gGUIContainer* buttonsBar);
	gGUIContainer* getButtonsBar();
	void resetTitleBar();
	void resetButtonsBar();

	void setMinimizeButton(gGUIImageButton* minimizeButton);
	void setMaximizeButton(gGUIImageButton* maximizeButton);
	void setExitButton(gGUIImageButton* exitButton);

	void setMinimizeEvent(bool minimizeEvent);
	bool getMinimizeEvent();
	void setMaximizeEvent(bool maximizeEvent);
	bool getMaximizeEvent();
	void setRestoreEvent(bool restoreEvent);
	bool getRestoreEvent();
	void setExitEvent(bool exitEvent);
	bool getExitEvent();

	void setIsMaximized(bool isMaximized);

	void mouseMoved(int x, int y);
	void mousePressed(int x, int y, int button);
	void mouseDragged(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
private:
	gGUIContainer* titlebar;
	gGUIContainer* buttonsbar;

	gGUIImageButton* minimizebutton;
	gGUIImageButton* maximizebutton;
	gGUIImageButton* exitbutton;

	gGUIContainer deftitlebar;
	gGUISizer deftitlebarsizer;
	gGUIBitmap deftitlebarbitmap;
	gGUIText deftitlebartext;
	gGUIImageButton deftitlebarminimizebutton;
	gGUIImageButton deftitlebarmaximizebutton;
	gGUIImageButton deftitlebarexitbutton;

	static const int deftitlebarheight = 27;
	static const int deftitlebarbitmapwidth = 38;
	static const int deftitlebarbuttonwidth = 48;
	static const int defbuttonsbarheight = 45;
	static const int defbuttonsbarbuttonwidth = 100;
	static const int defbuttonsbarbuttonheight = 27;

	gGUIContainer defbuttonsbar;
	gGUISizer defbuttonsbarsizer;
	gGUIButton defbuttonsbarokbutton;

	bool minimizeevent;
	bool maximizeevent;
	bool restoreevent;
	bool exitevent;

	bool minimizebuttonminimizetrigger;
	bool maximizebuttonmaximizetrigger;
	bool maximizebuttonrestoretrigger;
	bool exitbuttonexittrigger;

	bool ismaximized;
	bool isdragged;
	int dragposx;
	int dragposy;
};

#endif /* UI_GGUIDIALOGUE_H_ */
