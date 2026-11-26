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
 * gGUIBBCodeParser.cpp
 */

#include "gGUIBBCodeParser.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdlib>
#include <sstream>

namespace {

enum class TagKind {
	BOLD,
	ITALIC,
	UNDERLINE,
	STRIKETHROUGH,
	COLOR,
	BGCOLOR,
	SIZE,
	CODE,
	NOPARSE,
	URL
};

struct StyleFrame {
	TagKind kind;
	gGUIBBCodeColor color;
	gGUIBBCodeColor bgColor;
	int fontSize = 0;
	std::string url;
	int linkId = 0;
};

std::string trimAscii(const std::string& s) {
	size_t start = 0;
	while(start < s.size() && std::isspace(static_cast<unsigned char>(s[start]))) start++;
	size_t end = s.size();
	while(end > start && std::isspace(static_cast<unsigned char>(s[end - 1]))) end--;
	return s.substr(start, end - start);
}

std::string stripQuotes(const std::string& s) {
	if(s.size() >= 2 && ((s.front() == '"' && s.back() == '"') || (s.front() == '\'' && s.back() == '\''))) {
		return trimAscii(s.substr(1, s.size() - 2));
	}
	return s;
}

std::string toLowerAscii(const std::string& s) {
	std::string out = s;
	for(char& c : out) {
		c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
	}
	return out;
}

int hexVal(char c) {
	if(c >= '0' && c <= '9') return c - '0';
	if(c >= 'a' && c <= 'f') return 10 + (c - 'a');
	if(c >= 'A' && c <= 'F') return 10 + (c - 'A');
	return -1;
}

gGUIBBCodeStyle computeStyleFromStack(const std::vector<StyleFrame>& stack) {
	gGUIBBCodeStyle style;
	for(const StyleFrame& frame : stack) {
		switch(frame.kind) {
		case TagKind::BOLD:
			style.bold = true;
			break;
		case TagKind::ITALIC:
			style.italic = true;
			break;
		case TagKind::UNDERLINE:
			style.underline = true;
			break;
		case TagKind::STRIKETHROUGH:
			style.strikethrough = true;
			break;
		case TagKind::COLOR:
			style.hasColor = true;
			style.color = frame.color;
			break;
		case TagKind::BGCOLOR:
			style.hasBgColor = true;
			style.bgColor = frame.bgColor;
			break;
		case TagKind::SIZE:
			style.fontSize = frame.fontSize;
			break;
		case TagKind::CODE:
			style.code = true;
			break;
		case TagKind::NOPARSE:
			break;
		case TagKind::URL:
			style.hasUrl = true;
			style.url = frame.url;
			style.linkId = frame.linkId;
			break;
		}
	}
	return style;
}

bool matchTagKind(const std::string& lowerName, TagKind& outKind) {
	if(lowerName == "b") { outKind = TagKind::BOLD; return true; }
	if(lowerName == "i") { outKind = TagKind::ITALIC; return true; }
	if(lowerName == "u") { outKind = TagKind::UNDERLINE; return true; }
	if(lowerName == "s") { outKind = TagKind::STRIKETHROUGH; return true; }
	if(lowerName == "color") { outKind = TagKind::COLOR; return true; }
	if(lowerName == "bgcolor") { outKind = TagKind::BGCOLOR; return true; }
	if(lowerName == "size") { outKind = TagKind::SIZE; return true; }
	if(lowerName == "code") { outKind = TagKind::CODE; return true; }
	if(lowerName == "noparse") { outKind = TagKind::NOPARSE; return true; }
	if(lowerName == "url") { outKind = TagKind::URL; return true; }
	return false;
}

bool tagRequiresValue(TagKind kind) {
	return kind == TagKind::COLOR || kind == TagKind::BGCOLOR || kind == TagKind::SIZE;
}

bool tagAcceptsOptionalValue(TagKind kind) {
	return kind == TagKind::URL;
}

// Finds a case-insensitive closing [/url] tag (allowing optional inner whitespace) starting at startPos.
size_t findClosingUrlTag(const std::string& input, size_t startPos) {
	const size_t n = input.size();
	size_t i = startPos;
	while(i < n) {
		size_t open = input.find('[', i);
		if(open == std::string::npos) return std::string::npos;
		size_t close = std::string::npos;
		for(size_t k = open + 1; k < n; ++k) {
			if(input[k] == ']') {
				close = k;
				break;
			}
			if(input[k] == '[' || input[k] == '\n' || input[k] == '\r') {
				break;
			}
		}
		if(close == std::string::npos) {
			i = open + 1;
			continue;
		}
		std::string inner = trimAscii(input.substr(open + 1, close - open - 1));
		if(!inner.empty() && inner[0] == '/') {
			std::string name = toLowerAscii(trimAscii(inner.substr(1)));
			if(name == "url") {
				return open;
			}
		}
		i = close + 1;
	}
	return std::string::npos;
}

// Extracts candidate plain text between [url] and [/url], skipping known inline BBCode tags without recursion.
std::string extractUrlCandidateText(const std::string& input, size_t startPos, size_t endPos) {
	std::string out;
	out.reserve(endPos > startPos ? (endPos - startPos) : 0);
	size_t pos = startPos;
	while(pos < endPos) {
		if(input[pos] == '[') {
			size_t close = std::string::npos;
			for(size_t k = pos + 1; k < endPos; ++k) {
				if(input[k] == ']') {
					close = k;
					break;
				}
				if(input[k] == '[' || input[k] == '\n' || input[k] == '\r') {
					break;
				}
			}
			if(close != std::string::npos) {
				std::string inner = trimAscii(input.substr(pos + 1, close - pos - 1));
				std::string body = inner;
				if(!body.empty() && body[0] == '/') body = trimAscii(body.substr(1));
				size_t eq = body.find('=');
				std::string name = toLowerAscii(trimAscii(eq == std::string::npos ? body : body.substr(0, eq)));
				TagKind dummy;
				if(name == "br" || matchTagKind(name, dummy)) {
					pos = close + 1;
					continue;
				}
			}
		}
		out.push_back(input[pos]);
		pos++;
	}
	return out;
}

// Finds a case-insensitive literal closing tag (e.g. "[/code]" or "[/noparse]") starting at pos.
size_t findClosingTagCaseInsensitive(const std::string& input, size_t startPos, const std::string& lowerCloseTag) {
	const size_t n = input.size();
	const size_t m = lowerCloseTag.size();
	if(m == 0 || startPos + m > n) return std::string::npos;
	for(size_t i = startPos; i + m <= n; ++i) {
		if(input[i] != '[') continue;
		bool match = true;
		for(size_t j = 0; j < m; ++j) {
			if(static_cast<char>(std::tolower(static_cast<unsigned char>(input[i + j]))) != lowerCloseTag[j]) {
				match = false;
				break;
			}
		}
		if(match) return i;
	}
	return std::string::npos;
}

void appendTextToDoc(gGUIBBCodeDocument& doc, const std::string& chunk, int rawStart, int rawEnd, const gGUIBBCodeStyle& style) {
	if(chunk.empty()) return;

	int plainStart = static_cast<int>(doc.plainText.size());
	doc.plainText += chunk;
	int plainEnd = static_cast<int>(doc.plainText.size());

	// Populate byte-level plain -> raw map.
	// When chunk comes directly from raw input (same length), map 1:1; for synthetic chunks like [br] -> "\n", map to rawStart.
	const bool oneToOne = (rawEnd - rawStart) == static_cast<int>(chunk.size());
	for(size_t i = 0; i < chunk.size(); ++i) {
		doc.plainToRawByte.push_back(oneToOne ? (rawStart + static_cast<int>(i)) : rawStart);
	}

	if(!doc.spans.empty() && doc.spans.back().style == style && doc.spans.back().rawEndByte == rawStart) {
		doc.spans.back().text += chunk;
		doc.spans.back().rawEndByte = rawEnd;
		doc.spans.back().plainEndByte = plainEnd;
	} else {
		gGUIBBCodeSpan span;
		span.text = chunk;
		span.style = style;
		span.rawStartByte = rawStart;
		span.rawEndByte = rawEnd;
		span.plainStartByte = plainStart;
		span.plainEndByte = plainEnd;
		doc.spans.push_back(std::move(span));
	}
}

struct InlinePiece {
	std::string text;
	gGUIBBCodeStyle style;
	int plainStartByte = 0;
	int plainEndByte = 0;
	float width = 0.0f;
};

enum class InlineTokenType {
	WORD,
	SPACE,
	NEWLINE
};

struct InlineToken {
	InlineTokenType type = InlineTokenType::WORD;
	std::vector<InlinePiece> pieces;
	int plainStartByte = 0;
	int plainEndByte = 0;
};

void appendRunToLine(
	gGUIBBCodeLayoutLine& line,
	const std::string& text,
	const gGUIBBCodeStyle& style,
	int plainStartByte,
	int plainEndByte,
	const gGUIBBCodeParser::MeasureWidthFunc& measureWidth,
	const gGUIBBCodeParser::FontMetricsFunc& fontMetrics) {
	if(text.empty()) return;

	gGUIBBCodeFontMetrics fm = fontMetrics(style);
	if(fm.ascent > line.ascent) line.ascent = fm.ascent;
	if(fm.descent > line.descent) line.descent = fm.descent;

	if(!line.runs.empty() && line.runs.back().style == style && line.runs.back().plainEndByte == plainStartByte) {
		gGUIBBCodeLayoutRun& last = line.runs.back();
		last.text += text;
		last.plainEndByte = plainEndByte;
		last.width = measureWidth(last.text, style);
		if(fm.ascent > last.ascent) last.ascent = fm.ascent;
		if(fm.descent > last.descent) last.descent = fm.descent;
		line.width = last.x + last.width;
		line.plainEndByte = plainEndByte;
		return;
	}

	gGUIBBCodeLayoutRun run;
	run.text = text;
	run.style = style;
	run.x = line.width;
	run.width = measureWidth(text, style);
	run.ascent = fm.ascent;
	run.descent = fm.descent;
	run.plainStartByte = plainStartByte;
	run.plainEndByte = plainEndByte;

	if(line.runs.empty()) {
		line.plainStartByte = plainStartByte;
	}
	line.plainEndByte = plainEndByte;
	line.width += run.width;
	line.runs.push_back(std::move(run));
}

float measureByteXInLine(
	const gGUIBBCodeLayoutLine& line,
	int byteOffset,
	const gGUIBBCodeParser::MeasureWidthFunc& measureWidth) {
	if(line.runs.empty() || byteOffset <= line.plainStartByte) return 0.0f;
	if(byteOffset >= line.plainEndByte) return line.width;

	for(const gGUIBBCodeLayoutRun& run : line.runs) {
		if(byteOffset <= run.plainStartByte) {
			return run.x;
		}
		if(byteOffset >= run.plainStartByte && byteOffset <= run.plainEndByte) {
			int localLen = byteOffset - run.plainStartByte;
			if(localLen <= 0) return run.x;
			if(localLen >= static_cast<int>(run.text.size())) return run.x + run.width;
			return run.x + measureWidth(run.text.substr(0, static_cast<size_t>(localLen)), run.style);
		}
	}
	return line.width;
}

} // namespace

