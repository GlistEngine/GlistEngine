/*
 * gGUIText.h
 *
 *  Created on: Sep 1, 2021
 *      Author: noyan
 */

#ifndef UI_GGUITEXT_H_
#define UI_GGUITEXT_H_

#include "gGUIControl.h"


class gGUIText: public gGUIControl {
public:

    static const int TEXTALIGNMENT_LEFT = 0, TEXTALIGNMENT_CENTER = 1, TEXTALIGNMENT_RIGHT = 2, TEXTALIGNMENT_JUSTIFY = 3;

    gGUIText();
    virtual ~gGUIText();

    void set(gBaseApp* root, gBaseGUIObject* topParentGUIObject, gBaseGUIObject* parentGUIObject, int parentSlotLineNo, int parentSlotColumnNo, int x, int y, int w, int h);

    void setText(std::string text);

    std::string getText();

    void setTextAlignment(int textAligment);

    void draw();

private:
    std::string text;
    std::vector<std::string> line;
    std::vector<int> linefirstx;
    int linenum;
    int textalignment;
    int fontsize, lineh;

    void resetText();
    void resetAlignment();

    std::vector<std::string> splitString(const std::string& textToSplit, gFont* font, int lineWidth);
};

#endif /* UI_GGUITEXT_H_ */
