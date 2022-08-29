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

	void setMessage(std::string message);
	std::string getMessage();

	void setExitEvent(bool exitEvent);
	bool getExitEvent();

	void setInitLeft(int initLeft);
	void setInitTop(int initTop);

	void showDialogue(std::string title, std::string message, int dialogueType, int iconType);
	void mouseDragged(int x, int y, int button);

	void setTitleBar(gGUIContainer* titleBar);
	void setButtonsBar(gGUIContainer* buttonsBar);

	void setExitButton(gGUIImageButton* exitButton);
private:
	std::string message;
	int dialoguetype;
	int icontype;

	std::string dialoguetypename[dialoguetypenum];
	std::string icontypename[icontypenum];

	bool exitevent;
	bool exitbuttonexittrigger;
	bool okbuttonexittrigger;
	bool yesbuttonexittrigger;
	bool nobuttonexittrigger;
	bool cancelbuttonexittrigger;

	int initleft;
	int inittop;

	gGUIContainer* titlebar;
	gGUIContainer* buttonsbar;

	gGUIImageButton* newexitbutton;
};

#endif /* UI_GGUIDIALOGUE_H_ */
