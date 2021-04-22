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

	bool load(std::string fullPath, int size, bool isAntialiased = true, int dpi = 96);
	bool loadFont(std::string fontPath, int size, bool isAntialiased = true, int dpi = 96);

	void drawText(std::string text, float x, float y);

	float getStringWidth(std::string text);
	float getStringHeight(std::string text);

	std::string getPath();
	int getSize();
	bool isLoaded();
	bool isAntialised();
	int getDpi();

private:
	  bool isloaded;
	  std::string fullpath;
	  bool isantialiased;
	  int dpi;
	  int fontsize;
	  int characternumlimit;
	  int border;

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


	  void resizeVectors(int num);
	  bool insertData(unsigned char* srcData, int srcWidth, int srcHeight, int componentNum, unsigned char* dstData, int dstWidth, int dstHeight, int dstComponentNum, size_t dstFirstX, size_t dstFirstY) const;
	  std::wstring s2ws(const std::string& s);

	  GLint index1, posx1, posy1, index2, posx2, index3, posy3;
	  std::wstring text1, text2, text3;
	  int len1, c1, cid1, len2, cid2, cy2, len3, cid3, cy3, y3;
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
