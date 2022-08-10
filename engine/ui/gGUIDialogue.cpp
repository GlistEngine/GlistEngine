/*
 * gGUIDialogue.cpp
 *
 *  Created on: 27 Jul 2022
 *      Author: Umut Can
 */

#include "gGUIDialogue.h"
#include "gGUISizer.h"


gGUIDialogue::gGUIDialogue() {
	guisizer = nullptr;

	dialoguetypename[DIALOGUETYPE_NONE] = "none";
	dialoguetypename[DIALOGUETYPE_OK] = "ok";
	dialoguetypename[DIALOGUETYPE_OKCANCEL] = "okcancel";
	dialoguetypename[DIALOGUETYPE_YESNO] = "yesno";
	dialoguetypename[DIALOGUETYPE_YESNOCANCEL] = "yesnocancel";
	icontypename[ICONTYPE_INFO] = "info";
	icontypename[ICONTYPE_WARNING] = "warning";
	icontypename[ICONTYPE_ERROR] = "error";
	icontypename[ICONTYPE_QUESTION] = "question";

	exitbutton.setButtonColor({1.0f, 0.0f, 0.0f});
	exitbutton.setPressedButtonColor({0.6f, 0.0f, 0.0f});

	exitbutton.setTitle("X");
	okbutton.setTitle("OK");
	yesbutton.setTitle("YES");
	nobutton.setTitle("NO");
	cancelbutton.setTitle("CANCEL");

	exitevent = false;

	imageloaded = false;
}

gGUIDialogue::~gGUIDialogue() {
}

void gGUIDialogue::update() {
	if(guisizer) guisizer->update();

	if (exitbutton.isPressed() || okbutton.isPressed() || yesbutton.isPressed() || nobutton.isPressed() || cancelbutton.isPressed()) exitevent = true;
}

void gGUIDialogue::draw() {
	if(guisizer) {
		guisizer->draw();

		// DIALOGUE TITLE
		gColor oldcolor = *renderer->getColor();
		renderer->setColor(middlegroundcolor);
		gDrawRectangle(left, top, width - width / 8, height / 8, width - width / 8, height / 8);
		renderer->setColor(fontcolor);
		font->drawText(title, left + width / 32, top + height / 14);

		// DIALOGUE BORDERS
		gDrawLine(left, top, right, top);
		gDrawLine(left, bottom, right, bottom);
		gDrawLine(left, top, left, bottom);
		gDrawLine(right, top, right, bottom);
		renderer->setColor(&oldcolor);
	}

	if (imageloaded) dialogueicon.draw(left + width / 12, top + height / 4, height / 3, height / 3);
}

void gGUIDialogue::setMessage(std::string message) {
	this->message = message;
}

std::string gGUIDialogue::getMessage() {
	return message;
}

void gGUIDialogue::setExitEvent(bool exitEvent) {
	this->exitevent = exitEvent;
}

bool gGUIDialogue::getExitEvent() {
	return exitevent;
}

void gGUIDialogue::showDialogue(std::string title, std::string message, int dialogueType, int iconType) {

	this->title = title;
	this->message = message;

	// EXIT BUTTON
	guisizer->setControl(0, 0, &exitbutton);
	exitbutton.setSize(width / 8, height / 8);
	exitbutton.left += width - width / 8;

	// DIALOGUE ICON
	dialogueicon.loadImage("dialogueicons/" + icontypename[iconType] + "tpicon.png");
	imageloaded = true;

	// MESSAGE TEXT
	// One line for each 24 characters of message, don't touch to width, only height
	int linecount = (message.length() / 24) + 1;
	gLogi("Line") << message.length();
	gLogi("Line") << linecount;

	guisizer->setControl(1, 0, &messagetext);
	messagetext.width = width * 27 / 48;
	// messagetext.height = height * 2 / 3; // FOR 4 LINES (MAX 96 CHARACTERS)
	messagetext.height = height * 2 / (12 / linecount);
	messagetext.left += width * 3 / 8;
	// messagetext.top += height * 2 / 15;	// FOR 4 LINES (MAX 96 CHARACTERS)
	gLogi("Top") << messagetext.top;
	messagetext.top += (height - messagetext.height) / 3;
	messagetext.setText(message);

	// BUTTONS PANEL
	guisizer->setControl(2, 0, &buttonspanel);
	buttonspanelsizer.setSize(1, 3);
	buttonspanelsizer.enableBorders(false);
	buttonspanelsizer.enableBackgroundFill(false);
	buttonspanel.setSizer(&buttonspanelsizer);

	// OK BUTTON
	if (dialoguetypename[dialogueType] == "ok" || dialoguetypename[dialogueType] == "okcancel") {
		buttonspanelsizer.setControl(0, 1, &okbutton);
		okbutton.setSize(width / 5, height / 5);
		okbutton.left += width / 15;
		okbutton.top -= height / 8;
	}

	// CANCEL BUTTON
	if (dialoguetypename[dialogueType] == "okcancel" || dialoguetypename[dialogueType] == "yesnocancel") {
		buttonspanelsizer.setControl(0, 2, &cancelbutton);
		cancelbutton.setSize(width / 5, height / 5);
		cancelbutton.left += width / 15;
		cancelbutton.top -= height / 8;
	}

	// YES BUTTON
	if (dialoguetypename[dialogueType] == "yesno" || dialoguetypename[dialogueType] == "yesnocancel") {
		if (dialoguetypename[dialogueType] == "yesno") buttonspanelsizer.setControl(0, 1, &yesbutton);
		else if (dialoguetypename[dialogueType] == "yesnocancel") buttonspanelsizer.setControl(0, 0, &yesbutton);
		yesbutton.setSize(width / 5, height / 5);
		yesbutton.left += width / 15;
		yesbutton.top -= height / 8;
	}

	// NO BUTTON
	if (dialoguetypename[dialogueType] == "yesno" || dialoguetypename[dialogueType] == "yesnocancel") {
		if (dialoguetypename[dialogueType] == "yesno") buttonspanelsizer.setControl(0, 2, &nobutton);
		else if (dialoguetypename[dialogueType] == "yesnocancel") buttonspanelsizer.setControl(0, 1, &nobutton);
		nobutton.setSize(width / 5, height / 5);
		nobutton.left += width / 15;
		nobutton.top -= height / 8;
	}

}
