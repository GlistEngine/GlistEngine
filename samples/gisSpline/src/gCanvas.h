/*
 * Copyright (C) 2016 Nitra Games Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef GCANVAS_H_
#define GCANVAS_H_

#include "gApp.h"
#include "gBaseCanvas.h"
#include "gFont.h"
#include "gSpline.h"

#include <glm/glm.hpp>
#include <vector>

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
	void resetControlPoints();
	void drawBackground();
	void drawControlPolygon();
	void drawCurve();
	void drawControlPoints();
	void drawMarker();
	void drawInterface();
	int findControlPoint(int x, int y) const;
	glm::vec2 clampToWorkspace(int x, int y);

	gApp* root;
	gSpline spline;
	gFont titlefont;
	gFont bodyfont;
	gFont smallfont;
	std::vector<glm::vec2> controlpoints;

	float animationposition;
	bool animationpaused;
	int selectedpoint;
	int hoveredpoint;
};

#endif /* GCANVAS_H_ */
