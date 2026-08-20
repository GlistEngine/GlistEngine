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
 * gSpline.h
 *
 * Created on: Aug 20, 2026
 * Authors: Bahar Kucukozer, Mehmet Sefa Ciftci
 */

#ifndef GRAPHICS_GSPLINE_H_
#define GRAPHICS_GSPLINE_H_

#include "gMesh.h"

#include <glm/glm.hpp>
#include <vector>

class gSpline : public gMesh {
public:
	gSpline();
	virtual ~gSpline();

	void setPoint(const std::vector<glm::vec2>& points);
	void setPoint(const std::vector<glm::vec3>& points);

	void addPoint(glm::vec2 point);
	void addPoint(glm::vec3 point);

	void setPointAtIndex(int index, glm::vec2 point);
	void setPointAtIndex(int index, glm::vec3 point);

	glm::vec3 getPointAtIndex(int index) const;
	glm::vec2 getPoint2DAtIndex(int index) const;

	std::vector<glm::vec3> getPoints() const;
	std::vector<glm::vec2> getPoints2D() const;

	void clearPoints();

	void setDegree(int degree);
	int getDegree() const;

	glm::vec3 getPoint(float t) const;
	glm::vec2 getPoint2D(float t) const;

	void setResolution(int resolution);
	int getResolution() const;

	void draw() const;

private:
	int getEffectiveDegree() const;
	void updateKnotVector();
	int findSpan(float t) const;

	std::vector<glm::vec3> points;
	std::vector<float> knots;

	int degree;
	int resolution;
	bool is3D;
};

#endif /* GRAPHICS_GSPLINE_H_ */
