/*
 * gGUIText.h
 *
 *  Created on: Sep 1, 2021
 *      Author: noyan
 */

#ifndef UI_GGUITEXT_H_
#define UI_GGUITEXT_H_

#include "gGUIControl.h"
#include "gGUIBBCodeParser.h"
#include <functional>
#include <map>
#include <memory>
#include <set>
#include <utility>


class gGUIText: public gGUIControl {
public:

    static const int TEXTALIGNMENT_LEFT = 0, TEXTALIGNMENT_CENTER = 1, TEXTALIGNMENT_RIGHT = 2, TEXTALIGNMENT_JUSTIFY = 3;
    static const int TEXTVERTICALALIGNMENT_TOP = 0, TEXTVERTICALALIGNMENT_CENTER = 1, TEXTVERTICALALIGNMENT_BOTTOM = 2;

    gGUIText();
    virtual ~gGUIText();

    void set(gBaseApp* root, gBaseGUIObject* topParentGUIObject, gBaseGUIObject* parentGUIObject, int parentSlotLineNo, int parentSlotColumnNo, int x, int y, int w, int h);

    void setText(std::string text);

    std::string getText();
    std::string getPlainText();

    void setTextAlignment(int textAligment);
    void setTextVerticalAlignment(int verticalTextAlignment);

    int getTextAlignment();
    int getVerticalTextAlignment();

    void setTextColor(const gColor& color);
    gColor* getTextColor();

    void draw() override;

    void setDisabled(bool isDisabled);
    bool isDisabled() const;

    bool getHasContent();

    /**
     * Enables or disables BBCode rich-text rendering for this text control.
     * Disabled by default to preserve 100% plain-text backward compatibility.
     *
     * Supported tags include [b], [i], [u], [s], [color], [bgcolor], [size],
     * [url]...[/url], [url=...]...[/url], [code], [noparse], and [br].
     */
    void enableBBCode(bool isEnabled = true);
    bool isBBCodeEnabled() const;

    void setBBCodeBoldFont(gFont* font);
    void setBBCodeItalicFont(gFont* font);
    void setBBCodeBoldItalicFont(gFont* font);
    void setBBCodeCodeFont(gFont* font);
    void setBBCodeCodeBackgroundColor(const gColor& color);
    void setBBCodeCodeTextColor(const gColor& color);

    void setLinkColor(const gColor& color);
    gColor* getLinkColor();
    void setLinkHoverColor(const gColor& color);
    gColor* getLinkHoverColor();
    void setLinkVisitedColor(const gColor& color);
    gColor* getLinkVisitedColor();
    void setLinkUnderline(bool isEnabled);
    bool isLinkUnderlineEnabled() const;

    /**
     * Controls whether clicking a [url] link automatically opens it in the
     * system default browser via gOpenUrlInDefaultBrowser(). Enabled by default.
     * Regardless of this setting, clicking a URL also dispatches
     * G_GUIEVENT_URLCLICKED and invokes any registered URL click callback.
     */
    void setAutoOpenUrl(bool isEnabled);
    bool isAutoOpenUrlEnabled() const;
    void setUrlClickCallback(std::function<void(const std::string& url, const std::string& label)> callback);

    std::string getHoveredUrl() const;
    std::string getLastClickedUrl() const;
    std::string getUrlAt(int x, int y);
    bool isUrlVisited(const std::string& url) const;
    void clearVisitedUrls();

    int getCursor(int x, int y) override;
    void mouseMoved(int x, int y) override;
    void mousePressed(int x, int y, int button) override;
    void mouseDragged(int x, int y, int button) override;
    void mouseReleased(int x, int y, int button) override;
    void mouseExited() override;
    int calculateContentHeight() override;

private:
    std::string text;
    std::vector<std::string> line;
    std::vector<int> linefirstx;
    int linenum;
    int textalignment;
    int textverticalalignment;
	int verticaloffset;
    int fontsize, lineh;
    float linespacingfactor;
    bool isdisabled;
    gColor textcolor;
    bool hascontent;
    void resetText();
    void resetAlignment();

    std::vector<std::string> splitString(const std::string& textToSplit, gFont* font, int lineWidth);

    // BBCode rich-text & hyperlink state
    bool isbbcodeenabled;
    bool bbcodedocdirty;
    bool bbcodelayoutdirty;
    int bbcodelayoutwidth;
    int bbcodelayoutheight;
    int bbcodelayoutfontsize;
    gFont* bbcodelayoutfont;
    int bbcodelayoutalignment;
    int bbcodelayoutvalignment;
    float bbcodeverticaloffset;
    float bbcodetotalheight;
    gGUIBBCodeDocument bbcodedoc;
    std::vector<gGUIBBCodeLayoutLine> bbcodelines;
    std::vector<float> bbcodelineoffsets;

    gFont* bbcodeboldfont;
    gFont* bbcodeitalicfont;
    gFont* bbcodebolditalicfont;
    gFont* bbcodecodefont;
    gColor bbcodecodebgcolor;
    bool bbcodecodebgcolorset;
    gColor bbcodecodetextcolor;
    bool bbcodecodetextcolorset;
    std::map<std::pair<std::string, int>, std::shared_ptr<gFont>> bbcodesizedfonts;

    gColor linkcolor;
    bool linkcolorset;
    gColor linkhovercolor;
    bool linkhovercolorset;
    gColor linkvisitedcolor;
    bool linkvisitedcolorset;
    bool linkunderline;
    bool autoopenurl;
    std::function<void(const std::string& url, const std::string& label)> urlclickcallback;

    int hoveredlinkid;
    std::string hoveredurl;
    int pressedlinkid;
    std::string pressedurl;
    std::string pressedlabel;
    std::string lastclickedurl;
    std::set<std::string> visitedurls;

    void invalidateBBCodeDoc();
    void invalidateBBCodeLayout();
    void clearLinkInteractionState();
    void ensureBBCodeDoc();
    void ensureBBCodeLayout();
    gFont* resolveBBCodeFont(const gGUIBBCodeStyle& style, bool& outFauxBold);
    float measureBBCodeText(const std::string& str, const gGUIBBCodeStyle& style);
    gGUIBBCodeFontMetrics measureBBCodeMetrics(const gGUIBBCodeStyle& style);
    void applyBBCodeAlignment();
    void drawBBCode();
    gGUIBBCodeLinkHit hitTestBBCodeLink(int x, int y);
    bool isDarkThemeBackground() const;
    gColor getEffectiveLinkColor() const;
    gColor getEffectiveLinkHoverColor() const;
    gColor getEffectiveLinkVisitedColor() const;
};

#endif /* UI_GGUITEXT_H_ */
