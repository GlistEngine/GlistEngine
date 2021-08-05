/*
 * gFont.cpp
 *
 *  Created on: May 17, 2020
 *      Author: noyan
 */

#include "gFont.h"
#include <iostream>


gFont::gFont() {
	isloaded = false;
	fullpath = "";
	isantialiased = false;
	fontsize = 0;
	dpi = 0;
}

gFont::~gFont() {
}

bool gFont::load(std::string fullPath, int size, bool isAntialiased, int dpi) {
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
		std::string errorstr = "freetype error";
		if (err == 1) errorstr = "wrong file name";
		gLoge("gFont") << "Freetype error: " << errorstr.c_str();
		return false;
	}

	FT_Set_Char_Size(fontface, fontsize << 6, fontsize << 6, dpi, dpi);
	lineheight = fontsize * 1.43f;
	letterspacing = 1;
	spacesize = 1;
	border = 3;
	characternumlimit = 10000;

	resizeVectors(characternumlimit);

	isloaded = true;
	return true;
}

bool gFont::loadFont(std::string fontPath, int size, bool isAntialiased, int dpi) {
	return load(gGetFontsDir() + fontPath, size, isAntialiased, dpi);
}

void gFont::drawText(std::string text, float x, float y) {
	  index1 = 0;
	  posx1 = x;
	  posy1 = y;

	  text1 = s2ws(text);
	  len1 = text1.length();

	  while (index1 < len1) {
	      c1 = text1[index1];
	      if (c1 == '\n') {
	          posy1 += lineheight;
	          posx1 = x ; //reset X Pos back to zero
	      } else if (c1 == ' ') {
	          cid1 = getCharID('a');
	          posx1 += cpset[cid1].width * letterspacing * spacesize;
	      } else {
	          cid1 = getCharID(c1);
	          if (cpset[cid1].character == unloadedchar) loadChar(cid1);
	          textures[cid1].draw(posx1, posy1 + cpset[cid1].dytop);
	          posx1 += cpset[cid1].advance * letterspacing;
	      }
	    index1++;
	  }
}

float gFont::getStringWidth(std::string text) {
	  index2 = 0;
	  posx2 = 0;

	  text2 = s2ws(text);
	  len2 = text2.length();

	  while (index2 < len2) {
	      cid2 = text2[index2];
	      if (cid2 == ' ') {
	          cy2 = getCharID('a');
	          posx2 += cpset[cy2].width * letterspacing * spacesize;
	      } else {
	          cy2 = getCharID(cid2);
	          if (cpset[cy2].character == unloadedchar) loadChar(cy2);
	          posx2 += cpset[cy2].advance * letterspacing;
	      }
	    index2++;
	  }

	  return posx2;
}

float gFont::getStringHeight(std::string text) {
	  index3 = 0;
	  posy3 = 0;

	  text3 = s2ws(text);
	  len3 = text3.length();

	  while (index3 < len3) {
	      cid3 = text3[index3];
	      y3 = 0;
	      if (cid3 == ' ') {
	          cy3 = getCharID('a');
	          y3 = -cpset[cy3].dytop;
	      } else {
	          cy3 = getCharID(cid3);
	          if (cpset[cy3].character == unloadedchar) loadChar(cy3);
	          y3 = -cpset[cy3].dytop;
	      }

	      if (y3 > posy3) posy3 = y3;

	    index3++;
	  }

	  return posy3;
}

std::string gFont::getPath() {
	return fullpath;
}

int gFont::getSize() {
	return fontsize;
}

bool gFont::isLoaded() {
	return isloaded;
}

bool gFont::isAntialised() {
	return isantialiased;
}

int gFont::getDpi() {
	return dpi;
}

void gFont::resizeVectors(int num) {
	if (num <= 0) return;

	characternumlimit = num;

	std::vector<charProperties>().swap(cpset);
	std::vector<gTexture>().swap(textures);
	std::vector<int>().swap(loadedcharacters);

	// initialize character info and textures
	cpset.resize(characternumlimit);
	for (int i=0; i<characternumlimit; ++i) cpset[i].character = unloadedchar;

	textures.resize(characternumlimit);

	// load 'a' character for display space char
	loadChar(getCharID('a'));
}

int gFont::getCharID(const int& c) {
	tempint = (int)c;
	tempcharno = 0;
	for (; tempcharno != (int)loadedcharacters.size(); ++tempcharno) {
		if (loadedcharacters[tempcharno] == tempint) {
			break;
		}
	}
	if (tempcharno == loadedcharacters.size()) {
		// char num check
		if (tempcharno >= characternumlimit) {
			gLoge("gFont") << "Error: character number limit exceeded!";
			return tempcharno = 0;
		} else {
			loadedcharacters.push_back(tempint);
		}
	}
	return tempcharno;
}


