/*
 * gGUIText.cpp
 *
 *  Created on: Sep 1, 2021
 *      Author: noyan
 */

#include "gGUIText.h"
#include "gBaseApp.h"
#include "gBaseCanvas.h"
#include "gAppManager.h"
#include "gGUIManager.h"
#include "gGUIEvents.h"
#include "gUtils.h"
#include <algorithm>
#include <cmath>
#include <sstream>


gGUIText::gGUIText() {
    text = "";
    fontsize = getFont()->getSize();
    linespacingfactor = 0.4f;
	lineh = fontsize + (fontsize * linespacingfactor);
    width = 0;
    linenum = 0;
    isdisabled = false;
	setTextAlignment(TEXTALIGNMENT_LEFT);
	setTextVerticalAlignment(TEXTVERTICALALIGNMENT_CENTER);
	textcolor = fontcolor;
	hascontent = false;

	isbbcodeenabled = false;
	bbcodedocdirty = true;
	bbcodelayoutdirty = true;
	bbcodelayoutwidth = -1;
	bbcodelayoutheight = -1;
	bbcodelayoutfontsize = -1;
	bbcodelayoutfont = nullptr;
	bbcodelayoutalignment = -1;
	bbcodelayoutvalignment = -1;
	bbcodeverticaloffset = 0.0f;
	bbcodetotalheight = 0.0f;
	bbcodeboldfont = nullptr;
	bbcodeitalicfont = nullptr;
	bbcodebolditalicfont = nullptr;
	bbcodecodefont = nullptr;
	bbcodecodebgcolorset = false;
	bbcodecodetextcolorset = false;
	linkcolor.set(0.082f, 0.396f, 0.753f, 1.0f);
	linkcolorset = false;
	linkhovercolor.set(0.051f, 0.278f, 0.631f, 1.0f);
	linkhovercolorset = false;
	linkvisitedcolor.set(0.416f, 0.106f, 0.604f, 1.0f);
	linkvisitedcolorset = false;
	linkunderline = true;
	autoopenurl = true;
	hoveredlinkid = 0;
	pressedlinkid = 0;
}

gGUIText::~gGUIText() {
}

void gGUIText::set(gBaseApp* root, gBaseGUIObject* topParentGUIObject, gBaseGUIObject* parentGUIObject, int parentSlotLineNo, int parentSlotColumnNo, int x, int y, int w, int h) {
    gGUIControl::set(root, topParentGUIObject, parentGUIObject, parentSlotLineNo, parentSlotColumnNo, x, y, w, h);
    fontsize = getFont()->getSize();
    linespacingfactor = 0.4f;
	lineh = fontsize + (fontsize * linespacingfactor);
    if(isbbcodeenabled) {
        invalidateBBCodeLayout();
    } else {
        resetText();
    }
}

void gGUIText::setText(std::string text) {
    this->text = text;
    hascontent = !text.empty();
    clearLinkInteractionState();
    if(isbbcodeenabled) {
        invalidateBBCodeDoc();
    } else {
        resetText();
    }
}

std::string gGUIText::getText() {
	return text;
}

std::string gGUIText::getPlainText() {
	if(!isbbcodeenabled) {
		return text;
	}
	ensureBBCodeDoc();
	return bbcodedoc.plainText;
}

void gGUIText::setTextAlignment(int alignment) {
    textalignment = alignment;
    if(isbbcodeenabled) {
        invalidateBBCodeLayout();
    } else {
        resetAlignment();
    }
}

