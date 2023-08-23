/*
 * gTextbox.cpp
 *
 *  Created on: Sep 2, 2021
 *      Author: noyan
 */

#include "gGUITextbox.h"
#include "gAppManager.h"
#include "gBaseApp.h"
#include "gBaseCanvas.h"

#include <cwchar>
#include <codecvt>
#include <cctype>


gGUITextbox::gGUITextbox() {
	setlocale(LC_ALL, "en_US.UTF-8");
	boxw = width;
	boxh = 24;
	cursorposx = 0;
	cursorposy = 0;
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
	keypresstimelimit2 = 40;
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
	commandpressed = false;
	commandcpressed = false;
	commandvpressed = false;
	commandxpressed = false;
	commandapressed = false;
	commandzpressed = false;
	ctrlcpressed = false;
	ctrlvpressed = false;
	ctrlxpressed = false;
	ctrlapressed = false;
	ctrlzpressed = false;
	isdragging = false;
	clicktimediff = 250;
	clicktime = gGetSystemTimeMillis();
	previousclicktime = clicktime - 2 * clicktimediff;
	firstclicktime = previousclicktime - 2 * clicktimediff;
	isdoubleclicked = false;
	istripleclicked = false;
	iseditable = true;
	initx = 4;
	firstchar = 0;
	firstutf = 0;
	firstposx = 0;
	lastutf = 0;
	isselectedall = false;
	rowsnum = 1;
	lineheight = font->getStringHeight("ly");
	ismultiline = false;
	rowsnumexceeded = false;
	currentline = 1;
	linecount = 1;
	lastdrawnline = 1;
	linetopmargin = 4;
	lines.clear();
	lines.push_back("");
	leftlimit = 0;
	rightlimit = 0;
	toplimit = 0;
	bottomlimit = 0;
	isnumeric = false;
	ispassword = false;
	dotradius = 0;
	isbackgroundenabled = true;
	totalh = boxh;
	hdiff = boxh / 4;
	firstx = 0;
	firsty = 0;
	boxshrinked = false;
	boxexpanded = false;
	arrowkeypressed = false;
	arrowamount = 0;
	textalignment = TEXTALIGNMENT_LEFT;
	textalignmentamount = 5;
	cursormoveamount = 1;
	textmoveamount = 0;
	textcolor = fontcolor;
	colorset = false;
	setTextAlignment(textalignment, boxw, initx);
	
	widthexceeded = false;
	widthAdjusmentDelay = 0;
}


gGUITextbox::~gGUITextbox() {
}

void gGUITextbox::set(gBaseApp* root, gBaseGUIObject* topParentGUIObject, gBaseGUIObject* parentGUIObject, int parentSlotLineNo, int parentSlotColumnNo, int x, int y, int w, int h) {
	gGUIControl::set(root, topParentGUIObject, parentGUIObject, parentSlotLineNo, parentSlotColumnNo, x, y, w, h);
	leftlimit = left;
	rightlimit = right;
	toplimit = top;
	bottomlimit = bottom;
	arrowamount = 0;
	if(!root) return;
	manager = root->getAppManager()->getGUIManager();
	textfont = manager->getFont(gGUIManager::FONT_FREESANS);
}

void gGUITextbox::setText(const std::string& text) {
	this->text = text;
	letterlength.clear();
	letterpos.clear();
	letterlength = readString(text);
	letterpos = calculateAllLetterPositions();
	lastutf = calculateLastUtf();
	if(ismultiline) calculateLines();
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
	if(iseditable && x >= left && x < right && y >= top + hdiff && y < top + totalh + hdiff) return CURSOR_IBEAM;
	return CURSOR_ARROW;
}

void gGUITextbox::setLineCount(int linecount) {
	this->rowsnum = linecount;
	if(linecount > 1) {
		totalh = boxh + (lineheight + linetopmargin) * (linecount - 1);
		ismultiline = true;
		lines.clear();
		calculateLines();
	}
}

int gGUITextbox::getLineCount() {
	return rowsnum;
}

void gGUITextbox::setSize(int width, int height) {
	this->width = width;
	if(width > rightlimit - leftlimit) {
		this->width = rightlimit - leftlimit;
		left = leftlimit;
		right = rightlimit;
	} else if(this->width + left > rightlimit) {
		right = rightlimit;
		left = right - this->width;
	}

	this->height = height;
	if(height > bottomlimit - toplimit) {
		this->height = bottomlimit - toplimit;
		top = toplimit;
		bottom = bottomlimit;
	} else if(this->height + top > bottomlimit) {
		bottom = bottomlimit;
		top = bottom - this->height;
	}

	boxw = this->width;
	boxh = this->height;

	lastutf = calculateLastUtf();
	calculateLines();
}

void gGUITextbox::setNumeric(bool command) {
	isnumeric = command;
}

bool gGUITextbox::isNumeric() {
	return isnumeric;
}

void gGUITextbox::addLeftMargin(int leftmargin) {
	left += leftmargin;
	if(left < leftlimit) left = leftlimit;

	right = width + left;
	if(right > rightlimit) {
		right = rightlimit;
		left = right - width;
	}
}

void gGUITextbox::addTopMargin(int topmargin) {
	top += topmargin;
	if(top < toplimit) top = toplimit;

	bottom = height + top;
	if(bottom > bottomlimit) {
		bottom = bottomlimit;
		top = bottom - height;
	}
}

void gGUITextbox::setLineTopMargin(int linetopmargin) {
	this->linetopmargin = linetopmargin;
}

void gGUITextbox::setPassword(bool isPassword) {
	ispassword = isPassword;
	if(ismultiline) {
		rowsnum = 1;
		linecount = 1;
		lastdrawnline = 1;
		currentline = 1;
		ismultiline = false;
	}
	dotradius = lineheight / 3;
	cleanText();
}

bool gGUITextbox::isPassword() {
	return ispassword;
}

void gGUITextbox::update() {
	if(widthAdjusmentDelay < 5) widthAdjusmentDelay++;
	if(editmode) {
		cursorshowcounter++;
		if(cursorshowcounter >= cursorshowcounterlimit) {
			cursorshowcounter = 0;
		}

		handleKeys();
	}
	if(boxw - width > 0) boxshrinked = true;
	else if(boxw - width < 0) boxexpanded = true;
	boxw = width;
	if(textfont->getStringWidth(text.substr(firstutf, lastutf)) >= width - 2 * initx && boxshrinked && !ismultiline) {
		if(!ismultiline && !ispassword) {
			do {
				firstutf += letterlength[firstchar];
				firstposx = textfont->getStringWidth(text.substr(0, firstutf));
				firstchar++;
				cursorposx = textfont->getStringWidth(text.substr(firstutf, cursorposutf - firstutf));
			} while(cursorposx >= width - 2 * initx);
			letterpos.clear();
			letterpos = calculateAllLetterPositions();
			lastutf = calculateLastUtf();
		} else if(ispassword) {
			cursorposx -= 3 * dotradius;
		} else {
			cursorposx = textfont->getStringWidth(text.substr(firstutf, cursorposutf - firstutf));
		}
	} else if(boxexpanded && firstchar > 0) {
		if(!ismultiline && !ispassword) {
			do {
				firstutf -= letterlength[firstchar];
				firstposx = textfont->getStringWidth(text.substr(0, firstutf));
				firstchar--;
				cursorposx = textfont->getStringWidth(text.substr(firstutf, cursorposutf - firstutf));
				lastutf += letterlength[firstchar];
			} while(firstchar > 0 && textfont->getStringWidth(text.substr(firstchar, lastutf)) < width - 2 * initx);
			letterpos.clear();
			letterpos = calculateAllLetterPositions();
			lastutf = calculateLastUtf();
		}
	}

	if(ismultiline && boxshrinked) {
		for(int i = 0; i < linecount; i++){
			if(textfont->getStringWidth(lines[i]) >= width - 2 * initx) {
				setText(text);
				findCursorPosition();
				setText(text);
				break;
			}
		}
	} else if(ismultiline && boxexpanded && widthAdjusmentDelay > 3) {
		for(int i = 0; i < linecount; i++){
			if(textfont->getStringWidth(lines[i]) < width - 2 * initx) {
				setText(text);
				break;
			}
		}
		calculateLineCount();
	}
	boxshrinked = false;
	boxexpanded = false;

	while(totalh > height) {
		rowsnum--;
		totalh = boxh + (lineheight + linetopmargin) * (rowsnum - 1);
	}

	if(lastdrawnline > rowsnum) rowsnumexceeded = true;
	else rowsnumexceeded = false;

}

