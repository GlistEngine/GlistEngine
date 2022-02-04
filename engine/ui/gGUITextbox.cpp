/*
 * gTextbox.cpp
 *
 *  Created on: Sep 2, 2021
 *      Author: noyan
 */

#include "gGUITextbox.h"
#include <cwchar>
#include <codecvt>
#include "gBaseApp.h"
#include "gBaseCanvas.h"


gGUITextbox::gGUITextbox() {
	std::setlocale(LC_ALL, "en_US.UTF-8");
	boxw = width;
	boxh = 24;
	cursorposx = 0;
	cursorposchar = 0;
	text = "";
	cursorshowcounterlimit = 80;
	cursorshowlimit = 40;
	cursorshowcounter = 0;
	editmode = false;
}

gGUITextbox::~gGUITextbox() {
}

int gGUITextbox::getCursor(int x, int y) {
	if(x >= left && x < right && y >= top && y < top + boxh) return CURSOR_IBEAM;
	return CURSOR_ARROW;
}

void gGUITextbox::update() {
	if(editmode) {
		cursorshowcounter++;
		if(cursorshowcounter >= cursorshowcounterlimit) {
			cursorshowcounter = 0;
		}
	}
}

void gGUITextbox::draw() {
	gColor oldcolor = *renderer->getColor();
	renderer->setColor(backgroundcolor);
	gDrawRectangle(left, top, width, boxh, true);
	renderer->setColor(fontcolor);
	font->drawText(text, left + 2, top + 13);
	if(editmode && cursorshowcounter <= cursorshowlimit) gDrawLine(left + 2 + cursorposx, top + 4, left + 2 + cursorposx, top + boxh - 4);
	renderer->setColor(&oldcolor);
}

void gGUITextbox::keyPressed(int key) {
//	gLogi("Textbox") << "keyPressed:" << key;
	if(editmode) {
		if(key == 259 && cursorposchar > 0) { // BACKDELETE
			int cw = font->getStringWidth(text.substr(cursorposchar - 1, 1));
			text = text.substr(0, cursorposchar - 1) + text.substr(cursorposchar, text.length() - cursorposchar);
			cursorposx -= cw;
			cursorposchar--;
		} else if (key == 263 && cursorposchar > 0) { // LEFT ARROW
			int cw = font->getStringWidth(text.substr(cursorposchar - 1, 1));
			cursorposx -= cw;
			cursorposchar--;
		} else if (key == 262 && cursorposchar < text.length()) { // RIGHT ARROW
			int cw = font->getStringWidth(text.substr(cursorposchar, 1));
			cursorposx += cw;
			cursorposchar++;
		} else if (key == 261 && cursorposchar < text.length()) { // DELETE
			text = text.substr(0, cursorposchar) + text.substr(cursorposchar + 1, text.length() - cursorposchar + 1);
		} else if (key == 257 || key == 335) { // ENTER
			root->getCurrentCanvas()->onGuiEvent(id, G_GUIEVENT_TEXTBOXENTRY, text);
		}
	}
}

void gGUITextbox::charPressed(unsigned int codepoint) {
//	gLogi("Textbox") << "charPressed:" << codepoint;
//	gLogi("Textbox") << "cp:" << gCodepointToStr(codepoint);
	if(editmode) {
		text += gCodepointToStr(codepoint);
		cursorposx = left + 4 + font->getStringWidth(text);
		cursorposchar++;
	}
}

void gGUITextbox::mousePressed(int x, int y, int button) {
	if(x >= left && x < right && y >= top && y < top + boxh) {
		if(!editmode) cursorshowcounter = 0;
		editmode = true;
		return;
	}
	editmode = false;
}

void gGUITextbox::mouseReleased(int x, int y, int button) {

}


