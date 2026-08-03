/*
 * gFont.cpp
 *
 *  Created on: May 17, 2020
 *      Author: noyan
 */

#include "gFont.h"
#include "gWindowEvents.h"
#include "gTracy.h"

#include <cstring>
#include <codecvt>
#include <iostream>
#include <locale>
#ifdef ANDROID
#include "gAndroidUtil.h"
#endif


gFont::gFont() = default;

gFont::~gFont() {
	delete atlastexture;
	atlastexture = nullptr;
	if (fontface) {
		for (std::pair<const int, gTexture*> pair : chartextures) {
			delete pair.second;
		}
		chartextures.clear();
		charproperties.clear();
		FT_Done_Face(fontface);
		FT_Done_FreeType(ftlib);
	}
}

bool gFont::load(const std::string& fullPath, int size, bool isAntialiased, int dpi) {
	G_PROFILE_ZONE_SCOPED_N("gFont::load()");
	fullpath = fullPath;
	isantialiased = isAntialiased;
	fontsize = size;
	this->dpi = dpi;

	//--------------- load the library and typeface
	FT_Error err = FT_Init_FreeType(&ftlib);
	if (err) {
		gLoge("gFont") << "Error loading freetype";
		return false;
	}
	err = FT_New_Face(ftlib, fullPath.c_str(), 0, &fontface);
	if (err) {
		const char* errorstr = (err == 1) ? "wrong file name" : "freetype error";
		gLoge("gFont") << "Freetype error: " << errorstr;
		return false;
	}

	scale = renderer->getScaleMultiplier();
	int scaledsize = static_cast<int>(std::round(fontsize * scale));
	FT_Set_Char_Size(fontface,
					 scaledsize << 6,
					 scaledsize << 6,
					 dpi,
					 dpi);
	lineheight = fontsize * 1.43f;
	letterspacing = 1;
	spacesize = 1;
	border = 4;
	characternumlimit = 1000;

	iskerning = FT_HAS_KERNING(fontface);

	if (!buildAtlas()) {
		resizeVectors(characternumlimit);
	}

	isloaded = true;
	return true;
}

bool gFont::loadFont(const std::string& fontPath, int size, bool isAntialiased, int dpi) {
	return load(gGetFontsDir() + fontPath, size, isAntialiased, dpi);
}

void gFont::setTextRenderMode(TextRenderMode renderMode) {
	textrendermode = renderMode;
}

gFont::TextRenderMode gFont::getTextRenderMode() const {
	return textrendermode;
}

void gFont::getVisualBoundsX(const std::string& text, float& xmin, float& xmax) {
	float posx = 0.0f;

	std::wstring wtext = s2ws(text);
	size_t len = wtext.length();

	int previous = -1;

	xmin = std::numeric_limits<float>::max();
	xmax = std::numeric_limits<float>::lowest();

	const float b = border / scale;

	for(size_t i = 0; i < len; ++i) {
		int c = wtext[i];
		if(c == '\n') break;

		if(charproperties.find(c) == charproperties.end()) {
			loadChar(c);
		}

		posx += getKerning(c, previous);

		const CharProperties& p = charproperties[c];

		float gx1 = roundIfRequired(posx + p.dxleft  + b);
		float gx2 = roundIfRequired(posx + p.dxright + b);

		if(gx1 < xmin) xmin = gx1;
		if(gx2 > xmax) xmax = gx2;

		posx += p.advance * letterspacing * (c == ' ' ? spacesize : 1.0f);
		previous = c;
	}

	if(xmin == std::numeric_limits<float>::max()) {
		xmin = 0.0f;
		xmax = 0.0f;
	}
}

void gFont::onEvent(gEvent& event) {
	gEventDispatcher dispatcher(event);
	dispatcher.dispatch<gWindowScaleChangedEvent>([this](gWindowScaleChangedEvent& e) -> bool {
		reloadFont();
		return false;
	});
}

void gFont::reloadFont() {
	if (!isloaded || !fontface) {
		return;
	}

	float newscale = renderer->getScaleMultiplier();
	if (newscale == scale) {
		return;
	}

	scale = newscale;
	int scaledsize = static_cast<int>(std::round(fontsize * scale));
	FT_Set_Char_Size(fontface,
					 scaledsize << 6,
					 scaledsize << 6,
					 dpi,
					 dpi);

	// Clear all cached glyphs
	for (std::pair<const int, gTexture*> pair : chartextures) {
		delete pair.second;
	}
	chartextures.clear();
	charproperties.clear();
	delete atlastexture;
	atlastexture = nullptr;

	if (!buildAtlas()) {
		// Reload space character for the legacy per-glyph texture path.
		loadChar(' ');
	}
}

