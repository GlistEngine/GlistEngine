 /*
 * gCanvas.h
 *
 *  Created on: May 6, 2020
 *      Author: Noyan Culum
 */

#ifndef GCANVAS_H_
#define GCANVAS_H_

#include "gBaseCanvas.h"
#include "gApp.h"
#include "gImage.h"
#include "gSkybox.h"
#include "gPostProcessManager.h"
#include "gModel.h"
#include "gLight.h"
#include "gBox.h"
#include "gSphere.h"
#include "gCone.h"
#include "gShadowMap.h"
#include "gBloom.h"
#include "gFont.h"
#include "gFXAA.h"

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
	gSkybox sky[2];
	gApp* root;

	bool isRotationPaused;
	gCamera camera;
	gBox cubeMesh;
	gSphere sphereMesh;
	gCone coneMesh{1, 2, glm::vec2(0.0f, 0.0f)};

	float orbitAngle;
	gFont font;
	gPostProcessManager ppm;
	gFXAA* FXAA;
};

#endif /* GCANVAS_H_ */
