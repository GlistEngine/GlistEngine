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

	title = "DIALOGUE BOX";
	message = "This is a dialogue box.";
	dialoguetype = DIALOGUETYPE_NONE;
	icontype = ICONTYPE_INFO;

	dialoguetypename[DIALOGUETYPE_NONE] = "none";
	dialoguetypename[DIALOGUETYPE_OK] = "ok";
	dialoguetypename[DIALOGUETYPE_OKCANCEL] = "okcancel";
	dialoguetypename[DIALOGUETYPE_YESNO] = "yesno";
	dialoguetypename[DIALOGUETYPE_YESNOCANCEL] = "yesnocancel";
	icontypename[ICONTYPE_INFO] = "info";
	icontypename[ICONTYPE_WARNING] = "warning";
	icontypename[ICONTYPE_ERROR] = "error";
	icontypename[ICONTYPE_QUESTION] = "question";


	exitevent = false;
	exitbuttonexittrigger = false;
	okbuttonexittrigger = false;
	yesbuttonexittrigger = false;
	nobuttonexittrigger = false;
	cancelbuttonexittrigger = false;

	initleft = left;
	inittop = top;

}

gGUIDialogue::~gGUIDialogue() {
}

void gGUIDialogue::update() {
	if(guisizer) guisizer->update();

	if (exitbuttonexittrigger && !newexitbutton->isPressed())  exitevent = true; exitbuttonexittrigger = false;
	if (newexitbutton->isPressed()) exitbuttonexittrigger = true;


}

