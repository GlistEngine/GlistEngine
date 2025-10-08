/*
 * gFont.cpp
 *
 *  Created on: May 17, 2020
 *      Author: noyan
 */

#include "gFont.h"
#include "gTracy.h"

#include <iostream>
#include <locale>
#include <codecvt>
#ifdef ANDROID
#include "gAndroidUtil.h"
#endif


gFont::gFont() {
	isloaded = false;
	fullpath = "";
	isantialiased = false;
	fontsize = 0;
	dpi = 0;
	iskerning = false;
	fontface = nullptr;
	ftlib = nullptr;
}

gFont::~gFont() {
	if(fontface != nullptr) {
		for (const auto& item : textures) {
			delete item;
		}
		textures.clear();
		cpset.clear();
		loadedcharacters.clear();
		bitmappixels = nullptr;
		lcsrc = nullptr;
		lcbptr = nullptr;
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
	  index1 = 0;
	  posx1 = x;
	  posy1 = y;

	  text1 = s2ws(text);
	  len1 = text1.length();

	  while (index1 < len1) {
	      c1 = text1[index1];
	      if(index1 > 0) cold1 = text1[index1 - 1];
	      else cold1 = -1;
	      if (c1 == '\n') {
	          posy1 += lineheight;
	          posx1 = x; //reset X Pos back to zero
	      } else {
	          cid1 = getCharID(c1);
	          if (cpset[cid1].character == unloadedchar) loadChar(cid1);
	          posx1 += getKerning(cid1, cold1);
	          textures[cid1]->draw(posx1 + cpset[cid1].leftmargin, posy1 + cpset[cid1].dytop);
	          posx1 += cpset[cid1].advance * letterspacing * (c1 == ' ' ? spacesize : 1);
	      }
	    index1++;
	  }
}

void gFont::drawTextVerticallyFlipped(const std::string& text, float x, float y) {
	G_PROFILE_ZONE_SCOPED_N("gFont::drawTextVerticallyFlipped()");
	index1 = 0;
	posx1 = x;
	posy1 = y;

	text1 = s2ws(text);
	len1 = text1.length();

	while (index1 < len1) {
		c1 = text1[index1];
		if(index1 > 0) {
			cold1 = text1[index1 - 1];
		} else {
			cold1 = -1;
		}
		if (c1 == '\n') {
			posy1 -= lineheight;
			posx1 = x; //reset X Pos back to zero
		} else {
			cid1 = getCharID(c1);
			if (cpset[cid1].character == unloadedchar) {
				loadChar(cid1);
			}
			posx1 += getKerning(cid1, cold1);
			gTexture* texture = textures[cid1];
			texture->draw(posx1 + cpset[cid1].leftmargin, posy1 - cpset[cid1].dytop, texture->getWidth(), -texture->getHeight());
			posx1 += cpset[cid1].advance * letterspacing * (c1 == ' ' ? spacesize : 1);
		}
		index1++;
	}
}

void gFont::drawTextHorizontallyFlipped(const std::string& text, float x, float y) {
	G_PROFILE_ZONE_SCOPED_N("gFont::drawTextHorizontallyFlipped()");
	index1 = 0;
	posx1 = x;
	posy1 = y;

	text1 = s2ws(text);
	len1 = text1.length();

	// Calculate the total width
	int totalwidth = 0;
	for (int i = 0; i < len1; ++i) {
	    wchar_t c = text1[i];
	    int cid = getCharID(c);
	    if (cpset[cid].character == unloadedchar) {
	        loadChar(cid);
	    }
	    totalwidth += cpset[cid].advance * letterspacing * (c == ' ' ? spacesize : 1);
	}

	if (len1 > 0) {
	    wchar_t lastchar = text1[len1 - 1];
	    int lastcid = getCharID(lastchar);
	    totalwidth += cpset[lastcid].advance * letterspacing * (lastchar == ' ' ? spacesize : 1);
	}

	// Set the starting position based on the total text width
	posx1 = x + totalwidth;

	while (index1 < len1) {
	    c1 = text1[index1];
	    if (index1 > 0) {
	        cold1 = text1[index1 - 1];
	    } else {
	        cold1 = -1;
	    }
	    if (c1 == '\n') {
	        posy1 += lineheight;
	        posx1 = x + totalwidth; // reset X Pos back to initial adjusted value
	    } else {
	        cid1 = getCharID(c1);
	        if (cpset[cid1].character == unloadedchar) {
	            loadChar(cid1);
	        }

	        int kerning = getKerning(cid1, cold1);
	        posx1 -= kerning;

	        gTexture* texture = textures[cid1];
	        int drawx = posx1 - cpset[cid1].leftmargin - texture->getWidth();

	        texture->draw(drawx, posy1 + cpset[cid1].dytop, -texture->getWidth(), texture->getHeight());

	        int advance = cpset[cid1].advance * letterspacing * (c1 == ' ' ? spacesize : 1);
	        posx1 -= advance;
	    }
	    index1++;
	}
}

float gFont::getStringWidth(const std::string& text) {
	G_PROFILE_ZONE_SCOPED_N("gFont::getStringWidth()");
	  index2 = 0;
	  posx2 = 0;

	  text2 = s2ws(text);
	  len2 = text2.length();

	  while (index2 < len2) {
	      cid2 = text2[index2];
	      if(index2 > 0) cold2 = text2[index2 - 1];
	      else cold2 = -1;
	      cy2 = getCharID(cid2);
	      if (cpset[cy2].character == unloadedchar) loadChar(cy2);
	      posx2 += getKerning(cid2, cold2);
	      posx2 += cpset[cy2].advance * letterspacing * (cid2 == ' ' ? spacesize : 1);
	      index2++;
	  }

	  return posx2;
}

float gFont::getStringHeight(const std::string& text) {
	G_PROFILE_ZONE_SCOPED_N("gFont::getStringHeight()");
	  index3 = 0;
	  posy3 = 0;

	  text3 = s2ws(text);
	  len3 = text3.length();

	  while (index3 < len3) {
	      cid3 = text3[index3];
	      y3 = 0;
	      cy3 = getCharID(cid3);
	      if (cpset[cy3].character == unloadedchar) loadChar(cy3);
	      y3 = -cpset[cy3].dytop;
	      if (y3 > posy3) posy3 = y3;
	      index3++;
	  }
	  return posy3;
}

float gFont::getLineHeight() {
	return lineheight;
}

const std::string& gFont::getPath() const {
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
	G_PROFILE_ZONE_SCOPED_N("gFont::resizeVectors()");
	G_PROFILE_ZONE_VALUE(num);
	if (num <= 0) return;

	characternumlimit = num;

	std::vector<charProperties>().swap(cpset);
	std::vector<int>().swap(loadedcharacters);

	// initialize character info and textures
	cpset.resize(characternumlimit);
	for (int i=0; i<characternumlimit; ++i) cpset[i].character = unloadedchar;

	for (const auto& item : textures) {
		delete item;
	}
	textures.clear();
	textures.resize(characternumlimit);

	// load ' ' character for display space char
	loadChar(getCharID(' '));
}

int gFont::getCharID(const int& c) {
	G_PROFILE_ZONE_SCOPED_N("gFont::getCharID()");
	tempint = (int)c;
	tempcharno = 0;
	//search the �d of a character
	auto it = std::find(loadedcharacters.begin(), loadedcharacters.end(), tempint);

	if(it != loadedcharacters.end()){
		//if finded calculate position
		tempcharno = std::distance(loadedcharacters.begin(), it);
	}else {
		//if not
		if(loadedcharacters.size() >= loadedcharacters.max_size()){
			//if reached the max size, resize deque
			loadedcharacters.resize(loadedcharacters.size() + 1000);
		}
		//add new char to deque
		loadedcharacters.push_back(tempint);
		tempcharno = loadedcharacters.size() - 1; //return last index
	}

	return tempcharno;
}


void gFont::loadChar(const int& charID) {
	G_PROFILE_ZONE_SCOPED_N("gFont::loadChar()");
	  lci = charID;

	  lcerr = FT_Load_Glyph(fontface, FT_Get_Char_Index(fontface, loadedcharacters[lci]), isantialiased ?  FT_LOAD_FORCE_AUTOHINT : FT_LOAD_DEFAULT);
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

	  unsigned char lcdata[lcdatah * lcdataw * 4];
	  lcdatanum = lcdataw * lcdatah;
	  for (lci2 = 0; lci2 < lcdatanum; lci2++) {
		  lcdata[lci2 * 4] = 255;
		  lcdata[lci2 * 4 + 1] = 255;
		  lcdata[lci2 * 4 + 2] = 255;
		  lcdata[lci2 * 4 + 3] = 0;
	  }

	  if (isantialiased) {
		  bitmappixels = lcbitmap.buffer;
		  for (lci3 = 0; lci3 < lcdatanum; lci3++) {
			  lcdata[lci3 * 4 + 3] = bitmappixels[lci3];
		  }
	  } else {
	    lcsrc = lcbitmap.buffer;
	    for(lcj = 0; lcj < lcbitmap.rows; ++lcj) {
	      lcb = 0;
	      lcbptr = lcsrc;
	      for(lck = 0; lck < lcbitmap.width; ++lck) {
			lcdata[(lck + lcj * lcdataw) * 4] = 255;
	        if (lck % 8 == 0) lcb = (*(lcbptr++));
	        lcdata[(lck + lcj * lcdataw) * 4 + 3] = lcb & 0x80 ? 255 : 0;
	        lcb <<= 1;
	      }
	      lcsrc += lcbitmap.pitch;
	    }
	  }

	  lclongside = border * 2;
	  if(cpset[lci].texturewidth > cpset[lci].textureheight) {
		  lclongside += cpset[lci].texturewidth;
	  } else {
		  lclongside += cpset[lci].textureheight;
	  }

	  lclongest = 1;
	  while (lclongside > lclongest) {
	    lclongest <<= 1;
	  }
	  lcpixelsw = lclongest;
	  lcpixelsh = lcpixelsw;

	  unsigned char* lcpixels = new unsigned char[lcpixelsw * lcpixelsh * 4];
	  lcapsize = lcpixelsw * lcpixelsh;
	  for (lci4 = 0; lci4 < lcapsize; lci4++) {
		  lcpixels[lci4 * 4] = 255;
		  lcpixels[lci4 * 4 + 1] = 255;
		  lcpixels[lci4 * 4 + 2] = 255;
		  lcpixels[lci4 * 4 + 3] = 0;
	  }

	  insertData(lcdata, lcdataw, lcdatah, 4, lcpixels, lcpixelsw, lcpixelsh, 4, border, border);

	  textures[lci] = new gTexture(lcpixelsw, lcpixelsh, GL_RGBA, false);

	  if (isantialiased) {
	    textures[lci]->setFiltering(gTexture::TEXTUREMINMAGFILTER_LINEAR, gTexture::TEXTUREMINMAGFILTER_LINEAR);
	  } else {
		  textures[lci]->setFiltering(gTexture::TEXTUREMINMAGFILTER_NEAREST, gTexture::TEXTUREMINMAGFILTER_NEAREST);
	  }

	  textures[lci]->setData(lcpixels, lcpixelsw, lcpixelsh, 4, false, false);
	  delete[] lcpixels;
}


bool gFont::insertData(unsigned char* srcData, int srcWidth, int srcHeight, int componentNum, unsigned char* dstData, int dstWidth, int dstHeight, int dstComponentNum, size_t dstFirstX, size_t dstFirstY) const {
	G_PROFILE_ZONE_SCOPED_N("gFont::insertData()");
	size_t pdrows = (dstFirstX + srcWidth <= dstWidth ? srcWidth : dstWidth - dstFirstX) * componentNum;
	size_t pdcolumns = dstFirstY + srcHeight <= dstHeight ? srcHeight : dstHeight - dstFirstY;
	unsigned char* pddstpix = dstData + ((dstFirstX + dstFirstY * dstWidth) * dstComponentNum);
	unsigned char* pdsrcpix = srcData;
	size_t lcsrcstride = srcWidth * componentNum;
	size_t lcdststride = dstWidth * dstComponentNum;

	for(size_t y = 0; y < pdcolumns; y++) {
		memcpy(pddstpix, pdsrcpix, pdrows);
		pddstpix += lcdststride;
		pdsrcpix += lcsrcstride;
	}

	return true;
}

int gFont::getKerning(int c, int previousC) {
	G_PROFILE_ZONE_SCOPED_N("gFont::getKerning()");
    if(fontface && iskerning) {
        // Convert the characters to indices
        FT_UInt index1 = FT_Get_Char_Index(fontface, previousC);
        FT_UInt index2 = FT_Get_Char_Index(fontface, c);

        // Get the kerning vector
        FT_Vector kerning;
        FT_Get_Kerning(fontface, index1, index2, FT_KERNING_DEFAULT, &kerning);

        // X advance is already in pixels for bitmap fonts
        if (!FT_IS_SCALABLE(fontface))
            return kerning.x;

        return kerning.x >> 6;
//        FT_Vector kerning;
//        FT_Get_Kerning(fontface, FT_Get_Char_Index(fontface, previousC), FT_Get_Char_Index(fontface, c), FT_KERNING_DEFAULT, &kerning);
//        return kerning.x >> 6;
    }
	return 0;
}

#ifdef WIN32
#include <windows.h>
#endif

std::wstring gFont::s2ws(const std::string& s) {
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
