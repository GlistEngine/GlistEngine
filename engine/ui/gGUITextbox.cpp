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
	cursorposutf = 0;
	text = "";
	cursorshowcounterlimit = 80;
	cursorshowlimit = 40;
	cursorshowcounter = 0;
	editmode = false;
	keystate = KEY_NONE;
	keypresstime = -1;
	keypresstimelimit1 = 50;
	keypresstimelimit2 = 55;
	selectionmode = false;
	selectionposchar1 = 0;
	selectionposchar2 = 0;
	selectionposx1 = 0;
	selectionposx2 = 0;
	selectionposutf1 = 0;
	selectionposutf2 = 0;
	selectionboxx1 = 0;
	selectionboxx2 = 0;
	selectionboxw = 0;
	shiftpressed = false;
	ctrlpressed = false;
	ctrlcpressed = false;
	ctrlvpressed = false;
	ctrlxpressed = false;
	ctrlapressed = false;
	isdragging = false;
	clicktimediff = 250;
	clicktime = gGetSystemTimeMillis();
	previousclicktime = clicktime - 2 * clicktimediff;
	firstclicktime = previousclicktime - 2 * clicktimediff;
	isdoubleclicked = false;
	istripleclicked = false;
	iseditable = true;
}

gGUITextbox::~gGUITextbox() {
}

void gGUITextbox::setText(const std::string& text) {
	this->text = text;
	letterlength.clear();
	std::vector<short> lettersize = readString(text);
	for(int i = 0; i < lettersize.size(); i++) letterlength.push_back(lettersize[i]);
	std::vector<int> newpos = calculateClickPosition(right - 1, top + 1);
	cursorposchar = newpos[0];
	cursorposx = newpos[1];
	cursorposutf = newpos[2];
}

std::string gGUITextbox::getText() {
	return text;
}

void gGUITextbox::setEditable(bool isEditable) {
	iseditable = isEditable;
}

bool gGUITextbox::isEditable() {
	return iseditable;
}

int gGUITextbox::getCursor(int x, int y) {
	if(iseditable && x >= left && x < right && y >= top && y < top + boxh) return CURSOR_IBEAM;
	return CURSOR_ARROW;
}

void gGUITextbox::update() {
	if(editmode) {
		cursorshowcounter++;
		if(cursorshowcounter >= cursorshowcounterlimit) {
			cursorshowcounter = 0;
		}

		handleKeys();
	}
}

void gGUITextbox::draw() {
	gColor oldcolor = *renderer->getColor();
	renderer->setColor(textbackgroundcolor);
	gDrawRectangle(left, top, width, boxh, true);
	renderer->setColor(foregroundcolor);
	gDrawRectangle(left, top, width, boxh, false);

	if(selectionmode) {
		if(selectionposx2 >= selectionposx1) {
			selectionboxx1 = selectionposx1;
			selectionboxw = selectionposx2 - selectionboxx1;
		} else {
			selectionboxx1 = selectionposx2;
			selectionboxw = selectionposx1 - selectionposx2;
		}
		if(isfocused) renderer->setColor(255, 128, 0);
		else renderer->setColor(middlegroundcolor);
		gDrawRectangle(left + selectionboxx1 + 5, top + 2, selectionboxw, boxh - 4, true);
	}

	renderer->setColor(fontcolor);
	font->drawText(text, left + 2, top + 14);

	if(editmode && (cursorshowcounter <= cursorshowlimit || keystate)) gDrawLine(left + 5 + cursorposx, top + 4, left + 5 + cursorposx, top + boxh - 4);
	renderer->setColor(&oldcolor);
}

void gGUITextbox::keyPressed(int key) {
//	gLogi("Textbox") << "keyPressed:" << key;
	if(key == G_KEY_C && ctrlpressed) ctrlcpressed = true;
	else if(key == G_KEY_V && ctrlpressed) ctrlvpressed = true;
	else if(key == G_KEY_X && ctrlpressed) ctrlxpressed = true;
	else if(key == G_KEY_A && ctrlpressed) ctrlapressed = true;

	int pressedkey = KEY_NONE;
	switch(key) {
	case G_KEY_BACKSPACE:
		pressedkey = KEY_BACKSPACE;
		keypresstime = 0;
		break;
	case G_KEY_LEFT:
		pressedkey = KEY_LEFT;
		keypresstime = 0;
		break;
	case G_KEY_RIGHT:
		pressedkey = KEY_RIGHT;
		keypresstime = 0;
		break;
	case G_KEY_DELETE:
		pressedkey = KEY_DELETE;
		keypresstime = 0;
		break;
	case G_KEY_ENTER:
		pressedkey = KEY_ENTER;
		keypresstime = 0;
		break;
	case G_KEY_LEFT_SHIFT:
		shiftpressed = true;
		startSelection();
		break;
	case G_KEY_RIGHT_SHIFT:
		shiftpressed = true;
		startSelection();
		break;
	case G_KEY_LEFT_CONTROL:
		ctrlpressed = true;
		break;
	case G_KEY_RIGHT_CONTROL:
		ctrlpressed = true;
		break;
	default:
		break;
	}
	keystate |= pressedkey;
}