void gGUIDialogue::draw() {
	if(guisizer) {

		gColor oldcolor = *renderer->getColor();
		renderer->setColor(textbackgroundcolor);
		gDrawRectangle(left, top - titlebar->height, width, titlebar->height, true);
		renderer->setColor(middlegroundcolor);
		gDrawRectangle(left, top, width, height + buttonsbar->height, true);

		// DIALOGUE TITLE
		renderer->setColor(fontcolor);
		// font->drawText(title, left + width / 12, top + height / 14);

		// DIALOGUE BORDERS
		gDrawLine(left, top - titlebar->height, right, top - titlebar->height);
		gDrawLine(left, bottom + buttonsbar->height, right, bottom + buttonsbar->height);
		gDrawLine(left, top - titlebar->height, left, bottom + buttonsbar->height);
		gDrawLine(right, top - titlebar->height, right, bottom + buttonsbar->height);
		// gDrawLine(left + width / 24, top + height / 8, right - width / 24, top + height / 8);
		// gDrawLine(left + width / 24, top + height * 3 / 4, right - width / 24, top + height * 3 / 4);

		renderer->setColor(&oldcolor);

		guisizer->draw();
	}

	if (titlebar) titlebar->draw();
	if (buttonsbar) buttonsbar->draw();
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

void gGUIDialogue::setInitLeft(int initLeft) {
	this->initleft = initLeft;
}

void gGUIDialogue::setInitTop(int initTop) {
	this->inittop = initTop;
}

void gGUIDialogue::showDialogue(std::string title, std::string message, int dialogueType, int iconType) {

	this->title = title;
	this->message = message;
	this->dialoguetype = dialogueType;
	this->icontype = iconType;

	guisizer->enableBackgroundFill(false);
	/*
	// EXIT BUTTON
	guisizer->setControl(0, 0, &exitbutton);
	exitbutton.setSize(width / 8, height / 8);
	exitbutton.left += width - (width / 8) - (width / 24);

	// DIALOGUE ICON
	dialogueicon.loadImage("dialogueicons/" + icontypename[iconType] + "tpicon.png");
	// newdialogueicon = resources.getIconImage(gGUIResources::ICON_INFO);

	// MESSAGE TEXT
	int linecount = (this->message.length() / 24) + 1; // One line for each 23 characters of message

	guisizer->setControl(1, 0, &messagetext);
	messagetext.setText(message);
	messagetext.setTextAlignment(gGUIText::TEXTALIGNMENT_CENTER);
	messagetext.width = width / 6;
	messagetext.height = height * 5 / (48 / 2);
	// 	messagetext.height = height * 5 / (48 / linecount);
	// messagetext.left += (width - messagetext.width) / 2;
	messagetext.top += height / 3;

	/* guisizer->setControl(1, 0, &messagetext);
	messagetext.setText(message);
	messagetext.width = width * 27 / 48;
	messagetext.height = height * 5 / (48 / linecount);
	messagetext.left += width * 3 / 8;
	messagetext.top += ((height * 5 / 8) - messagetext.height) / 2 + messagetext.height / 12;

	guisizer->setControl(2, 0, &buttonssizer);
	buttonssizer.setSize(1, 3);

	int buttonwidth = width / 4;
	int buttonheight = height / 7;
	int leftoffset = ((width / 3) - buttonwidth) / 2;
	int topoffset = ((height / 4) -  buttonheight) / 2;

	okbutton.setSize(buttonwidth, buttonheight);
	cancelbutton.setSize(buttonwidth, buttonheight);
	yesbutton.setSize(buttonwidth, buttonheight);
	nobutton.setSize(buttonwidth, buttonheight);

	// OK BUTTON
	if (dialoguetypename[dialoguetype] == "ok" || dialoguetypename[dialoguetype] == "okcancel") {
		if (dialoguetypename[dialoguetype] == "ok") buttonssizer.setControl(0, 1, &okbutton);
		else buttonssizer.setControl(0, 0, &okbutton);
		okbutton.left += leftoffset;
		okbutton.top += topoffset;
	}

	// CANCEL BUTTON
	if (dialoguetypename[dialoguetype] == "okcancel" || dialoguetypename[dialoguetype] == "yesnocancel") {
		buttonssizer.setControl(0, 2, &cancelbutton);
		cancelbutton.left += leftoffset;
		cancelbutton.top += topoffset;
	}

	// YES BUTTON
	if (dialoguetypename[dialoguetype] == "yesno" || dialoguetypename[dialoguetype] == "yesnocancel") {
		buttonssizer.setControl(0, 0, &yesbutton);
		yesbutton.left += leftoffset;
		yesbutton.top += topoffset;
	}

	// NO BUTTON
	if (dialoguetypename[dialoguetype] == "yesno" || dialoguetypename[dialoguetype] == "yesnocancel") {
		if (dialoguetypename[dialoguetype] == "yesno")buttonssizer.setControl(0, 2, &nobutton);
		else if (dialoguetypename[dialoguetype] == "yesnocancel") buttonssizer.setControl(0, 1, &nobutton);
		nobutton.left += leftoffset;
		nobutton.top += topoffset;
	}

	*/

}

void gGUIDialogue::mouseDragged(int x, int y, int button) {
	if (x > left - width && x < left + width - width / 8 && y >= top - height && y < top + height / 8) {
		int buttontopoffset = (6 * (height / 8)) + (((height / 4) -  (height / 7)) / 2);
		int buttonleftoffset = (((width / 3) - (width / 4)) / 2);

		left = x;
		top = y;
		right = x + width;
		bottom = y + height;

		guisizer->left = x;
		guisizer->top = y;
		guisizer->right = x + guisizer->width;
		guisizer->bottom = y + guisizer->height;

		titlebar->left = x;
		titlebar->top = y;
		titlebar->right = x + titlebar->width;
		titlebar->bottom = y + titlebar->height;

		buttonsbar->left = x;
		buttonsbar->top = y;
		buttonsbar->right = x + buttonsbar->width;
		buttonsbar->bottom = y + buttonsbar->height;
	}
}

void gGUIDialogue::setTitleBar(gGUIContainer* titleBar) {
	this->titlebar = titleBar;
	titlebar->height = height * 0.06;
	titlebar->set(root, this, this, 0, 0,
				left,
				top - titlebar->height,
				width,
				titlebar->height
		);
}

void gGUIDialogue::setButtonsBar(gGUIContainer* buttonsBar) {
	this->buttonsbar = buttonsBar;
	buttonsbar->height = height * 0.1;
	buttonsbar->set(root, this, this, 0, 0,
				left,
				top + height,
				width,
				buttonsbar->height
		);
}

void gGUIDialogue::setExitButton(gGUIImageButton* exitButton) {
	this->newexitbutton = exitButton;
}
