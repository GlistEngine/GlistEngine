/*
 * gFont.cpp
 *
 *  Created on: May 17, 2020
 *      Author: noyan
 */

#include "gFont.h"
#include "gTracy.h"

#include <codecvt>
#include <iostream>
#include <locale>
#ifdef ANDROID
#include "gAndroidUtil.h"
#endif


gFont::gFont() = default;

gFont::~gFont() {
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
	FT_Set_Char_Size(fontface,
					 static_cast<int>(fontsize * scale) << 6,
					 static_cast<int>(fontsize * scale) << 6,
					 dpi,
					 dpi);
	lineheight = fontsize * 1.43f;
	letterspacing = 1;
	spacesize = 1;
	border = 3;
	characternumlimit = 1000;

	iskerning = FT_HAS_KERNING(fontface);

	resizeVectors(characternumlimit);

	isloaded = true;
	return true;
}

bool gFont::loadFont(const std::string& fontPath, int size, bool isAntialiased, int dpi) {
	return load(gGetFontsDir() + fontPath, size, isAntialiased, dpi);
}

void gFont::drawText(const std::string& text, float x, float y) {
	G_PROFILE_ZONE_SCOPED_N("gFont::drawText()");
	float posx = x;
	float posy = y;

	std::wstring wtext = s2ws(text);
	size_t len = wtext.length();

	int previous = -1;
	for (size_t i = 0; i < len; ++i) {
		int c = wtext[i];

		if (c == '\n') {
			posy += lineheight;
			posx = x;
		} else {
			if (charproperties.find(c) == charproperties.end()) {
				loadChar(c);
			}
			posx += getKerning(c, previous);
			chartextures[c]->draw(glm::vec2(posx + charproperties[c].leftmargin, posy + charproperties[c].dytop),
								  glm::vec2(charproperties[c].texturewidth, charproperties[c].textureheight));
			posx += charproperties[c].advance * letterspacing * (c == ' ' ? spacesize : 1.0f);
		}
		previous = c;
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
			if (charproperties.find(c) == charproperties.end()) {
				loadChar(c);
			}
			posx += getKerning(c, previous);
			chartextures[c]->draw(glm::vec2(posx + charproperties[c].leftmargin, posy - charproperties[c].dytop),
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
		if (charproperties.find(c) == charproperties.end()) {
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
			if (charproperties.find(c) == charproperties.end()) {
				loadChar(c);
			}

			float kerning = getKerning(c, prevChar);
			posx -= kerning;

			float drawx = posx - charproperties[c].leftmargin - charproperties[c].texturewidth;
			chartextures[c]->draw(glm::vec2(drawx, posy + charproperties[c].dytop),
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
	if (scale > 1) {
		loadflags |= FT_LOAD_NO_HINTING;
	}
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

	int longside = border * 2 + std::max(dataw, datah);

	// Find next power of 2
	int longest = 1;
	while (longside > longest) {
		longest <<= 1;
	}
	int pixelsw = longest;
	int pixelsh = longest;

	std::vector<unsigned char> pixels(pixelsw * pixelsh * 4);
	for (int i = 0; i < pixelsw * pixelsh; ++i) {
		int idx = i * 4;
		pixels[idx] = 255;
		pixels[idx + 1] = 255;
		pixels[idx + 2] = 255;
		pixels[idx + 3] = 0;
	}

	insertData(data.data(), dataw, datah, 4, pixels.data(), pixelsw, pixelsh, 4, border, border);

	chartextures[charCode] = new gTexture(pixelsw, pixelsh, GL_RGBA, false);

	chartextures[charCode]->setFiltering(
			isantialiased ? gTexture::TEXTUREMINMAGFILTER_LINEAR : gTexture::TEXTUREMINMAGFILTER_NEAREST,
			isantialiased ? gTexture::TEXTUREMINMAGFILTER_LINEAR : gTexture::TEXTUREMINMAGFILTER_NEAREST);

	chartextures[charCode]->setData(pixels.data(), pixelsw, pixelsh, 4, false, false);

	// Prepare properties
	CharProperties& props = charproperties[charCode];

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

	props.texturewidth = static_cast<float>(chartextures[charCode]->getWidth()) / scale;
	props.textureheight = static_cast<float>(chartextures[charCode]->getHeight()) / scale;
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
