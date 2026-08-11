#include "gCanvas.h"

#include "gApp.h"

#include <algorithm>
#include <vector>

namespace {
constexpr int AtlasColumns = 6;
constexpr int GlyphSize = 209;
const std::string Characters = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
}

gCanvas::gCanvas(gApp* root) : gBaseCanvas(root) {
}

void gCanvas::setup() {
	std::vector<gRect> coordinates;
	coordinates.reserve(Characters.size());
	for (int i = 0; i < static_cast<int>(Characters.size()); ++i) {
		const int left = (i % AtlasColumns) * GlyphSize;
		const int top = (i / AtlasColumns) * GlyphSize;
		coordinates.emplace_back(left, top, left + GlyphSize, top + GlyphSize);
	}

	if (!bitmapfont.loadImage("bitmap-font-atlas.png")
			|| !bitmapfont.setCharacters(Characters, coordinates)) {
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