void gGUITextbox::calculateLineCount() {
	for(int i = linecount - 1; i >= 0 ; i--) {
		if(lines[i] == "") {
			if(lastdrawnline == linecount) lastdrawnline--;
			if(currentline == linecount) currentline--;
			linecount--;
		}
		else break;
	}
}

void gGUITextbox::enableBackground(bool isEnabled) {
	isbackgroundenabled = isEnabled;
}

bool gGUITextbox::isBackgroundEnabled() {
	return isbackgroundenabled;
}

void gGUITextbox::draw() {
	gColor oldcolor = *renderer->getColor();
	if(isbackgroundenabled) {
		renderer->setColor(foregroundcolor);
		gDrawRectangle(left - firstx, top + hdiff - firsty, width, boxh / 2 + totalh, false);
	}
	renderer->setColor(textbackgroundcolor);
	gDrawRectangle(left - firstx, top + hdiff - firsty, width,  totalh, true);

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
		int line = rowsnum - (lastdrawnline - currentline);
		bool firstline = false;
		if(line <= 1 || currentline == 1) firstline = true;
		//check selection box drawing
		gDrawRectangle(left + selectionboxx1 - firstx + textalignmentamount - (textfont->getStringWidth(text) / 2 * textalignment), top + hdiff + linetopmargin * firstline - firsty + lineheight * 3/2  * ((currentline - 1) * !rowsnumexceeded + (line - 1) * rowsnumexceeded) , selectionboxw, lineheight * 5 / 3, true);
		firstline = false;
	}
	if(!colorset) textcolor = fontcolor;
	renderer->setColor(textcolor);
	if(ispassword) {
		int doty = top + lineheight + linetopmargin;
		int dotlen = 3 * dotradius;
		int dotinit = 2 * dotradius + initx;
		int dotlimit = (right - left) / (3 * dotradius) - 1;
		if(dotlimit > text.size()) dotlimit = text.size();
		for(int i = 0; i < dotlimit; i++) gDrawCircle(left + dotinit + i * dotlen, doty, dotradius, true);
	} else if(rowsnum == 1) {
		textfont->drawText(text.substr(firstutf, lastutf), left - (cursorposx * textmoveamount - arrowamount) - textfont->getStringWidth(" ") / 2 - firstx + textalignmentamount, top + hdiff + lineheight + linetopmargin - firsty);
	} else {
		if(text.size() == 0) currentline = 1;
		for(int i = 0; i < rowsnum; i++) {
			if(lines[i] == "") continue;
			textfont->drawText(lines[i + (lastdrawnline - rowsnum) * rowsnumexceeded], left - (cursorposx * textmoveamount) - textfont->getStringWidth(" ") / 2 - firstx + textalignmentamount, top + hdiff + (i + 1) * (lineheight + linetopmargin) - firsty);
		}
	}

	if(editmode && (cursorshowcounter <= cursorshowlimit || keystate)) {
		int linebottom = 0;
		if(currentline <= rowsnum && lastdrawnline <= rowsnum) linebottom = top + hdiff +  currentline * (lineheight + linetopmargin) - firsty;
		else linebottom = top + hdiff + (rowsnum - (lastdrawnline - currentline)) * (lineheight + linetopmargin) - firsty;
		gDrawLine(left + (cursorposx * cursormoveamount + arrowamount) - firstx + textalignmentamount, linebottom - lineheight,
				left + (cursorposx * cursormoveamount + arrowamount) - firstx + textalignmentamount, linebottom + lineheight * 2 / 3);
	}
	renderer->setColor(&oldcolor);
}

void gGUITextbox::keyPressed(int key) {
//	gLogi("Textbox") << "keyPressed:" << key;
	if(key == G_KEY_C && ctrlpressed) ctrlcpressed = true;
	else if(key == G_KEY_V && ctrlpressed) ctrlvpressed = true;
	else if(key == G_KEY_X && ctrlpressed) ctrlxpressed = true;
	else if(key == G_KEY_A && ctrlpressed) ctrlapressed = true;
	else if(key == G_KEY_Z && ctrlpressed) ctrlzpressed = true;

	if(key == G_KEY_C && commandpressed) commandcpressed = true;
	else if(key == G_KEY_V && commandpressed) commandvpressed = true;
	else if(key == G_KEY_X && commandpressed) commandxpressed = true;
	else if(key == G_KEY_A && commandpressed) commandapressed = true;
	else if(key == G_KEY_Z && commandpressed) commandzpressed = true;

	int pressedkey = KEY_NONE;
	switch(key) {
	case G_KEY_BACKSPACE:
		pressedkey = KEY_BACKSPACE;
		keypresstime = 0;
		break;
	case G_KEY_LEFT:
		pressedkey = KEY_LEFT;
		keypresstime = 0;
		arrowkeypressed = true;
		break;
	case G_KEY_RIGHT:
		pressedkey = KEY_RIGHT;
		keypresstime = 0;
		break;
	case G_KEY_UP:
		pressedkey = KEY_UP;
		keypresstime = 0;
		break;
	case G_KEY_DOWN:
		pressedkey = KEY_DOWN;
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
	case G_KEY_LEFT_SUPER:
		commandpressed = true;
		break;
	case G_KEY_RIGHT_SUPER:
		commandpressed = true;
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
		arrowkeypressed = false;
		break;
	case G_KEY_RIGHT:
		pressedkey = KEY_RIGHT;
		keypresstime = -1;
		break;
	case G_KEY_UP:
		pressedkey = KEY_UP;
		keypresstime = -1;
		break;
	case G_KEY_DOWN:
		pressedkey = KEY_DOWN;
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
	case G_KEY_LEFT_SUPER:
		commandpressed = false;
		break;
	case G_KEY_RIGHT_SUPER:
		commandpressed = false;
		break;
	default:
		break;
	}
	keystate &= ~pressedkey;
}

void gGUITextbox::handleKeys() {
	if(keypresstime >= 0) {
		keypresstime++;
		if(keypresstime >= keypresstimelimit2) keypresstime = keypresstimelimit1;

		if(keypresstime == 1 || keypresstime == keypresstimelimit1) {
			pressKey();
		}
		return;
	}

	if((selectionmode && (selectionposchar1 != selectionposchar2)) || isdoubleclicked || istripleclicked || ctrlapressed || ctrlvpressed || ctrlzpressed || commandapressed || commandvpressed || commandzpressed) {
		if((isdoubleclicked || istripleclicked || ctrlcpressed || ctrlvpressed || ctrlxpressed || ctrlapressed || ctrlzpressed
												|| commandcpressed || commandvpressed || commandxpressed || commandapressed || commandzpressed)) {
			pressKey();
		}
		ctrlcpressed = false;
		ctrlvpressed = false;
		ctrlxpressed = false;
		ctrlapressed = false;
		ctrlzpressed = false;
		commandcpressed = false;
		commandvpressed = false;
		commandxpressed = false;
		commandapressed = false;
		commandzpressed = false;
		isdoubleclicked = false;
		istripleclicked = false;
		return;
	}
}

