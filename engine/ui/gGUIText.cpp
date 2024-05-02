/*
 * gGUIText.cpp
 *
 *  Created on: Sep 1, 2021
 *      Author: noyan
 */

#include "gGUIText.h"


gGUIText::gGUIText() {
    text = "";
    fontsize = font->getSize();
    linespacingfactor = 0.4f;
	lineh = fontsize + (fontsize * linespacingfactor);
    width = 0;
    linenum = 0;
    isdisabled = false;
	setTextAlignment(TEXTALIGNMENT_LEFT);
	setTextVerticalAlignment(TEXTVERTICALALIGNMENT_CENTER);
	textcolor = fontcolor;
}

gGUIText::~gGUIText() {
}

void gGUIText::set(gBaseApp* root, gBaseGUIObject* topParentGUIObject, gBaseGUIObject* parentGUIObject, int parentSlotLineNo, int parentSlotColumnNo, int x, int y, int w, int h) {
    gGUIControl::set(root, topParentGUIObject, parentGUIObject, parentSlotLineNo, parentSlotColumnNo, x, y, w, h);
    fontsize = font->getSize();
    linespacingfactor = 0.4f;
	lineh = fontsize + (fontsize * linespacingfactor);
    resetText();
}

void gGUIText::setText(std::string text) {
    this->text = text;
    resetText();
}

std::string gGUIText::getText() {
	return text;
}

void gGUIText::setTextAlignment(int alignment) {
    textalignment = alignment;
    resetAlignment();
}

void gGUIText::setTextVerticalAlignment(int alignment) {
	textverticalalignment = alignment;
	resetAlignment();
}

int gGUIText::getTextAlignment() {
	return textalignment;
}

int gGUIText::getVerticalTextAlignment() {
	return textverticalalignment;
}

void gGUIText::setTextColor(const gColor& color) {
	textcolor = color;
}

gColor* gGUIText::getTextColor() {
	return &textcolor;
}

void gGUIText::draw() {
    gColor* oldcolor = renderer->getColor();
    if(isdisabled) {
		renderer->setColor(disabledbuttonfontcolor);
	} else {
		renderer->setColor(&textcolor);
	}

    for (int i = 0; i < linenum; i++) {
		font->drawText(line[i], left + linefirstx[i], top + fontsize + (i * lineh) + verticaloffset);
	}
    renderer->setColor(oldcolor);
}

void gGUIText::resetText() {
    if (width <= 0) {
		return;
	}
    linenum = 0;
    line.clear();
    std::vector<std::string> lines;
    std::vector<std::string> lineswithrules;

    // Split the text based on "\n" character
    std::istringstream iss(text);
    std::string singleline;
    while (std::getline(iss, singleline, '\n')) {
        lines.push_back(singleline);
    }

    // Process each line and apply alignment rules
    for (const std::string& singleline : lines) {
        std::vector<std::string> splitlines = gSplitString(singleline, "\n");
//        std::vector<std::string> splitlines = splitString(singleline, font, width);
        for (const std::string& splitline : splitlines) {
            lineswithrules.push_back(splitline);
        }
    }
    linenum = lineswithrules.size();
    line.clear();
    line.insert(line.end(), lineswithrules.begin(), lineswithrules.end());
    resetAlignment();
}

void gGUIText::resetAlignment() {
    linefirstx.clear();
    int spacewidth = font->getStringWidth(" ");
    for (int i = 0; i < linenum; i++) {
        int linetextwidth = font->getStringWidth(line[i]);
        int lineindent = 0;

        if (textalignment == TEXTALIGNMENT_CENTER) {
            lineindent = (width - linetextwidth) / 2;
        } else if (textalignment == TEXTALIGNMENT_RIGHT) {
            lineindent = std::max(width - linetextwidth, 0);
        } else if (textalignment == TEXTALIGNMENT_JUSTIFY) {
        	 int numspaces = std::count(line[i].begin(), line[i].end(), ' ');
				if (numspaces > 0) {
					int totalspacing = width - linetextwidth;
				    int extraspacing = (totalspacing % spacewidth) - font->getStringWidth(" ");
					totalspacing /= spacewidth;
					int spacing = totalspacing / numspaces * 2;
					if(spacing == 0) spacing = 1;
					std::string justifiedline;
					size_t pos = 0;
					size_t lastpos = 0;
					while ((pos = line[i].find(' ', pos)) != std::string::npos) {
						justifiedline += line[i].substr(lastpos, pos - lastpos);
						int spacestoadd = spacing;
						justifiedline.append(spacestoadd, ' ');
						if(extraspacing > 0){
							justifiedline.append(extraspacing, ' ');
							extraspacing--;
						}
						lastpos = ++pos;
					}
					justifiedline += line[i].substr(lastpos);
				    line[i] = justifiedline;
				}
	    }
        linefirstx.push_back(lineindent);
    }
    if (textalignment == TEXTALIGNMENT_RIGHT) {
        int margin = 5;
        for (int i = 0; i < linenum; i++) {
            //int linetextwidth = font->getStringWidth(line[i]);
            linefirstx[i] -= margin;
        }
    }
	if (textalignment == TEXTALIGNMENT_JUSTIFY) {
		int margin = 3;
		for (int i = 0; i < linenum; i++) {
			//int linetextwidth = font->getStringWidth(line[i]);
			linefirstx[i] -= margin;
		}
	}
	if (textverticalalignment == TEXTVERTICALALIGNMENT_BOTTOM) {
		int totalheight = 0;
		for (int i = 0; i < linenum; i++) {
			totalheight += font->getStringHeight(line[i]);
		}
		verticaloffset = height - totalheight - 2;
	}
	if (textverticalalignment == TEXTVERTICALALIGNMENT_CENTER) {
		int totalheight = 0;
		for (int i = 0; i < linenum; i++) {
			totalheight += font->getStringHeight(line[i]);
		}
		verticaloffset = (height - totalheight) / 2 - 2;
	}
}

std::vector<std::string> gGUIText::splitString(const std::string& textToSplit, gFont* font, int lineWidth) {
    std::vector<std::string> tokens;
    if (textToSplit.empty() || lineWidth == 0) return tokens;

    std::istringstream iss(textToSplit); // Create an std::istringstream object to read the text.
    std::string word;
    std::string line;
    while (iss >> word) { //if word is not empty enter the loop
        if (font->getStringWidth(line + " " + word) <= lineWidth) { // Check if adding the word to the existing line, with a space in between, does not exceed the line width.
            if (!line.empty()) line += " "; // If the line is not empty, add a space before adding the word.
            line += word; // Add the word to the line.
        } else { // If adding the current word exceeds the line width,
            tokens.push_back(line); // Add the current line to the vector of parsed lines.
            line = word; // Create a new line and add the word to it.
        }
    }
    if (!line.empty()) {
        tokens.push_back(line);
    }
    return tokens;
}

void gGUIText::setDisabled(bool isDisabled) {
	isdisabled = isDisabled;
}