int gGUIBBCodeParser::utf8CharLength(const std::string& s, int bytePos) {
	if(bytePos < 0 || bytePos >= static_cast<int>(s.size())) return 0;
	int len = 1;
	while(bytePos + len < static_cast<int>(s.size()) && (static_cast<unsigned char>(s[bytePos + len]) & 0xC0) == 0x80) {
		len++;
	}
	return len;
}

int gGUIBBCodeParser::countUtf8Chars(const std::string& s, int startByte, int endByte) {
	if(startByte < 0) startByte = 0;
	if(endByte > static_cast<int>(s.size())) endByte = static_cast<int>(s.size());
	if(startByte >= endByte) return 0;
	int count = 0;
	int pos = startByte;
	while(pos < endByte) {
		int clen = utf8CharLength(s, pos);
		if(clen <= 0) break;
		count++;
		pos += clen;
	}
	return count;
}

int gGUIBBCodeParser::charIndexToByteOffset(const gGUIBBCodeDocument& doc, int charIndex) {
	if(charIndex <= 0) return 0;
	int byteOff = 0;
	int limit = std::min(charIndex, static_cast<int>(doc.plainCharLengths.size()));
	for(int i = 0; i < limit; ++i) {
		byteOff += doc.plainCharLengths[i];
	}
	if(byteOff > static_cast<int>(doc.plainText.size())) byteOff = static_cast<int>(doc.plainText.size());
	return byteOff;
}

