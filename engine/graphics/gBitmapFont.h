/*
 * gBitmapFont.h
 *
 * Loads pre-rendered glyphs from a bitmap and draws text by sampling the
 * configured character rectangles.
 */

#ifndef ENGINE_GRAPHICS_GBITMAPFONT_H_
#define ENGINE_GRAPHICS_GBITMAPFONT_H_

#include "gImage.h"
#include "gRect.h"

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

class gBitmapFont {
public:
	/** Loads a bitmap font image from an absolute or relative filesystem path. */
	bool load(const std::string& fullPath);

	/** Loads a bitmap font image from the application's assets/images folder. */
	bool loadImage(const std::string& imagePath);
	bool loadFontImage(const std::string& imagePath);

	/**
	 * Sets the UTF-8 character list. Each code point is paired with the
	 * rectangle at the same index in the coordinate list.
	 */
	void setCharacterList(const std::string& characters);
	void setCharacters(const std::string& characters);

	/**
	 * Sets glyph rectangles in bitmap pixel coordinates. gRect values use
	 * left, top, right and bottom edges.
	 */
	void setCharacterCoordinates(const std::vector<gRect>& coordinates);

	/** Sets the character and coordinate lists together. */
	bool setCharacters(const std::string& characters, const std::vector<gRect>& coordinates);

	/** Draws UTF-8 text with x and y representing its top-left corner. */
	void drawText(const std::string& text, float x, float y);

	void setScale(float scale);
	float getScale() const;
	void setLetterSpacing(float spacing);
	float getLetterSpacing() const;
	void setLineHeight(float height);
	float getLineHeight() const;

	bool isLoaded() const;
	bool isConfigured() const;
	int getCharacterCount() const;

private:
	void rebuildGlyphs();

	gImage bitmap;
	std::vector<std::uint32_t> characterlist;
	std::vector<gRect> coordinatelist;
	std::unordered_map<std::uint32_t, gRect> glyphs;
	float scale = 1.0f;
	float letterspacing = 0.0f;
	float lineheight = 0.0f;
	float calculatedlineheight = 0.0f;
	bool loaded = false;
	bool configured = false;
};

#endif /* ENGINE_GRAPHICS_GBITMAPFONT_H_ */
