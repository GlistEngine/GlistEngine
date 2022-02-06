/*
 * gGUIText.cpp
 *
 *  Created on: Sep 1, 2021
 *      Author: noyan
 */

#include "gGUIText.h"


gGUIText::gGUIText() {
	textalignment = TEXTALIGNMENT_LEFT;
	text = "";
	fontsize = font->getSize();
	lineh = 14 * fontsize / 10;
}

gGUIText::~gGUIText() {
}

void gGUIText::set(gBaseApp* root, gBaseGUIObject* parentGUIObject, int parentSlotLineNo, int parentSlotColumnNo, int x, int y, int w, int h) {
	gGUIControl::set(root, parentGUIObject, parentSlotLineNo, parentSlotColumnNo, x, y, w, h);
	resetText();
}

void gGUIText::setText(std::string text) {
	this->text = text;
	resetText();
}

void gGUIText::setTextAlignment(int textAligment) {
	textalignment = textAligment;
	resetAlignment();
}

void gGUIText::draw() {
	gColor oldcolor = *renderer->getColor();
	renderer->setColor(fontcolor);
	for (int i = 0; i < linenum; i++) font->drawText(line[i], left + linefirstx[i], top + fontsize + (i * lineh));
	renderer->setColor(&oldcolor);
}

void gGUIText::resetText() {
	linenum = 0;
	for (int i = 0; i < maxlinenum; i++) {
		line[i] = "";
		linefirstx[i] = 0;
	}

	std::vector<std::string> lines = splitString(text, font, width);
	linenum = lines.size();
	for (int i = 0; i < linenum; i++) line[i] = lines[i];
	resetAlignment();
}

void gGUIText::resetAlignment() {
	for (int i = 0; i < linenum; i++) {
		linefirstx[i] = textalignment * (width - font->getStringWidth(line[i])) / 2;
	}
}

std::vector<std::string> gGUIText::splitString(const std::string& textToSplit, gFont* font, int lineWidth) {
	std::vector<std::string> tokens;
	if(textToSplit == "" || lineWidth == 0) return tokens;

	size_t prev = 0, pos = 0, posprevious = 0;
	do {
		bool islinecomplete = false;
		std::string token = "";

		do {
			pos = textToSplit.find(" ", posprevious + 1);
			if (pos == std::string::npos) {
				pos = textToSplit.length();
				islinecomplete = true;
			}
			int length = font->getStringWidth(textToSplit.substr(prev, pos - prev));
			if(length > lineWidth) {
				pos = posprevious;
				islinecomplete = true;
			}
			token = textToSplit.substr(prev, pos - prev);
			posprevious = pos;
		} while(!islinecomplete);

		if (!token.empty()) tokens.push_back(token);
		prev = pos + 1;
	} while (pos < textToSplit.length() && prev < textToSplit.length());
	return tokens;
}
