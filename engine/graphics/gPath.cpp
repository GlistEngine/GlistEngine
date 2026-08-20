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
 * gPath.cpp
 *
 * Created on: Aug 18, 2026
 * Authors: Bahar Kucukozer, Mehmet Sefa Ciftci
 */

#include "gPath.h"

#include "gArc.h"
#include "gBezier.h"
#include "gLine.h"
#include "gMesh.h"

#include <glm/common.hpp>
#include <glm/geometric.hpp>


gPath::gSubPath::gSubPath(gLine& line, int pointcount)
	: type(TYPE_LINE), currentpoint(0) {
	const std::vector<gVertex>& vertices = line.getVertices();
	std::vector<glm::vec3> linepoints;

	if (line.getDrawMode() == gMesh::DRAWMODE_TRIANGLES && vertices.size() >= 4) {
		linepoints.push_back(
				(vertices[0].position + vertices[2].position) * 0.5f);

		linepoints.push_back(
				(vertices[1].position + vertices[3].position) * 0.5f);
	} else if (!vertices.empty()) {
		linepoints.push_back(vertices.front().position);

		if (vertices.size() > 1) {
			linepoints.push_back(vertices.back().position);
		}
	}

	points = samplePoints(linepoints, pointcount);
}

gPath::gSubPath::gSubPath(gArc& arc, int pointcount)
	: type(TYPE_ARC), currentpoint(0) {
	const std::vector<gVertex>& vertices = arc.getVertices();
	std::vector<glm::vec3> arcpoints;

	std::size_t firstpoint =
			arc.getDrawMode() == gMesh::DRAWMODE_TRIANGLEFAN && !vertices.empty() ? 1 : 0;

	for (std::size_t i = firstpoint; i < vertices.size(); i++) {
		arcpoints.push_back(vertices[i].position);
	}

	points = samplePoints(arcpoints, pointcount);
}

gPath::gSubPath::gSubPath(
		const gBezier& bezier,
		int pointcount)
	: type(TYPE_BEZIER), currentpoint(0) {
	if (pointcount <= 0) {
		return;
	}

	points.reserve(pointcount);

	if (pointcount == 1) {
		points.push_back(bezier.getPoint(0.0f));
		return;
	}

	for (int i = 0; i < pointcount; i++) {
		float t = static_cast<float>(i) / static_cast<float>(pointcount - 1);

		points.push_back(bezier.getPoint(t));
	}
}

gPath::gSubPath::gSubPath(
		const gPath& path,
		int pointcount)
	: type(TYPE_PATH), currentpoint(0) {
	points = samplePoints(path.getPoints(), pointcount);
}

gPath::gSubPath::Type gPath::gSubPath::getType() const {
	return type;
}

int gPath::gSubPath::getPointCount() const {
	return static_cast<int>(points.size());
}

glm::vec3 gPath::gSubPath::getPoint(int pointno) {
	if (pointno < 0 || pointno >= getPointCount()) {
		return glm::vec3(0.0f);
	}

	return points[pointno];
}

const std::vector<glm::vec3>&
gPath::gSubPath::getPoints() const {
	return points;
}

bool gPath::gSubPath::hasNextPoint() const {
	return currentpoint < points.size();
}

glm::vec3 gPath::gSubPath::getNextPoint() {
	if (!hasNextPoint()) {
		return glm::vec3(0.0f);
	}

	return points[currentpoint++];
}

void gPath::gSubPath::reset() {
	currentpoint = 0;
}