bool gFont::buildAtlas() {
	if (!fontface) return false;

	delete atlastexture;
	atlastexture = nullptr;
	charproperties.clear();
	atlaspixels.resize(static_cast<size_t>(atlaswidth) * atlasheight * 4);
	for (size_t i = 0; i < atlaspixels.size(); i += 4) {
		atlaspixels[i] = 255;
		atlaspixels[i + 1] = 255;
		atlaspixels[i + 2] = 255;
		atlaspixels[i + 3] = 0;
	}
	atlascursorx = 0;
	atlascursory = 0;
	atlasrowheight = 0;
	atlasbuilding = true;

	// The existing font cache limit is 1000 codepoints. Prepacking the same range
	// covers Latin, Latin Extended (including Turkish), Greek and Cyrillic while
	// keeping the atlas immutable after it has entered a submitted frame.
	for (int c = 32; c < characternumlimit; ++c) {
		if (FT_Get_Char_Index(fontface, c) != 0 || c == ' ') loadChar(c);
	}
	atlasbuilding = false;

	bool hasglyph = false;
	for (const auto& entry : charproperties) {
		if (entry.second.inatlas) {
			hasglyph = true;
			break;
		}
	}
	if (!hasglyph) {
		atlaspixels.clear();
		return false;
	}

	atlastexture = new gTexture(atlaswidth, atlasheight, GL_RGBA, false);
	atlastexture->setFiltering(
			isantialiased ? gTexture::TEXTUREMINMAGFILTER_LINEAR : gTexture::TEXTUREMINMAGFILTER_NEAREST,
			isantialiased ? gTexture::TEXTUREMINMAGFILTER_LINEAR : gTexture::TEXTUREMINMAGFILTER_NEAREST);
	atlastexture->setData(atlaspixels.data(), atlaswidth, atlasheight, 4, false, false);
	atlaspixels.clear();
	atlaspixels.shrink_to_fit();

	gLogi("gFont") << "Glyph atlas ready: " << atlaswidth << "x" << atlasheight
			<< ", cached glyphs: " << charproperties.size();
	return true;
}

void gFont::drawText(const std::string& text, float x, float y) {
	G_PROFILE_ZONE_SCOPED_N("gFont::drawText()");
	std::wstring wtext = s2ws(text);
	bool canbatch = textrendermode == TextRenderMode::ATLAS && atlastexture != nullptr && !wtext.empty();
	for (wchar_t wc : wtext) {
		const int c = static_cast<int>(wc);
		if (c == '\n') continue;
		auto it = charproperties.find(c);
		if (it == charproperties.end() || !it->second.inatlas) {
			canbatch = false;
			break;
		}
	}

	if (canbatch) {
		batchvertices.clear();
		batchvertices.reserve(wtext.size() * 6 * 4);
		float posx = x;
		float posy = y;
		int previous = -1;

		auto appendvertex = [this](float px, float py, float u, float v) {
			batchvertices.push_back(px);
			batchvertices.push_back(py);
			batchvertices.push_back(u);
			batchvertices.push_back(v);
		};

		for (wchar_t wc : wtext) {
			const int c = static_cast<int>(wc);
			if (c == '\n') {
				posy += lineheight;
				posx = x;
				previous = -1;
				continue;
			}

			const CharProperties& p = charproperties[c];
			posx += getKerning(c, previous);
			const float x0 = roundIfRequired(posx + p.leftmargin);
			const float y0 = roundIfRequired(posy + p.dytop);
			const float x1 = x0 + p.texturewidth;
			const float y1 = y0 + p.textureheight;

			appendvertex(x0, y0, p.atlasu0, p.atlasv0);
			appendvertex(x1, y0, p.atlasu1, p.atlasv0);
			appendvertex(x1, y1, p.atlasu1, p.atlasv1);
			appendvertex(x0, y0, p.atlasu0, p.atlasv0);
			appendvertex(x1, y1, p.atlasu1, p.atlasv1);
			appendvertex(x0, y1, p.atlasu0, p.atlasv1);

			posx += p.advance * letterspacing * (c == ' ' ? spacesize : 1.0f);
			previous = c;
		}

		if (!batchvertices.empty()) {
			renderer->setProjectionMatrix2d(glm::ortho(0.0f, static_cast<float>(renderer->getWidth()),
					static_cast<float>(renderer->getHeight()), 0.0f, -1.0f, 1.0f));
			gColor* color = renderer->getColor();
			const glm::vec4 tint(color->r, color->g, color->b, color->a);
			renderer->drawTexturedTriangles2D(atlastexture->getId(), tint, renderer->getProjectionMatrix2d(),
					batchvertices.data(), static_cast<int>(batchvertices.size() / 4));
		}
	} else {
		float posx = x;
		float posy = y;
		int previous = -1;
		for (wchar_t wc : wtext) {
			const int c = static_cast<int>(wc);
			if (c == '\n') {
				posy += lineheight;
				posx = x;
				previous = -1;
				continue;
			}
			if (chartextures.find(c) == chartextures.end()) loadChar(c);
			auto texture = chartextures.find(c);
			if (texture == chartextures.end() || texture->second == nullptr) continue;

			const CharProperties& p = charproperties[c];
			posx += getKerning(c, previous);
			const float drawx = roundIfRequired(posx + p.leftmargin);
			const float drawy = roundIfRequired(posy + p.dytop);
			texture->second->draw(glm::vec2(drawx, drawy), glm::vec2(p.texturewidth, p.textureheight));
			posx += p.advance * letterspacing * (c == ' ' ? spacesize : 1.0f);
			previous = c;
		}
	}
}