void gFont::loadChar(const int& charID) {
	  lci = charID;

	  lcerr = FT_Load_Glyph(fontface, FT_Get_Char_Index(fontface, loadedcharacters[lci]), FT_LOAD_DEFAULT);
	  if(lcerr) gLoge("gFont") << "Error FT_Load_Glyph";

	  if (isantialiased == true) FT_Render_Glyph(fontface->glyph, FT_RENDER_MODE_NORMAL);
	  else FT_Render_Glyph(fontface->glyph, FT_RENDER_MODE_MONO);

	  FT_Bitmap& lcbitmap = fontface->glyph->bitmap;

	  cpset[lci].character = loadedcharacters[lci];
	  cpset[lci].height = fontface->glyph->bitmap_top;
	  cpset[lci].width = fontface->glyph->bitmap.width;
	  cpset[lci].advance = fontface->glyph->advance.x >> 6;
	  cpset[lci].topmargin = fontface->glyph->bitmap.rows;
	  cpset[lci].leftmargin = fontface->glyph->bitmap_left;

	  lcdataw = cpset[lci].width;
	  lcdatah = lcbitmap.rows;

	  cpset[lci].texturewidth = lcdataw;
	  cpset[lci].textureheight = lcdatah;

	  lcfheight = cpset[lci].height;
	  lcbwidth = cpset[lci].width;
	  lctop = cpset[lci].topmargin - cpset[lci].height;
	  lclextent	= cpset[lci].leftmargin;

	  lcstretch = 0;
	  lccorr = (float)(((fontsize - lcfheight) + lctop) - fontsize);

	  cpset[lci].dxleft = (float) lclextent;
	  cpset[lci].dytop = -lctop + lccorr;
	  cpset[lci].dxright = lclextent + lcbwidth + lcstretch;
	  cpset[lci].dybottom = lcfheight + lccorr + lcstretch;

	  unsigned char lcdata[lcdatah * lcdataw * 2];
	  lcdatanum = lcdataw * lcdatah;
	  for (lci2 = 0; lci2 < lcdatanum; lci2++) {
		  lcdata[lci2 * 2] = 255;
		  lcdata[lci2 * 2 + 1] = 0;
	  }

	  if (isantialiased == true) {
		  bitmappixels = lcbitmap.buffer;
		  for (lci3 = 0; lci3 < lcdatanum; lci3++) {
			  lcdata[lci3 * 2 + 1] = bitmappixels[lci3];
		  }
	  } else {
	    lcsrc = lcbitmap.buffer;
	    for(lcj = 0; lcj < lcbitmap.rows; ++lcj) {
	      lcb = 0;
	      lcbptr = lcsrc;
	      for(lck = 0; lck < lcbitmap.width; ++lck) {
	        lcdata[2 * (lck + lcj * lcdataw)] = 255;
	        if (lck % 8 == 0) lcb = (*(lcbptr++));
	        lcdata[2 * (lck + lcj * lcdataw) + 1] = lcb & 0x80 ? 255 : 0;
	        lcb <<= 1;
	      }
	      lcsrc += lcbitmap.pitch;
	    }
	  }

	  lclongside = border * 2;
	  cpset[lci].texturewidth > cpset[lci].textureheight ? lclongside += cpset[lci].texturewidth : lclongside += cpset[lci].textureheight;

	  lclongest = 1;
	  while (lclongside > lclongest) {
	    lclongest <<= 1;
	  }
	  lcpixelsw = lclongest;
	  lcpixelsh = lcpixelsw;

	  unsigned char lcpixels[lcpixelsw * lcpixelsh * 2];
	  lcapsize = lcpixelsw * lcpixelsh;
	  for (lci4 = 0; lci4 < lcapsize; lci4++) {
		  lcpixels[lci4 * 2] = 255;
		  lcpixels[lci4 * 2 + 1] = 0;
	  }

	  insertData(lcdata, lcdataw, lcdatah, 2, lcpixels, lcpixelsw, lcpixelsh, 2, border, border);

	  textures[lci] = gTexture(lcpixelsw, lcpixelsh, GL_LUMINANCE_ALPHA, false);

	  if (isantialiased && fontsize > 20) {
	    textures[lci].setFiltering(gTexture::TEXTUREMINMAGFILTER_LINEAR, gTexture::TEXTUREMINMAGFILTER_LINEAR);
	  } else {
		  textures[lci].setFiltering(gTexture::TEXTUREMINMAGFILTER_NEAREST, gTexture::TEXTUREMINMAGFILTER_NEAREST);
	  }

	  textures[lci].loadData(lcpixels, lcpixelsw, lcpixelsh, 2);
}


bool gFont::insertData(unsigned char* srcData, int srcWidth, int srcHeight, int componentNum, unsigned char* dstData, int dstWidth, int dstHeight, int dstComponentNum, size_t dstFirstX, size_t dstFirstY) const {
	size_t pdrows = (dstFirstX + srcWidth <= dstWidth ? srcWidth : dstWidth - dstFirstX) * componentNum;
	size_t pdcolumns = dstFirstY + srcHeight <= dstHeight ? srcHeight : dstHeight - dstFirstY;
	unsigned char* pddstpix = dstData + ((dstFirstX + dstFirstY * dstWidth) * dstComponentNum);
	unsigned char* pdsrcpix = srcData;
	size_t lcsrcstride = srcWidth * componentNum;
	size_t lcdststride = dstWidth * dstComponentNum;

	for(size_t y = 0; y < pdcolumns; y++){
		memcpy(pddstpix, pdsrcpix, pdrows);
		pddstpix += lcdststride;
		pdsrcpix += lcsrcstride;
	}

	return true;
}

std::wstring gFont::s2ws(const std::string& s) {
    std::string curLocale = setlocale(LC_ALL, "");
    const char* _Source = s.c_str();
    size_t _Dsize = mbstowcs(NULL, _Source, 0) + 1;
    wchar_t *_Dest = new wchar_t[_Dsize];
    wmemset(_Dest, 0, _Dsize);
    mbstowcs(_Dest,_Source,_Dsize);
    std::wstring result = _Dest;
    delete []_Dest;
    setlocale(LC_ALL, curLocale.c_str());
    return result;
}