int gGUIBBCodeParser::byteOffsetToCharIndex(const gGUIBBCodeDocument& doc, int byteOffset) {
	if(byteOffset <= 0) return 0;
	int acc = 0;
	for(size_t i = 0; i < doc.plainCharLengths.size(); ++i) {
		if(acc >= byteOffset) return static_cast<int>(i);
		acc += doc.plainCharLengths[i];
	}
	return static_cast<int>(doc.plainCharLengths.size());
}

bool gGUIBBCodeParser::parseColor(const std::string& rawValue, gGUIBBCodeColor& outColor) {
	std::string v = stripQuotes(trimAscii(rawValue));
	if(v.empty()) return false;

	// Hex formats: #RGB, #RGBA, #RRGGBB, #RRGGBBAA
	if(v[0] == '#') {
		std::string hex = v.substr(1);
		if(hex.size() == 3 || hex.size() == 4) {
			int r = hexVal(hex[0]);
			int g = hexVal(hex[1]);
			int b = hexVal(hex[2]);
			int a = (hex.size() == 4) ? hexVal(hex[3]) : 15;
			if(r < 0 || g < 0 || b < 0 || a < 0) return false;
			outColor.r = (r * 17) / 255.0f;
			outColor.g = (g * 17) / 255.0f;
			outColor.b = (b * 17) / 255.0f;
			outColor.a = (a * 17) / 255.0f;
			return true;
		}
		if(hex.size() == 6 || hex.size() == 8) {
			int vals[8];
			for(size_t i = 0; i < hex.size(); ++i) {
				vals[i] = hexVal(hex[i]);
				if(vals[i] < 0) return false;
			}
			outColor.r = (vals[0] * 16 + vals[1]) / 255.0f;
			outColor.g = (vals[2] * 16 + vals[3]) / 255.0f;
			outColor.b = (vals[4] * 16 + vals[5]) / 255.0f;
			outColor.a = (hex.size() == 8) ? ((vals[6] * 16 + vals[7]) / 255.0f) : 1.0f;
			return true;
		}
		return false;
	}

	std::string lower = toLowerAscii(v);

	// rgb(r,g,b) or rgba(r,g,b,a)
	if((lower.rfind("rgb(", 0) == 0 || lower.rfind("rgba(", 0) == 0) && lower.back() == ')') {
		bool hasAlpha = (lower[3] == 'a');
		std::string inner = lower.substr(hasAlpha ? 5 : 4, lower.size() - (hasAlpha ? 6 : 5));
		std::vector<float> nums;
		std::stringstream ss(inner);
		std::string part;
		while(std::getline(ss, part, ',')) {
			std::string trimmed = trimAscii(part);
			if(trimmed.empty()) return false;
			char* endPtr = nullptr;
			float val = std::strtof(trimmed.c_str(), &endPtr);
			if(endPtr == trimmed.c_str() || *endPtr != '\0') return false;
			nums.push_back(val);
		}
		if((!hasAlpha && nums.size() != 3) || (hasAlpha && nums.size() != 4)) return false;
		auto clamp01 = [](float x) { return std::max(0.0f, std::min(1.0f, x)); };
		outColor.r = clamp01(nums[0] / 255.0f);
		outColor.g = clamp01(nums[1] / 255.0f);
		outColor.b = clamp01(nums[2] / 255.0f);
		if(hasAlpha) {
			outColor.a = (nums[3] <= 1.0f && nums[3] >= 0.0f) ? nums[3] : clamp01(nums[3] / 255.0f);
		} else {
			outColor.a = 1.0f;
		}
		return true;
	}

	// Standard named colors
	struct NamedColor {
		const char* name;
		float r, g, b;
	};
	static const NamedColor kNamedColors[] = {
		{"red", 1.0f, 0.0f, 0.0f},
		{"green", 0.0f, 0.50196f, 0.0f},
		{"lime", 0.0f, 1.0f, 0.0f},
		{"blue", 0.0f, 0.0f, 1.0f},
		{"cyan", 0.0f, 1.0f, 1.0f},
		{"aqua", 0.0f, 1.0f, 1.0f},
		{"magenta", 1.0f, 0.0f, 1.0f},
		{"fuchsia", 1.0f, 0.0f, 1.0f},
		{"yellow", 1.0f, 1.0f, 0.0f},
		{"black", 0.0f, 0.0f, 0.0f},
		{"white", 1.0f, 1.0f, 1.0f},
		{"gray", 0.5f, 0.5f, 0.5f},
		{"grey", 0.5f, 0.5f, 0.5f},
		{"darkgray", 0.25f, 0.25f, 0.25f},
		{"dark_gray", 0.25f, 0.25f, 0.25f},
		{"darkgrey", 0.25f, 0.25f, 0.25f},
		{"lightgray", 0.75f, 0.75f, 0.75f},
		{"light_gray", 0.75f, 0.75f, 0.75f},
		{"lightgrey", 0.75f, 0.75f, 0.75f},
		{"orange", 1.0f, 0.5f, 0.0f},
		{"pink", 1.0f, 0.69f, 0.773f},
		{"violet", 0.5f, 0.0f, 1.0f},
		{"purple", 0.50196f, 0.0f, 0.50196f},
		{"brown", 0.647f, 0.165f, 0.165f},
		{"gold", 1.0f, 0.843f, 0.0f},
		{"silver", 0.753f, 0.753f, 0.753f},
		{"navy", 0.0f, 0.0f, 0.50196f},
		{"teal", 0.0f, 0.50196f, 0.50196f},
		{"maroon", 0.50196f, 0.0f, 0.0f},
		{"olive", 0.50196f, 0.50196f, 0.0f}
	};

	for(const NamedColor& nc : kNamedColors) {
		if(lower == nc.name) {
			outColor.r = nc.r;
			outColor.g = nc.g;
			outColor.b = nc.b;
			outColor.a = 1.0f;
			return true;
		}
	}
	return false;
}