void gGUITextbox::keyReleased(int key) {
	int pressedkey = KEY_NONE;
	switch(key) {
	case G_KEY_BACKSPACE:
		pressedkey = KEY_BACKSPACE;
		keypresstime = -1;
		break;
	case G_KEY_LEFT:
		pressedkey = KEY_LEFT;
		keypresstime = -1;
		break;
	case G_KEY_RIGHT:
		pressedkey = KEY_RIGHT;
		keypresstime = -1;
		break;
	case G_KEY_DELETE:
		pressedkey = KEY_DELETE;
		keypresstime = -1;
		break;
	case G_KEY_ENTER:
		pressedkey = KEY_ENTER;
		keypresstime = -1;
		break;
	case G_KEY_NP_ENTER:
		pressedkey = KEY_ENTER;
		keypresstime = -1;
		break;
	case G_KEY_LEFT_SHIFT:
		shiftpressed = false;
		break;
	case G_KEY_RIGHT_SHIFT:
		shiftpressed = false;
		break;
	case G_KEY_LEFT_CONTROL:
		ctrlpressed = false;
		break;
	case G_KEY_RIGHT_CONTROL:
		ctrlpressed = false;
		break;
	default:
		break;
	}
	keystate &= ~pressedkey;
}

void gGUITextbox::handleKeys() {
	if((selectionmode && (selectionposchar1 != selectionposchar2)) || isdoubleclicked || istripleclicked || ctrlapressed || ctrlvpressed) {
		if((isdoubleclicked || istripleclicked || ctrlcpressed || ctrlvpressed || ctrlxpressed || ctrlapressed || ctrlvpressed)) {
			pressKey();
		}
		ctrlcpressed = false;
		ctrlvpressed = false;
		ctrlxpressed = false;
		ctrlapressed = false;
		isdoubleclicked = false;
		istripleclicked = false;
		return;
	}

	if(keypresstime >= 0) {
		keypresstime++;
		if(keypresstime >= keypresstimelimit2) keypresstime = keypresstimelimit1;

		if(keypresstime == 1 || keypresstime == keypresstimelimit1) {
			pressKey();
		}
	}
}