void gFont::drawTextVerticallyFlipped(const std::string& text, float x, float y) {
	G_PROFILE_ZONE_SCOPED_N("gFont::drawTextVerticallyFlipped()");
	float posx = x;
	float posy = y;

	std::wstring wtext = s2ws(text);
	size_t len = wtext.length();

	int previous = -1;
	for (size_t i = 0; i < len; ++i) {
		int c = wtext[i];

		if (c == '\n') {
			posy -= lineheight;
			posx = x;
		} else {
			if (chartextures.find(c) == chartextures.end()) {
				loadChar(c);
			}
			if (chartextures.find(c) == chartextures.end() || chartextures[c] == nullptr) continue;
			posx += getKerning(c, previous);
			float drawx = roundIfRequired(posx + charproperties[c].leftmargin);
			float drawy = roundIfRequired(posy - charproperties[c].dytop);
			chartextures[c]->draw(glm::vec2(drawx, drawy),
								  glm::vec2(charproperties[c].texturewidth, -charproperties[c].textureheight));
			posx += charproperties[c].advance * letterspacing * (c == ' ' ? spacesize : 1.0f);
		}
		previous = c;
	}
}

void gFont::drawTextHorizontallyFlipped(const std::string& text, float x, float y) {
	G_PROFILE_ZONE_SCOPED_N("gFont::drawTextHorizontallyFlipped()");
	float posy = y;

	std::wstring wtext = s2ws(text);
	size_t len = wtext.length();

	// Calculate the total width
	float totalwidth = 0.0f;
	for (size_t i = 0; i < len; ++i) {
		int c = wtext[i];
		if (chartextures.find(c) == chartextures.end()) {
			loadChar(c);
		}
		totalwidth += charproperties[c].advance * letterspacing * (c == ' ' ? spacesize : 1.0f);
	}

	if (len > 0) {
		int lastchar = wtext[len - 1];
		totalwidth += charproperties[lastchar].advance * letterspacing * (lastchar == ' ' ? spacesize : 1.0f);
	}

	// Set the starting position based on the total text width
	float posx = x + totalwidth;

	int prevChar = -1;
	for (int i = 0; i < len; ++i) {
		int c = wtext[i];

		if (c == '\n') {
			posy += lineheight;
			posx = x + totalwidth;
		} else {
			if (chartextures.find(c) == chartextures.end()) {
				loadChar(c);
			}
			if (chartextures.find(c) == chartextures.end() || chartextures[c] == nullptr) continue;

			float kerning = getKerning(c, prevChar);
			posx -= kerning;
			float drawx = roundIfRequired(posx - charproperties[c].leftmargin - charproperties[c].texturewidth);
			float drawy = roundIfRequired(posy + charproperties[c].dytop);
			chartextures[c]->draw(glm::vec2(drawx, drawy),
								  glm::vec2(-charproperties[c].texturewidth, charproperties[c].textureheight));
			posx -= charproperties[c].advance * letterspacing * (c == ' ' ? spacesize : 1.0f);
		}
		prevChar = c;
	}
}