bool gGUIBBCodeParser::parseSize(const std::string& rawValue, int baseFontSize, int& outSize) {
	std::string v = toLowerAscii(stripQuotes(trimAscii(rawValue)));
	if(v.empty()) return false;
	if(baseFontSize <= 0) baseFontSize = 11;

	// Percentage: e.g. 150%
	if(v.back() == '%') {
		std::string numStr = v.substr(0, v.size() - 1);
		if(numStr.empty()) return false;
		char* endPtr = nullptr;
		float pct = std::strtof(numStr.c_str(), &endPtr);
		if(endPtr == numStr.c_str() || *endPtr != '\0' || pct <= 0.0f) return false;
		int computed = static_cast<int>(std::round(baseFontSize * (pct / 100.0f)));
		outSize = std::max(MIN_FONT_SIZE, std::min(MAX_FONT_SIZE, computed));
		return true;
	}

	// Optional px / pt suffix
	if(v.size() > 2 && (v.substr(v.size() - 2) == "px" || v.substr(v.size() - 2) == "pt")) {
		v = v.substr(0, v.size() - 2);
		if(v.empty()) return false;
	}

	// Relative delta: +4 / -2
	bool isRelative = (v[0] == '+' || v[0] == '-');
	char* endPtr = nullptr;
	long val = std::strtol(v.c_str(), &endPtr, 10);
	if(endPtr == v.c_str() || *endPtr != '\0') return false;

	int computed = isRelative ? (baseFontSize + static_cast<int>(val)) : static_cast<int>(val);
	if(!isRelative && computed <= 0) return false;
	outSize = std::max(MIN_FONT_SIZE, std::min(MAX_FONT_SIZE, computed));
	return true;
}

bool gGUIBBCodeParser::parseUrl(const std::string& rawValue, std::string& outUrl) {
	std::string v = trimAscii(stripQuotes(trimAscii(rawValue)));
	if(v.empty()) return false;

	// Reject control characters and embedded whitespace.
	for(unsigned char c : v) {
		if(c <= 0x20 || c == 0x7F) return false;
	}

	// Check for scheme before any '/', '?', '#'.
	size_t colonPos = std::string::npos;
	for(size_t i = 0; i < v.size(); ++i) {
		char c = v[i];
		if(c == ':') {
			colonPos = i;
			break;
		}
		if(c == '/' || c == '?' || c == '#') {
			break;
		}
	}

	if(colonPos != std::string::npos) {
		std::string scheme = toLowerAscii(v.substr(0, colonPos));
		if(scheme == "http" || scheme == "https" || scheme == "ftp" || scheme == "ftps") {
			if(v.size() <= colonPos + 3 || v.compare(colonPos, 3, "://") != 0) {
				return false;
			}
			std::string rest = v.substr(colonPos + 3);
			if(rest.empty() || rest[0] == '/' || rest[0] == '?' || rest[0] == '#') {
				return false;
			}
			outUrl = scheme + "://" + rest;
			return true;
		}
		if(scheme == "mailto") {
			std::string rest = v.substr(colonPos + 1);
			if(rest.empty()) return false;
			outUrl = "mailto:" + rest;
			return true;
		}
		// Reject unsafe or unsupported schemes (javascript:, data:, file:, etc.).
		return false;
	}

	// Allow bare www.<host> shorthand and normalize to https://www.<host>.
	std::string lower = toLowerAscii(v);
	if(lower.rfind("www.", 0) == 0 && v.size() > 4) {
		char next = v[4];
		if(next == '.' || next == '/' || next == '?' || next == '#') return false;
		outUrl = "https://" + v;
		return true;
	}

	return false;
}