void gGUITextbox::pressKey() {
	if((keystate & KEY_BACKSPACE) && (cursorposchar > 0 || (selectionmode && (selectionposchar1 > 0 || selectionposchar2 > 0)))) { // BACKDELETE
		if(selectionmode && selectionposchar1 != selectionposchar2) {
			int sepc1, sepx1, sepu1, sepc2, sepx2, sepu2;
			if(selectionposx2 >= selectionposx1) {
				sepc1 = selectionposchar1;
				sepx1 = selectionposx1;
				sepu1 = selectionposutf1;
				sepc2 = selectionposchar2;
				sepx2 = selectionposx2;
				sepu2 = selectionposutf2;
			} else {
				sepc1 = selectionposchar2;
				sepx1 = selectionposx2;
				sepu1 = selectionposutf2;
				sepc2 = selectionposchar1;
				sepx2 = selectionposx1;
				sepu2 = selectionposutf1;
			}
			std::string newtext = "";
			if(sepu1 > 0) newtext = text.substr(0, sepu1);
			if(sepu2 < text.length()) newtext += text.substr(sepu2, text.length() - sepu2 + 1);
			text = newtext;
			letterlength.erase(letterlength.begin() + sepc1, letterlength.begin() + sepc2);
			cursorposx = sepx1;
			cursorposutf = sepu1;
			cursorposchar = sepc1;
		} else {
			int cw = font->getStringWidth(text.substr(cursorposutf - letterlength[cursorposchar - 1], letterlength[cursorposchar - 1]));
			text = text.substr(0, cursorposutf - letterlength[cursorposchar - 1]) + text.substr(cursorposutf, text.length() - cursorposutf);
			cursorposx -= cw;
			cursorposutf -= letterlength[cursorposchar - 1];
			letterlength.erase(letterlength.begin() + cursorposchar - 1);
			cursorposchar--;
		}
		selectionmode = false;
	} else if((keystate & KEY_LEFT) && cursorposchar > 0) { // LEFT ARROW
		if(!shiftpressed) selectionmode = false;
		int cw = font->getStringWidth(text.substr(cursorposutf - letterlength[cursorposchar - 1], letterlength[cursorposchar - 1]));
		cursorposx -= cw;
		cursorposutf -= letterlength[cursorposchar - 1];
		cursorposchar--;
		if(selectionmode) {
			selectionposchar2 = cursorposchar;
			selectionposx2 = cursorposx;
			selectionposutf2 = cursorposutf;
		}
	} else if((keystate & KEY_RIGHT) && cursorposchar < letterlength.size()) { // RIGHT ARROW
		if(!shiftpressed) selectionmode = false;
		int cw = font->getStringWidth(text.substr(cursorposutf, letterlength[cursorposchar]));
		cursorposx += cw;
		cursorposutf += letterlength[cursorposchar];
		cursorposchar++;
		if(selectionmode) {
			selectionposchar2 = cursorposchar;
			selectionposx2 = cursorposx;
			selectionposutf2 = cursorposutf;
		}
	} else if((keystate & KEY_DELETE) && cursorposchar < letterlength.size()) { // DELETE
		if(selectionmode && selectionposchar1 != selectionposchar2) {
			int sepc1, sepx1, sepu1, sepc2, sepx2, sepu2;
			if(selectionposx2 >= selectionposx1) {
				sepc1 = selectionposchar1;
				sepx1 = selectionposx1;
				sepu1 = selectionposutf1;
				sepc2 = selectionposchar2;
				sepx2 = selectionposx2;
				sepu2 = selectionposutf2;
			} else {
				sepc1 = selectionposchar2;
				sepx1 = selectionposx2;
				sepu1 = selectionposutf2;
				sepc2 = selectionposchar1;
				sepx2 = selectionposx1;
				sepu2 = selectionposutf1;
			}
			std::string newtext = "";
			if(sepu1 > 0) newtext = text.substr(0, sepu1);
			if(sepu2 < text.length()) newtext += text.substr(sepu2, text.length() - sepu2 + 1);
			text = newtext;
			letterlength.erase(letterlength.begin() + sepc1, letterlength.begin() + sepc2);
			cursorposx = sepx1;
			cursorposutf = sepu1;
			cursorposchar = sepc1;
		} else {
			text = text.substr(0, cursorposutf) + text.substr(cursorposutf + letterlength[cursorposchar], text.length() - (cursorposutf + letterlength[cursorposchar]));
			letterlength.erase(letterlength.begin() + cursorposchar);
		}
		selectionmode = false;
	} else if((keystate & KEY_ENTER)) { // ENTER
		selectionmode = false;
		root->getCurrentCanvas()->onGuiEvent(id, G_GUIEVENT_TEXTBOXENTRY, text);
	} else if(ctrlcpressed) {
		int seput1 = selectionposutf1;
		int seput2 = selectionposutf2;
		if(selectionposutf1 > selectionposutf2) {
			seput1 = selectionposutf2;
			seput2 = selectionposutf1;
		}
		root->getAppManager()->setClipboardString(text.substr(seput1, seput2 - seput1));
	} else if(ctrlxpressed) {
		int seput1 = selectionposutf1;
		int seput2 = selectionposutf2;
		if(selectionposutf1 > selectionposutf2) {
			seput1 = selectionposutf2;
			seput2 = selectionposutf1;
		}
		root->getAppManager()->setClipboardString(text.substr(seput1, seput2 - seput1));
		int sepc1, sepx1, sepu1, sepc2, sepx2, sepu2;
		if(selectionposx2 >= selectionposx1) {
			sepc1 = selectionposchar1;
			sepx1 = selectionposx1;
			sepu1 = selectionposutf1;
			sepc2 = selectionposchar2;
			sepx2 = selectionposx2;
			sepu2 = selectionposutf2;
		} else {
			sepc1 = selectionposchar2;
			sepx1 = selectionposx2;
			sepu1 = selectionposutf2;
			sepc2 = selectionposchar1;
			sepx2 = selectionposx1;
			sepu2 = selectionposutf1;
		}
		std::string newtext = "";
		if(sepu1 > 0) newtext = text.substr(0, sepu1);
		if(sepu2 < text.length()) newtext += text.substr(sepu2, text.length() - sepu2 + 1);
		text = newtext;
		letterlength.erase(letterlength.begin() + sepc1, letterlength.begin() + sepc2);
		cursorposx = sepx1;
		cursorposutf = sepu1;
		cursorposchar = sepc1;
		selectionmode = false;
	} else if(ctrlapressed) {
		selectionmode = true;
		std::vector<int> clickpos = clickTextbox(left + 1, top + 1);
		selectionposchar1 = clickpos[0];
		selectionposx1 = clickpos[1];
		selectionposutf1 = clickpos[2];
		std::vector<int> clickpos2 = clickTextbox(right - 1, top + boxh - 1);
		selectionposchar2 = clickpos2[0];
		selectionposx2 = clickpos2[1];
		selectionposutf2 = clickpos2[2];
		cursorposchar = selectionposchar2;
		cursorposx = selectionposx2;
		cursorposutf = selectionposutf2;
	} else if(ctrlvpressed) {
		if(selectionmode && selectionposchar1 != selectionposchar2) {
			int sepc1, sepx1, sepu1, sepc2, sepx2, sepu2;
			if(selectionposx2 >= selectionposx1) {
				sepc1 = selectionposchar1;
				sepx1 = selectionposx1;
				sepu1 = selectionposutf1;
				sepc2 = selectionposchar2;
				sepx2 = selectionposx2;
				sepu2 = selectionposutf2;
			} else {
				sepc1 = selectionposchar2;
				sepx1 = selectionposx2;
				sepu1 = selectionposutf2;
				sepc2 = selectionposchar1;
				sepx2 = selectionposx1;
				sepu2 = selectionposutf1;
			}
			std::string newtext = "";
			if(sepu1 > 0) newtext = text.substr(0, sepu1);
			if(sepu2 < text.length()) newtext += text.substr(sepu2, text.length() - sepu2 + 1);
			text = newtext;
			letterlength.erase(letterlength.begin() + sepc1, letterlength.begin() + sepc2);
			cursorposx = sepx1;
			cursorposutf = sepu1;
			cursorposchar = sepc1;
		}

		std::string pastedtext = root->getAppManager()->getClipboardString();
		std::vector<short> lettersize = readString(pastedtext);
		int pastedtextw = font->getStringWidth(pastedtext);

		std::string firsttext = "";
		if(cursorposutf > 0) firsttext = text.substr(0, cursorposutf);
		std::string lasttext = "";
		if(cursorposutf < text.length()) lasttext = text.substr(cursorposutf, text.length() - cursorposutf + 1);
		text = firsttext + pastedtext + lasttext;

		if(text.length() == 0) {
			letterlength = lettersize;
			std::vector<int> newpos = calculateClickPosition(right - 1, top + 1);
			cursorposchar = newpos[0];
			cursorposx = newpos[1];
			cursorposutf = newpos[2];
		} else if(cursorposchar < letterlength.size()) {
			for(int i = lettersize.size() - 1; i >= 0; i--) letterlength.insert(letterlength.begin() + cursorposchar, lettersize[i]);
			std::vector<int> newpos = calculateClickPosition(left + cursorposx + pastedtextw, top + 1);
			cursorposchar = newpos[0];
			cursorposx = newpos[1];
			cursorposutf = newpos[2];
		} else {
			for(int i = 0; i < lettersize.size(); i++) letterlength.push_back(lettersize[i]);
			std::vector<int> newpos = calculateClickPosition(right - 1, top + 1);
			cursorposchar = newpos[0];
			cursorposx = newpos[1];
			cursorposutf = newpos[2];
		}
		selectionmode = false;
	} else if(isdoubleclicked) {
		bool leftchar = false;
		bool rightchar = false;
		if(cursorposchar > 0 && text[cursorposutf - 1] != ' ') leftchar = true;
		if(cursorposchar < text.length() && text[cursorposutf] != ' ') rightchar = true;
		if(!leftchar && !rightchar) return;
		selectionmode = true;
		selectionposchar1 = cursorposchar;
		selectionposx1 = cursorposx;
		selectionposutf1 = cursorposutf;
		selectionposchar2 = cursorposchar;
		selectionposx2 = cursorposx;
		selectionposutf2 = cursorposutf;

		if(leftchar) {
			std::vector<int> posdata = calculateClickPosition(left + cursorposx + 4, top + 1);
			int pdc = posdata[0];
			int pdx = posdata[1];
			int pdu = posdata[2];
			do {
				int tw = font->getStringWidth(text.substr(pdu - letterlength[pdc - 1], letterlength[pdc - 1]));
				pdx -= (tw + 2);
				std::vector<int> posdata2 = calculateClickPosition(left + pdx + 4, top + 1);
				pdc = posdata2[0];
				pdx = posdata2[1];
				pdu = posdata2[2];
			} while(pdc > 0 && text[pdu - 1] != ' ');
			selectionposchar1 = pdc;
			selectionposx1 = pdx;
			selectionposutf1 = pdu;
		}
		if(rightchar) {
			std::vector<int> posdata = calculateClickPosition(left + calculateLetterPosition(cursorposchar)[0], top + 1);
			int pdc = posdata[0];
			int pdx = posdata[1];
			int pdu = posdata[2];
			do {
				int tw = font->getStringWidth(text.substr(pdu, letterlength[pdc]));
				pdx += tw + 1;
				std::vector<int> posdata2 = calculateClickPosition(left + pdx, top + 1);
				pdc = posdata2[0];
				pdx = posdata2[1];
				pdu = posdata2[2];
			} while(pdc < letterlength.size() && text[pdu] != ' ');
			selectionposchar2 = pdc;
			selectionposx2 = pdx;
			selectionposutf2 = pdu;
		}
	} else if(istripleclicked) {
		selectionmode = true;
		std::vector<int> clickpos = clickTextbox(left + 1, top + 1);
		selectionposchar1 = clickpos[0];
		selectionposx1 = clickpos[1];
		selectionposutf1 = clickpos[2];
		std::vector<int> clickpos2 = clickTextbox(right - 1, top + boxh - 1);
		selectionposchar2 = clickpos2[0];
		selectionposx2 = clickpos2[1];
		selectionposutf2 = clickpos2[2];
		cursorposchar = selectionposchar2;
		cursorposx = selectionposx2;
		cursorposutf = selectionposutf2;
	}
}

