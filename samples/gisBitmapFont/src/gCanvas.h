#ifndef GCANVAS_H_
#define GCANVAS_H_

#include "gBaseCanvas.h"
#include "gBitmapFont.h"

#include <string>

class gApp;

class gCanvas : public gBaseCanvas {
public:
	explicit gCanvas(gApp* root);
	void setup() override;
	void update() override;
	void draw() override;
	void keyPressed(int key) override;

private:
	gBitmapFont bitmapfont;
	int atlascolumns;
	int glyphsize;
	std::string characters;
	float scale;
};

#endif