gGUIBBCodeDocument gGUIBBCodeParser::parse(const std::string& input, int baseFontSize) {
	gGUIBBCodeDocument doc;
	if(baseFontSize <= 0) baseFontSize = 11;

	std::vector<StyleFrame> stack;
	gGUIBBCodeStyle currentStyle;
	int nextLinkId = 0;
	size_t pos = 0;
	const size_t n = input.size();

	while(pos < n) {
		size_t openBracket = input.find('[', pos);
		if(openBracket == std::string::npos) {
			appendTextToDoc(doc, input.substr(pos), static_cast<int>(pos), static_cast<int>(n), currentStyle);
			break;
		}

		if(openBracket > pos) {
			appendTextToDoc(doc, input.substr(pos, openBracket - pos), static_cast<int>(pos), static_cast<int>(openBracket), currentStyle);
		}

		// Scan for matching ']' before any nested '[' or newline.
		size_t closeBracket = std::string::npos;
		for(size_t k = openBracket + 1; k < n; ++k) {
			if(input[k] == ']') {
				closeBracket = k;
				break;
			}
			if(input[k] == '[' || input[k] == '\n' || input[k] == '\r') {
				break;
			}
		}

		if(closeBracket == std::string::npos) {
			// No closing ']' before next '[' or EOF: emit '[' as literal text and continue.
			appendTextToDoc(doc, "[", static_cast<int>(openBracket), static_cast<int>(openBracket + 1), currentStyle);
			pos = openBracket + 1;
			continue;
		}

		std::string inner = input.substr(openBracket + 1, closeBracket - openBracket - 1);
		std::string trimmedInner = trimAscii(inner);

		if(trimmedInner.empty()) {
			appendTextToDoc(doc, input.substr(openBracket, closeBracket - openBracket + 1),
				static_cast<int>(openBracket), static_cast<int>(closeBracket + 1), currentStyle);
			pos = closeBracket + 1;
			continue;
		}

		bool isClosing = false;
		std::string tagBody = trimmedInner;
		if(tagBody[0] == '/') {
			isClosing = true;
			tagBody = trimAscii(tagBody.substr(1));
		}

		std::string tagName;
		std::string tagParam;
		bool hasParam = false;
		size_t eqPos = tagBody.find('=');
		if(eqPos != std::string::npos) {
			tagName = toLowerAscii(trimAscii(tagBody.substr(0, eqPos)));
			tagParam = trimAscii(tagBody.substr(eqPos + 1));
			hasParam = true;
		} else {
			tagName = toLowerAscii(tagBody);
		}

		// Self-closing line break [br]
		if(!isClosing && !hasParam && tagName == "br") {
			appendTextToDoc(doc, "\n", static_cast<int>(openBracket), static_cast<int>(closeBracket + 1), currentStyle);
			pos = closeBracket + 1;
			continue;
		}

		TagKind kind;
		if(!matchTagKind(tagName, kind)) {
			// Unknown tag: preserve literally.
			appendTextToDoc(doc, input.substr(openBracket, closeBracket - openBracket + 1),
				static_cast<int>(openBracket), static_cast<int>(closeBracket + 1), currentStyle);
			pos = closeBracket + 1;
			continue;
		}

		if(isClosing) {
			if(hasParam) {
				// Closing tags cannot have parameters -> treat as literal.
				appendTextToDoc(doc, input.substr(openBracket, closeBracket - openBracket + 1),
					static_cast<int>(openBracket), static_cast<int>(closeBracket + 1), currentStyle);
				pos = closeBracket + 1;
				continue;
			}

			// Search stack from top down for matching opening frame (gracefully handles cross-nesting).
			int foundIdx = -1;
			for(int i = static_cast<int>(stack.size()) - 1; i >= 0; --i) {
				if(stack[i].kind == kind) {
					foundIdx = i;
					break;
				}
			}

			if(foundIdx >= 0) {
				stack.erase(stack.begin() + foundIdx);
				currentStyle = computeStyleFromStack(stack);
			} else {
				// Stray unmatched closing tag -> fallback to literal text.
				appendTextToDoc(doc, input.substr(openBracket, closeBracket - openBracket + 1),
					static_cast<int>(openBracket), static_cast<int>(closeBracket + 1), currentStyle);
			}
			pos = closeBracket + 1;
			continue;
		}

		// Opening tag
		if(tagRequiresValue(kind) && !hasParam) {
			appendTextToDoc(doc, input.substr(openBracket, closeBracket - openBracket + 1),
				static_cast<int>(openBracket), static_cast<int>(closeBracket + 1), currentStyle);
			pos = closeBracket + 1;
			continue;
		}
		if(!tagRequiresValue(kind) && !tagAcceptsOptionalValue(kind) && hasParam) {
			appendTextToDoc(doc, input.substr(openBracket, closeBracket - openBracket + 1),
				static_cast<int>(openBracket), static_cast<int>(closeBracket + 1), currentStyle);
			pos = closeBracket + 1;
			continue;
		}

		StyleFrame frame;
		frame.kind = kind;
		bool valid = true;

		if(kind == TagKind::COLOR) {
			valid = parseColor(tagParam, frame.color);
		} else if(kind == TagKind::BGCOLOR) {
			valid = parseColor(tagParam, frame.bgColor);
		} else if(kind == TagKind::SIZE) {
			int currentBase = (currentStyle.fontSize > 0) ? currentStyle.fontSize : baseFontSize;
			valid = parseSize(tagParam, currentBase, frame.fontSize);
		} else if(kind == TagKind::URL) {
			if(hasParam) {
				valid = parseUrl(tagParam, frame.url);
			} else {
				size_t closeUrlPos = findClosingUrlTag(input, closeBracket + 1);
				if(closeUrlPos == std::string::npos) {
					valid = false;
				} else {
					std::string candidate = extractUrlCandidateText(input, closeBracket + 1, closeUrlPos);
					valid = parseUrl(candidate, frame.url);
				}
			}
			if(valid) {
				frame.linkId = ++nextLinkId;
			}
		}

		if(!valid) {
			appendTextToDoc(doc, input.substr(openBracket, closeBracket - openBracket + 1),
				static_cast<int>(openBracket), static_cast<int>(closeBracket + 1), currentStyle);
			pos = closeBracket + 1;
			continue;
		}

		// Literal-content tags: [code]...[/code] and [noparse]...[/noparse]
		if(kind == TagKind::CODE || kind == TagKind::NOPARSE) {
			const std::string closeLiteral = (kind == TagKind::CODE) ? "[/code]" : "[/noparse]";
			size_t bodyStart = closeBracket + 1;
			size_t closePos = findClosingTagCaseInsensitive(input, bodyStart, closeLiteral);

			gGUIBBCodeStyle innerStyle = currentStyle;
			if(kind == TagKind::CODE) innerStyle.code = true;

			if(closePos == std::string::npos) {
				// Unclosed [code] or [noparse] -> safely render rest of string literally.
				appendTextToDoc(doc, input.substr(bodyStart), static_cast<int>(bodyStart), static_cast<int>(n), innerStyle);
				pos = n;
			} else {
				if(closePos > bodyStart) {
					appendTextToDoc(doc, input.substr(bodyStart, closePos - bodyStart),
						static_cast<int>(bodyStart), static_cast<int>(closePos), innerStyle);
				}
				pos = closePos + closeLiteral.size();
			}
			continue;
		}

		stack.push_back(frame);
		currentStyle = computeStyleFromStack(stack);
		pos = closeBracket + 1;
	}

	// Sentinel mapping for end-of-plain-text -> end-of-raw-text.
	doc.plainToRawByte.push_back(static_cast<int>(n));

	// Compute UTF-8 character byte lengths for plainText.
	int p = 0;
	while(p < static_cast<int>(doc.plainText.size())) {
		int clen = utf8CharLength(doc.plainText, p);
		if(clen <= 0) break;
		doc.plainCharLengths.push_back(static_cast<short>(clen));
		p += clen;
	}

	return doc;
}