void gGUITextbox::pressKey() {
	if((keystate & KEY_BACKSPACE) && (cursorposchar > 0 || (selectionmode && (selectionposchar1 > 0 || selectionposchar2 > 0)))) { // BACKSPACE
		pushToStack();
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
			if(isselectedall) {
				cleanText();
			} else {
				std::string newtext = "";
				std::string deletedtext;
				if(sepu1 > 0) newtext = text.substr(0, sepu1);
				deletedtext = text.substr(sepu1, sepu2 - sepu1);
				if(sepu2 < text.length()) newtext += text.substr(sepu2, text.length() - sepu2 + 1);
				text = newtext;
				letterlength.erase(letterlength.begin() + sepc1, letterlength.begin() + sepc2);
				letterpos = calculateAllLetterPositions();
				if(!ismultiline && !ispassword) {
					cursorposx = sepx1;
					cursorposutf = sepu1;
					cursorposchar = sepc1;
					if(firstchar > 0) {
						firstchar -= calculateCharNum(deletedtext);
						int oldutf = firstutf;
						for(int i = 0; i < deletedtext.length(); i++) {
							firstutf -= letterlength[sepc1 + i];
						}
						if(firstchar < 0) {
							firstchar = 0;
							firstutf = 0;
						} else {
							cursorposx += textfont->getStringWidth(text.substr(firstutf, oldutf - firstutf));
						}
						firstposx = textfont->getStringWidth(text.substr(0, firstutf));
					} else {
						if(sepu2 - sepu1 >= lastutf) {
							firstutf = sepu1;
							firstchar = sepc1;
							firstposx = sepx1;
						}
					}
				} else if(ispassword) {
					cursorposx = sepx1;
				} else if(ismultiline) {
					cursorposx = sepx1;
					cursorposutf = sepu1;
					cursorposchar = sepc1;
				}
			}
		} else {
			int cw = textfont->getStringWidth(text.substr(cursorposutf - letterlength[cursorposchar - 1], letterlength[cursorposchar - 1]));
			text = text.substr(0, cursorposutf - letterlength[cursorposchar - 1]) + text.substr(cursorposutf, text.length() - cursorposutf);
			if(ispassword) {
				cursorposx -= 3 * dotradius;
				if(cursorposchar == text.size() + 1 && (text.size() + 2) * 3 * dotradius > width) {
					cursorposx += 3 * dotradius;
				}
			}
			else {
				cursorposx -= cw;
					if(firstutf > 0 && !ismultiline) {
						int icw = textfont->getStringWidth(text.substr(firstutf - 1, letterlength[firstchar - 1]));
						firstutf -= letterlength[firstchar];
						firstposx = textfont->getStringWidth(text.substr(0, firstutf));
						firstchar--;
						cursorposx += icw;
					}
			}
			cursorposutf -= letterlength[cursorposchar - 1];
			letterlength.erase(letterlength.begin() + cursorposchar - 1);
			letterpos = calculateAllLetterPositions();
			cursorposchar--;
			if(cursorposx < 0) {
				if(!ismultiline && !ispassword) {
					firstutf -= letterlength[firstchar];
					firstposx = textfont->getStringWidth(text.substr(0, firstutf));
					firstchar--;
					cursorposx = 0;
					lastutf = calculateLastUtf();
				} else if(ismultiline){
					if(lines[currentline - 1].size() == 0 || currentline == lastdrawnline - rowsnum + 1) {
						if(lastdrawnline > 1) lastdrawnline--;
						if(linecount > 1) linecount--;
					}
					if(currentline > 1) currentline--;
					std::vector<int> clickpos = calculateCursorPositionMultiline(right - 1, top + currentline * (lineheight + linetopmargin));
					//cursorposchar = clickpos[0];
					cursorposx = clickpos[1];
					//cursorposutf = clickpos[2]
				} else if(ispassword) {
					cursorposx = 0;
					cursorposchar = 0;
					cursorposutf = 0;
				}
			}
		}
		selectionmode = false;
		lastutf = calculateLastUtf();
		if(ismultiline) {
			setText(text);
			if(lastdrawnline == linecount && linecount < lines.size() && lastdrawnline > 1) lastdrawnline--;
			linecount = lines.size();
			if(ismultiline && lines[currentline - 1].size() == 0 && cursorposx > 0) {
				if(currentline >= 2) cursorposx = textfont->getStringWidth(lines[currentline - 2]);
				linecount--;
				if(lastdrawnline > 1) lastdrawnline--;
				currentline--;
			}
		}
	} else if((keystate & KEY_LEFT) && cursorposchar > 0) { // LEFT ARROW
		if(!shiftpressed) selectionmode = false;
		if(ispassword) cursorposx -= 3 * dotradius;
		else {
			int cw = textfont->getStringWidth(text.substr(cursorposutf - letterlength[cursorposchar - 1], letterlength[cursorposchar - 1]));
			cursorposx -= cw;
			if(textalignment == gBaseGUIObject::TEXTALIGNMENT_RIGHT) {
				arrowamount -= textfont->getStringWidth(text.substr(cursorposutf - letterlength[cursorposchar - 1], letterlength[cursorposchar - 1]));
				if(arrowamount < -textfont->getStringWidth(text)) arrowamount = -textfont->getStringWidth(text);
			}
			else if(textalignment == gBaseGUIObject::TEXTALIGNMENT_MIDDLE) {
				arrowamount -= textfont->getStringWidth(text.substr(cursorposutf - letterlength[cursorposchar - 1], letterlength[cursorposchar - 1])) / 2;
				int middle;
				if(text.length() % 2 == 0) middle = text.length() / 2;
				else middle = text.length() / 2 + 1;
				if(arrowamount < -textfont->getStringWidth(text.substr(0, middle))) arrowamount = -textfont->getStringWidth(text.substr(0, middle));
			}
		}
		cursorposutf -= letterlength[cursorposchar - 1];
		cursorposchar--;
		if(cursorposx < 0) {
			if(!ismultiline && !ispassword) {
				firstutf -= letterlength[firstchar];
				firstposx = textfont->getStringWidth(text.substr(0, firstutf));
				firstchar--;
				cursorposx = 0;
				lastutf = calculateLastUtf();

				if(selectionmode) {
				std::vector<int> clickpos = calculateClickPosition(left + cursorposx, top + 1);
					selectionposchar2 = clickpos[0];
					selectionposx2 = clickpos[1];
					selectionposutf2 = clickpos[2];

					selectionposx1 += textfont->getStringWidth(text.substr(selectionposutf1, letterlength[selectionposchar1]));
					if(selectionposx1 > right) selectionposx1 = right;
					else if(selectionposx1 < left) selectionposx1 = left;
				}
			} else if(ispassword) {
				cursorposx = 0;
			} else {
				if(currentline > 1) currentline--;
				std::vector<int> clickpos = calculateCursorPositionMultiline(right - 1, top + currentline * (lineheight + linetopmargin));
				//cursorposchar = clickpos[0];
				cursorposx = clickpos[1];
				//cursorposutf = clickpos[2];
				if(currentline <= lastdrawnline - rowsnum) lastdrawnline--;
			}

		} else {
			if(selectionmode) {
				selectionposchar2 = cursorposchar;
				selectionposx2 = cursorposx;
				selectionposutf2 = cursorposutf;
			}
		}
	} else if((keystate & KEY_RIGHT) && cursorposchar < text.size()) { // RIGHT ARROW
		if(!shiftpressed) selectionmode = false;
		int cw = textfont->getStringWidth(text.substr(cursorposutf, letterlength[cursorposchar]));
		if(ispassword) cursorposx += 3 * dotradius;
		else {
			cursorposx += cw;
			if(textalignment == gBaseGUIObject::TEXTALIGNMENT_RIGHT) {
				arrowamount += textfont->getStringWidth(text.substr(cursorposutf, letterlength[cursorposchar]));
				if(arrowamount > 0) arrowamount = 0;
			}
			else if(textalignment == gBaseGUIObject::TEXTALIGNMENT_MIDDLE) {
				arrowamount += textfont->getStringWidth(text.substr(cursorposutf, letterlength[cursorposchar])) / 2;
				if(arrowamount > textfont->getStringWidth(text) / 2) arrowamount = textfont->getStringWidth(text) / 2;
			}
		}
		cursorposutf += letterlength[cursorposchar];
		cursorposchar++;
		if(cursorposx >= width - 2 * initx) {
			if(!ismultiline && !ispassword) {
				int temp = firstutf;
				firstutf += letterlength[firstchar];
				firstposx = textfont->getStringWidth(text.substr(0, firstutf));
				firstchar++;
				cursorposx -= cw;
				lastutf = cursorposutf - firstutf;
				std::vector<int> clickpos = calculateClickPosition(cursorposx + left, top + 1);
				if(selectionmode) {
					int temp1 = selectionposchar2;
					int temp2 = selectionposx2 - selectionposx1;
					int temp3 = selectionposx2;

					selectionposchar2 = clickpos[0];
					selectionposx2 = clickpos[1];
					selectionposutf2 = clickpos[2];

					if(selectionposchar2 == temp1) selectionposx1 = selectionposx2 - temp2;
					else selectionposx1 -= textfont->getStringWidth(text.substr(firstutf + lastutf, letterlength[firstutf + lastutf + 1])); //should cause a crash
					if(selectionposx1 < 0) selectionposx1 = 0;

				}

				cursorposchar = clickpos[0];
				cursorposx = clickpos[1];
				cursorposutf = clickpos[2];
				if(cursorposchar > firstutf + lastutf) {
					cursorposchar = firstutf + lastutf;
					cursorposutf = cursorposchar;
				}
			} else if(ismultiline) {
				if(currentline == linecount) return;
				if(lines[currentline - 1] == "") return;
				currentline++;
				std::vector<int> clickpos = calculateCursorPositionMultiline(left + 1, top + currentline * (lineheight + linetopmargin));
				//cursorposchar = clickpos[0];
				cursorposx = clickpos[1];
				//cursorposutf = clickpos[2];
				if(currentline > linecount) currentline = linecount;
				if(currentline > lastdrawnline && lastdrawnline < linecount) lastdrawnline++;
			} else if(ispassword) {
				cursorposx -= 3 * dotradius;
			}
		} else if(ismultiline && cursorposchar == lineendchar[currentline - 1] + 1) {
			if(currentline == linecount) return;
			if(lines[currentline - 1] == "") return;
			currentline++;
			std::vector<int> clickpos = calculateCursorPositionMultiline(left + 1, top + currentline * (lineheight + linetopmargin));
			//cursorposchar = clickpos[0];
			cursorposx = clickpos[1];
			//cursorposutf = clickpos[2];
			if(currentline > linecount) currentline = linecount;
			if(currentline > lastdrawnline && lastdrawnline < linecount) lastdrawnline++;
		} else {
			if(selectionmode) {
				selectionposchar2 = cursorposchar;
				selectionposx2 = cursorposx;
				selectionposutf2 = cursorposutf;
			}
		}
	} else if(ismultiline && (keystate & KEY_UP) && currentline > 1) { // UP ARROW
		currentline--;
		if(currentline <= lastdrawnline - rowsnum) lastdrawnline--;
		std::vector<int> clickpos = calculateCursorPositionMultiline(cursorposx, top + currentline * (lineheight + linetopmargin));
		cursorposchar = clickpos[0];
		cursorposx = clickpos[1];
		cursorposutf = clickpos[2];
	} else if(ismultiline && (keystate & KEY_DOWN) && currentline < linecount) { // DOWN ARROW
		currentline++;
		if(currentline > lastdrawnline) lastdrawnline++;
		std::vector<int> clickpos = calculateCursorPositionMultiline(cursorposx, top + currentline * (lineheight + linetopmargin));
		cursorposchar = clickpos[0];
		cursorposx = clickpos[1];
		cursorposutf = clickpos[2];
	} else if((keystate & KEY_DELETE) && cursorposchar < letterlength.size()) { // DELETE
		pushToStack();
		if(selectionmode && selectionposchar1 != selectionposchar2) {
			if(isselectedall) {
				cleanText();
				return;
			}
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
			std::string deletedtext = text.substr(sepu1, sepu2 - sepu1);
			if(sepu2 < text.length()) newtext += text.substr(sepu2, text.length() - sepu2 + 1);
			text = newtext;
			letterlength.erase(letterlength.begin() + sepc1, letterlength.begin() + sepc2);
			letterpos = calculateAllLetterPositions();
			cursorposx = sepx1;
			cursorposutf = sepu1;
			cursorposchar = sepc1;
			if(firstchar > 0) {
				firstchar -= deletedtext.length();
				int oldutf = firstutf;
				for(int i = 0; i < deletedtext.length(); i++) {
					firstutf -= letterlength[sepc1 + i];
				}
				if(firstchar < 0) {
					firstchar = 0;
					firstutf = 0;
				} else {
					cursorposx += textfont->getStringWidth(text.substr(firstutf, oldutf - firstutf));
				}
				firstposx = textfont->getStringWidth(text.substr(0, firstutf));
			} else {
				if(sepu2 - sepu1 >= lastutf) {
					firstutf = sepu1;
					firstchar = sepc1;
					firstposx = sepx1;
				}
			}
		} else {
			text = text.substr(0, cursorposutf) + text.substr(cursorposutf + letterlength[cursorposchar], text.length() - (cursorposutf + letterlength[cursorposchar]));
			if(firstutf > 0 && !ismultiline) {
				int icw = textfont->getStringWidth(text.substr(firstutf - letterlength[firstchar - 1], letterlength[firstchar - 1]));
				firstutf -= letterlength[firstchar];
				firstposx = textfont->getStringWidth(text.substr(0, firstutf));
				firstchar--;
				cursorposx += icw;
			}
			letterlength.erase(letterlength.begin() + cursorposchar);
			letterpos = calculateAllLetterPositions();
		}
		selectionmode = false;
		lastutf = calculateLastUtf();
		if(ismultiline) {
			setText(text);
			if(lines[currentline - 1].size() == 0) {
				currentline--;
				lastdrawnline--;
				linecount--;
				std::vector<int> clickpos = calculateCursorPositionMultiline(right - 1, top + currentline * (lineheight + linetopmargin));
				cursorposchar = clickpos[0];
				cursorposx = clickpos[1];
				cursorposutf = clickpos[2];
			} else if(lines[linecount - 1] == "") {
				if(linecount > 1) linecount--;
				if(lastdrawnline > 1) lastdrawnline--;
			}
		}
	} else if((keystate & KEY_ENTER)) { // ENTER
		selectionmode = false;
		root->getCurrentCanvas()->onGuiEvent(id, G_GUIEVENT_TEXTBOXENTRY, text);
		if(ismultiline) {
			std::string newtext = "";
			if(cursorposchar > 0) newtext = text.substr(0, cursorposutf);
			newtext += "\n";
			if(cursorposchar < letterlength.size()) newtext += text.substr(cursorposutf, text.length() - cursorposutf);
			text = newtext;
			if(lastdrawnline == currentline || lastdrawnline < rowsnum) lastdrawnline++;
			linecount++;
			currentline++;
			setText(text);
			std::vector<int> clickpos = calculateCursorPositionMultiline(left + 1, top + currentline * (lineheight + linetopmargin));
			cursorposx = clickpos[1];
			cursorposutf++;
			cursorposchar++;
		}
	} else if(ctrlcpressed || commandcpressed) { //ctrl c
		if(isselectedall) {
			appmanager->setClipboardString(text);
			return;
		}
		int seput1 = selectionposutf1;
		int seput2 = selectionposutf2;
		if(selectionposutf1 > selectionposutf2) {
			seput1 = selectionposutf2;
			seput2 = selectionposutf1;
		}
		appmanager->setClipboardString(text.substr(seput1, seput2 - seput1));
	} else if(ctrlxpressed || commandxpressed) { //ctrl x
		pushToStack();
		if(isselectedall) {
			cleanText();
			appmanager->setClipboardString(text);
			if(ismultiline) setText(text);
			return;
		}
		int seput1 = selectionposutf1;
		int seput2 = selectionposutf2;
		if(selectionposutf1 > selectionposutf2) {
			seput1 = selectionposutf2;
			seput2 = selectionposutf1;
		}
		appmanager->setClipboardString(text.substr(seput1, seput2 - seput1));
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
		letterpos = calculateAllLetterPositions();
		cursorposx = sepx1;
		if(ispassword) findCursorPositionPassword();
		cursorposutf = sepu1;
		cursorposchar = sepc1;
		if(sepu2 - sepu1 >= lastutf) {
			firstutf = sepu1;
			firstchar = sepc1;
			firstposx = sepx1;
		}
		selectionmode = false;
		lastutf = calculateLastUtf();
		if(ismultiline) {
			setText(text);
			if(lines[currentline - 1] == "") {
				if(currentline > 1) currentline--;
				if(lastdrawnline > 1) lastdrawnline--;
				if(linecount > 1) linecount--;
				std::vector<int> clickpos = calculateCursorPositionMultiline(right - 1, top + currentline * (lineheight + linetopmargin));
				cursorposchar = clickpos[0];
				cursorposx = clickpos[1];
				cursorposutf = clickpos[2];
			}
		}
	} else if(ctrlapressed || commandapressed) { //ctrl a
		//in multiline all lines should be selected
		selectionmode = true;
		isselectedall = true;
		if(!ispassword) {
			std::vector<int> clickpos = clickTextbox(left + 1, top + 1);
			selectionposchar1 = clickpos[0];
			selectionposx1 = clickpos[1];
			selectionposutf1 = clickpos[2];
			std::vector<int> clickpos2 = clickTextbox(right - 1, top + boxh - 1);
			selectionposchar2 = clickpos2[0];
			selectionposx2 = clickpos2[1];
			selectionposutf2 = clickpos2[2];
		} else if(ispassword) {
			selectionposchar1 = 0;
			selectionposx1 = left;
			selectionposutf1 = 0;

			selectionposchar2 = text.size() - 1;
			selectionposx2 = left + text.size() * 3 * dotradius;
			if(selectionposx2 > right) selectionposx2 = right;
			selectionposutf2 = text.size() - 1;
		}
		cursorposchar = selectionposchar2;
		cursorposx = selectionposx2;
		cursorposutf = selectionposutf2;
	} else if(ctrlvpressed || commandvpressed) { //ctrl v
		pushToStack();
		if(isnumeric) {
			std::string testtext = appmanager->getClipboardString();
			for(int i = 0; i < testtext.size(); i++) if(testtext[i] < 48 || testtext[i] > 57) return;
		}
		if(selectionmode && selectionposchar1 != selectionposchar2) {
			if(isselectedall) {
				std::string newtext = appmanager->getClipboardString();
				setText(newtext);
				cursorposchar = text.length();
				cursorposx = font->getStringWidth(text);
				cursorposutf = lastutf;
				selectionmode = false;
				isselectedall = false;
				return;
			}
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
			letterpos = calculateAllLetterPositions();
			cursorposx = sepx1;
			if(ispassword) findCursorPositionPassword();
			cursorposutf = sepu1;
			cursorposchar = sepc1;
			if(sepu2 - sepu1 >= lastutf) {
				firstutf = sepu1;
				firstchar = sepc1;
				firstposx = sepx1;
			}
		}

		std::string pastedtext = appmanager->getClipboardString();
		if(pastedtext.size() == 0) return;
		std::vector<short> lettersize = readString(pastedtext);
		int pastedtextw = textfont->getStringWidth(pastedtext);
		int pastedtextl = pastedtext.length();
		int pastedtextsize = pastedtext.size();

		std::string firsttext = "";
		if(cursorposutf > 0) firsttext = text.substr(0, cursorposutf);
		int firsttextw = font->getStringWidth(firsttext);
		int firsttextl = firsttext.length();
		int firsttextsize = firsttext.size();
		std::string lasttext = "";
		if(cursorposutf < text.length()) lasttext = text.substr(cursorposutf, text.length() - cursorposutf + 1);
		int oldtextlength = text.length();
		text = firsttext + pastedtext + lasttext;

		if(text.length() == 0) {
			letterlength = lettersize;
		} else if(cursorposchar < letterlength.size()) {
			for(int i = lettersize.size() - 1; i >= 0; i--) letterlength.insert(letterlength.begin() + cursorposchar, lettersize[i]);
		} else {
			for(int i = 0; i < lettersize.size(); i++) letterlength.push_back(lettersize[i]);
		}
		letterpos = calculateAllLetterPositions();
		lastutf = calculateLastUtf();
		cursorposchar = firsttextl + pastedtextl;
		cursorposutf = firsttextsize + pastedtextsize;
		cursorposx = firsttextw + pastedtextw - firstposx;
		if(cursorposx >= width - 2 * initx) {
			if(!ismultiline && !ispassword) {
				do {
					firstutf += letterlength[firstchar];
					firstposx = font->getStringWidth(text.substr(0, firstutf));
					firstchar++;
					cursorposx = font->getStringWidth(text.substr(firstutf, cursorposutf - firstutf));
				} while(cursorposx >= width - 2 * initx);
				lastutf = calculateLastUtf();
			} else if(ismultiline) {
				setText(text);
				findCursorPosition();
			} else if(ispassword) {
				cursorposx -= 3 * dotradius;
			} else {
				cursorposx = font->getStringWidth(text.substr(firstutf, cursorposutf - firstutf));
			}
		}
		selectionmode = false;
		isselectedall = false;
		if(ismultiline) setText(text);
	} else if(ctrlzpressed || commandzpressed){ //ctrl z
		if(undostack.empty()) return; //there is nothing to be undone
		setText(undostack.top()); //undo changes to text
		undostack.pop();

		//undo cursor changes
		cursorposx = cursorposxstack.top();
		cursorposxstack.pop();
		cursorposy = cursorposystack.top();
		cursorposystack.pop();
		cursorposchar = cursorposcharstack.top();
		cursorposcharstack.pop();
		cursorposutf = cursorposutfstack.top();
		cursorposutfstack.pop();

		//undo first character changes
		firstchar = firstcharstack.top();
		firstcharstack.pop();
		firstutf = firstutfstack.top();
		firstutfstack.pop();
		firstposx = firstposxstack.top();
		firstposxstack.pop();

		//undo line changes for multiline
		if(ismultiline) {
			currentline = currentlinestack.top();
			currentlinestack.pop();
			lastdrawnline = lastdrawnlinestack.top();
			lastdrawnlinestack.pop();
			linecount = linecountstack.top();
			linecountstack.pop();
		}
	} else if(isdoubleclicked) {
		bool leftchar = false;
		bool rightchar = false;
		if(cursorposchar > 0 && (isLetter(text[cursorposutf - 1]) || isNumber(text[cursorposutf - 1]))) leftchar = true;
		if(cursorposchar < text.length() && (isLetter(text[cursorposutf]) || isNumber(text[cursorposutf]))) rightchar = true;
		if(!leftchar && !rightchar) return;
		selectionmode = true;
		selectionposchar1 = cursorposchar;
		selectionposx1 = cursorposx;
		selectionposutf1 = cursorposutf;
		selectionposchar2 = cursorposchar;
		selectionposx2 = cursorposx;
		selectionposutf2 = cursorposutf;
		if(leftchar && !ismultiline) {
			std::vector<int> posdata = calculateClickPosition(left + cursorposx + initx, top + 1);
			int pdc = posdata[0];
			int pdx = posdata[1];
			int pdu = posdata[2];
			do {
				int tw = textfont->getStringWidth(text.substr(pdu - letterlength[pdc - 1], letterlength[pdc - 1]));
				pdx -= (tw + 2);
				std::vector<int> posdata2 = calculateClickPosition(left + pdx + initx, top + 1);
				pdc = posdata2[0];
				pdx = posdata2[1];
				pdu = posdata2[2];
			} while(pdc > firstchar && (isLetter(text[pdu - 1]) || isNumber(text[pdu - 1])));
			selectionposchar1 = pdc;
			selectionposx1 = pdx;
			selectionposutf1 = pdu;
		} else if(leftchar && ismultiline) {
			std::vector<int> posdata = calculateCursorPositionMultiline(left + cursorposx + initx, top + 1);
			int pdc = posdata[0];
			int pdx = posdata[1];
			int pdu = posdata[2];
			do {
				int tw = textfont->getStringWidth(text.substr(pdu - letterlength[pdc - 1], letterlength[pdc - 1]));
				pdx -= tw;
				pdu -= letterlength[pdc - 1];
				pdc--;
			} while(pdx > 0 && (isLetter(text[pdu - 1]) || isNumber(text[pdu - 1])));
			selectionposchar1 = pdc;
			selectionposx1 = pdx;
			selectionposutf1 = pdu;
		}
		if(rightchar && !ismultiline) {
			std::vector<int> posdata = calculateClickPosition(left + cursorposx + initx, top + 1);
			int pdc = posdata[0];
			int pdx = posdata[1];
			int pdu = posdata[2];
			do {
				int tw = textfont->getStringWidth(text.substr(pdu, letterlength[pdc]));
				pdx += tw + 1;
				std::vector<int> posdata2 = calculateClickPosition(left + pdx, top + 1);
				pdc = posdata2[0];
				pdx = posdata2[1];
				pdu = posdata2[2];
			} while(pdc < letterlength.size() && (isLetter(text[pdu]) || isNumber(text[pdu])));
			selectionposchar2 = pdc;
			selectionposx2 = pdx;
			selectionposutf2 = pdu;
		} else if(rightchar && ismultiline) {
			std::vector<int> posdata = calculateCursorPositionMultiline(left + cursorposx + initx, top + 1);
			int pdc = posdata[0];
			int pdx = posdata[1];
			int pdu = posdata[2];
			int linewidth = textfont->getStringWidth(lines[currentline - 1]);
			do {
				int tw = textfont->getStringWidth(text.substr(pdu, letterlength[pdc]));
				pdx += tw + 1;
				std::vector<int> posdata2 = calculateCursorPositionMultiline(left + pdx, top + 1);
				pdc = posdata2[0];
				pdx = posdata2[1];
				pdu = posdata2[2];
			} while(pdx < linewidth &&  (isLetter(text[pdu]) || isNumber(text[pdu])));
			selectionposchar2 = pdc;
			selectionposx2 = pdx;
			selectionposutf2 = pdu;
		}
	} else if(istripleclicked) {
		selectionmode = true;
		if(!ismultiline) isselectedall = true;
		if(ismultiline) {
			std::vector<int> clickpos = calculateCursorPositionMultiline(left + 1, top + 1);
			selectionposchar1 = clickpos[0];
			selectionposx1 = clickpos[1];
			selectionposutf1 = clickpos[2];
			std::vector<int> clickpos2 = calculateCursorPositionMultiline(right - 1, top + boxh - 1);
			selectionposchar2 = clickpos2[0];
			selectionposx2 = clickpos2[1];
			selectionposutf2 = clickpos2[2];
			cursorposchar = selectionposchar2;
			cursorposx = selectionposx2;
			cursorposutf = selectionposutf2;
		} else {
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
}

void gGUITextbox::charPressed(unsigned int codepoint) {
//	gLogi("Textbox") << "charPressed:" << codepoint;
//	gLogi("Textbox") << "cp:" << gCodepointToStr(codepoint);
	pushToStack();
	if(editmode) {
		if(isnumeric && (codepoint < 48 || codepoint > 57) && codepoint != 44 && codepoint != 46) return;
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
			letterpos = calculateAllLetterPositions();
			cursorposx = sepx1;
			cursorposutf = sepu1;
			cursorposchar = sepc1;
			selectionmode = false;
		}
		int previouslengthutf = text.length();
		std::string addedtext = "";
		std::string newtext = "";
		std::string oldtext = "";
		if(cursorposchar > 0) oldtext = text.substr(0, cursorposutf);
		addedtext = gCodepointToStr(codepoint);
		if(linecount > currentline && ismultiline) {
			if(textfont->getStringWidth(lines[linecount - 1]) + textfont->getStringWidth(addedtext) >= width - 2 *initx)
				widthexceeded = true;
		} else if(textfont->getStringWidth(lines[currentline - 1]) + textfont->getStringWidth(addedtext) >= width - 2 *initx  && ismultiline)
			widthexceeded = true;
		newtext = oldtext + addedtext;
		if(cursorposchar < letterlength.size()) newtext += text.substr(cursorposutf, text.length() - cursorposutf);
		text = newtext;
		int diffutf = text.length() - previouslengthutf;
		if(previouslengthutf == 0 || cursorposchar == letterlength.size()) letterlength.push_back(diffutf);
		else letterlength.insert(letterlength.begin() + cursorposchar, diffutf);
		letterpos = calculateAllLetterPositions();

		if(ispassword) cursorposx += 3 * dotradius;
		else cursorposx += textfont->getStringWidth(addedtext);
		cursorposchar++;
		cursorposutf += diffutf;
		lastutf += diffutf;
		
		if(cursorposx >= width - 2 * initx  && ismultiline) {
			widthexceeded = false;
		}
		if(cursorposx >= width - 2 * initx || widthexceeded) {
			if(!ismultiline && !ispassword) {
				do {
					firstutf += letterlength[firstchar];
					firstposx = textfont->getStringWidth(text.substr(0, firstutf));
					firstchar++;
					cursorposx = textfont->getStringWidth(text.substr(firstutf, cursorposutf - firstutf));
				} while(cursorposx >= width - 2 * initx);
				lastutf = calculateLastUtf();
			} else if(ismultiline) {
				setText(text);
				findCursorPosition();
			} else if(ispassword) {
				cursorposx -= 3 * dotradius;
			} else {
				cursorposx = textfont->getStringWidth(text.substr(firstutf, cursorposutf - firstutf));
			}
		}
		
		if(ismultiline) {
			setText(text);
			if(text.length() - cursorposutf == lines[currentline - 1].size()) {
				cursorposx = 0;
			}
		}

//		gLogi("Textbox") << "cp cx:" << cursorposx;
	}
}