float gFont::getStringWidth(const std::string& text) {
	G_PROFILE_ZONE_SCOPED_N("gFont::getStringWidth()");
	float width = 0.0f;

	std::wstring wtext = s2ws(text);
	size_t len = wtext.length();

	int previous = -1;
	for (size_t i = 0; i < len; ++i) {
		int c = wtext[i];
		if (charproperties.find(c) == charproperties.end()) {
			loadChar(c);
		}
		width += getKerning(c, previous);
		width += charproperties[c].advance * letterspacing * (c == ' ' ? spacesize : 1.0f);
		previous = c;
	}

	return width;
}

float gFont::getStringHeight(const std::string& text) {
	G_PROFILE_ZONE_SCOPED_N("gFont::getStringHeight()");
	std::wstring wtext = s2ws(text);
	size_t len = wtext.length();

	float maxheight = 0.0f;
	for (size_t i = 0; i < len; ++i) {
		int c = wtext[i];
		if (charproperties.find(c) == charproperties.end()) {
			loadChar(c);
		}
		float charHeight = -charproperties[c].dytop;
		if (charHeight > maxheight) {
			maxheight = charHeight;
		}
	}
	return maxheight;
}

float gFont::getLineHeight() const {
	return lineheight;
}

const std::string& gFont::getPath() const {
	return fullpath;
}

int gFont::getSize() const {
	return static_cast<int>(fontsize);
}

bool gFont::isLoaded() const {
	return isloaded;
}

bool gFont::isAntialised() const {
	return isantialiased;
}

int gFont::getDpi() const {
	return dpi;
}

void gFont::resizeVectors(int num) {
	G_PROFILE_ZONE_SCOPED_N("gFont::resizeVectors()");
	G_PROFILE_ZONE_VALUE(num);
	if (num <= 0) {
		return;
	}

	characternumlimit = num;

	charproperties.clear();
	for (std::pair<const int, gTexture*> pair : chartextures) {
		delete pair.second;
	}
	chartextures.clear();

	// load ' ' character for display space char
	loadChar(' ');
}