std::vector<gGUIBBCodeLayoutLine> gGUIBBCodeParser::layout(
	const gGUIBBCodeDocument& doc,
	float maxWidth,
	bool multiline,
	float lineTopMargin,
	const MeasureWidthFunc& measureWidth,
	const FontMetricsFunc& fontMetrics) {
	std::vector<gGUIBBCodeLayoutLine> lines;

	gGUIBBCodeStyle baseStyle;
	gGUIBBCodeFontMetrics baseMetrics = fontMetrics(baseStyle);
	if(baseMetrics.ascent <= 0.0f) baseMetrics.ascent = 11.0f;
	if(baseMetrics.descent <= 0.0f) baseMetrics.descent = 3.0f;

	auto makeEmptyLine = [&](int startByte) {
		gGUIBBCodeLayoutLine line;
		line.ascent = baseMetrics.ascent;
		line.descent = baseMetrics.descent;
		line.plainStartByte = startByte;
		line.plainEndByte = startByte;
		return line;
	};

	if(doc.plainText.empty() || doc.spans.empty()) {
		gGUIBBCodeLayoutLine empty = makeEmptyLine(0);
		empty.lineHeight = empty.ascent + empty.descent + lineTopMargin + 4.0f;
		empty.topY = 0.0f;
		empty.baselineY = empty.ascent + 2.0f;
		lines.push_back(std::move(empty));
		return lines;
	}

	if(!multiline) {
		gGUIBBCodeLayoutLine line = makeEmptyLine(0);
		for(const gGUIBBCodeSpan& span : doc.spans) {
			std::string singleLineText = span.text;
			for(char& c : singleLineText) {
				if(c == '\n' || c == '\r') c = ' ';
			}
			appendRunToLine(line, singleLineText, span.style, span.plainStartByte, span.plainEndByte, measureWidth, fontMetrics);
		}
		line.lineHeight = line.ascent + line.descent + lineTopMargin + 4.0f;
		line.topY = 0.0f;
		line.baselineY = line.ascent + 2.0f;
		line.plainStartChar = 0;
		line.plainEndChar = static_cast<int>(doc.plainCharLengths.size());
		for(gGUIBBCodeLayoutRun& run : line.runs) {
			run.plainStartChar = byteOffsetToCharIndex(doc, run.plainStartByte);
			run.plainEndChar = byteOffsetToCharIndex(doc, run.plainEndByte);
		}
		lines.push_back(std::move(line));
		return lines;
	}

	// Multiline layout: tokenize styled spans into WORD / SPACE / NEWLINE tokens.
	const float availableW = std::max(20.0f, maxWidth);
	std::vector<InlineToken> tokens;
	InlineToken currentWord;
	currentWord.type = InlineTokenType::WORD;

	auto flushWord = [&]() {
		if(!currentWord.pieces.empty()) {
			currentWord.plainStartByte = currentWord.pieces.front().plainStartByte;
			currentWord.plainEndByte = currentWord.pieces.back().plainEndByte;
			tokens.push_back(std::move(currentWord));
			currentWord = InlineToken();
			currentWord.type = InlineTokenType::WORD;
		}
	};

	for(const gGUIBBCodeSpan& span : doc.spans) {
		int i = 0;
		const int len = static_cast<int>(span.text.size());
		while(i < len) {
			char ch = span.text[i];
			if(ch == '\r') {
				i++;
				continue;
			}
			if(ch == '\n') {
				flushWord();
				InlineToken nl;
				nl.type = InlineTokenType::NEWLINE;
				nl.plainStartByte = span.plainStartByte + i;
				nl.plainEndByte = span.plainStartByte + i + 1;
				tokens.push_back(std::move(nl));
				i++;
				continue;
			}
			if(ch == ' ' || ch == '\t') {
				flushWord();
				std::string sp(1, ch == '\t' ? ' ' : ch);
				InlinePiece p;
				p.text = sp;
				p.style = span.style;
				p.plainStartByte = span.plainStartByte + i;
				p.plainEndByte = span.plainStartByte + i + 1;
				p.width = measureWidth(p.text, p.style);

				InlineToken spTok;
				spTok.type = InlineTokenType::SPACE;
				spTok.plainStartByte = p.plainStartByte;
				spTok.plainEndByte = p.plainEndByte;
				spTok.pieces.push_back(std::move(p));
				tokens.push_back(std::move(spTok));
				i++;
				continue;
			}

			// Accumulate contiguous non-whitespace characters within this span.
			int start = i;
			while(i < len && span.text[i] != ' ' && span.text[i] != '\t' && span.text[i] != '\n' && span.text[i] != '\r') {
				int clen = utf8CharLength(span.text, i);
				if(clen <= 0) break;
				i += clen;
			}
			InlinePiece piece;
			piece.text = span.text.substr(static_cast<size_t>(start), static_cast<size_t>(i - start));
			piece.style = span.style;
			piece.plainStartByte = span.plainStartByte + start;
			piece.plainEndByte = span.plainStartByte + i;
			piece.width = measureWidth(piece.text, piece.style);
			currentWord.pieces.push_back(std::move(piece));
		}
	}
	flushWord();

	lines.push_back(makeEmptyLine(0));

	for(const InlineToken& tok : tokens) {
		if(tok.type == InlineTokenType::NEWLINE) {
			lines.back().endedWithHardBreak = true;
			lines.push_back(makeEmptyLine(tok.plainEndByte));
			continue;
		}

		if(tok.type == InlineTokenType::SPACE) {
			const InlinePiece& sp = tok.pieces[0];
			gGUIBBCodeLayoutLine& cur = lines.back();
			if(!cur.runs.empty() && cur.width + sp.width > availableW) {
				// Soft-wrap at space boundary (do not carry wrap space onto next line).
				lines.push_back(makeEmptyLine(sp.plainEndByte));
			} else {
				appendRunToLine(cur, sp.text, sp.style, sp.plainStartByte, sp.plainEndByte, measureWidth, fontMetrics);
			}
			continue;
		}

		// WORD token
		float wordWidth = 0.0f;
		for(const InlinePiece& piece : tok.pieces) {
			wordWidth += piece.width;
		}

		gGUIBBCodeLayoutLine* cur = &lines.back();
		if(!cur->runs.empty() && cur->width + wordWidth > availableW) {
			lines.push_back(makeEmptyLine(tok.plainStartByte));
			cur = &lines.back();
		}

		if(cur->width + wordWidth <= availableW) {
			for(const InlinePiece& piece : tok.pieces) {
				appendRunToLine(*cur, piece.text, piece.style, piece.plainStartByte, piece.plainEndByte, measureWidth, fontMetrics);
			}
		} else {
			// Word itself exceeds availableW -> break by UTF-8 codepoints cleanly.
			for(const InlinePiece& piece : tok.pieces) {
				int idx = 0;
				const int plen = static_cast<int>(piece.text.size());
				while(idx < plen) {
					int clen = utf8CharLength(piece.text, idx);
					if(clen <= 0) break;
					std::string chStr = piece.text.substr(static_cast<size_t>(idx), static_cast<size_t>(clen));
					float chW = measureWidth(chStr, piece.style);
					if(!cur->runs.empty() && cur->width + chW > availableW) {
						lines.push_back(makeEmptyLine(piece.plainStartByte + idx));
						cur = &lines.back();
					}
					appendRunToLine(*cur, chStr, piece.style, piece.plainStartByte + idx, piece.plainStartByte + idx + clen, measureWidth, fontMetrics);
					idx += clen;
				}
			}
		}
	}

	// Compute vertical positions and UTF-8 character index bounds for every line and run.
	float runningY = 0.0f;
	for(gGUIBBCodeLayoutLine& line : lines) {
		line.lineHeight = line.ascent + line.descent + lineTopMargin + 4.0f;
		line.topY = runningY;
		line.baselineY = runningY + line.ascent + 2.0f;
		runningY += line.lineHeight;

		line.plainStartChar = byteOffsetToCharIndex(doc, line.plainStartByte);
		line.plainEndChar = byteOffsetToCharIndex(doc, line.plainEndByte);
		for(gGUIBBCodeLayoutRun& run : line.runs) {
			run.plainStartChar = byteOffsetToCharIndex(doc, run.plainStartByte);
			run.plainEndChar = byteOffsetToCharIndex(doc, run.plainEndByte);
		}
	}

	return lines;
}

