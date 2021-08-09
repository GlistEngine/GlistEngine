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
 * gCircle.h
 *
 *  Created on: 9 Tem 2021
 *      Author: YavuzBilginoglu
 */

#ifndef GRAPHICS_PRIMITIVES_GCIRCLE_H_
#define GRAPHICS_PRIMITIVES_GCIRCLE_H_

#include "gMesh.h"

/*
 * Creates objects with the shape of a circle.
 */
class gCircle: public gMesh {
public:

	/*
	 * Creates objects with the shape of a circle.
	 * Calculating coordinate information.
	 * @param xCenter x-coordinate of the circle's center
	 * @param yCenter y-coordinate of the circle's center
	 * @param radius radius of the circle
	 * @param isFilled signifies whether the circle is full or empty
	 * @param numberOfsides  indicates the detail value of the circle.
	 */
	gCircle();
	gCircle(float xCenter, float yCenter, float radius, bool isFilled, float numberOfSides = 64.0f);
	virtual ~gCircle();

   /*
	* Provides draw using mesh class.
	*/
	void draw();

	/*
	 * Determines whether the projection is two-dimensional.
	 * The coordinates of the circle vertices created are taken.
	 * Provides draw using mesh class.
	 *
	 * @param xCenter x-coordinate of the circle's center
	 * @param yCenter y-coordinate of the circle's center
	 * @param radius radius of the circle
	 * @param isFilled signifies whether the circle is full or empty
	 * @param numberOfsides  indicates the detail value of the circle.
	 */
	void draw(float xCenter, float yCenter, float radius, bool isFilled, float numberOfSides = 64.0f);

private:

	/*
	 * The circle is formed by triangles.
	 * These triangles vertices form  circle.
	 * Calculating coordinate information.
	 * The more side are given, the sharper the circle will be, as the number of triangles will increase.
	 * But it does not provide any visible change after 64 sides given as default.
	 *
	 * @param xCenter x-coordinate of the circle's center
	 * @param yCenter y-coordinate of the circle's center
	 * @param radius radius of the circle
	 * @param isFilled signifies whether the circle is full or empty
	 * @param numberOfsides  indicates the detail value of the circle.
	 */
	void setCirclePoints(float xCenter, float yCenter, float radius, bool isFilled, float numberOfSides = 64.0f);
};

#endif /* GRAPHICS_PRIMITIVES_GCIRCLE_H_ */
