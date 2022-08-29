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

	void setMinimizeEvent(bool minimizeEvent);
	bool getMinimizeEvent();
	void setMaximizeEvent(bool maximizeEvent);
	bool getMaximizeEvent();
	void setExitEvent(bool exitEvent);
	bool getExitEvent();

	void showDialogue();
	void mouseDragged(int x, int y, int button);

	void setTitleBar(gGUIContainer* titleBar);
	void setButtonsBar(gGUIContainer* buttonsBar);
	void resetTitleBar();
	void resetButtonsBar();

	void setMinimizeButton(gGUIImageButton* minimizeButton);
	void setMaximizeButton(gGUIImageButton* maximizeButton);
	void setExitButton(gGUIImageButton* exitButton);

	void mouseMoved(int x, int y);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
private:
	bool minimizeevent;
	bool maximizeevent;
	bool exitevent;

	bool exitbuttonexittrigger;
	bool minimizebuttonminimizetrigger;

	gGUIContainer* titlebar;
	gGUIContainer* buttonsbar;

	gGUIImageButton* minimizebutton;
	gGUIImageButton* maximizebutton;
	gGUIImageButton* exitbutton;

	// DEFAULT TITLEBAR ELEMENTS
	gGUIContainer deftitlebar;
	gGUISizer deftitlebarsizer;
	gGUIBitmap deftitlebarbitmap;
	gGUIText deftitlebartext;
	gGUIImageButton deftitlebarminimizebutton;
	gGUIImageButton deftitlebarmaximizebutton;
	gGUIImageButton deftitlebarexitbutton;

	// DEFAULT BUTTONSBAR ELEMENTS
	gGUIContainer defbuttonsbar;
	gGUISizer defbuttonsbarsizer;
	gGUIButton defbuttonsbarokbutton;
};

#endif /* UI_GGUIDIALOGUE_H_ */