int gGUITextbox::calculateLastUtf() {
	int lutf = firstutf;
 	for(int i = firstchar; i < letterlength.size(); i++) {
		if(letterpos[i] + textfont->getStringWidth(text.substr(lutf, letterlength[i])) - firstposx < width - 2 * initx) {
			if(textfont->getStringWidth(text.substr(lutf, letterlength[i])) == 0) break;
			lutf += letterlength[i];
		}
		else break;
	}
	return lutf - firstutf;
}

int gGUITextbox::calculateCharNoFromUtf(int letterUtfNo) {
	int charno = 0;
	int charutf = 0;
	for (int i = 0; i < letterlength.size(); i++) {
		if(charutf >= letterUtfNo) break;
		charutf += letterlength[i];
		charno = i + 1;
	}
	return charno;
}

int gGUITextbox::calculateCharNum(std::string str) {
	int charnum = 0;
	int i = 0;

	while (i < str.length()) {
		if ((str[i] & 0xC0) != 0x80) {
			// This is the start of a new character
			++charnum;
		}
		++i;
	}

	return charnum;
}

int gGUITextbox::findFirstSpace(int lineend) {
	for(int i = lineend; i > lineend - lastutf; i--) {
		if (text[i] == ' ' || text[i] == '\n') return i;
	}
	return lineend - 1;
}

