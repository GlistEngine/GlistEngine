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
 * gLine.h
 *
 *  Created on: May 7, 2021
 *      Author: noyan
 */

#ifndef GRAPHICS_PRIMITIVES_GLINE_H_
#define GRAPHICS_PRIMITIVES_GLINE_H_

#include "gMesh.h"


class gLine: public gMesh {
public:
	gLine();
	gLine(float x1, float y1, float x2, float y2);
	gLine(float x1, float y1, float z1, float x2, float y2, float z2);

	void setPoints(float x1, float y1, float x2, float y2);
	void setPoints(float x1, float y1, float z1, float x2, float y2, float z2);

	void draw();
	void draw(float x1, float y1, float x2, float y2);
	void draw(float x1, float y1, float z1, float x2, float y2, float z2);

	void setThickness(float thickness);

private:
	std::vector<gVertex> verticessb;
	std::vector<unsigned int> indicessb;
	void setLinePoints(float x1, float y1, float z1, float x2, float y2, float z2);
	gVertex vertex1, vertex2;
	float thickness;

};

#endif /* GRAPHICS_PRIMITIVES_GLINE_H_ */
