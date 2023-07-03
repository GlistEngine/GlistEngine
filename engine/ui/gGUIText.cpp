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
	width = 0;
	linenum = 0;
}

gGUIText::~gGUIText() {
}

void gGUIText::set(gBaseApp* root, gBaseGUIObject* topParentGUIObject, gBaseGUIObject* parentGUIObject, int parentSlotLineNo, int parentSlotColumnNo, int x, int y, int w, int h) {
	gGUIControl::set(root, topParentGUIObject, parentGUIObject, parentSlotLineNo, parentSlotColumnNo, x, y, w, h);
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
	if(width <= 0) return;
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
        int lineTextWidth = font->getStringWidth(line[i]);
        int lineIndent = 0;

        if (textalignment == TEXTALIGNMENT_CENTER) {
            lineIndent = (width - lineTextWidth) / 2;  // Calculate the indentation for the line based on the difference between the width of the GUI element and the width of the text line.
        } else if (textalignment == TEXTALIGNMENT_RIGHT) {
            lineIndent = std::max(width - lineTextWidth, 0);
        }
        linefirstx[i] = lineIndent;
    }
    if (textalignment == TEXTALIGNMENT_RIGHT) {
        int margin = 5;
        for (int i = 0; i < linenum; i++) {
            int lineTextWidth = font->getStringWidth(line[i]);
            linefirstx[i] -= margin; // Decrease the indentation value for the current line by the margin value
        }
    }
}

std::vector<std::string> gGUIText::splitString(const std::string& textToSplit, gFont* font, int lineWidth) {
    std::vector<std::string> tokens;
    if (textToSplit.empty() || lineWidth == 0) return tokens;

    std::istringstream iss(textToSplit); // Convert the given text into an std::istringstream object to enable splitting the text into word tokens.
    std::string word; // Create a string to hold each word read from the text.
    std::string line; // Create a string to temporarily store the constructed line.
    while (iss >> word) { // Read a word from the text, and if the word is not empty, enter the loop.
        if (font->getStringWidth(line + " " + word) <= lineWidth) { // Check if adding the current word to the existing line, with a space in between, exceeds the lineWidth.
            if (!line.empty()) line += " "; // If the line is not empty, add a space before adding the word.
            line += word; // Add the word to the line.
        } else {
            tokens.push_back(line);
            line = word;
        }
    }
    if (!line.empty()) {
        tokens.push_back(line);
    }
    return tokens;
}
