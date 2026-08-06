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
 * gCircle.cpp
 *
 *  Created on: 9 Tem 2021
 *      Author: YavuzBilginoglu
 */

#include "gCircle.h"

gCircle::gCircle() {

}

gCircle::gCircle(float xCenter, float yCenter, float radius, bool isFilled, float numberOfSides, float rotateAngle, float pivotx, float pivoty) {
	isprojection2d = true;
	setCirclePoints(xCenter, yCenter, radius, isFilled, numberOfSides, rotateAngle, pivotx, pivoty);
}

gCircle::~gCircle() {

}

void gCircle::setPoints(float xCenter, float yCenter, float radius, bool isFilled, float numberOfSides, float rotateAngle, float pivotx, float pivoty) {
	isprojection2d = true;
	setCirclePoints(xCenter, yCenter, radius, isFilled, numberOfSides, rotateAngle, pivotx, pivoty);
}

void gCircle::draw(){
	isprojection2d = true;
	gMesh::draw();
}

void gCircle::draw(float xCenter, float yCenter, float radius, bool isFilled, float numberOfSides, float rotateAngle, float pivotx, float pivoty){
	isprojection2d = true;
	setCirclePoints(xCenter, yCenter, radius, isFilled, numberOfSides, rotateAngle, pivotx, pivoty);
	gMesh::draw();
}

void gCircle::setCirclePoints(float xCenter, float yCenter, float radius, bool isFilled, float numberOfSides, float rotateAngle, float pivotx, float pivoty) {
	float angle = PI * 2 / numberOfSides;
	std::vector<gVertex> verticessb;

	float pivotPx = (xCenter - radius) + pivotx * (2.0f * radius);
	float pivotPy = (yCenter - radius) + pivoty * (2.0f * radius);

	float cosA = cos(rotateAngle);
	float sinA = sin(rotateAngle);

	for(int i = 0; i <= numberOfSides; i++) {
		float nextAngle = angle * i;
		gVertex vertex;

		float px = radius * cos(nextAngle) + xCenter;
		float py = radius * sin(nextAngle) + yCenter;

		if (rotateAngle != 0.0f) {
			float relX = px - pivotPx;
			float relY = py - pivotPy;

			vertex.position.x = relX * cosA - relY * sinA + pivotPx;
			vertex.position.y = relX * sinA + relY * cosA + pivotPy;
		} else {
			vertex.position.x = px;
			vertex.position.y = py;
		}

		vertex.position.z = 0.0f;
		verticessb.push_back(vertex);
	}

	auto verticesptr = std::make_shared<std::vector<gVertex>>(verticessb);
	setVertices(verticesptr);
	if(isFilled == false) setDrawMode(gMesh::DRAWMODE_LINESTRIP);
	else setDrawMode(gMesh::DRAWMODE_TRIANGLEFAN);
}
