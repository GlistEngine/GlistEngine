/*
 * gGUIDialogue.h
 *
 *  Created on: 27 Tem 2022
 *      Author: Umut Can
 */

#ifndef UI_GGUIDIALOGUE_H_
#define UI_GGUIDIALOGUE_H_

#include "gGUIForm.h"
#include "gGUIButton.h"
#include "gGUIText.h"
#include "gGUIPanel.h"


class gGUIDialogue: public gGUIForm {
public:

	enum {
		DIALOGUETYPE_OK, DIALOGUETYPE_OKCANCEL, DIALOGUETYPE_YESNO, DIALOGUETYPE_YESNOCANCEL
	};

	enum {
		ICONTYPE_INFO, ICONTYPE_WARNING, ICONTYPE_ERROR, ICONTYPE_QUESTION
	};

	static const int dialoguetypenum = 4;
	static const int icontypenum = 4;

	gGUIDialogue();
	virtual ~gGUIDialogue();

	void update();
	void draw();

	void setMessage(std::string message);
	std::string getMessage();

	void showDialogue(std::string title, std::string message);
private:
	std::string message;
	std::string dialoguetypename[dialoguetypenum];
	std::string icontypename[icontypenum];

	gGUIButton exitbutton;
	gGUIButton rightbutton;
	gGUIButton midbutton;
	gGUIButton leftbutton;

	gGUIText dialoguetitletext;

	gGUIPanel dialoguepanel;
	gGUISizer dialoguepanelsizer;
};

#endif /* UI_GGUIDIALOGUE_H_ */
