/****************************************************************************
 * Copyright (c) 2014 Nitra Games Ltd., Istanbul, Turkey                    *
 *                                                                          *
 * Permission is hereby granted, free of charge, to any person obtaining a  *
 * copy of this software and associated documentation files (the            *
 * "Software"), to deal in the Software without restriction, including      *
 * without limitation the rights to use, copy, modify, merge, publish,      *
 * distribute, distribute with modifications, sublicense, and/or sell       *
 * copies of the Software, and to permit persons to whom the Software is    *
 * furnished to do so, subject to the following conditions:                 *
 *                                                                          *
 * The above copyright notice and this permission notice should not be      *
 * deleted from the source form of the Software.                            *
 *                                                                          *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS  *
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF               *
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.   *
 * IN NO EVENT SHALL THE ABOVE COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,   *
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR    *
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR    *
 * THE USE OR OTHER DEALINGS IN THE SOFTWARE.                               *
 *                                                                          *
 * Except as contained in this notice, the name(s) of the above copyright   *
 * holders shall not be used in advertising or otherwise to promote the     *
 * sale, use or other dealings in this Software without prior written       *
 * authorization.                                                           *
 ****************************************************************************/

/*
 * gGUIBBCodeParser.h
 *
 * Lightweight BBCode parser and rich-text layout helper for GlistEngine GUI controls.
 */

#ifndef UI_GGUIBBCODEPARSER_H_
#define UI_GGUIBBCODEPARSER_H_

#include <functional>
#include <string>
#include <vector>

struct gGUIBBCodeColor {
	float r = 1.0f;
	float g = 1.0f;
	float b = 1.0f;
	float a = 1.0f;

	bool operator==(const gGUIBBCodeColor& other) const {
		return r == other.r && g == other.g && b == other.b && a == other.a;
	}
	bool operator!=(const gGUIBBCodeColor& other) const {
		return !(*this == other);
	}
};

struct gGUIBBCodeStyle {
	bool bold = false;
	bool italic = false;
	bool underline = false;
	bool strikethrough = false;
	bool code = false;
	bool hasColor = false;
	gGUIBBCodeColor color;
	bool hasBgColor = false;
	gGUIBBCodeColor bgColor;
	int fontSize = 0; // 0 = inherit textbox base font size

	bool operator==(const gGUIBBCodeStyle& other) const {
		return bold == other.bold &&
			italic == other.italic &&
			underline == other.underline &&
			strikethrough == other.strikethrough &&
			code == other.code &&
			hasColor == other.hasColor &&
			(!hasColor || color == other.color) &&
			hasBgColor == other.hasBgColor &&
			(!hasBgColor || bgColor == other.bgColor) &&
			fontSize == other.fontSize;
	}
	bool operator!=(const gGUIBBCodeStyle& other) const {
		return !(*this == other);
	}
};

struct gGUIBBCodeSpan {
	std::string text;
	gGUIBBCodeStyle style;
	int rawStartByte = 0;
	int rawEndByte = 0;
	int plainStartByte = 0;
	int plainEndByte = 0;
};

struct gGUIBBCodeDocument {
	std::string plainText;
	std::vector<gGUIBBCodeSpan> spans;
	// Size == plainText.size() + 1; maps each plain-text UTF-8 byte index to a raw markup byte index.
	std::vector<int> plainToRawByte;
	// Byte lengths of each UTF-8 character in plainText.
	std::vector<short> plainCharLengths;
};

struct gGUIBBCodeFontMetrics {
	float ascent = 11.0f;
	float descent = 3.0f;
};

struct gGUIBBCodeLayoutRun {
	std::string text;
	gGUIBBCodeStyle style;
	float x = 0.0f;
	float width = 0.0f;
	float ascent = 11.0f;
	float descent = 3.0f;
	int plainStartByte = 0;
	int plainEndByte = 0;
	int plainStartChar = 0;
	int plainEndChar = 0;
};

struct gGUIBBCodeLayoutLine {
	std::vector<gGUIBBCodeLayoutRun> runs;
	float width = 0.0f;
	float ascent = 11.0f;
	float descent = 3.0f;
	float lineHeight = 18.0f;
	float topY = 0.0f;
	float baselineY = 13.0f;
	int plainStartByte = 0;
	int plainEndByte = 0;
	int plainStartChar = 0;
	int plainEndChar = 0;
};

struct gGUIBBCodeHitResult {
	int charIndex = 0;
	int byteOffset = 0;
	int lineIndex = 0;
	float cursorX = 0.0f;
};

struct gGUIBBCodeSelectionRect {
	float x = 0.0f;
	float y = 0.0f;
	float w = 0.0f;
	float h = 0.0f;
	int lineIndex = 0;
};

/**
 * Stateless BBCode parser and layout engine.
 *
 * Supported tags (case-insensitive):
 *  - [b]...[/b]            Bold
 *  - [i]...[/i]            Italic
 *  - [u]...[/u]            Underline
 *  - [s]...[/s]            Strikethrough
 *  - [color=...]...[/color] Foreground color (#RGB, #RGBA, #RRGGBB, #RRGGBBAA, rgb(...), rgba(...), named colors)
 *  - [bgcolor=...]...[/bgcolor] Background highlight color
 *  - [size=...]...[/size]  Font size (absolute px/pt, relative +/-delta, or percentage %)
 *  - [code]...[/code]      Code span (disables inner BBCode parsing until [/code])
 *  - [noparse]...[/noparse] Literal text span (disables inner BBCode parsing until [/noparse])
 *  - [br]                  Explicit line break
 *
 * Malformed, unknown, or unmatched tags safely fall back to literal text.
 */
class gGUIBBCodeParser {
public:
	static constexpr int MIN_FONT_SIZE = 6;
	static constexpr int MAX_FONT_SIZE = 96;

	using MeasureWidthFunc = std::function<float(const std::string& text, const gGUIBBCodeStyle& style)>;
	using FontMetricsFunc = std::function<gGUIBBCodeFontMetrics(const gGUIBBCodeStyle& style)>;

	static gGUIBBCodeDocument parse(const std::string& input, int baseFontSize = 11);

	static bool parseColor(const std::string& value, gGUIBBCodeColor& outColor);
	static bool parseSize(const std::string& value, int baseFontSize, int& outSize);

	static std::vector<gGUIBBCodeLayoutLine> layout(
		const gGUIBBCodeDocument& doc,
		float maxWidth,
		bool multiline,
		float lineTopMargin,
		const MeasureWidthFunc& measureWidth,
		const FontMetricsFunc& fontMetrics);

	static gGUIBBCodeHitResult hitTest(
		const std::vector<gGUIBBCodeLayoutLine>& lines,
		float localX,
		float localY,
		const MeasureWidthFunc& measureWidth,
		float lineOffsetX = 0.0f);

	static gGUIBBCodeHitResult hitTestLine(
		const gGUIBBCodeLayoutLine& line,
		int lineIndex,
		float localX,
		const MeasureWidthFunc& measureWidth);

	static std::vector<gGUIBBCodeSelectionRect> computeSelectionRects(
		const std::vector<gGUIBBCodeLayoutLine>& lines,
		int selStartByte,
		int selEndByte,
		const MeasureWidthFunc& measureWidth);

	static int utf8CharLength(const std::string& s, int bytePos);
	static int countUtf8Chars(const std::string& s, int startByte, int endByte);
	static int charIndexToByteOffset(const gGUIBBCodeDocument& doc, int charIndex);
	static int byteOffsetToCharIndex(const gGUIBBCodeDocument& doc, int byteOffset);
};

#endif /* UI_GGUIBBCODEPARSER_H_ */
