/*
 * gGUIDialogue.h
 *
 *  Created on: 27 Jul 2022
 *      Author: Umut Can
 */

#ifndef UI_GGUIDIALOGUE_H_
#define UI_GGUIDIALOGUE_H_

#include "gGUIForm.h"
#include "gGUIButton.h"
#include "gGUIPanel.h"
#include "gGUIText.h"
#include "gImage.h"


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

	int initleft;
	int inittop;

	gGUIDialogue();
	virtual ~gGUIDialogue();

	void update();
	void draw();

	void setMessage(std::string message);
	std::string getMessage();

	void setExitEvent(bool exitEvent);
	bool getExitEvent();

	void showDialogue(std::string title, std::string message, int dialogueType, int iconType);
	// void mouseDragged(int x, int y, int button);
private:
	std::string message;
	int dialoguetype;
	int icontype;
	std::string dialoguetypename[dialoguetypenum];
	std::string icontypename[icontypenum];

	gGUIButton exitbutton;
	gGUIButton okbutton;
	gGUIButton yesbutton;
	gGUIButton nobutton;
	gGUIButton cancelbutton;

	bool exitevent;

	gGUIText messagetext;

	gImage dialogueicon;

	bool imageloaded;
	bool dialogueshown;

	gGUISizer stdsizer;

	gGUISizer buttonssizer;
};

#endif /* UI_GGUIDIALOGUE_H_ */
