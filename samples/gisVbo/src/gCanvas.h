 /*
 * gCanvas.h
 *
 *  Created on: May 6, 2020
 *      Author: Noyan Culum
 */

#ifndef GCANVAS_H_
#define GCANVAS_H_

#include "gBaseCanvas.h"
#include "gVbo.h"
#include "gLight.h"
#include "gShader.h"

class gCanvas : public gBaseCanvas {
public:
	gCanvas(gBaseApp *root);
	virtual ~gCanvas();

	void setup() override;
	void update() override;
	void draw() override;

	void keyPressed(int key) override;
	void keyReleased(int key) override;
	void mouseMoved(int x, int y ) override;
	void mouseDragged(int x, int y, int button) override;
	void mousePressed(int x, int y, int button) override;
	void mouseReleased(int x, int y, int button) override;
	void mouseEntered() override;
	void mouseExited() override;

	void showNotify() override;
	void hideNotify() override;

private:
	gVbo vbo1, vbo2;
	unsigned int indices[3];
	gVertex verts1[3];
	gVertex verts2[3];
	gLight light;
};

#endif /* GCANVAS_H_ */
