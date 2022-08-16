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

	exitbutton.setButtonColor(backgroundcolor);
	okbutton.setButtonColor(backgroundcolor);
	yesbutton.setButtonColor(backgroundcolor);
	nobutton.setButtonColor(backgroundcolor);
	cancelbutton.setButtonColor(backgroundcolor);

	exitbutton.setTitle("X");
	okbutton.setTitle("OK");
	yesbutton.setTitle("YES");
	nobutton.setTitle("NO");
	cancelbutton.setTitle("CANCEL");

	exitevent = false;
	exitbuttonexittrigger = false;
	okbuttonexittrigger = false;
	yesbuttonexittrigger = false;
	nobuttonexittrigger = false;
	cancelbuttonexittrigger = false;

	stdsizer.setSize(3, 3);
	float stdlineproportions[3] = {0.125f, 0.625f, 0.250f};
	stdsizer.setLineProportions(stdlineproportions);

	initleft = left;
	inittop = top;
}

gGUIDialogue::~gGUIDialogue() {
}

void gGUIDialogue::update() {
	if(guisizer) guisizer->update();

	/* if (exitbutton.isPressed() || okbutton.isPressed() || yesbutton.isPressed() || nobutton.isPressed() || cancelbutton.isPressed()) {
		exittrigger = true;

		/* left = initleft;
		top = inittop;
		right = initleft + width;
		bottom = inittop + height;

		guisizer->left = initleft;
		guisizer->top = inittop;
		guisizer->right = guisizer->left + initleft;
		guisizer->bottom = guisizer->top + inittop;

		exitbutton.left = (width - (width / 8) - (width / 24));
		exitbutton.top = 0;
		exitbutton.right = exitbutton.left + exitbutton.width;
		exitbutton.bottom = exitbutton.top + exitbutton.height;

		messagetext.left = 0;
		messagetext.top = (height / 8) + (height / 3);
		messagetext.right = messagetext.left + messagetext.width;
		messagetext.bottom = messagetext.top + messagetext.height;

		buttonssizer.left = 0;
		buttonssizer.top = (6 * (height / 8));
		buttonssizer.right = buttonssizer.left + buttonssizer.width;
		buttonssizer.bottom = buttonssizer.top + buttonssizer.height;

		if (dialoguetypename[dialoguetype] == "ok") okbutton.left = (width / 3) + (((width / 3) - (width / 4)) / 2);
		else okbutton.left = (((width / 3) - (width / 4)) / 2);
		okbutton.top = (6 * (height / 8)) + (((height / 4) -  (height / 7)) / 2);
		okbutton.right = okbutton.left + okbutton.width;
		okbutton.bottom = okbutton.top + okbutton.height;

		cancelbutton.left = (2 * (width / 3)) + (((width / 3) - (width / 4)) / 2);
		cancelbutton.top = (6 * (height / 8)) + (((height / 4) -  (height / 7)) / 2);
		cancelbutton.right = cancelbutton.left + cancelbutton.width;
		cancelbutton.bottom = cancelbutton.top + cancelbutton.height;

		yesbutton.left = (((width / 3) - (width / 4)) / 2);
		yesbutton.top = (6 * (height / 8)) + (((height / 4) -  (height / 7)) / 2);
		yesbutton.right = yesbutton.left + yesbutton.width;
		yesbutton.bottom = yesbutton.top + yesbutton.height;

		if (dialoguetypename[dialoguetype] == "yesno") nobutton.left = (2 * (width / 3)) + (((width / 3) - (width / 4)) / 2);
		else nobutton.left = (width / 3) + (((width / 3) - (width / 4)) / 2);
		nobutton.top = (6 * (height / 8)) + (((height / 4) -  (height / 7)) / 2);
		nobutton.right = nobutton.left + nobutton.width;
		nobutton.bottom = nobutton.right + nobutton.height;
	} */

	if (exitbuttonexittrigger && !exitbutton.isPressed())  exitevent = true; exitbuttonexittrigger = false;
	if (exitbutton.isPressed()) exitbuttonexittrigger = true;

	if (okbuttonexittrigger && !okbutton.isPressed())  exitevent = true; okbuttonexittrigger = false;
	if (okbutton.isPressed()) okbuttonexittrigger = true;

	if (yesbuttonexittrigger && !yesbutton.isPressed())  exitevent = true; yesbuttonexittrigger = false;
	if (yesbutton.isPressed()) yesbuttonexittrigger = true;

	if (nobuttonexittrigger && !nobutton.isPressed())  exitevent = true; nobuttonexittrigger = false;
	if (nobutton.isPressed()) nobuttonexittrigger = true;

	if (cancelbuttonexittrigger && !cancelbutton.isPressed())  exitevent = true; cancelbuttonexittrigger = false;
	if (cancelbutton.isPressed()) cancelbuttonexittrigger = true;

}

