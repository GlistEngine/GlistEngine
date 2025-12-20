/*
 * Copyright (C) 2016 Nitra Games Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef ENGINE_GRAPHICS_GFONT_H_
#define ENGINE_GRAPHICS_GFONT_H_

#include "gNode.h"
#include <map>
#include <vector>
#ifdef EMSCRIPTEN
#include "ft2build.h"
#include FT_FREETYPE_H
#else
#include "freetype2/ft2build.h"
#endif
#ifdef ANDROID
#include "freetype2/freetype/freetype.h"
#include <android/asset_manager.h>
#endif
#if TARGET_OS_IPHONE || TARGET_OS_SIMULATOR
#include "freetype2/freetype/freetype.h"
#endif
#if defined(WIN32) || defined(LINUX) || TARGET_OS_OSX
#include FT_FREETYPE_H
#endif


#include "gMesh.h"
#include "gTexture.h"
#include <unordered_map>


class gFont : public gNode {
public:
	gFont();
	virtual ~gFont();

	/**
	 * Loads a font from the given fullPath
	 *
	 * Developers could use this function for loading font and giving text's features.
	 *
	 * Developers do not need to enter the isAntialiased and the dpi values because
	 * those values are given automatically.
	 *
	 * Font files may be stored anywhere on the drives.
	 *
	 * @param std::string fullPath The full path to the font file. It should contain the
	 * full path of the folder where the font is located.
	 * @param int size - The size of the font
	 * @param bool isAntialised - This is used in digital imaging to reduce visual imperfections and developers don't need to give value.
	 * @param int dpi - The measure of resolution used for printed text or images and developers don't need to give value
	 *
	 * @return boolean indicating if font is loaded
	 */
	bool load(const std::string& fullPath, int size, bool isAntialiased = true, int dpi = 96);

	/**
	 * Loads a font from the given fontPath
	 *
	 * Developers could use this function for loading font and giving text's features.
	 *
	 * Developers do not need to enter the isAntialiased and the dpi values because
	 * those values are given automatically.
	 *
	 * Font files should be stored in the GlistEngine's assets/fonts folder.
	 *
	 * @param std::string fontPath - The font path to the font file. It should contain the
	 * font path of the folder where the font is located.
	 * @param int size - This is the size of the font. Developers are recommended to use 6 and its multiples.
	 * @param bool isAntialised - This is used in digital imaging to reduce visual imperfections and developers don't need to give value.
	 * @param int dpi - This is a measure of resolution used for printed text or images and developers don't need to give value
	 *
	 * @return boolean indicating if font is loaded
	 */
	bool loadFont(const std::string& fontPath, int size, bool isAntialiased = true, int dpi = 96);

	/**
	 * Draws the text to the given screen coordinates
	 *
	 * @param std::string text - The text to be printed.
	 * @param float x - x coordinate of the region you want to draw
,	 * @param float y - y coordinate of the region you want to draw
	 */
	void drawText(const std::string& text, float x, float y);


	/**
	 * @brief Draws the given text vertically flipped at the specified position.
	 *
	 * @see drawText
	 */
	void drawTextVerticallyFlipped(const std::string& text, float x, float y);

	/**
	 * @brief Draws the given text horizontally flipped at the specified location.
	 *
	 * @see drawText
	 */
	void drawTextHorizontallyFlipped(const std::string& text, float x, float y);

	/**
	 * Calculates text's width
	 *
	 * May cause performance drops when used inside the main loop.
	 *
	 * @param text The text to calculate the width
	 *
	 * @return The width of the given text
	 */
	float getStringWidth(const std::string& text);


	/**
	 * Calculates text's height
	 *
	 * May cause performance drops when used inside the main loop.
	 *
	 * @param text The text to calculate the height
	 *
	 * @return The height of the given text
	 */
	float getStringHeight(const std::string& text);

	/**
	 * Returns the line's height
	 *
	 * @return Line height
	 */
	float getLineHeight() const;

	/**
	 * Returns the path of the loaded font.
	 *
	 * @return The path of the font
	 */
	const std::string& getPath() const;

	/**
	 * Returns the font's size
	 *
	 * @returns Font size
	 */
	int getSize() const;

	/**
	 * Returns if the font is loaded or not
	 *
	 * @returns The font is loaded or not
	 */
	bool isLoaded() const;

	/**
	 * Returns if the antialiasing is enabled
	 *
	 * @returns The antialiasing
	 */
	bool isAntialised() const;

	/**
	 * Returns the dpi of the font
	 * Dpi is a measure of resolution used for printed text or images,
	 * the higher the dots per inch, the higher the resolution.
	 *
	 * @returns The dpi value
	 */
	int getDpi() const;

private:
	bool isloaded = false;
	std::string fullpath;
	bool isantialiased = false;
	int dpi = 0;
	float fontsize = 0.0f;
	int characternumlimit = 0;
	int border = 3;
	bool iskerning = false;

	FT_Library ftlib = nullptr;
	FT_Face fontface = nullptr;

	float lineheight = 0.0f;
	float letterspacing = 1.0f;
	float spacesize = 1.0f;
	float scale = 1.0f;

	void loadChar(int charCode);

	struct CharProperties {
		float height = 0.0f, width = 0.0f;
		float topmargin = 0.0f, leftmargin = 0.0f;
		float advance = 0.0f;
		float texturewidth = 0.0f, textureheight = 0.0f;
		float dxleft = 0.0f, dxright = 0.0f, dytop = 0.0f, dybottom = 0.0f;
	};

	std::unordered_map<int, CharProperties> charproperties;
	std::unordered_map<int, gTexture*> chartextures;

	float getKerning(int c, int prevC) const;
	void resizeVectors(int num);
	bool insertData(const unsigned char* srcData, int srcWidth, int srcHeight, int componentNum,
					unsigned char* dstData, int dstWidth, int dstHeight, int dstComponentNum,
					size_t dstFirstX, size_t dstFirstY) const;
	std::wstring s2ws(const std::string& s) const;
	float roundIfRequired(float val);
};

#endif /* ENGINE_GRAPHICS_GFONT_H_ */
