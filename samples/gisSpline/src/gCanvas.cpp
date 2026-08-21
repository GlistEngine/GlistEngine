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

#include "gCanvas.h"

#include "gRenderer.h"

#include <algorithm>
#include <cmath>
#include <string>

namespace {

const float GSPLINE_PI = 3.14159265358979323846f;
const int GSPLINE_MAX_DEGREE = 5;
const float GSPLINE_CONTROL_POINT_RADIUS = 9.0f;

}

gCanvas::gCanvas(gApp* root)
	: gBaseCanvas(root), root(root), animationposition(0.0f),
	  animationpaused(false), selectedpoint(-1), hoveredpoint(-1) {
}

gCanvas::~gCanvas() {
}

void gCanvas::setup() {
	enableAlphaBlending();
	titlefont.loadFont("FreeSansBold.ttf", 26);
	bodyfont.loadFont("FreeSans.ttf", 16);
	smallfont.loadFont("FreeSans.ttf", 13);

	spline.setDegree(3);
	spline.setResolution(180);
	resetControlPoints();
}

void gCanvas::update() {
	if (!animationpaused) {
		animationposition += static_cast<float>(root->getElapsedTime()) * 0.16f;
		animationposition = std::fmod(animationposition, 1.0f);
	}
}

void gCanvas::draw() {
	clearColor(7, 12, 28);
	drawBackground();
	drawControlPolygon();
	drawCurve();
	drawControlPoints();
	drawMarker();
	drawInterface();
}

void gCanvas::drawBackground() {
	setColor(20, 32, 62, 120);
	gDrawRoundedRectangle(28, 24, getWidth() - 56, 92, 16, true);

	setColor(83, 100, 148, 22);
	for (int x = 0; x < getWidth(); x += 40) {
		gDrawLine(x, 140, x, getHeight() - 64);
	}
	for (int y = 140; y < getHeight() - 64; y += 40) {
		gDrawLine(0, y, getWidth(), y);
	}

	setColor(39, 110, 241, 12);
	gDrawCircle(getWidth() * 0.18f, getHeight() * 0.72f, 150, true);
	setColor(168, 85, 247, 10);
	gDrawCircle(getWidth() * 0.80f, getHeight() * 0.32f, 180, true);
}

void gCanvas::drawControlPolygon() {
	if (controlpoints.size() < 2) {
		return;
	}

	setColor(148, 163, 184, 90);
	for (std::size_t i = 1; i < controlpoints.size(); i++) {
		gDrawLine(
				controlpoints[i - 1].x,
				controlpoints[i - 1].y,
				controlpoints[i].x,
				controlpoints[i].y,
				1.0f);
	}
}

void gCanvas::drawCurve() {
	if (controlpoints.size() < 2) {
		return;
	}

	const int glowresolution = 180;
	glm::vec2 previouspoint = spline.getPoint2D(0.0f);

	setColor(34, 211, 238, 24);
	for (int i = 1; i <= glowresolution; i++) {
		float t = static_cast<float>(i) / glowresolution;
		glm::vec2 currentpoint = spline.getPoint2D(t);
		gDrawLine(previouspoint.x, previouspoint.y,
				currentpoint.x, currentpoint.y, 14.0f);
		previouspoint = currentpoint;
	}

	previouspoint = spline.getPoint2D(0.0f);
	setColor(56, 189, 248, 120);
	for (int i = 1; i <= glowresolution; i++) {
		float t = static_cast<float>(i) / glowresolution;
		glm::vec2 currentpoint = spline.getPoint2D(t);
		gDrawLine(previouspoint.x, previouspoint.y,
				currentpoint.x, currentpoint.y, 4.0f);
		previouspoint = currentpoint;
	}

	// Use gSpline's own renderer for the crisp center line.
	setColor(207, 250, 254);
	spline.draw();
}

void gCanvas::drawControlPoints() {
	for (std::size_t i = 0; i < controlpoints.size(); i++) {
		bool isactive = static_cast<int>(i) == selectedpoint
				|| static_cast<int>(i) == hoveredpoint;

		setColor(129, 140, 248, isactive ? 80 : 35);
		gDrawCircle(controlpoints[i].x, controlpoints[i].y,
				isactive ? 19.0f : 15.0f, true);

		setColor(isactive ? 250 : 199, isactive ? 204 : 210,
				isactive ? 21 : 254);
		gDrawCircle(controlpoints[i].x, controlpoints[i].y,
				GSPLINE_CONTROL_POINT_RADIUS, true);

		setColor(15, 23, 42);
		gDrawCircle(controlpoints[i].x, controlpoints[i].y, 3.0f, true);
	}
}

void gCanvas::drawMarker() {
	glm::vec2 marker = spline.getPoint2D(animationposition);
	float pulse = 2.0f
			+ 2.0f * std::sin(animationposition * GSPLINE_PI * 8.0f);

	for (int i = 5; i >= 1; i--) {
		float trailt = animationposition - i * 0.018f;
		if (trailt < 0.0f) {
			trailt += 1.0f;
		}
		glm::vec2 trailpoint = spline.getPoint2D(trailt);
		setColor(236, 72, 153, 18 + (5 - i) * 12);
		gDrawCircle(trailpoint.x, trailpoint.y, 3.0f + i * 0.6f, true);
	}

	setColor(236, 72, 153, 45);
	gDrawCircle(marker.x, marker.y, 18.0f + pulse, true);
	setColor(244, 114, 182);
	gDrawCircle(marker.x, marker.y, 7.0f, true);
	setColor(255, 255, 255);
	gDrawCircle(marker.x, marker.y, 2.5f, true);
}

