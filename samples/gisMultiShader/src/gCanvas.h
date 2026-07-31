/*
 * gCanvas.h
 *
 *  Created on: July 30, 2026
 *      Author: Hasan Cem Eren
 */

#ifndef GCANVAS_H_
#define GCANVAS_H_

#include "gBaseCanvas.h"
#include "gApp.h"
#include "gImage.h"
#include "gCamera.h"
#include "gBox.h"
#include "gModel.h"
#include "gFont.h"

// Includes material multi-pass rendering and shader management modules.
#include "gMaterial.h"
#include "gShader.h"

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
	gImage logo;
	gCamera camera;
	gFont font;

	// Car model instance and box instance for testing multi-mesh material extra pass.
	gModel carmodel;
	gBox box;

	// material for box mesh
	gMaterial boxmaterial;
	// Extra shader pass instances.
	gShader outlineshader;
	gShader glowshader;

	// Toggle flag for dynamic extra shader attachment on box.
	bool isboxshaderactive;
	// Uniform variable for shader
	float glowtime;
};

#endif /* GCANVAS_H_ */