void gGUITextbox::charPressed(unsigned int codepoint) {
//	gLogi("Textbox") << "charPressed:" << codepoint;
//	gLogi("Textbox") << "cp:" << gCodepointToStr(codepoint);
	if(editmode) {
		if(selectionmode && selectionposchar1 != selectionposchar2) {
			int sepc1, sepx1, sepu1, sepc2, sepx2, sepu2;
			if(selectionposx2 >= selectionposx1) {
				sepc1 = selectionposchar1;
				sepx1 = selectionposx1;
				sepu1 = selectionposutf1;
				sepc2 = selectionposchar2;
				sepx2 = selectionposx2;
				sepu2 = selectionposutf2;
			} else {
				sepc1 = selectionposchar2;
				sepx1 = selectionposx2;
				sepu1 = selectionposutf2;
				sepc2 = selectionposchar1;
				sepx2 = selectionposx1;
				sepu2 = selectionposutf1;
			}
			std::string newtext = "";
			if(sepu1 > 0) newtext = text.substr(0, sepu1);
			if(sepu2 < text.length()) newtext += text.substr(sepu2, text.length() - sepu2 + 1);
			text = newtext;
			letterlength.erase(letterlength.begin() + sepc1, letterlength.begin() + sepc2);
			cursorposx = sepx1;
			cursorposutf = sepu1;
			cursorposchar = sepc1;
			selectionmode = false;
		}

		int previouslengthutf = text.length();
		std::string newtext = "";
		if(cursorposchar > 0) newtext = text.substr(0, cursorposutf);
		newtext += gCodepointToStr(codepoint);
		if(cursorposchar < letterlength.size()) newtext += text.substr(cursorposutf, text.length() - cursorposutf);
		text = newtext;
		int diffutf = text.length() - previouslengthutf;
		if(cursorposchar == 0 || cursorposchar == letterlength.size()) letterlength.push_back(diffutf);
		else letterlength.insert(letterlength.begin() + cursorposchar, diffutf);
		cursorposchar++;
		cursorposutf += diffutf;
		cursorposx = font->getStringWidth(text.substr(0, cursorposutf));
	}
}