gGUIBBCodeHitResult gGUIBBCodeParser::hitTestLine(
	const gGUIBBCodeLayoutLine& line,
	int lineIndex,
	float localX,
	const MeasureWidthFunc& measureWidth) {
	gGUIBBCodeHitResult res;
	res.lineIndex = lineIndex;
	res.charIndex = line.plainStartChar;
	res.byteOffset = line.plainStartByte;
	res.cursorX = 0.0f;

	if(line.runs.empty() || localX <= 0.0f) {
		return res;
	}

	if(localX >= line.width) {
		res.charIndex = line.plainEndChar;
		res.byteOffset = line.plainEndByte;
		res.cursorX = line.width;
		return res;
	}

	for(const gGUIBBCodeLayoutRun& run : line.runs) {
		if(localX < run.x) {
			res.charIndex = run.plainStartChar;
			res.byteOffset = run.plainStartByte;
			res.cursorX = run.x;
			return res;
		}
		if(localX <= run.x + run.width) {
			float targetInRun = localX - run.x;
			int bytePos = 0;
			int charOffset = 0;
			float prevW = 0.0f;
			const int rlen = static_cast<int>(run.text.size());

			while(bytePos < rlen) {
				int clen = utf8CharLength(run.text, bytePos);
				if(clen <= 0) break;
				int nextByte = bytePos + clen;
				float nextW = measureWidth(run.text.substr(0, static_cast<size_t>(nextByte)), run.style);
				if(targetInRun < prevW + (nextW - prevW) * 0.5f) {
					res.charIndex = run.plainStartChar + charOffset;
					res.byteOffset = run.plainStartByte + bytePos;
					res.cursorX = run.x + prevW;
					return res;
				}
				prevW = nextW;
				bytePos = nextByte;
				charOffset++;
			}

			res.charIndex = run.plainEndChar;
			res.byteOffset = run.plainEndByte;
			res.cursorX = run.x + run.width;
			return res;
		}
	}

	res.charIndex = line.plainEndChar;
	res.byteOffset = line.plainEndByte;
	res.cursorX = line.width;
	return res;
}

