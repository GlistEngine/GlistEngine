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
#include "freetype2/ft2build.h"
#ifdef ANDROID
#include "freetype2/freetype/freetype.h"
#include <android/asset_manager.h>
#endif
#if defined(WIN32) || defined(LINUX) || defined(APPLE)
#include FT_FREETYPE_H
#endif


#include "gTexture.h"
#include "gMesh.h"



class gFont: public gNode {
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
	int getSize();

	/**
	 * Returns if the font is loaded or not
	 *
	 * @returns The font is loaded or not
	 */
	bool isLoaded();

	/**
	 * Returns if the antialiasing is enabled
	 *
	 * @returns The antialiasing
	 */
	bool isAntialised();

	/**
	 * Returns the dpi of the font
	 * Dpi is a measure of resolution used for printed text or images,
	 * the higher the dots per inch, the higher the resolution.
	 *
	 * @returns The dpi value
	 */
	int getDpi();

private:
	  bool isloaded;
	  std::string fullpath;
	  bool isantialiased;
	  int dpi;
	  int fontsize;
	  int characternumlimit;
	  int border;
	  bool iskerning;

	  FT_Library ftlib;
	  FT_Face fontface;

	  float lineheight;
	  float letterspacing;
	  float	spacesize;

	  int getCharID(const int& c);
	  void loadChar(const int& charID);

	  typedef struct {
	    int character;
	    int height, width;
	    int topmargin,leftmargin;
	    int advance;
	    float texturewidth,textureheight;
	    float dxleft, dxright, dytop, dybottom;
	  } charProperties;

	  std::vector<charProperties> cpset;
	  std::vector<gTexture> textures;
	  std::vector<int> loadedcharacters;
	  static const int unloadedchar = 0;
#ifdef ANDROID
      AAsset* androidasset;
#endif


	  int getKerning(int c, int prevC);
	  void resizeVectors(int num);
	  bool insertData(unsigned char* srcData, int srcWidth, int srcHeight, int componentNum, unsigned char* dstData, int dstWidth, int dstHeight, int dstComponentNum, size_t dstFirstX, size_t dstFirstY) const;
	  std::wstring s2ws(const std::string& s);

	  GLint index1, posx1, posy1, index2, posx2, index3, posy3;
	  std::wstring text1, text2, text3;
	  int len1, c1, cid1, cold1, len2, cid2, cold2, cy2, len3, cid3, cy3, y3;
	  int tempint, tempcharno;
	  int lci, lci2, lci3, lci4, lcj, lck;
	  int lcdataw, lcdatah, lcdatanum;
	  int lclongside, lclongest, lcpixelsw, lcpixelsh, lcapsize;
	  FT_Error lcerr;
	  GLint lcfheight, lcbwidth, lctop, lclextent;
	  GLfloat lcstretch, lccorr;
	  unsigned char* bitmappixels;
	  unsigned char* lcsrc;
	  unsigned char* lcbptr;
	  unsigned char lcb;
};

#endif /* ENGINE_GRAPHICS_GFONT_H_ */
