/*
 * gGUIDialogue.cpp
 *
 *  Created on: 27 Tem 2022
 *      Author: Umut Can
 */

#include "gGUIDialogue.h"
#include "gGUISizer.h"


gGUIDialogue::gGUIDialogue() {
	guisizer = nullptr;

	dialoguetypename[DIALOGUETYPE_OK] = "ok";
	dialoguetypename[DIALOGUETYPE_OKCANCEL] = "okcancel";
	dialoguetypename[DIALOGUETYPE_YESNO] = "yesno";
	dialoguetypename[DIALOGUETYPE_YESNOCANCEL] = "yesnocancel";
	icontypename[ICONTYPE_INFO] = "info";
	icontypename[ICONTYPE_WARNING] = "warning";
	icontypename[ICONTYPE_ERROR] = "error";
	icontypename[ICONTYPE_QUESTION] = "question";
}

gGUIDialogue::~gGUIDialogue() {
}

void gGUIDialogue::update() {
	if(guisizer) guisizer->update();
}

void gGUIDialogue::draw() {
	if(guisizer) guisizer->draw();

	gColor oldcolor = *renderer->getColor();
	renderer->setColor(fontcolor);
	font->drawText(title, left + 2, top + 12);
	renderer->setColor(&oldcolor);
}

void gGUIDialogue::setMessage(std::string message) {
	this->message = message;
}

std::string gGUIDialogue::getMessage() {
	return message;
}

void gGUIDialogue::showDialogue(std::string title, std::string message) {

	this->title = title;

	// PANEL
	guisizer->setControl(1, 0, &dialoguepanel);
	dialoguepanelsizer.setSize(1, 1);
	dialoguepanel.setSizer(&dialoguepanelsizer);

	// EXIT BUTTON
	guisizer->setControl(0, 0, &exitbutton);
	exitbutton.setButtonColor({255, 0, 0});
	exitbutton.setPressedButtonColor({128, 0, 0});
	exitbutton.setTitle("X");
	exitbutton.setSize(width / 8, height / 8);
	exitbutton.left += width - width / 8;
}