gGUIBBCodeHitResult gGUIBBCodeParser::hitTest(
	const std::vector<gGUIBBCodeLayoutLine>& lines,
	float localX,
	float localY,
	const MeasureWidthFunc& measureWidth,
	float lineOffsetX) {
	if(lines.empty()) return gGUIBBCodeHitResult{};

	int targetLine = 0;
	if(localY < 0.0f) {
		targetLine = 0;
	} else {
		targetLine = static_cast<int>(lines.size()) - 1;
		for(size_t i = 0; i < lines.size(); ++i) {
			if(localY >= lines[i].topY && localY < lines[i].topY + lines[i].lineHeight) {
				targetLine = static_cast<int>(i);
				break;
			}
		}
	}

	return hitTestLine(lines[targetLine], targetLine, localX - lineOffsetX, measureWidth);
}

gGUIBBCodeLinkHit gGUIBBCodeParser::hitTestLink(
	const std::vector<gGUIBBCodeLayoutLine>& lines,
	float localX,
	float localY,
	const std::vector<float>& lineOffsetsX) {
	gGUIBBCodeLinkHit result;
	if(lines.empty()) return result;

	for(size_t i = 0; i < lines.size(); ++i) {
		const gGUIBBCodeLayoutLine& line = lines[i];
		if(localY < line.topY || localY >= line.topY + line.lineHeight) {
			continue;
		}

		float offsetX = (i < lineOffsetsX.size()) ? lineOffsetsX[i] : 0.0f;
		float x = localX - offsetX;

		for(size_t r = 0; r < line.runs.size(); ++r) {
			const gGUIBBCodeLayoutRun& run = line.runs[r];
			if(!run.style.hasUrl || run.width <= 0.0f) continue;
			if(x >= run.x && x <= run.x + run.width) {
				float runTop = std::max(line.topY, line.baselineY - run.ascent - 2.0f);
				float runBottom = std::min(line.topY + line.lineHeight, line.baselineY + run.descent + 3.0f);
				if(localY < runTop || localY > runBottom) continue;

				result.hit = true;
				result.linkId = run.style.linkId;
				result.url = run.style.url;
				result.lineIndex = static_cast<int>(i);
				result.runIndex = static_cast<int>(r);

				// Reconstruct full visible label for this link occurrence (including multi-run or wrapped links).
				int lastLineIdx = -1;
				for(size_t li = 0; li < lines.size(); ++li) {
					for(const gGUIBBCodeLayoutRun& lr : lines[li].runs) {
						if(lr.style.hasUrl && lr.style.linkId == result.linkId) {
							if(lastLineIdx >= 0 && lastLineIdx != static_cast<int>(li) && !result.displayText.empty() && result.displayText.back() != ' ' && !lr.text.empty() && lr.text.front() != ' ') {
								result.displayText.push_back(' ');
							}
							result.displayText += lr.text;
							lastLineIdx = static_cast<int>(li);
						}
					}
				}
				return result;
			}
		}
		break;
	}

	return result;
}

std::vector<gGUIBBCodeSelectionRect> gGUIBBCodeParser::computeSelectionRects(
	const std::vector<gGUIBBCodeLayoutLine>& lines,
	int selStartByte,
	int selEndByte,
	const MeasureWidthFunc& measureWidth) {
	std::vector<gGUIBBCodeSelectionRect> rects;
	if(lines.empty() || selStartByte == selEndByte) return rects;

	int u1 = std::min(selStartByte, selEndByte);
	int u2 = std::max(selStartByte, selEndByte);

	for(size_t i = 0; i < lines.size(); ++i) {
		const gGUIBBCodeLayoutLine& line = lines[i];
		if(u2 <= line.plainStartByte || u1 >= line.plainEndByte) {
			// Handle empty line strictly inside a multi-line selection (e.g. blank line between paragraphs).
			if(line.runs.empty() && u1 < line.plainStartByte && u2 > line.plainEndByte) {
				gGUIBBCodeSelectionRect r;
				r.x = 0.0f;
				r.y = line.topY;
				r.w = 4.0f;
				r.h = line.lineHeight;
				r.lineIndex = static_cast<int>(i);
				rects.push_back(r);
			}
			continue;
		}

		int lineSelStart = std::max(u1, line.plainStartByte);
		int lineSelEnd = std::min(u2, line.plainEndByte);
		if(lineSelStart >= lineSelEnd) continue;

		float x1 = measureByteXInLine(line, lineSelStart, measureWidth);
		float x2 = measureByteXInLine(line, lineSelEnd, measureWidth);
		float w = x2 - x1;
		if(w <= 0.0f) w = 4.0f;

		gGUIBBCodeSelectionRect rect;
		rect.x = x1;
		rect.y = line.topY;
		rect.w = w;
		rect.h = line.lineHeight;
		rect.lineIndex = static_cast<int>(i);
		rects.push_back(rect);
	}

	return rects;
}