void gFont::loadChar(int charCode) {
	G_PROFILE_ZONE_SCOPED_N("gFont::loadChar()");
	FT_Int32 loadflags = isantialiased ? FT_LOAD_TARGET_NORMAL : FT_LOAD_MONOCHROME;
	FT_Error error = FT_Load_Glyph(fontface, FT_Get_Char_Index(fontface, charCode), loadflags);
	if (error) {
		gLoge("gFont") << "Error FT_Load_Glyph";
		return;
	}

	FT_GlyphSlot glyph = fontface->glyph;
	FT_Render_Glyph(glyph, isantialiased ? FT_RENDER_MODE_NORMAL : FT_RENDER_MODE_MONO);

	FT_Bitmap& bitmap = glyph->bitmap;

	int dataw = bitmap.width;
	int datah = bitmap.rows;

	std::vector<unsigned char> data(datah * dataw * 4);
	int datanum = dataw * datah;

	// Initialize with white color and transparent alpha
	for (int i = 0; i < datanum; ++i) {
		int idx = i * 4;
		data[idx] = 255;
		data[idx + 1] = 255;
		data[idx + 2] = 255;
		data[idx + 3] = 0;
	}

	if (isantialiased) {
		for (int i = 0; i < datanum; ++i) {
			data[i * 4 + 3] = bitmap.buffer[i];
		}
	} else {
		unsigned char* src = bitmap.buffer;
		for (int i = 0; i < bitmap.rows; ++i) {
			unsigned char lcb = 0;
			unsigned char* bptr = src;
			for (int j = 0; j < bitmap.width; ++j) {
				data[(j + i * dataw) * 4] = 255;
				if (j % 8 == 0) {
					lcb = *bptr++;
				}
				data[(j + i * dataw) * 4 + 3] = (lcb & 0x80) ? 255 : 0;
				lcb <<= 1;
			}
			src += bitmap.pitch;
		}
	}

	int pixelsw;
	int pixelsh;
	if (atlasbuilding) {
		pixelsw = border * 2 + dataw;
		pixelsh = border * 2 + datah;
	} else {
		const int longside = border * 2 + std::max(dataw, datah);
		// The legacy per-glyph path keeps its original square power-of-two texture.
		int longest = 1;
		while (longside > longest) longest <<= 1;
		pixelsw = longest;
		pixelsh = longest;
	}

	std::vector<unsigned char> pixels(pixelsw * pixelsh * 4);
	for (int i = 0; i < pixelsw * pixelsh; ++i) {
		int idx = i * 4;
		pixels[idx] = 255;
		pixels[idx + 1] = 255;
		pixels[idx + 2] = 255;
		pixels[idx + 3] = 0;
	}

	insertData(data.data(), dataw, datah, 4, pixels.data(), pixelsw, pixelsh, 4, border, border);

	// Prepare properties
	CharProperties& props = charproperties[charCode];
	if (atlasbuilding) {
		if (atlascursorx + pixelsw > atlaswidth) {
			atlascursorx = 0;
			atlascursory += atlasrowheight;
			atlasrowheight = 0;
		}
		if (atlascursory + pixelsh <= atlasheight) {
			insertData(pixels.data(), pixelsw, pixelsh, 4, atlaspixels.data(), atlaswidth, atlasheight, 4,
					static_cast<size_t>(atlascursorx), static_cast<size_t>(atlascursory));
			props.atlasu0 = static_cast<float>(atlascursorx) / atlaswidth;
			props.atlasv0 = static_cast<float>(atlascursory) / atlasheight;
			props.atlasu1 = static_cast<float>(atlascursorx + pixelsw) / atlaswidth;
			props.atlasv1 = static_cast<float>(atlascursory + pixelsh) / atlasheight;
			props.inatlas = true;
			atlascursorx += pixelsw;
			atlasrowheight = std::max(atlasrowheight, pixelsh);
		} else {
			props.inatlas = false;
		}
	} else {
		auto existing = chartextures.find(charCode);
		if (existing != chartextures.end()) delete existing->second;
		chartextures[charCode] = new gTexture(pixelsw, pixelsh, GL_RGBA, false);
		chartextures[charCode]->setFiltering(
				isantialiased ? gTexture::TEXTUREMINMAGFILTER_LINEAR : gTexture::TEXTUREMINMAGFILTER_NEAREST,
				isantialiased ? gTexture::TEXTUREMINMAGFILTER_LINEAR : gTexture::TEXTUREMINMAGFILTER_NEAREST);
		chartextures[charCode]->setData(pixels.data(), pixelsw, pixelsh, 4, false, false);
	}

	// Simply divide all metrics by scale - no rounding
	props.height = static_cast<float>(glyph->bitmap_top) / scale;
	props.width = static_cast<float>(glyph->bitmap.width) / scale;
	props.topmargin = static_cast<float>(glyph->bitmap.rows) / scale;
	props.leftmargin = static_cast<float>(glyph->bitmap_left) / scale;

	float lctop = props.topmargin - props.height;
	float lccorr = ((fontsize - props.height) + lctop) - fontsize;

	props.dxleft = props.leftmargin;
	props.dytop = -lctop + lccorr;
	props.dxright = props.leftmargin + props.width;
	props.dybottom = props.height + lccorr;

	props.texturewidth = static_cast<float>(pixelsw) / scale;
	props.textureheight = static_cast<float>(pixelsh) / scale;
	// Divide by 64 first (26.6 fixed point), then by scale to preserve precision
	props.advance = (glyph->advance.x / 64.0f) / scale;
}

bool gFont::insertData(const unsigned char* srcData, int srcWidth, int srcHeight, int componentNum,
					   unsigned char* dstData, int dstWidth, int dstHeight, int dstComponentNum,
					   size_t dstFirstX, size_t dstFirstY) const {
	G_PROFILE_ZONE_SCOPED_N("gFont::insertData()");

	size_t copywidth = std::min(static_cast<size_t>(srcWidth), static_cast<size_t>(dstWidth) - dstFirstX) * componentNum;
	size_t copyheight = std::min(static_cast<size_t>(srcHeight), static_cast<size_t>(dstHeight) - dstFirstY);

	unsigned char* dstpixel = dstData + ((dstFirstX + dstFirstY * dstWidth) * dstComponentNum);
	unsigned char* srcpixel = const_cast<unsigned char*>(srcData);
	size_t srcstride = srcWidth * componentNum;
	size_t dststride = dstWidth * dstComponentNum;

	for (size_t y = 0; y < copyheight; ++y) {
		std::memcpy(dstpixel, srcpixel, copywidth);
		dstpixel += dststride;
		srcpixel += srcstride;
	}

	return true;
}