void gGUITextbox::mousePressed(int x, int y, int button) {
	if(!iseditable) return;

	if(x >= left && x < right && y >= top && y < top + boxh && button == 0) {
		firstclicktime = previousclicktime;
		previousclicktime = clicktime;
		clicktime = gGetSystemTimeMillis();
		if(clicktime - previousclicktime <= clicktimediff) {
			isdoubleclicked = true;
			if(previousclicktime - firstclicktime <= clicktimediff) {
				isdoubleclicked = false;
				istripleclicked = true;
			}
			return;
		}


		std::vector<int> clickpos = clickTextbox(x, y);
		if(editmode && shiftpressed) {
			if(!selectionmode) {
				selectionposchar1 = cursorposchar;
				selectionposx1 = cursorposx;
				selectionposutf1 = cursorposutf;
			}
			selectionposchar2 = clickpos[0];
			selectionposx2 = clickpos[1];
			selectionposutf2 = clickpos[2];
		} else {
			cursorposchar = clickpos[0];
			cursorposx = clickpos[1];
			cursorposutf = clickpos[2];
			selectionmode = false;
		}
		return;
	}
}

void gGUITextbox::mouseReleased(int x, int y, int button) {
	if(!iseditable) return;

	if(x >= left && x < right && y >= top && y < top + boxh && button == 0) {
		return;
	}
	if(!isdragging) editmode = false;
	isdragging = false;
}

