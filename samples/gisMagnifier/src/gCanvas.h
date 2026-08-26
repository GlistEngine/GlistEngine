/*
 * gCanvas.h
 *
 * Created on: Aug 26, 2026
 * Authors: Bahar Kucukozer, Mehmet Sefa Ciftci
 */

#ifndef GCANVAS_H_
#define GCANVAS_H_

#include "gApp.h"
#include "gImage.h"
#include "gMagnifier.h"
#include "gPostProcessManager.h"


class gCanvas : public gBaseCanvas {
public:
	gCanvas(gApp* root);
	virtual ~gCanvas();

	void setup();
	void update();
	void draw();

	void keyPressed(int key);
	void keyReleased(int key);
	void charPressed(unsigned int codepoint);
	void mouseMoved(int x, int y);
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void mouseScrolled(int x, int y);
	void mouseEntered();
	void mouseExited();
	void windowResized(int w, int h);

	void deviceOrientationChanged(DeviceOrientation deviceorientation);

	void touchMoved(int x, int y, int fingerId);
	void touchPressed(int x, int y, int fingerId);
	void touchReleased(int x, int y, int fingerId);

	void pause();
	void resume();

	void showNotify();
	void hideNotify();

private:
	gApp* root;
	gImage background;
	gImage logo;
	gImage magnifierframe;
	gPostProcessManager postprocessmanager;
	gMagnifier* magnifier;
	int magnifierx;
	int magnifiery;
};

#endif /* GCANVAS_H_ */
