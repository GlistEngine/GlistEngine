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
/**
 *	Loads a font from the given fullPath
 *
 *	Users could use this function for loading font and giving text's features.
 *
 *	The Function benefit to give text's option.
 *
 *	Users do not need to enter the isAntialiased and the dpi values because
 *	those values are given automatically.
 *
 *	Font styles ist stored by GlistEngine's folder.
 *	If developers want to use new style they can add their style
 *
 *
 *	@param fullPath The full path to the font file. It should contain the
 *  full path of the folder where the font is located.
 *
 *  @param The size is the size of the font
 *
 *	@param the isAntialised is used in digital imaging to reduce visual imperfections and users don't need to give value.
 *
 *	@param the dpi is a measure of resolution used for printed text or images and users don't need to give value
 *
 *
 *	@return the function will load the font style and
 *	return an error if it encounters an error loading
 */
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
/**
 *	Loads a font from the given fontPath
 *
 *	Users could use this function for loading font and giving text's features.
 *
 *	The Function benefit to give text's option.
 *
 *	Users do not need to enter the isAntialiased and the dpi values because
 *	those values are given automatically.
 *
 *	Font styles ist stored by GlistEngine's folder.
 *	If developers want to use new style they can add their style
 *	in glistapp/assets/fonts
 *
 *	@param fontPath The font path to the font file. It should contain the
 *  font path of the folder where the font is located.
 *
 *  @param The size is the size of the font. Users are recommended to use 6 and its multiples.
 *
 *  @param the isAntialised is used in digital imaging to reduce visual imperfections and users don't need to give value.
 *
 *	@param the dpi is a measure of resolution used for printed text or images and users don't need to give value
 *
 *
 *	@return the function will load the font style and
 *	return an error if it encounters an error loading
 */
bool gFont::loadFont(std::string fontPath, int size, bool isAntialiased, int dpi) {
	return load(gGetFontsDir() + fontPath, size, isAntialiased, dpi);
}
/**
 *	This function allows to print the text
 *	we want to use in the coordinates we specify on the screen.
 *
 *	Users should be careful when printing on the screen
 *	and compare the height and width values of the text
 *	with the screen and draw them in the right place.
 *
 *	It should be ensured that the text is defined and
 *	a font suitable for the language used should be selected
 *	so that it does not create pollution in the image.
 *
 *	Users should pay attention to the values they give to the x and y axes.
 *	Values must be in screen coordinates
 *
 *	@param The first parameter should be the text to be printed.
 *
 *	@param x axis value of the region you want to draw
 *
 *	@param y-axis value of the region you want to draw
 *
 *	@return Users will see the text users want to print on the screen
 */
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
/**
 * Users could find text's width with this function
 * With this function, users can find the x-axis size
 * of the text they use.
 *
 * @param Users just give a parametre and this is text to be used
 *
 * @return The output will be a number of type float of text.
 * This number is the width of the text
 */
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
/**
 * Users could find text's height with this function
 * With this function, users can find the y-axis size
 * of the text they use.
 *
 * @param Users just give a parametre and this is text to be used
 *
 * @return The output will be a number of type float of text.
 * This number is the height of the text
 */
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
/**
 * This function returns users where the font is in.
 * Users could find with this function font's folder which is in.
 *
 * No parameters are required when using this function
 * Users should pay attention to data type
 *
 * @return If the user makes an assignment, he returns fullpath name
 */
std::string gFont::getPath() {
	return fullpath;
}
/**
 * This function is returned to the font size's
 * Size is has to be integer. Users should be careful when assigning
 *
 * No parameters are required when using this function
 *
 * @returns This function returns the font size that users have assigned to their font.
 */
int gFont::getSize() {
	return fontsize;
}
/**
 *	Users could use this function for check
 *	This function is used to check if the font is loaded
 *
 *	@returns the function returns 0 or 1 if the font's is loaded it will be one
 *	If it isn't loaded it will be zero
 */
bool gFont::isLoaded() {
	return isloaded;
}
/**
 * the isAntialised is used in digital imaging to reduce visual imperfections.
 * This function lets users know if isAntialised is done or not.
 *
 * @returns If the isAntialised is done function returns true
 * if not it returns false
 */
bool gFont::isAntialised() {
	return isantialiased;
}
/**
 * First users have to know what is the dpi
 * Dpi is a measure of resolution used for printed text or images,
 * the higher the dots per inch, the higher the resolution.
 *
 * @returns This function returns to the dpi value
 */
int gFont::getDpi() {
	return dpi;
}

/**
 * This function lets users change the size of the vector to any size users want.
 * the function changes the size of vector. If num is smaller than current size then
 * extra elements are destroyed.
 *
 * @param num is new container size.
 *
 * @return Users could see their new shape if they make assign
 */
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
/**
 * This function allows chars to be recognized
 *
 * This function allows users to find the ID of the character as an integer
 * with this function, users could access the integer value of char characters.
 *
 * @param integer equivalent of char character
 *
 * @return char's value
 */
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

/**
 *	This function provides the mathematical properties of the entered char.
 *
 *	Users could use this function for loading char's values.
 *	through this function char measures are found and char is defined
 *	The found values allow users to use chars in fonts
 *
 *	@param users could choose which char is wanted to use. charID has to be integer so users have to know char's number
 *
 *	@return the function will load the char which one users choose
 */
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

/**
 * This function is used to assign properties to be used to recognize chars.
 *
 * the function finds the values of the char
 *
 * the function completes the inserting process of the data
 * by making comparisons with the entered data.
 *
 * Users must enter values in the correct place for the Function
 * to compare data correctly
 *
 * @param as parameters char data's value
 *
 * @param srcWidth of selected char
 *
 * @param srcHeight of selected char
 *
 * @param component number of selected char
 *
 * @param x axis of selected char
 *
 * @param y axis of selected char
 *
 * @param dstData of selected char
 *
 * @param dstWidth of selected char
 *
 * @param dstHeight of selected char
 *
 * @param dstComponent of selected char
 *
 * @param dstFirstX of selected char
 *
 * @param dstFirstY of selected char
 *
 *  the values of the data to compare values are entered into the function.
 *
 * @return The function is loaded datas and if users want to check this function, they can return true or
 * false by assigning an assignment to the function
 */
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
/**
 * This function allows users to convert std::string to LPWSTR
 * LPWSTR is pointer to constant Unicode (wide) string.
 *
 * @param Users could write which std::string they want to convert.
 *
 * @return Users could see their converted objects if they assign
 */
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
