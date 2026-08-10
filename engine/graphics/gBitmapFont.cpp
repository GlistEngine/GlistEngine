/*
 * gBitmapFont.cpp
 */

#include "gBitmapFont.h"

#include "gUtils.h"

#include <algorithm>
#include <utility>

bool gBitmapFont::load(const std::string& fullPath) {
	gImage loadedbitmap;
	loaded = loadedbitmap.load(fullPath) != 0
			&& loadedbitmap.getWidth() > 0 && loadedbitmap.getHeight() > 0;
	if (loaded) bitmap = std::move(loadedbitmap);
	rebuildGlyphs();
	return loaded;
}

bool gBitmapFont::loadImage(const std::string& imagePath) {
	gImage loadedbitmap;
	loaded = loadedbitmap.loadImage(imagePath) != 0
			&& loadedbitmap.getWidth() > 0 && loadedbitmap.getHeight() > 0;
	if (loaded) bitmap = std::move(loadedbitmap);
	rebuildGlyphs();
	return loaded;
}

bool gBitmapFont::loadFontImage(const std::string& imagePath) {
	return loadImage(imagePath);
}

void gBitmapFont::setCharacterList(const std::string& characters) {
	characterlist.clear();
	for (std::uint32_t codepoint : gUTF8Iterator(characters)) {
		characterlist.push_back(codepoint);
	}
	rebuildGlyphs();
}

void gBitmapFont::setCharacters(const std::string& characters) {
	setCharacterList(characters);
}

void gBitmapFont::setCharacterCoordinates(const std::vector<gRect>& coordinates) {
	coordinatelist = coordinates;
	rebuildGlyphs();
}

bool gBitmapFont::setCharacters(const std::string& characters,
		const std::vector<gRect>& coordinates) {
	setCharacterList(characters);
	setCharacterCoordinates(coordinates);
	return configured;
}

void gBitmapFont::drawText(const std::string& text, float x, float y) {
	if (!loaded || !configured || text.empty()) return;

	float drawx = x;
	float drawy = y;
	const float currentlineheight = (lineheight > 0.0f ? lineheight : calculatedlineheight) * scale;

	for (std::uint32_t codepoint : gUTF8Iterator(text)) {
		if (codepoint == '\n') {
			drawx = x;
			drawy += currentlineheight;
			continue;
		}

		auto glyph = glyphs.find(codepoint);
		if (glyph == glyphs.end()) continue;

		const gRect& source = glyph->second;
		const float drawwidth = source.getWidth() * scale;
		const float drawheight = source.getHeight() * scale;
		bitmap.drawSub(glm::vec2(drawx, drawy), glm::vec2(drawwidth, drawheight),
				glm::vec2(source.left(), source.top()),
				glm::vec2(source.getWidth(), source.getHeight()));
		drawx += drawwidth + letterspacing * scale;
	}
}

void gBitmapFont::setScale(float scale) {
	this->scale = std::max(0.0f, scale);
}

float gBitmapFont::getScale() const {
	return scale;
}

void gBitmapFont::setLetterSpacing(float spacing) {
	letterspacing = spacing;
}

float gBitmapFont::getLetterSpacing() const {
	return letterspacing;
}

void gBitmapFont::setLineHeight(float height) {
	lineheight = std::max(0.0f, height);
}

float gBitmapFont::getLineHeight() const {
	return lineheight > 0.0f ? lineheight : calculatedlineheight;
}

bool gBitmapFont::isLoaded() const {
	return loaded;
}

bool gBitmapFont::isConfigured() const {
	return configured;
}

int gBitmapFont::getCharacterCount() const {
	return static_cast<int>(glyphs.size());
}

void gBitmapFont::rebuildGlyphs() {
	glyphs.clear();
	calculatedlineheight = 0.0f;
	configured = !characterlist.empty() && characterlist.size() == coordinatelist.size();
	if (!configured) return;

	for (std::size_t i = 0; i < characterlist.size(); ++i) {
		const gRect& rectangle = coordinatelist[i];
		if (rectangle.getWidth() <= 0 || rectangle.getHeight() <= 0) {
			configured = false;
			glyphs.clear();
			calculatedlineheight = 0.0f;
			return;
		}
		if (loaded && (rectangle.left() < 0 || rectangle.top() < 0
				|| rectangle.right() > bitmap.getWidth()
				|| rectangle.bottom() > bitmap.getHeight())) {
			configured = false;
			glyphs.clear();
			calculatedlineheight = 0.0f;
			return;
		}
		glyphs[characterlist[i]] = rectangle;
		calculatedlineheight = std::max(calculatedlineheight,
				static_cast<float>(rectangle.getHeight()));
	}
}