float gFont::getKerning(int c, int previousC) const {
	G_PROFILE_ZONE_SCOPED_N("gFont::getKerning()");
	if (!fontface || !iskerning || previousC == -1) {
		return 0.0f;
	}

	FT_UInt index1 = FT_Get_Char_Index(fontface, previousC);
	FT_UInt index2 = FT_Get_Char_Index(fontface, c);

	FT_Vector kerning;
	FT_Get_Kerning(fontface, index1, index2, FT_KERNING_DEFAULT, &kerning);

	// X advance is already in pixels for bitmap fonts
	if (!FT_IS_SCALABLE(fontface)) {
		return kerning.x / scale;
	}

	return (kerning.x / scale) / 64.0f;
}

#ifdef WIN32
#include <windows.h>
#endif

std::wstring gFont::s2ws(const std::string& s) const {
	G_PROFILE_ZONE_SCOPED_N("gFont::s2ws()");
#ifdef WIN32
	int size_needed = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, nullptr, 0);
	std::wstring wstr(size_needed, 0);
	MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, &wstr[0], size_needed);
	wstr.pop_back(); // remove null terminator
	return wstr;
#else
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
	return converter.from_bytes(s);
#endif
}

float gFont::roundIfRequired(float val) {
	if (scale > 1) {
		return std::round(val * scale) / scale;
	}
	return val;
}

/*
 * Writing By: Engin Kutlu
 * */
std::vector<std::string> gFont::wrapSentenceByWidth(const std::string& text, float maxWidth, TextAlign align) {
    std::vector<std::string> lines;
    std::vector<std::string> words;
    std::vector<float> widths;

    size_t start = 0;
    size_t end;

    while ((end = text.find(' ', start)) != std::string::npos) {
        std::string word = text.substr(start, end - start);
        if (!word.empty()) {
            words.push_back(word);
            widths.push_back(getStringWidth(word));
        }
        start = end + 1;
    }

    std::string lastword = text.substr(start);
    if (!lastword.empty()) {
        words.push_back(lastword);
        widths.push_back(getStringWidth(lastword));
    }

    float spacewidth = getStringWidth(" ");
    float currentwidth = 0.0f;
    std::string currentline;

    for (size_t i = 0; i < words.size(); i++) {
        float newwidth = currentline.empty()
            ? widths[i]
            : currentwidth + spacewidth + widths[i];

        if (newwidth <= maxWidth) {
            if (!currentline.empty())
                currentline += " ";
            currentline += words[i];
            currentwidth = newwidth;
        } else {
            if (!currentline.empty())
                lines.push_back(currentline);
            currentline = words[i];
            currentwidth = widths[i];
        }
    }

    if (!currentline.empty())
        lines.push_back(currentline);

    for (size_t li = 0; li < lines.size(); li++) {
        std::string& line = lines[li];

        if (align == TextAlign::JUSTIFY) {
            if (li == lines.size() - 1)
                continue;

            std::vector<std::string> parts;
            size_t pos = 0, found;

            while ((found = line.find(' ', pos)) != std::string::npos) {
                parts.push_back(line.substr(pos, found - pos));
                pos = found + 1;
            }
            parts.push_back(line.substr(pos));

            if (parts.size() <= 1)
                continue;

            float wordswidth = 0.0f;
            for (const auto& y : parts)
                wordswidth += getStringWidth(y);

            float extraspace = maxWidth - wordswidth;
            if (extraspace <= 0.0f)
                continue;

            int gapcount = static_cast<int>(parts.size()) - 1;
            int totalspaces = static_cast<int>(extraspace / spacewidth);

            int basespaces = totalspaces / gapcount;
            int remainder = totalspaces % gapcount;

            std::string justified;
            for (int i = 0; i < parts.size(); i++) {
                justified += parts[i];
                if (i < gapcount) {
                	int spaces = basespaces + (i >= gapcount - remainder ? 1 : 0);
                    justified += std::string(spaces, ' ');
                }
            }

            line = justified;
        } else if (align == TextAlign::RIGHT || align == TextAlign::CENTER) {
            float linewidth = getStringWidth(line);
            float extraspace = maxWidth - linewidth;
            if (extraspace <= 0.0f)
                continue;

            int spacecount = static_cast<int>(extraspace / spacewidth);

            if (align == TextAlign::RIGHT) {
                line = std::string(spacecount, ' ') + line;
            } else if (align == TextAlign::CENTER) {
                int leftspaces = spacecount / 2;
                line = std::string(leftspaces, ' ') + line;
            }
        }
    }

    return lines;
}
