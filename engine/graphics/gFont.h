/*
 * gFont.h
 *
 *  Created on: May 17, 2020
 *      Author: noyan
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


class gFont : public gNode {
public:

	gFont();
	virtual ~gFont();

	void load(std::string fullPath, int size);
	void loadFont(std::string fontPath, int size);
	void drawText(std::string text, float x, float y);

	float getStringWidth(std::string text);
	float getStringHeight(std::string text);

private:

	/// Holds all state information relevant to a character as loaded using FreeType
	struct Character {
	    unsigned int TextureID; // ID handle of the glyph texture
	    glm::ivec2   Size;      // Size of glyph
	    glm::ivec2   Bearing;   // Offset from baseline to left/top of glyph
	    unsigned int Advance;   // Horizontal offset to advance to next glyph
	};

	std::map<GLchar, Character> Characters;
    FT_Library ft;
    FT_Face face;
    unsigned int vao, vbo;

    int fontsize;

    unsigned char ci;
    std::string::const_iterator c, cs1, cs2;
    Character ch;
    float xpos, ypos;
    float csx, csy;
    float cthy;
    float ssw, ssh;
};

#endif /* ENGINE_GRAPHICS_GFONT_H_ */