void gCanvas::drawInterface() {
	setColor(240, 249, 255);
	titlefont.drawText("B-SPLINE PLAYGROUND", 54, 62);

	setColor(148, 163, 184);
	bodyfont.drawText(
			"A clamped, uniform B-Spline evaluated with De Boor's algorithm",
			54, 91);

	std::string degreeinfo = "DEGREE  " + std::to_string(spline.getDegree());
	float pillwidth = smallfont.getStringWidth(degreeinfo) + 34.0f;
	float pillx = getWidth() - pillwidth - 52.0f;
	setColor(79, 70, 229, 180);
	gDrawRoundedRectangle(pillx, 48, pillwidth, 32, 16, true);
	setColor(238, 242, 255);
	smallfont.drawText(degreeinfo, pillx + 17, 69);

	std::string status = animationpaused ? "PAUSED" : "PLAYING";
	setColor(animationpaused ? 251 : 74, animationpaused ? 191 : 222,
			animationpaused ? 36 : 128);
	gDrawCircle(54, getHeight() - 31, 4.0f, true);
	setColor(148, 163, 184);
	smallfont.drawText(status, 67, getHeight() - 26);

	std::string instructions =
			"DRAG POINTS   |   UP / DOWN: DEGREE   |   SPACE: PAUSE   |   R: RESET";
	float instructionswidth = smallfont.getStringWidth(instructions);
	smallfont.drawText(instructions,
			getWidth() - instructionswidth - 46, getHeight() - 26);
}

void gCanvas::resetControlPoints() {
	controlpoints = {
			{getWidth() * 0.09f, getHeight() * 0.62f},
			{getWidth() * 0.22f, getHeight() * 0.30f},
			{getWidth() * 0.36f, getHeight() * 0.72f},
			{getWidth() * 0.49f, getHeight() * 0.38f},
			{getWidth() * 0.63f, getHeight() * 0.59f},
			{getWidth() * 0.77f, getHeight() * 0.25f},
			{getWidth() * 0.91f, getHeight() * 0.56f}
	};

	spline.setPoint(controlpoints);
	selectedpoint = -1;
	hoveredpoint = -1;
}

int gCanvas::findControlPoint(int x, int y) const {
	const float pickradius = 18.0f;
	for (int i = static_cast<int>(controlpoints.size()) - 1; i >= 0; i--) {
		float dx = controlpoints[i].x - x;
		float dy = controlpoints[i].y - y;
		if (dx * dx + dy * dy <= pickradius * pickradius) {
			return i;
		}
	}
	return -1;
}

glm::vec2 gCanvas::clampToWorkspace(int x, int y) {
	return glm::vec2(
			std::max(35.0f, std::min(static_cast<float>(getWidth() - 35),
					static_cast<float>(x))),
			std::max(145.0f, std::min(static_cast<float>(getHeight() - 65),
					static_cast<float>(y))));
}

void gCanvas::keyPressed(int key) {
	if (key == G_KEY_SPACE) {
		animationpaused = !animationpaused;
	} else if (key == G_KEY_UP) {
		int maxdegree = std::min(GSPLINE_MAX_DEGREE,
				static_cast<int>(controlpoints.size()) - 1);
		spline.setDegree(std::min(spline.getDegree() + 1, maxdegree));
	} else if (key == G_KEY_DOWN) {
		spline.setDegree(std::max(spline.getDegree() - 1, 1));
	} else if (key == G_KEY_R) {
		spline.setDegree(3);
		animationposition = 0.0f;
		resetControlPoints();
	}
}

void gCanvas::keyReleased(int /*key*/) {
}

void gCanvas::charPressed(unsigned int /*codepoint*/) {
}

void gCanvas::mouseMoved(int x, int y) {
	hoveredpoint = findControlPoint(x, y);
}

void gCanvas::mouseDragged(int x, int y, int button) {
	if (button == MOUSEBUTTON_LEFT && selectedpoint >= 0) {
		controlpoints[selectedpoint] = clampToWorkspace(x, y);
		spline.setPointAtIndex(selectedpoint, controlpoints[selectedpoint]);
	}
}

void gCanvas::mousePressed(int x, int y, int button) {
	if (button == 0) {
		selectedpoint = findControlPoint(x, y);
	}
}

void gCanvas::mouseReleased(int x, int y, int button) {
	if (button == 0) {
		selectedpoint = -1;
		hoveredpoint = findControlPoint(x, y);
	}
}

void gCanvas::mouseScrolled(int /*x*/, int /*y*/) {
}

void gCanvas::mouseEntered() {
}

void gCanvas::mouseExited() {
	hoveredpoint = -1;
}

void gCanvas::windowResized(int /*w*/, int /*h*/) {
	resetControlPoints();
}

void gCanvas::showNotify() {
}

void gCanvas::hideNotify() {
}
