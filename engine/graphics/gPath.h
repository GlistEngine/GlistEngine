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
 * gPath.h
 *
 * Created on: Aug 18, 2026
 * Authors: Bahar Kucukozer, Mehmet Sefa Ciftci
 */

#ifndef GRAPHICS_GPATH_H_
#define GRAPHICS_GPATH_H_

#include <cstddef>
#include <glm/glm.hpp>
#include <vector>

class gArc;
class gBezier;
class gLine;

class gPath {
public:
	class gSubPath {
	public:
		enum Type {
			TYPE_LINE,
			TYPE_ARC,
			TYPE_BEZIER,
			TYPE_PATH
		};

		gSubPath(gLine& line, int pointcount);
		gSubPath(gArc& arc, int pointcount);
		gSubPath(const gBezier& bezier, int pointcount);
		gSubPath(const gPath& path, int pointcount);

		Type getType() const;
		int getPointCount() const;
		glm::vec3 getPoint(int pointno);
		const std::vector<glm::vec3>& getPoints() const;

		bool hasNextPoint() const;
		glm::vec3 getNextPoint();
		void reset();

	private:
		static std::vector<glm::vec3> samplePoints(
				const std::vector<glm::vec3>& sourcepoints,
				int pointcount);

		Type type;
		std::vector<glm::vec3> points;
		std::size_t currentpoint;
	};

	gPath();
	virtual ~gPath();

	void clear();

	void addSubPath(const gSubPath& subpath);
	void addSubPath(gLine& line, int pointcount);
	void addSubPath(gArc& arc, int pointcount);
	void addSubPath(const gBezier& bezier, int pointcount);
	void addSubPath(const gPath& path, int pointcount);

	int getSubPathCount() const;
	int getPointCount() const;
	glm::vec3 getPoint(int pointno);
	glm::vec3 getPoint(int subpathno, int subpathpointno);

	const std::vector<gSubPath>& getSubPaths() const;
	std::vector<glm::vec3> getPoints() const;

	bool hasNextPoint() const;
	glm::vec3 getNextPoint();
	void reset();

private:
	std::vector<gSubPath> subpaths;
	std::size_t currentsubpath;
};

#endif /* GRAPHICS_GPATH_H_ */