std::vector<glm::vec3> gPath::gSubPath::samplePoints(
		const std::vector<glm::vec3>& sourcepoints,
		int pointcount) {
	std::vector<glm::vec3> sampledpoints;

	if (pointcount <= 0 || sourcepoints.empty()) {
		return sampledpoints;
	}

	sampledpoints.reserve(pointcount);

	if (pointcount == 1 || sourcepoints.size() == 1) {
		for (int i = 0; i < pointcount; i++) {
			sampledpoints.push_back(sourcepoints.front());
		}

		return sampledpoints;
	}

	std::vector<float> distances(sourcepoints.size(), 0.0f);

	for (std::size_t i = 1; i < sourcepoints.size(); i++) {
		distances[i] = distances[i - 1] + glm::distance(
												  sourcepoints[i - 1],
												  sourcepoints[i]);
	}

	float totallength = distances.back();

	if (totallength == 0.0f) {
		for (int i = 0; i < pointcount; i++) {
			sampledpoints.push_back(sourcepoints.front());
		}

		return sampledpoints;
	}

	std::size_t segment = 1;

	for (int i = 0; i < pointcount; i++) {
		float targetdistance =
				totallength * static_cast<float>(i) / static_cast<float>(pointcount - 1);

		while (segment < distances.size() - 1 && distances[segment] < targetdistance) {
			segment++;
		}

		float segmentlength =
				distances[segment] - distances[segment - 1];

		float t = segmentlength > 0.0f
						  ? (targetdistance - distances[segment - 1]) / segmentlength
						  : 0.0f;

		sampledpoints.push_back(
				glm::mix(
						sourcepoints[segment - 1],
						sourcepoints[segment],
						t));
	}

	return sampledpoints;
}

gPath::gPath() : currentsubpath(0) {
}

gPath::~gPath() {
}

void gPath::clear() {
	subpaths.clear();
	currentsubpath = 0;
}

void gPath::addSubPath(const gSubPath& subpath) {
	subpaths.push_back(subpath);
	subpaths.back().reset();
}

void gPath::addSubPath(gLine& line, int pointcount) {
	subpaths.emplace_back(line, pointcount);
}

void gPath::addSubPath(gArc& arc, int pointcount) {
	subpaths.emplace_back(arc, pointcount);
}

void gPath::addSubPath(
		const gBezier& bezier,
		int pointcount) {
	subpaths.emplace_back(bezier, pointcount);
}

void gPath::addSubPath(
		const gPath& path,
		int pointcount) {
	subpaths.emplace_back(path, pointcount);
}

int gPath::getSubPathCount() const {
	return static_cast<int>(subpaths.size());
}

int gPath::getPointCount() const {
	int pointcount = 0;

	for (const gSubPath& subpath : subpaths) {
		pointcount += subpath.getPointCount();
	}

	return pointcount;
}

glm::vec3 gPath::getPoint(int pointno) {
	if (pointno < 0) {
		return glm::vec3(0.0f);
	}

	for (gSubPath& subpath : subpaths) {
		int subpathpointcount = subpath.getPointCount();

		if (pointno < subpathpointcount) {
			return subpath.getPoint(pointno);
		}

		pointno -= subpathpointcount;
	}

	return glm::vec3(0.0f);
}

glm::vec3 gPath::getPoint(int subpathno, int subpathpointno) {
	if (subpathno < 0 || subpathno >= getSubPathCount()) {
		return glm::vec3(0.0f);
	}

	return subpaths[subpathno].getPoint(subpathpointno);
}

const std::vector<gPath::gSubPath>&
gPath::getSubPaths() const {
	return subpaths;
}

std::vector<glm::vec3> gPath::getPoints() const {
	std::vector<glm::vec3> points;
	points.reserve(getPointCount());

	for (const gSubPath& subpath : subpaths) {
		const std::vector<glm::vec3>& subpathpoints =
				subpath.getPoints();

		points.insert(
				points.end(),
				subpathpoints.begin(),
				subpathpoints.end());
	}

	return points;
}

bool gPath::hasNextPoint() const {
	for (std::size_t i = currentsubpath;
		 i < subpaths.size();
		 i++) {
		if (subpaths[i].hasNextPoint()) {
			return true;
		}
	}

	return false;
}

glm::vec3 gPath::getNextPoint() {
	while (currentsubpath < subpaths.size()) {
		if (subpaths[currentsubpath].hasNextPoint()) {
			return subpaths[currentsubpath].getNextPoint();
		}

		currentsubpath++;
	}

	return glm::vec3(0.0f);
}

void gPath::reset() {
	currentsubpath = 0;

	for (gSubPath& subpath : subpaths) {
		subpath.reset();
	}
}