void gGUIText::setTextVerticalAlignment(int alignment) {
	textverticalalignment = alignment;
	if(isbbcodeenabled) {
		invalidateBBCodeLayout();
	} else {
		resetAlignment();
	}
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
    gColor oldcolor = *renderer->getColor();
    if(isbbcodeenabled) {
        drawBBCode();
        renderer->setColor(oldcolor);
        return;
    }

    if(isdisabled) {
		renderer->setColor(disabledbuttonfontcolor);
	} else {
		renderer->setColor(&textcolor);
	}

    for (int i = 0; i < linenum; i++) {
		getFont()->drawText(line[i], left + linefirstx[i], top + fontsize + (i * lineh) + verticaloffset);
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
//        std::vector<std::string> splitlines = gSplitString(singleline, "\n");
        std::vector<std::string> splitlines = splitString(singleline, getFont(), width);
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
    int spacewidth = getFont()->getStringWidth(" ");
    for (int i = 0; i < linenum; i++) {
        int linetextwidth = getFont()->getStringWidth(line[i]);
        int lineindent = 0;

        if (textalignment == TEXTALIGNMENT_CENTER) {
            lineindent = (width - linetextwidth) / 2;
        } else if (textalignment == TEXTALIGNMENT_RIGHT) {
            lineindent = std::max(width - linetextwidth, 0);
        } else if (textalignment == TEXTALIGNMENT_JUSTIFY) {
        	 int numspaces = std::count(line[i].begin(), line[i].end(), ' ');
				if (numspaces > 0) {
					int totalspacing = width - linetextwidth;
				    int extraspacing = (totalspacing % spacewidth) - getFont()->getStringWidth(" ");
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
    verticaloffset = 0;
	if (textverticalalignment == TEXTVERTICALALIGNMENT_BOTTOM) {
		int totalheight = 0;
		for (int i = 0; i < linenum; i++) {
			totalheight += getFont()->getStringHeight(line[i]);
		}
		verticaloffset = height - totalheight - 2;
	}
	if (textverticalalignment == TEXTVERTICALALIGNMENT_CENTER) {
		int totalheight = 0;
		for (int i = 0; i < linenum; i++) {
			totalheight += getFont()->getStringHeight(line[i]);
		}
		verticaloffset = (height - totalheight) / 2 - 2;
	}
}

std::vector<std::string> gGUIText::splitString(const std::string& textToSplit, gFont* font, int lineWidth) {
    std::vector<std::string> tokens;
    if (textToSplit.empty() || lineWidth == 0) {
    	 tokens.push_back(textToSplit);
    	 return tokens;
    }

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
	if(isdisabled) {
		clearLinkInteractionState();
	}
}

bool gGUIText::isDisabled() const {
	return isdisabled;
}

bool gGUIText::getHasContent() {
	return hascontent;
}

void gGUIText::enableBBCode(bool isEnabled) {
	if(isbbcodeenabled == isEnabled) return;
	isbbcodeenabled = isEnabled;
	clearLinkInteractionState();
	if(isbbcodeenabled) {
		invalidateBBCodeDoc();
	} else {
		resetText();
	}
}

bool gGUIText::isBBCodeEnabled() const {
	return isbbcodeenabled;
}

void gGUIText::setBBCodeBoldFont(gFont* font) {
	bbcodeboldfont = font;
	invalidateBBCodeLayout();
}

void gGUIText::setBBCodeItalicFont(gFont* font) {
	bbcodeitalicfont = font;
	invalidateBBCodeLayout();
}

void gGUIText::setBBCodeBoldItalicFont(gFont* font) {
	bbcodebolditalicfont = font;
	invalidateBBCodeLayout();
}

void gGUIText::setBBCodeCodeFont(gFont* font) {
	bbcodecodefont = font;
	invalidateBBCodeLayout();
}

void gGUIText::setBBCodeCodeBackgroundColor(const gColor& color) {
	bbcodecodebgcolor = color;
	bbcodecodebgcolorset = true;
}

void gGUIText::setBBCodeCodeTextColor(const gColor& color) {
	bbcodecodetextcolor = color;
	bbcodecodetextcolorset = true;
}

void gGUIText::setLinkColor(const gColor& color) {
	linkcolor = color;
	linkcolorset = true;
}

gColor* gGUIText::getLinkColor() {
	return &linkcolor;
}

void gGUIText::setLinkHoverColor(const gColor& color) {
	linkhovercolor = color;
	linkhovercolorset = true;
}

gColor* gGUIText::getLinkHoverColor() {
	return &linkhovercolor;
}

void gGUIText::setLinkVisitedColor(const gColor& color) {
	linkvisitedcolor = color;
	linkvisitedcolorset = true;
}

gColor* gGUIText::getLinkVisitedColor() {
	return &linkvisitedcolor;
}

void gGUIText::setLinkUnderline(bool isEnabled) {
	linkunderline = isEnabled;
}

bool gGUIText::isLinkUnderlineEnabled() const {
	return linkunderline;
}

void gGUIText::setAutoOpenUrl(bool isEnabled) {
	autoopenurl = isEnabled;
}

bool gGUIText::isAutoOpenUrlEnabled() const {
	return autoopenurl;
}

void gGUIText::setUrlClickCallback(std::function<void(const std::string& url, const std::string& label)> callback) {
	urlclickcallback = std::move(callback);
}

std::string gGUIText::getHoveredUrl() const {
	return hoveredurl;
}

std::string gGUIText::getLastClickedUrl() const {
	return lastclickedurl;
}

std::string gGUIText::getUrlAt(int x, int y) {
	if(!isbbcodeenabled || isdisabled) return "";
	gGUIBBCodeLinkHit hit = hitTestBBCodeLink(x, y);
	return hit.hit ? hit.url : "";
}

bool gGUIText::isUrlVisited(const std::string& url) const {
	return visitedurls.find(url) != visitedurls.end();
}

void gGUIText::clearVisitedUrls() {
	visitedurls.clear();
}

int gGUIText::getCursor(int x, int y) {
	if(!isbbcodeenabled || isdisabled) {
		return gGUIControl::getCursor(x, y);
	}
	gGUIBBCodeLinkHit hit = hitTestBBCodeLink(x, y);
	if(hit.hit) {
		return CURSOR_HAND;
	}
	return CURSOR_ARROW;
}

void gGUIText::mouseMoved(int x, int y) {
	if(!isbbcodeenabled || isdisabled) {
		hoveredlinkid = 0;
		hoveredurl.clear();
		return;
	}
	gGUIBBCodeLinkHit hit = hitTestBBCodeLink(x, y);
	if(hit.hit) {
		hoveredlinkid = hit.linkId;
		hoveredurl = hit.url;
	} else {
		hoveredlinkid = 0;
		hoveredurl.clear();
	}
}

void gGUIText::mouseDragged(int x, int y, int button) {
	mouseMoved(x, y);
}

void gGUIText::mousePressed(int x, int y, int button) {
	if(!isbbcodeenabled || isdisabled || button != 0) return;
	gGUIBBCodeLinkHit hit = hitTestBBCodeLink(x, y);
	if(hit.hit) {
		hoveredlinkid = hit.linkId;
		hoveredurl = hit.url;
		pressedlinkid = hit.linkId;
		pressedurl = hit.url;
		pressedlabel = hit.displayText;
	} else {
		pressedlinkid = 0;
		pressedurl.clear();
		pressedlabel.clear();
	}
}

void gGUIText::mouseReleased(int x, int y, int button) {
	if(!isbbcodeenabled || isdisabled || button != 0) {
		pressedlinkid = 0;
		pressedurl.clear();
		pressedlabel.clear();
		return;
	}
	if(pressedlinkid > 0) {
		gGUIBBCodeLinkHit hit = hitTestBBCodeLink(x, y);
		int activeLinkId = pressedlinkid;
		std::string activeUrl = pressedurl;
		std::string activeLabel = pressedlabel;
		pressedlinkid = 0;
		pressedurl.clear();
		pressedlabel.clear();

		if(hit.hit && hit.linkId == activeLinkId && !activeUrl.empty()) {
			lastclickedurl = activeUrl;
			visitedurls.insert(activeUrl);
			if(root != nullptr && root->getCurrentCanvas() != nullptr) {
				root->getCurrentCanvas()->onGuiEvent(id, G_GUIEVENT_URLCLICKED, activeUrl, activeLabel);
			}
			actionmanager.onGUIEvent(id, G_GUIEVENT_URLCLICKED, activeUrl, activeLabel);
			if(urlclickcallback) {
				urlclickcallback(activeUrl, activeLabel);
			}
			if(autoopenurl) {
				gOpenUrlInDefaultBrowser(activeUrl);
			}
		}
	}
}

void gGUIText::mouseExited() {
	hoveredlinkid = 0;
	hoveredurl.clear();
}

int gGUIText::calculateContentHeight() {
	if(!isbbcodeenabled) {
		return gGUIControl::calculateContentHeight();
	}
	ensureBBCodeLayout();
	int contentH = static_cast<int>(std::ceil(bbcodetotalheight));
	return std::max(height, contentH);
}

void gGUIText::invalidateBBCodeDoc() {
	bbcodedocdirty = true;
	bbcodelayoutdirty = true;
}

void gGUIText::invalidateBBCodeLayout() {
	bbcodelayoutdirty = true;
}

void gGUIText::clearLinkInteractionState() {
	hoveredlinkid = 0;
	hoveredurl.clear();
	pressedlinkid = 0;
	pressedurl.clear();
	pressedlabel.clear();
}

void gGUIText::ensureBBCodeDoc() {
	gFont* baseFont = getFont();
	int baseSize = baseFont ? baseFont->getSize() : 11;
	if(!bbcodedocdirty && bbcodelayoutfontsize == baseSize) return;
	bbcodedoc = gGUIBBCodeParser::parse(text, baseSize);
	bbcodedocdirty = false;
	bbcodelayoutdirty = true;
}

void gGUIText::ensureBBCodeLayout() {
	if(!isbbcodeenabled) return;
	ensureBBCodeDoc();
	if(width <= 0) {
		bbcodelines.clear();
		bbcodelineoffsets.clear();
		bbcodetotalheight = 0.0f;
		bbcodeverticaloffset = 0.0f;
		return;
	}

	gFont* baseFont = getFont();
	int baseSize = baseFont ? baseFont->getSize() : 11;
	if(!bbcodelayoutdirty &&
		bbcodelayoutwidth == width &&
		bbcodelayoutheight == height &&
		bbcodelayoutfontsize == baseSize &&
		bbcodelayoutfont == baseFont &&
		bbcodelayoutalignment == textalignment &&
		bbcodelayoutvalignment == textverticalalignment) {
		return;
	}

	fontsize = baseSize;
	lineh = fontsize + static_cast<int>(fontsize * linespacingfactor);

	float lineTopMargin = std::max(0.0f, fontsize * linespacingfactor - 2.0f);
	float maxW = static_cast<float>(width);
	if(textalignment == TEXTALIGNMENT_RIGHT) {
		maxW = std::max(20.0f, maxW - 5.0f);
	} else if(textalignment == TEXTALIGNMENT_JUSTIFY) {
		maxW = std::max(20.0f, maxW - 6.0f);
	}

	bbcodelines = gGUIBBCodeParser::layout(
		bbcodedoc,
		maxW,
		true,
		lineTopMargin,
		[this](const std::string& s, const gGUIBBCodeStyle& style) {
			return measureBBCodeText(s, style);
		},
		[this](const gGUIBBCodeStyle& style) {
			return measureBBCodeMetrics(style);
		});

	applyBBCodeAlignment();

	bbcodelayoutwidth = width;
	bbcodelayoutheight = height;
	bbcodelayoutfontsize = baseSize;
	bbcodelayoutfont = baseFont;
	bbcodelayoutalignment = textalignment;
	bbcodelayoutvalignment = textverticalalignment;
	bbcodelayoutdirty = false;
}

void gGUIText::applyBBCodeAlignment() {
	bbcodelineoffsets.clear();
	bbcodelineoffsets.reserve(bbcodelines.size());

	// Apply justification directly to run coordinates on soft-wrapped lines so draw & hit-test stay identical.
	if(textalignment == TEXTALIGNMENT_JUSTIFY && width > 20) {
		const float targetW = std::max(20.0f, static_cast<float>(width) - 6.0f);
		for(size_t li = 0; li + 1 < bbcodelines.size(); ++li) {
			gGUIBBCodeLayoutLine& ln = bbcodelines[li];
			if(ln.endedWithHardBreak || ln.runs.empty()) continue;

			// Split runs into word and single-space runs while preserving styles and byte ranges.
			std::vector<gGUIBBCodeLayoutRun> splitRuns;
			for(const gGUIBBCodeLayoutRun& r : ln.runs) {
				size_t pos = 0;
				while(pos < r.text.size()) {
					if(r.text[pos] == ' ') {
						gGUIBBCodeLayoutRun sp = r;
						sp.text = " ";
						sp.plainStartByte = r.plainStartByte + static_cast<int>(pos);
						sp.plainEndByte = sp.plainStartByte + 1;
						sp.width = measureBBCodeText(sp.text, sp.style);
						splitRuns.push_back(std::move(sp));
						pos++;
					} else {
						size_t start = pos;
						while(pos < r.text.size() && r.text[pos] != ' ') pos++;
						gGUIBBCodeLayoutRun wd = r;
						wd.text = r.text.substr(start, pos - start);
						wd.plainStartByte = r.plainStartByte + static_cast<int>(start);
						wd.plainEndByte = r.plainStartByte + static_cast<int>(pos);
						wd.width = measureBBCodeText(wd.text, wd.style);
						splitRuns.push_back(std::move(wd));
					}
				}
			}

			// Drop trailing spaces from width calculation.
			while(!splitRuns.empty() && splitRuns.back().text == " ") {
				splitRuns.pop_back();
			}

			int firstWord = -1;
			int lastWord = -1;
			float naturalW = 0.0f;
			for(int k = 0; k < static_cast<int>(splitRuns.size()); ++k) {
				naturalW += splitRuns[k].width;
				if(splitRuns[k].text != " ") {
					if(firstWord < 0) firstWord = k;
					lastWord = k;
				}
			}

			int spaceCount = 0;
			if(firstWord >= 0 && lastWord > firstWord) {
				for(int k = firstWord + 1; k < lastWord; ++k) {
					if(splitRuns[k].text == " ") spaceCount++;
				}
			}

			float extra = targetW - naturalW;
			if(spaceCount > 0 && extra > 0.5f && extra < targetW * 0.6f) {
				float addPerSpace = extra / static_cast<float>(spaceCount);
				float cursorX = 0.0f;
				for(int k = 0; k < static_cast<int>(splitRuns.size()); ++k) {
					splitRuns[k].x = cursorX;
					if(k > firstWord && k < lastWord && splitRuns[k].text == " ") {
						splitRuns[k].width += addPerSpace;
					}
					cursorX += splitRuns[k].width;
				}
				ln.runs = std::move(splitRuns);
				ln.width = cursorX;
			}
		}
	}

	bbcodetotalheight = 0.0f;
	for(const gGUIBBCodeLayoutLine& ln : bbcodelines) {
		float indent = 0.0f;
		if(textalignment == TEXTALIGNMENT_CENTER) {
			indent = (static_cast<float>(width) - ln.width) * 0.5f;
		} else if(textalignment == TEXTALIGNMENT_RIGHT) {
			indent = std::max(static_cast<float>(width) - ln.width, 0.0f) - 5.0f;
		} else if(textalignment == TEXTALIGNMENT_JUSTIFY) {
			indent = 0.0f;
		}
		bbcodelineoffsets.push_back(indent);
		bbcodetotalheight += ln.lineHeight;
	}

	bbcodeverticaloffset = 0.0f;
	if(textverticalalignment == TEXTVERTICALALIGNMENT_BOTTOM) {
		bbcodeverticaloffset = static_cast<float>(height) - bbcodetotalheight;
	} else if(textverticalalignment == TEXTVERTICALALIGNMENT_CENTER) {
		bbcodeverticaloffset = (static_cast<float>(height) - bbcodetotalheight) * 0.5f;
	}
}

gFont* gGUIText::resolveBBCodeFont(const gGUIBBCodeStyle& style, bool& outFauxBold) {
	outFauxBold = false;
	gFont* baseRegular = getFont();
	if(!baseRegular) return nullptr;

	gGUIManager* guiMgr = (appmanager != nullptr) ? appmanager->getGUIManager() : nullptr;
	gFont* defaultFreeSans = guiMgr ? guiMgr->getFont(gGUIManager::FONT_FREESANS, gGUIManager::FONTTYPE_REGULAR) : nullptr;
	bool isDefaultFont = (defaultFreeSans != nullptr && baseRegular == defaultFreeSans);

	gFont* variantFont = baseRegular;
	if(style.code && bbcodecodefont != nullptr) {
		variantFont = bbcodecodefont;
	} else if(style.bold && style.italic) {
		if(bbcodebolditalicfont != nullptr) variantFont = bbcodebolditalicfont;
		else if(isDefaultFont) variantFont = guiMgr->getFont(gGUIManager::FONT_FREESANS, gGUIManager::FONTTYPE_BOLDITALIC);
	} else if(style.bold) {
		if(bbcodeboldfont != nullptr) variantFont = bbcodeboldfont;
		else if(isDefaultFont) variantFont = guiMgr->getFont(gGUIManager::FONT_FREESANS, gGUIManager::FONTTYPE_BOLD);
	} else if(style.italic) {
		if(bbcodeitalicfont != nullptr) variantFont = bbcodeitalicfont;
		else if(isDefaultFont) variantFont = guiMgr->getFont(gGUIManager::FONT_FREESANS, gGUIManager::FONTTYPE_ITALIC);
	}

	if(!variantFont) variantFont = baseRegular;

	if(style.bold && variantFont == baseRegular && bbcodeboldfont == nullptr && bbcodebolditalicfont == nullptr) {
		outFauxBold = true;
	}

	if(style.fontSize > 0 && style.fontSize != variantFont->getSize() && variantFont->isLoaded() && !variantFont->getPath().empty() && renderer != nullptr) {
		std::pair<std::string, int> cacheKey(variantFont->getPath(), style.fontSize);
		auto it = bbcodesizedfonts.find(cacheKey);
		if(it != bbcodesizedfonts.end()) {
			return it->second ? it->second.get() : variantFont;
		}
		if(bbcodesizedfonts.size() < 32) {
			std::shared_ptr<gFont> sized = std::make_shared<gFont>();
			if(sized->load(variantFont->getPath(), style.fontSize, variantFont->isAntialised(), variantFont->getDpi())) {
				bbcodesizedfonts[cacheKey] = sized;
				return sized.get();
			}
			bbcodesizedfonts[cacheKey] = nullptr;
		}
	}

	return variantFont;
}

float gGUIText::measureBBCodeText(const std::string& str, const gGUIBBCodeStyle& style) {
	if(str.empty()) return 0.0f;
	bool fauxBold = false;
	gFont* f = resolveBBCodeFont(style, fauxBold);
	if(!f) return 0.0f;
	float w = f->getStringWidth(str);
	if(fauxBold) w += 0.6f;
	return w;
}

gGUIBBCodeFontMetrics gGUIText::measureBBCodeMetrics(const gGUIBBCodeStyle& style) {
	gGUIBBCodeFontMetrics fm;
	bool fauxBold = false;
	gFont* f = resolveBBCodeFont(style, fauxBold);
	if(f) {
		float asc = f->getStringHeight("l");
		if(asc <= 0.0f) asc = (f->getAscender() > 0.0f) ? f->getAscender() : static_cast<float>(std::max(8, f->getSize()));
		float desc = std::max(2.0f, std::ceil(f->getSize() * 0.28f));
		fm.ascent = asc;
		fm.descent = desc;
	}
	return fm;
}

bool gGUIText::isDarkThemeBackground() const {
	if(backgroundcolor == nullptr) return false;
	float lum = 0.299f * backgroundcolor->r + 0.587f * backgroundcolor->g + 0.114f * backgroundcolor->b;
	return lum < 0.5f;
}

gColor gGUIText::getEffectiveLinkColor() const {
	if(linkcolorset) return linkcolor;
	if(isDarkThemeBackground()) {
		return gColor(0.392f, 0.710f, 0.965f, 1.0f);
	}
	return gColor(0.082f, 0.396f, 0.753f, 1.0f);
}

gColor gGUIText::getEffectiveLinkHoverColor() const {
	if(linkhovercolorset) return linkhovercolor;
	if(isDarkThemeBackground()) {
		return gColor(0.565f, 0.792f, 0.976f, 1.0f);
	}
	return gColor(0.051f, 0.278f, 0.631f, 1.0f);
}

gColor gGUIText::getEffectiveLinkVisitedColor() const {
	if(linkvisitedcolorset) return linkvisitedcolor;
	if(isDarkThemeBackground()) {
		return gColor(0.808f, 0.576f, 0.847f, 1.0f);
	}
	return gColor(0.416f, 0.106f, 0.604f, 1.0f);
}

void gGUIText::drawBBCode() {
	ensureBBCodeLayout();
	if(bbcodelines.empty()) return;

	gColor defaultTextColor = isdisabled ? *disabledbuttonfontcolor : textcolor;

	gColor effectiveCodeBg = bbcodecodebgcolor;
	if(!bbcodecodebgcolorset) {
		if(isDarkThemeBackground()) {
			effectiveCodeBg.set(0.24f, 0.26f, 0.30f, 0.90f);
		} else {
			effectiveCodeBg.set(0.89f, 0.91f, 0.94f, 0.90f);
		}
	}

	gColor effectiveLinkColor = getEffectiveLinkColor();
	gColor effectiveLinkHoverColor = getEffectiveLinkHoverColor();
	gColor effectiveLinkVisitedColor = getEffectiveLinkVisitedColor();

	const float originX = static_cast<float>(left);
	const float originY = static_cast<float>(top) + bbcodeverticaloffset;

	// Pass 1: inline backgrounds ([bgcolor], [code], and subtle link hover/press highlight)
	bool wasAlphaBlending = renderer->isAlphaBlendingEnabled();
	for(size_t i = 0; i < bbcodelines.size(); ++i) {
		const gGUIBBCodeLayoutLine& ln = bbcodelines[i];
		float lineOffsetX = (i < bbcodelineoffsets.size()) ? bbcodelineoffsets[i] : 0.0f;
		float baselineY = originY + ln.baselineY;

		for(const gGUIBBCodeLayoutRun& run : ln.runs) {
			if(run.width <= 0.0f) continue;
			float runX = originX + lineOffsetX + run.x;

			if(run.style.hasBgColor) {
				if(run.style.bgColor.a < 1.0f && !renderer->isAlphaBlendingEnabled()) {
					renderer->enableAlphaBlending();
				}
				renderer->setColor(run.style.bgColor.r, run.style.bgColor.g, run.style.bgColor.b, run.style.bgColor.a);
				gDrawRectangle(runX, baselineY - run.ascent - 1.0f, run.width, run.ascent + run.descent + 2.0f, true);
			} else if(run.style.code) {
				if(effectiveCodeBg.a < 1.0f && !renderer->isAlphaBlendingEnabled()) {
					renderer->enableAlphaBlending();
				}
				renderer->setColor(&effectiveCodeBg);
				gDrawRectangle(runX - 1.5f, baselineY - run.ascent - 1.0f, run.width + 3.0f, run.ascent + run.descent + 2.0f, true);
			}

			if(!isdisabled && run.style.hasUrl && hoveredlinkid > 0 && run.style.linkId == hoveredlinkid) {
				if(!renderer->isAlphaBlendingEnabled()) {
					renderer->enableAlphaBlending();
				}
				float hoverAlpha = (pressedlinkid == hoveredlinkid) ? 0.22f : 0.12f;
				renderer->setColor(effectiveLinkHoverColor.r, effectiveLinkHoverColor.g, effectiveLinkHoverColor.b, hoverAlpha);
				gDrawRectangle(runX - 1.0f, baselineY - run.ascent - 1.0f, run.width + 2.0f, run.ascent + run.descent + 2.0f, true);
			}
		}
	}
	if(!wasAlphaBlending && renderer->isAlphaBlendingEnabled()) {
		renderer->disableAlphaBlending();
	}

	// Pass 2: styled text and decorations (underline, link underline, strikethrough)
	for(size_t i = 0; i < bbcodelines.size(); ++i) {
		const gGUIBBCodeLayoutLine& ln = bbcodelines[i];
		float lineOffsetX = (i < bbcodelineoffsets.size()) ? bbcodelineoffsets[i] : 0.0f;
		float baselineY = originY + ln.baselineY;

		for(const gGUIBBCodeLayoutRun& run : ln.runs) {
			if(run.text.empty()) continue;
			bool fauxBold = false;
			gFont* f = resolveBBCodeFont(run.style, fauxBold);
			if(!f) continue;

			float runX = originX + lineOffsetX + run.x;
			bool isHoveredLink = (!isdisabled && run.style.hasUrl && hoveredlinkid > 0 && run.style.linkId == hoveredlinkid);

			gColor runColor = defaultTextColor;
			if(!isdisabled) {
				if(run.style.hasColor) {
					runColor.set(run.style.color.r, run.style.color.g, run.style.color.b, run.style.color.a);
				} else if(run.style.hasUrl) {
					if(isHoveredLink) {
						runColor = effectiveLinkHoverColor;
					} else if(visitedurls.find(run.style.url) != visitedurls.end()) {
						runColor = effectiveLinkVisitedColor;
					} else {
						runColor = effectiveLinkColor;
					}
				} else if(run.style.code && bbcodecodetextcolorset) {
					runColor = bbcodecodetextcolor;
				}
			}

			renderer->setColor(&runColor);
			f->drawText(run.text, runX, baselineY);
			if(fauxBold) {
				f->drawText(run.text, runX + 0.6f, baselineY);
			}

			float lineThick = std::max(1.0f, static_cast<float>(f->getSize()) / 12.0f);
			bool shouldUnderline = run.style.underline || (run.style.hasUrl && (linkunderline || isHoveredLink));
			if(shouldUnderline && run.width > 0.0f) {
				float uThick = isHoveredLink ? std::max(1.5f, lineThick * 1.4f) : lineThick;
				float uy = baselineY + std::max(1.5f, run.descent * 0.45f);
				gDrawLine(runX, uy, runX + run.width, uy, uThick);
			}
			if(run.style.strikethrough && run.width > 0.0f) {
				float sy = baselineY - run.ascent * 0.35f;
				gDrawLine(runX, sy, runX + run.width, sy, lineThick);
			}
		}
	}
}

gGUIBBCodeLinkHit gGUIText::hitTestBBCodeLink(int x, int y) {
	if(!isbbcodeenabled || width <= 0) return gGUIBBCodeLinkHit{};
	ensureBBCodeLayout();
	if(bbcodelines.empty()) return gGUIBBCodeLinkHit{};

	float localX = static_cast<float>(x - left);
	float localY = static_cast<float>(y - top) - bbcodeverticaloffset;
	return gGUIBBCodeParser::hitTestLink(bbcodelines, localX, localY, bbcodelineoffsets);
}
