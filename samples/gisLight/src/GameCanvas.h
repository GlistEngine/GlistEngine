/*
 * GameCanvas.h
 *
 *  Created on: May 6, 2020
 *      Author: noyan
 */

#ifndef GAMECANVAS_H_
#define GAMECANVAS_H_

#include "gBaseCanvas.h"
#include "gBox.h"
#include "gCamera.h"
#include "gLight.h"
#include "gFont.h"


class GameCanvas : public gBaseCanvas {
public:
	GameCanvas(gBaseApp *root);
	virtual ~GameCanvas();

	void setup();
	void update();
	void draw();

	void keyPressed(int key);
	void keyReleased(int key);
	void mouseMoved(int x, int y );
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void mouseEntered();
	void mouseExited();

	void showNotify();
	void hideNotify();

private:
	gCamera camera;
	gBox box;
	gBox lightbox;
	gTexture boxface;
	gLight light;
	gFont font;
	gTexture boxdiffusemap, boxspecularmap, boxnormalmap;
};

#endif /* GAMECANVAS_H_ */
