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
	renderer->setColor(*backgroundcolor);
	gDrawRectangle(left, top, width, boxh, true);
	renderer->setColor(*fontcolor);
	font->drawText(text, left + 2, top + 13);
	if(editmode && cursorshowcounter <= cursorshowlimit) gDrawLine(left + 2 + cursorposx, top + 4, left + 2 + cursorposx, top + boxh - 4);
	renderer->setColor(oldcolor);
}

void gGUITextbox::keyPressed(int key) {
	if(editmode) {
//		gLogi("Textbox") << "keyPressed:" << key;
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
			root->getCurrentCanvas()->onGuiEvent(id, GUIEVENT_TEXTBOX_ENTRY, text);
		}
	}
}

void gGUITextbox::charPressed(unsigned int key) {
	if(editmode) {
//		gLogi("Textbox") << "charPressed:" << key;
		text += gCodepointToStr(key);
		cursorposx = left + 4 + font->getStringWidth(text);
		cursorposchar++;
	}
}

char gGUITextbox::toChars(unsigned int codePoint) {
	if (isBmpCodePoint(codePoint)) {
		gLogi("Textbox") << "BmpCodePoint";
		return (char)codePoint;
	} else {
		gLogi("Textbox") << "toSurrogates";
		return toSurrogates(codePoint);
	}
}

bool gGUITextbox::isBmpCodePoint(unsigned int codePoint) {
	return codePoint >> 8 == 0;
}

char gGUITextbox::toSurrogates(unsigned int codePoint) {
	// We write elements "backwards" to guarantee all-or-nothing
	char* c = new char[2];
	c[1] = lowSurrogate(codePoint);
	c[0] = highSurrogate(codePoint);
//	std::string str = (std::string)c;
//	std::string str;
//	str.append("" + c[0]);
//	str.append("" + c[1]);
//	using convert_typeX = std::codecvt_utf8<wchar_t>;
//	std::wstring_convert<convert_typeX, wchar_t> converterX;
//	return converterX.to_bytes(str);
	char c1 = *c;
	const char* c2 = (const char*)c;
//	delete c;
	gLogi("Textbox") << "cs:" << (std::string)c2 << ", c[1]:" << c[1];
	return c1;
}

const int gGUITextbox::MIN_HIGH_SURROGATE = 0x0000D800;
const int gGUITextbox::MIN_SUPPLEMENTARY_CODE_POINT = 0x010000;
const int gGUITextbox::MIN_LOW_SURROGATE  = 0x0000DC00;

char gGUITextbox::highSurrogate(unsigned int codePoint) {
	return (char) ((codePoint >> 10) + (MIN_HIGH_SURROGATE - (MIN_SUPPLEMENTARY_CODE_POINT >> 10)));
}

char gGUITextbox::lowSurrogate(unsigned int codePoint) {
	return (char) ((codePoint & 0x3ff) + MIN_LOW_SURROGATE);
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