void gGUITextbox::mousePressed(int x, int y, int button) {
	if(!iseditable) return;

	if(x >= left - (firstx * 2) && x < right - (firstx * 2) && y >= top + hdiff - (firsty * 2) && y < top + totalh + hdiff - (firsty * 2) && button == 0) {
		firstclicktime = previousclicktime;
		previousclicktime = clicktime;
		clicktime = gGetSystemTimeMillis();
		if(clicktime - previousclicktime <= clicktimediff) {
			isdoubleclicked = true;
			if(ismultiline) calculateLinePositionMultiline(x, y);
			if(previousclicktime - firstclicktime <= clicktimediff) {
				isdoubleclicked = false;
				istripleclicked = true;
				if(ismultiline) calculateLinePositionMultiline(x, y);
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
		} else if(ispassword) {
			int pos = (x - left) / (3 * dotradius);
			if(pos > text.size()) pos = text.size();
			cursorposchar = pos;
			cursorposutf = pos;
			cursorposx = pos * 3 * dotradius;
			selectionmode = false;
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

	if(x >= left && x < right && y >= top + hdiff && y < top + totalh + hdiff && button == 0) {
		return;
	}
	if(!isdragging) editmode = false;
	isdragging = false;
}

void gGUITextbox::mouseDragged(int x, int y, int button) {
	if(editmode && x >= left && x < right && y >= top && y < top + totalh && button == 2) {
		if(!selectionmode) startSelection();

		if(ispassword) {
			int pos = (x - left) / (3 * dotradius);
			if(pos > text.size()) pos = text.size();
			selectionposchar2 = pos;
			selectionposutf2 = pos;
			selectionposx2 = pos * 3 * dotradius;
			isdragging = true;
		} else {
			std::vector<int> dragpos = calculateClickPosition(x, y);
			selectionposchar2 = dragpos[0];
			selectionposx2 = dragpos[1];
			selectionposutf2 = dragpos[2];
			isdragging = true;
		}
	}
}

std::vector<int> gGUITextbox::clickTextbox(int x, int y) {
	if(!editmode) cursorshowcounter = 0;
	editmode = true;

	if(!ismultiline) return calculateClickPosition(x, y);
	return calculateClickPositionMultiline(x, y);
}

std::vector<int> gGUITextbox::calculateClickPosition(int x, int y) {
	std::vector<int> result;
	for(int i = 0; i < 3; i++) result.push_back(0);
	if(letterlength.size() != 0) {
		int clickxdiff = x - left;
		int poschar = firstchar;
		int posutf = firstutf;
		int posx = firstposx;
		for(int i = 0; i < letterlength.size(); i++) {
			if(poschar < letterlength.size()) {
				poschar += 1;
				posutf += letterlength[i];
			}
			posx = textfont->getStringWidth(text.substr(firstutf, posutf - firstutf));
			if(posx >= clickxdiff) {
				poschar -= 1;
				posutf -= letterlength[i];
				posx = textfont->getStringWidth(text.substr(firstutf, posutf - firstutf));
				break;
			}
		}
		
		result[0] = poschar;
		result[1] = posx;
		result[2] = posutf;
	}
	return result;
}

std::vector<int> gGUITextbox::calculateClickPositionMultiline(int x, int y) {
	std::vector<int> result;
	for(int i = 0; i < 3; i++) result.push_back(0);
	int selectedline = 1;
	for(int i = 0; i < rowsnum; i++) {
		if(y > top + hdiff + i * (lineheight + linetopmargin) && y < top + hdiff + (i + 1) * (lineheight + linetopmargin)) {
			if(lastdrawnline > rowsnum) selectedline = lastdrawnline - rowsnum + i + 1;
			else selectedline = i + 1;
			break;
		} else if(y >= top + hdiff + (i + 1) * (lineheight + linetopmargin)) {
			selectedline = lastdrawnline;
		}
	}
	if(lines[selectedline - 1] == "") {
		result[0] = cursorposchar;
		result[1] = cursorposx;
		result[2] = cursorposutf;
		return result;
	}
	currentline = selectedline;
	if(lastdrawnline < selectedline) lastdrawnline = selectedline;
	if(letterlength.size() != 0) {
		int clickxdiff = x - left;
		int poschar = firstchar;
		int posutf = firstutf;
		int startutf = firstutf;
		if(currentline > 1) {
			poschar = lineendchar[currentline - 2] + 1;
			posutf = poschar;
			startutf = posutf;
		}
		int posx = 0;
		for(int i = 0; i < letterlength.size(); i++) {
			poschar += 1;
			posutf += letterlength[i];
			posx = textfont->getStringWidth(text.substr(startutf, posutf - startutf));
			if(poschar == lineendchar[currentline - 1]) break;
			if(posx >= clickxdiff) {
				poschar -= 1;
				posutf -= letterlength[i];
				posx = textfont->getStringWidth(text.substr(startutf, posutf - startutf));
				break;
			}
		}
		result[0] = poschar;
		result[1] = posx;
		result[2] = posutf;
	}
	return result;
}

std::vector<int> gGUITextbox::calculateCursorPositionMultiline(int x, int y) {
	std::vector<int> result;
	for(int i = 0; i < 3; i++) result.push_back(0);
	
	if(letterlength.size() != 0) {
		int clickxdiff = x - left;
		int poschar = firstchar;
		int posutf = firstutf;
		int startutf = firstutf;
		if(currentline > 1) {
			poschar = lineendchar[currentline - 2] + 1;
			posutf = poschar;
			startutf = posutf;
		}
		int posx = 0;
		for(int i = 0; i < letterlength.size(); i++) {
			poschar += 1;
			posutf += letterlength[i];
			posx = textfont->getStringWidth(text.substr(startutf, posutf - startutf));
			if(poschar == lineendchar[currentline - 1]) break;
			if(posx >= clickxdiff) {
				poschar -= 1;
				posutf -= letterlength[i];
				posx = textfont->getStringWidth(text.substr(startutf, posutf - startutf));
				break;
			}
		}
		
		result[0] = poschar;
		result[1] = posx;
		result[2] = posutf;
	}
	return result;
}

void gGUITextbox::calculateLinePositionMultiline(int x, int y) {
	int selectedline = 1;
	for(int i = 0; i < rowsnum; i++) {
		if(y > top + hdiff + i * (lineheight + linetopmargin) && y < top + hdiff + (i + 1) * (lineheight + linetopmargin)) {
			if(lastdrawnline > rowsnum) selectedline = lastdrawnline - rowsnum + i + 1;
			else selectedline = i + 1;
			break;
		} else if(y >= top + hdiff + (i + 1) * (lineheight + linetopmargin)) {
			selectedline = lastdrawnline;
		}
	}
	if(selectedline > lastdrawnline && selectedline <= rowsnum) selectedline = lastdrawnline;
	currentline = selectedline;
}

std::vector<int> gGUITextbox::calculateLetterPosition(int letterCharNo) {
	std::vector<int> letterpos;

	int posutf = 0;
	int posx1 = 0;
	int posx2 = textfont->getStringWidth(text.substr(0, posutf));
	for(int i = 0; i < letterCharNo; i++) {
		posx1 = posx2;
		posutf += letterlength[i];
		posx2 = textfont->getStringWidth(text.substr(0, posutf));
	}

	letterpos.push_back(posx1);
	letterpos.push_back(posx2);

	return letterpos;
}

std::vector<int> gGUITextbox::calculateAllLetterPositions() {
	std::vector<int> ls;
	int lutf = 0;
	ls.push_back(0);
	if(letterlength.size() != 0) lutf = letterlength[0];
	for(int i = 1; i < letterlength.size(); i++) {
		lutf += letterlength[i];
		ls.push_back(textfont->getStringWidth(text.substr(0, lutf)));
	}

	return ls;
}

void gGUITextbox::calculateLines() {
	int linesize = 0;
	lines.clear();
	lineendchar.clear();
	firstchar = 0;
	firstutf = 0;
	firstposx = 0;

	if(text.size() == 0 || text == "") {
		for(int i = 0; i < rowsnum; i++) {
			lines.push_back("");
			lineendchar.push_back(0);
		}
		return;
	}
	for(int i = 0; i < linecount; i++) {
		if(firstutf >= text.size()) {
			lines.push_back("");
			lineendchar.push_back(firstutf);
			continue;
		}
		lastutf = calculateLastUtf();
		linesize = lastutf;

		if(firstutf + linesize < text.size()) {
			int lineendpoint = findFirstSpace(firstutf + linesize + 1);
			linesize = lineendpoint - firstutf;
		}

		if(text.substr(firstutf, linesize).find('\n') != std::string::npos) {
			linesize = text.substr(firstutf, linesize).find('\n');
		}

		if((boxshrinked || boxexpanded) && firstutf + linesize + 1 >= cursorposutf) {
			cursorposx = textfont->getStringWidth(text.substr(firstutf, cursorposutf));
		}
		lines.push_back(text.substr(firstutf, linesize));
		lineendchar.push_back(firstutf + linesize);
		firstchar += calculateCharNum(lines[i]) + 1;
		firstposx += textfont->getStringWidth(text.substr(firstutf, linesize + 1));
		firstutf += linesize + 1;
	}
	firstchar = 0;
	firstutf = 0;
	firstposx = 0;
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
            codepoints++;
        } else {
        }
    }
    return lettersizes;
}

bool gGUITextbox::isLetter(char c) {
    return std::isalpha(static_cast<unsigned char>(c));
}

bool gGUITextbox::isNumber(char c) {
	if(c >= 48 && c <= 57) return true;
	return false;
}

void gGUITextbox::findCursorPosition() {
	int linelastchar = 0;
	if(currentline == linecount) linelastchar = text.size();
	else for(int i = 0; i < currentline; i++) linelastchar += lines[i].size() + 1;
	if(cursorposutf >= linelastchar) {
		if(cursorposutf == lineendchar[currentline - 1]) {
			cursorposx = textfont->getStringWidth(lines[currentline - 1]);
		}
		else  {
			if(!(currentline < linecount)) {
				linecount++;
			}
			if(lastdrawnline < rowsnum || currentline == lastdrawnline) {
				lastdrawnline++;
			}
			currentline++;
			cursorposx = textfont->getStringWidth(text.substr(lineendchar[currentline - 2] + 1, cursorposutf - lineendchar[currentline - 2] - 1));
		}
	} else if(widthexceeded) {
		if(text.substr(cursorposutf, 1) != " ") {
			if(lastdrawnline < rowsnum || currentline == lastdrawnline) lastdrawnline++;
			linecount++;
			currentline++;
			int i = cursorposutf;
			while(text.substr(i - 1, 1) != " ") i--;
			cursorposx = textfont->getStringWidth(text.substr(i, cursorposutf - i));
		} else linecount++;
	}
	widthexceeded = false;
}

void gGUITextbox::findCursorPositionPassword() {
	cursorposx = cursorposchar * 3 * dotradius;
}

void gGUITextbox::cleanText() {
	text = "";
	setText("");
	cursorposx = 0;
	cursorposutf = 0;
	cursorposchar = 0;
	letterlength.clear();
	letterpos.clear();
	isselectedall = false;
	selectionmode = false;
	firstchar = 0;
	firstutf = 0;
	firstposx = 0;
	lastutf = 0;
	if(ismultiline) {
		currentline = 1;
		lastdrawnline = 1;
		linecount = 1;
		lines.clear();
		lineendchar.clear();
	}
}

int gGUITextbox::getTextboxh() {
	return boxh;
}

void gGUITextbox::enableVerticalMargin(bool isAlinged) {
	if(isAlinged)
		hdiff = boxh / 4;
	else
		hdiff = 0;
}

void gGUITextbox::pushToStack() {
	undostack.push(text);
	cursorposxstack.push(cursorposx);
	cursorposystack.push(cursorposy);
	cursorposcharstack.push(cursorposchar);
	cursorposutfstack.push(cursorposutf);
	firstcharstack.push(firstchar);
	firstutfstack.push(firstutf);
	firstposxstack.push(firstposx);
	if(ismultiline) {
		currentlinestack.push(currentline);
		lastdrawnlinestack.push(lastdrawnline);
		linecountstack.push(linecount);
	}
}


void gGUITextbox::setFirstX(int firstx) {
	this->firstx = firstx;
}

void gGUITextbox::setFirstY(int firsty) {
	this->firsty = firsty;
}

int gGUITextbox::getInitX() {
	return initx;
}

void gGUITextbox::setTextFont(gFont* textFont) {
	textfont = textFont;
}

void gGUITextbox::setTextAlignment(int textAlignment, float cellW, int initX) {
	textalignment = textAlignment;
	switch(textalignment) {
	case TEXTALIGNMENT_LEFT:
		textalignmentamount = initX + 1;
		cursormoveamount = 1;
		textmoveamount = 0;
		break;
	case TEXTALIGNMENT_MIDDLE:
		textalignmentamount = cellW / 2;
		cursormoveamount = 0.5f;
		textmoveamount = 0.5f;
		break;
	case TEXTALIGNMENT_RIGHT:
		textalignmentamount = cellW - initX - 1;
		cursormoveamount = 0;
		textmoveamount = 1;
		break;
	}
}

float gGUITextbox::getTextMoveAmount() {
	return textmoveamount;
}

void gGUITextbox::setTextColor(gColor *textColor) {
	textcolor = textColor;
	colorset = true;
}

int gGUITextbox::getCursorPosX() {
	return cursorposx;
}

void gGUITextbox::setCursorPosX(int cursorPosX, int length) {
	cursorposx = cursorPosX;
	cursorposchar = text.size() - (text.size() - length);
	cursorposutf = length;
	if(textalignment == gBaseGUIObject::TEXTALIGNMENT_MIDDLE) arrowamount -= cursorposx / 2;
}
