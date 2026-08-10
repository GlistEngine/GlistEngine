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
#include "gCamera.h"
#include "gCameraController.h"
#include "gLight.h"
#include "gBox.h"
#include "gSphere.h"
#include "gSkybox.h"
#include "gTexture.h"
#include "gModel.h"
#include "gFbo.h"
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
	void mouseMoved(int x, int y);
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
	gFbo ssrscenefbo;
	gFbo ssrreflectionfbo[2];
	gShader ssrshader;
	int ssrcurrentreflectionindex;
	bool ssrhasvalidhistory;
	int ssrframeindex;
	glm::mat4 ssrcaptureprojection;
	glm::mat4 ssrcaptureview;
	glm::mat4 ssrpreviousviewprojection;
	float ssrcapturenearclip;
	float ssrcapturefarclip;
	float ssrreflectivity;
	float ssrfresnelbias;
	float ssrfresnelpower;
	glm::vec3 ssrfallbackcolor;

	void setupssr();
	void resizessr(int width, int height);
	void beginssrcapture();
	void capturessrcameramatrices(gCamera& camera);
	void endssrcapture();
	void beginssrreflectionpass(gSkybox* skybox);
	void endssrreflectionpass();
	void beginssrcomposite();
	void endssrcomposite();

	gApp* root;

	gCamera camera;
	gCameraController camcontroller;

	gLight sun;
	gLight skyfill;
	glm::vec3 sundirection;

	gSkybox sky;

	gBox floortile;
	gTexture flooralbedo;
	gTexture floorroughness;
	gTexture floormetalness;

	gSphere showcasesphere;
	gTexture sphereroughness;
	gTexture spheremetalness;

	gModel doorframe;
	gModel columnastraleft;
	gModel columnastraright;
	gModel columnhollow;
	gModel columnround;
	gModel crate3;
	gModel crate4;
	gModel chest;

	void setupsky();
	void setupfloor();
	void setupsphere();
	void setupshowcase();
	void drawscene();
};

#endif
