/*
 * gGUIDialogue.cpp
 *
 *  Created on: 27 Tem 2022
 *      Author: Umut Can
 */

#include "gGUIDialogue.h"
#include "gGUISizer.h"
#include "gGUIMenubar.h"
#include "gGUIToolbar.h"

gGUIDialogue::gGUIDialogue() {
	guisizer = nullptr;
	menubar = nullptr;

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
	gLogi("Dialogue") << "draw";

	if(guisizer) guisizer->draw();
	if(toolbarnum > 0) for(int i = 0; i < toolbarnum; i++) toolbars[i]->draw();
	if(menubar) menubar->draw();
}

void gGUIDialogue::setMessage(std::string message) {
	this->message = message;
}

std::string gGUIDialogue::getMessage() {
	return message;
}

void gGUIDialogue::showDialogue(std::string title, std::string message) {
	gLogi("Dialogue") << "showDialogue";
}
