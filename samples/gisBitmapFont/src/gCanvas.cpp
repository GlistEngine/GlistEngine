#include "gCanvas.h"

#include "gApp.h"

#include <algorithm>
#include <vector>

gCanvas::gCanvas(gApp* root) : gBaseCanvas(root) {
}

void gCanvas::setup() {
	atlascolumns = 6;
	glyphsize = 209;
	characters = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
	scale = 0.25f;

	std::vector<gRect> coordinates;
	coordinates.reserve(characters.size());
	for (int i = 0; i < static_cast<int>(characters.size()); ++i) {
		const int left = (i % atlascolumns) * glyphsize;
		const int top = (i / atlascolumns) * glyphsize;
		coordinates.emplace_back(left, top, left + glyphsize, top + glyphsize);
	}

	if (!bitmapfont.loadImage("bitmap-font-atlas.png")
			|| !bitmapfont.setCharacters(characters, coordinates)) {
		gLoge("gisBitmapFont") << "Bitmap font could not be configured.";
	}

	bitmapfont.setScale(scale);
	bitmapfont.setLetterSpacing(8.0f);
	bitmapfont.setLineHeight(220.0f);
}

void gCanvas::update() {
}

void gCanvas::draw() {
	// The atlas uses black as its transparent-looking background.
	clearColor(0, 0, 0);
	setColor(255, 255, 255);
	bitmapfont.drawText("GLIST ENGINE\nBITMAP FONT\nSAMPLE 2026", 60.0f, 70.0f);
}

void gCanvas::keyPressed(int key) {
	if (key == G_KEY_UP) scale = std::min(0.5f, scale + 0.05f);
	if (key == G_KEY_DOWN) scale = std::max(0.1f, scale - 0.05f);
	bitmapfont.setScale(scale);
}