void gGUITextbox::mouseDragged(int x, int y, int button) {
	if(editmode && x >= left && x < right && y >= top && y < top + boxh && button == 2) {
		if(!selectionmode) startSelection();

		std::vector<int> dragpos = calculateClickPosition(x, y);
		selectionposchar2 = dragpos[0];
		selectionposx2 = dragpos[1];
		selectionposutf2 = dragpos[2];
		isdragging = true;
	}
}

std::vector<int> gGUITextbox::clickTextbox(int x, int y) {
	if(!editmode) cursorshowcounter = 0;
	editmode = true;

	return calculateClickPosition(x, y);
}

std::vector<int> gGUITextbox::calculateClickPosition(int x, int y) {
	std::vector<int> result;
	for(int i = 0; i < 3; i++) result.push_back(0);
	if(letterlength.size() != 0) {
		int clickxdiff = x - left;
		int poschar = 0;
		int posutf = 0;
		int posx = 0;
		for(int i = 0; i < letterlength.size(); i++) {
			poschar += 1;
			posutf += letterlength[i];
			posx = font->getStringWidth(text.substr(0, posutf));
			if(posx >= clickxdiff) {
				poschar -= 1;
				posutf -= letterlength[i];
				posx = font->getStringWidth(text.substr(0, posutf));
				break;
			}
		}
		result[0] = poschar;
		result[1] = posx;
		result[2] = posutf;
	}
	return result;
}

std::vector<int> gGUITextbox::calculateLetterPosition(int letterNo) {
	std::vector<int> letterpos;

	int posutf = 0;
	int posx1 = 0;
	int posx2 = 0;
	for(int i = 0; i < letterNo; i++) {
		if(i > 0) posx1 = posx2;
		posutf += letterlength[i];
		posx2 = font->getStringWidth(text.substr(0, posutf));
	}

	letterpos.push_back(posx1);
	letterpos.push_back(posx2);

	return letterpos;
}

void gGUITextbox::startSelection() {
	if(!selectionmode) {
		selectionposchar1 = cursorposchar;
		selectionposx1 = cursorposx;
		selectionposutf1 = cursorposutf;
		selectionposchar2 = selectionposchar1;
		selectionposx2 = selectionposx1;
		selectionposutf2 = selectionposutf1;
		selectionmode = true;
	}
}

std::vector<short> gGUITextbox::readString(const std::string& str) {
	std::vector<short> lettersizes;
    int codepoints = 0;
    for(int i = 0; i < str.length(); i++) {
        if((str[i] & 0xC0) != 0x80) {
        	// not UTF-8 intermediate byte
        	int lsize = 1;
        	if(i < str.length() - 1 && (str[i + 1] & 0xC0) == 0x80) lsize = 2;
        	lettersizes.push_back(lsize);
//        	gLogi("Textbox") << "ls " << i << ":" << lsize;
            codepoints++;
        } else {
        }
    }
    return lettersizes;
}