void gGUIDialogue::draw() {
	if(guisizer) {
		guisizer->draw();

		gColor oldcolor = *renderer->getColor();

		// DIALOGUE TITLE
		renderer->setColor(fontcolor);
		font->drawText(title, left + width / 12, top + height / 14);

		// COLORS FOR DRAWING
		gColor strokecolor = gColor(0.5f, 0.5f, 0.5f);
		gColor okcolor = gColor(0.0f, 0.0f, 0.8f);
		gColor yescolor = gColor(0.5f, 0.8f, 0.5f);
		gColor nocolor = gColor(0.8f, 0.0f, 0.0f);

		// DIALOGUE BORDERS
		renderer->setColor(strokecolor);
		gDrawLine(left, top, right, top);
		gDrawLine(left, bottom, right, bottom);
		gDrawLine(left, top, left, bottom);
		gDrawLine(right, top, right, bottom);
		gDrawLine(left + width / 24, top + height / 8, right - width / 24, top + height / 8);
		gDrawLine(left + width / 24, top + height * 3 / 4, right - width / 24, top + height * 3 / 4);

		int defbuttontopoffset = top + (6 * (height / 8)) + (((height / 4) -  (height / 7)) / 2) + (height / 7);

		// BUTTON UNDERLINES

		// LEFT BUTTON
		if (dialoguetypename[dialoguetype] == "okcancel" || dialoguetypename[dialoguetype] == "yesno" || dialoguetypename[dialoguetype] == "yesnocancel") {
			if (dialoguetypename[dialoguetype] == "okcancel") renderer->setColor(okcolor);
			else if (dialoguetypename[dialoguetype] == "yesno" || dialoguetypename[dialoguetype] == "yesnocancel") renderer->setColor(yescolor);
			gDrawLine(
					left + (((width / 3) - (width / 4)) / 2) + width / 48,
					defbuttontopoffset,
					left + (((width / 3) - (width / 4)) / 2) + (width / 4) - width / 48,
					defbuttontopoffset
					);
			renderer->setColor(strokecolor);
		}

		// MID BUTTON
		if (dialoguetypename[dialoguetype] == "ok" || dialoguetypename[dialoguetype] == "yesnocancel") {
			if (dialoguetypename[dialoguetype] == "ok") renderer->setColor(okcolor);
			else if (dialoguetypename[dialoguetype] == "yesnocancel") renderer->setColor(nocolor);
			gDrawLine(
					left + (width / 3) + (((width / 3) - (width / 4)) / 2) + width / 48,
					defbuttontopoffset,
					left + (width / 3) + (((width / 3) - (width / 4)) / 2) + (width / 4) - width / 48,
					defbuttontopoffset
					);
			renderer->setColor(strokecolor);
		}

		// RIGHT BUTTON
		if (dialoguetypename[dialoguetype] == "okcancel" || dialoguetypename[dialoguetype] == "yesno" || dialoguetypename[dialoguetype] == "yesnocancel") {
			if (dialoguetypename[dialoguetype] == "yesno") renderer->setColor(nocolor);
			gDrawLine(
					left + (2 * (width / 3)) + (((width / 3) - (width / 4)) / 2) + width / 48,
					defbuttontopoffset,
					left + (2 * (width / 3)) + (((width / 3) - (width / 4)) / 2) + (width / 4) - width / 48,
					defbuttontopoffset
					);
			renderer->setColor(strokecolor);
		}

		renderer->setColor(&oldcolor);

		dialogueicon.draw(left + (width - height / 4) / 2, top + height / 6, height / 4, height / 4);
	}
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

	// EXIT BUTTON
	guisizer->setControl(0, 0, &exitbutton);
	exitbutton.setSize(width / 8, height / 8);
	exitbutton.left += width - (width / 8) - (width / 24);

	// DIALOGUE ICON
	dialogueicon.loadImage("dialogueicons/" + icontypename[iconType] + "tpicon.png");

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
	messagetext.top += ((height * 5 / 8) - messagetext.height) / 2 + messagetext.height / 12; */

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

		exitbutton.left = x + (width - (width / 8) - (width / 24));
		exitbutton.top = y;
		exitbutton.right = exitbutton.left + exitbutton.width;
		exitbutton.bottom = exitbutton.top + exitbutton.height;

		messagetext.left = x;
		messagetext.top = y + (height / 8) + (height / 3);
		messagetext.right = messagetext.left + messagetext.width;
		messagetext.bottom = messagetext.top + messagetext.height;

		buttonssizer.left = x;
		buttonssizer.top = y + (6 * (height / 8));
		buttonssizer.right = buttonssizer.left + buttonssizer.width;
		buttonssizer.bottom = buttonssizer.top + buttonssizer.height;

		if (dialoguetypename[dialoguetype] == "ok") okbutton.left = x + (width / 3) + buttonleftoffset;
		else okbutton.left = x + buttonleftoffset;
		okbutton.top = y + buttontopoffset;
		okbutton.right = okbutton.left + okbutton.width;
		okbutton.bottom = okbutton.top + okbutton.height;

		cancelbutton.left = x + (2 * (width / 3)) + buttonleftoffset;
		cancelbutton.top = y + buttontopoffset;
		cancelbutton.right = cancelbutton.left + cancelbutton.width;
		cancelbutton.bottom = cancelbutton.top + cancelbutton.height;

		yesbutton.left = x + buttonleftoffset;
		yesbutton.top = y + buttontopoffset;
		yesbutton.right = yesbutton.left + yesbutton.width;
		yesbutton.bottom = yesbutton.top + yesbutton.height;

		if (dialoguetypename[dialoguetype] == "yesno") nobutton.left = x + (2 * (width / 3)) + buttonleftoffset;
		else nobutton.left = x + (width / 3) + buttonleftoffset;
		nobutton.top = y + buttontopoffset;
		nobutton.right = nobutton.left + nobutton.width;
		nobutton.bottom = nobutton.right + nobutton.height;
	}
}
