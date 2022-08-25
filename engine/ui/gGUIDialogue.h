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


class gGUIDialogue: public gGUIForm {
public:

	enum {
		DIALOGUETYPE_NONE, DIALOGUETYPE_OK, DIALOGUETYPE_OKCANCEL, DIALOGUETYPE_YESNO, DIALOGUETYPE_YESNOCANCEL
	};

	enum {
		ICONTYPE_INFO, ICONTYPE_WARNING, ICONTYPE_ERROR, ICONTYPE_QUESTION
	};

	static const int dialoguetypenum = 5;
	static const int icontypenum = 4;

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

	void showDialogue(std::string title, std::string message, int dialogueType, int iconType);
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
	std::string message;
	int dialoguetype;
	int icontype;

	std::string dialoguetypename[dialoguetypenum];
	std::string icontypename[icontypenum];

	bool minimizeevent;
	bool maximizeevent;
	bool exitevent;

	bool exitbuttonexittrigger;

	gGUIContainer* titlebar;
	gGUIContainer* buttonsbar;

	// gGUIContainer deftitlebar;

	// gGUISizer deftitlebarsizer;
	// gGUIBitmap deftitlebarbitmap;
	// gGUIImageButton deftitlebarexitbutton;
	// gGUIText deftitlebartext;

	gGUIImageButton* minimizebutton;
	gGUIImageButton* maximizebutton;
	gGUIImageButton* exitbutton;
};

#endif /* UI_GGUIDIALOGUE_H_ */
