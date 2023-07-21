 /*
 * GameCanvas.h
 *
 *  Created on: May 6, 2020
 *      Author: noyan
 */

#ifndef GAMECANVAS_H_
#define GAMECANVAS_H_

#include "gBaseCanvas.h"
#include "gApp.h"
#include "gImage.h"
#include "gPostProcessManager.h"
#include "gChromaticAberration.h"
#include "gDynamicGrain.h"
#include "gVignette.h"

class GameCanvas : public gBaseCanvas {
public:
	GameCanvas(gApp* root);
	virtual ~GameCanvas();

	void setup();
	void update();
	void draw();

	void keyPressed(int key);
	void keyReleased(int key);
	void charPressed(unsigned int codepoint);
	void mouseMoved(int x, int y );
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void mouseScrolled(int x, int y);
	void mouseEntered();
	void mouseExited();
	void windowResized(int w, int h);

	void showNotify();
	void hideNotify();

private:
	gApp* root;
	gImage background;
	gImage logo;
	gPostProcessManager postprocessmanager;
};

#endif /* GAMECANVAS_H_ */
