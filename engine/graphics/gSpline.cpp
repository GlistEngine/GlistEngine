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

/*
 * gSpline.cpp
 *
 * Created on: Aug 20, 2026
 * Authors: Bahar Kucukozer, Mehmet Sefa Ciftci
 */

#include "gSpline.h"

#include "gRenderer.h"

#include <algorithm>
#include <cmath>

gSpline::gSpline()
	: degree(3), resolution(50), is3D(false) {
}

gSpline::~gSpline() {
	points.clear();
	knots.clear();
}

void gSpline::setPoint(const std::vector<glm::vec2>& points) {
	this->points.clear();
	this->points.reserve(points.size());

	for (const glm::vec2& point : points) {
		this->points.push_back(glm::vec3(point.x, point.y, 0.0f));
	}

	is3D = false;
	updateKnotVector();
}

void gSpline::setPoint(const std::vector<glm::vec3>& points) {
	this->points = points;
	is3D = true;
	updateKnotVector();
}

void gSpline::addPoint(glm::vec2 point) {
	points.push_back(glm::vec3(point.x, point.y, 0.0f));
	updateKnotVector();
}

void gSpline::addPoint(glm::vec3 point) {
	points.push_back(point);
	is3D = true;
	updateKnotVector();
}

void gSpline::setPointAtIndex(int index, glm::vec2 point) {
	if (index < 0 || index >= static_cast<int>(points.size())) {
		return;
	}

	points[index] = glm::vec3(point.x, point.y, 0.0f);
}

void gSpline::setPointAtIndex(int index, glm::vec3 point) {
	if (index < 0 || index >= static_cast<int>(points.size())) {
		return;
	}

	points[index] = point;
	is3D = true;
}

glm::vec3 gSpline::getPointAtIndex(int index) const {
	if (index < 0 || index >= static_cast<int>(points.size())) {
		return glm::vec3(0.0f);
	}

	return points[index];
}

glm::vec2 gSpline::getPoint2DAtIndex(int index) const {
	glm::vec3 point = getPointAtIndex(index);
	return glm::vec2(point.x, point.y);
}

std::vector<glm::vec3> gSpline::getPoints() const {
	return points;
}

std::vector<glm::vec2> gSpline::getPoints2D() const {
	std::vector<glm::vec2> points2d;
	points2d.reserve(points.size());

	for (const glm::vec3& point : points) {
		points2d.push_back(glm::vec2(point.x, point.y));
	}

	return points2d;
}

void gSpline::clearPoints() {
	points.clear();
	knots.clear();
	is3D = false;
}

void gSpline::setDegree(int degree) {
	if (degree < 1) {
		degree = 1;
	}

	this->degree = degree;
	updateKnotVector();
}

int gSpline::getDegree() const {
	return degree;
}

glm::vec3 gSpline::getPoint(float t) const {
	if (points.empty()) {
		return glm::vec3(0.0f);
	}

	if (points.size() == 1) {
		return points.front();
	}

	float normalizedt = std::max(0.0f, std::min(1.0f, t));
	int effectivedegree = getEffectiveDegree();
	int span = findSpan(normalizedt);

	std::vector<glm::vec3> values(effectivedegree + 1);

	for (int i = 0; i <= effectivedegree; i++) {
		values[i] = points[span - effectivedegree + i];
	}

	for (int level = 1; level <= effectivedegree; level++) {
		for (int i = effectivedegree; i >= level; i--) {
			int knotindex = span - effectivedegree + i;

			float denominator =
					knots[knotindex + effectivedegree - level + 1]
					- knots[knotindex];

			float alpha = denominator > 0.0f
					? (normalizedt - knots[knotindex]) / denominator
					: 0.0f;

			values[i] =
					(1.0f - alpha) * values[i - 1]
					+ alpha * values[i];
		}
	}

	return values[effectivedegree];
}

glm::vec2 gSpline::getPoint2D(float t) const {
	glm::vec3 point = getPoint(t);
	return glm::vec2(point.x, point.y);
}

float gSpline::getSlope(float t) const {
	if (points.size() < 2) {
		return 0.0f;
	}

	constexpr float epsilon = 0.0001f;

	float normalizedt = std::max(0.0f, std::min(1.0f, t));
	float previoust = std::max(0.0f, normalizedt - epsilon);
	float nextt = std::min(1.0f, normalizedt + epsilon);

	glm::vec3 tangent = getPoint(nextt) - getPoint(previoust);

	float lengthSquared =
			tangent.x * tangent.x
			+ tangent.y * tangent.y;

	if (lengthSquared <= 0.000000000001f) {
		return 0.0f;
	}

	return std::atan2(tangent.y, tangent.x);
}

void gSpline::setResolution(int resolution) {
	if (resolution < 1) {
		resolution = 1;
	}

	this->resolution = resolution;
}

int gSpline::getResolution() const {
	return resolution;
}

void gSpline::draw() const {
	if (points.size() < 2) {
		return;
	}

	glm::vec3 previouspoint = getPoint(0.0f);

	for (int i = 1; i <= resolution; i++) {
		float t = static_cast<float>(i)
				/ static_cast<float>(resolution);

		glm::vec3 currentpoint = getPoint(t);

		if (is3D) {
			gDrawLine(
					previouspoint.x,
					previouspoint.y,
					previouspoint.z,
					currentpoint.x,
					currentpoint.y,
					currentpoint.z);
		} else {
			gDrawLine(
					previouspoint.x,
					previouspoint.y,
					currentpoint.x,
					currentpoint.y);
		}

		previouspoint = currentpoint;
	}
}

int gSpline::getEffectiveDegree() const {
	if (points.empty()) {
		return 0;
	}

	return std::min(
			degree,
			static_cast<int>(points.size()) - 1);
}

void gSpline::updateKnotVector() {
	knots.clear();

	if (points.empty()) {
		return;
	}

	int pointcount = static_cast<int>(points.size());
	int effectivedegree = getEffectiveDegree();
	int lastpoint = pointcount - 1;
	int knotcount = pointcount + effectivedegree + 1;

	knots.resize(knotcount, 0.0f);

	int interiorcount = lastpoint - effectivedegree;

	for (int i = 1; i <= interiorcount; i++) {
		knots[effectivedegree + i] =
				static_cast<float>(i)
				/ static_cast<float>(interiorcount + 1);
	}

	for (int i = lastpoint + 1; i < knotcount; i++) {
		knots[i] = 1.0f;
	}
}

int gSpline::findSpan(float t) const {
	int effectivedegree = getEffectiveDegree();
	int lastpoint = static_cast<int>(points.size()) - 1;

	if (t >= 1.0f) {
		return lastpoint;
	}

	if (t <= 0.0f) {
		return effectivedegree;
	}

	int low = effectivedegree;
	int high = lastpoint + 1;
	int middle = (low + high) / 2;

	while (t < knots[middle] || t >= knots[middle + 1]) {
		if (t < knots[middle]) {
			high = middle;
		} else {
			low = middle;
		}

		middle = (low + high) / 2;
	}

	return middle;
}
