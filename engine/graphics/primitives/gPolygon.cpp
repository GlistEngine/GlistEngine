/*
 * gPolygon.cpp
 *
 *  Created on: 19 Aðu 2026
 *      Author: merve
 */

#include "gPolygon.h"

gPolygon::gPolygon() {
}

gPolygon::~gPolygon() {
}

void gPolygon::addPoint(float x, float y) {
	pointsx.push_back(x);
	pointsy.push_back(y);
}

void gPolygon::clearPoints() {
	pointsx.clear();
	pointsy.clear();
}

void gPolygon::draw() {
	draw(0.0f, 0.0f);
}

void gPolygon::draw(float x, float y) {
	if (pointsx.size() < 3) return;

	drawPolygon(x, y);
}

void gPolygon::drawPolygon(float x, float y) {
	for (size_t i = 0; i < pointsx.size(); i++) {
		size_t nextIndex = (i + 1) % pointsx.size();
		renderer->drawLine(
			x + pointsx[i],
			y + pointsy[i],
			x + pointsx[nextIndex],
			y + pointsy[nextIndex]
		);
	}
}


