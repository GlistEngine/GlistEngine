/*
 * gPlane.cpp
 *
 *  Created on: Sep 1, 2020
 *      Author: noyan
 */

#include "gPlane.h"

gPlane::gPlane() {
	const float vpos[] = {
				1.0f,  1.0f, 0.0f,
				1.0f, -1.0f, 0.0f,
			   -1.0f, -1.0f, 0.0f,
			   -1.0f,  1.0f, 0.0f
			};

		const float tcoords[] = {
				1.0f, 1.0f, // top right
				1.0f, 0.0f, // bottom right
				0.0f, 0.0f, // bottom left
				0.0f, 1.0f  // top left
		};

		std::vector<gVertex> verticessb;
		std::vector<unsigned int> indicessb;

		for (int i=0; i<4; i++) {
			gVertex vertex;
			vertex.position.x = vpos[(i * 3)];
			vertex.position.y = vpos[(i * 3) + 1];
			vertex.position.z = vpos[(i * 3) + 2];
			vertex.texcoords.x = tcoords[(i * 2)];
			vertex.texcoords.y = tcoords[(i * 2) + 1];
			verticessb.push_back(vertex);
		}

		indicessb.push_back(0);
		indicessb.push_back(1);
		indicessb.push_back(3);
		indicessb.push_back(1);
		indicessb.push_back(2);
		indicessb.push_back(3);

		setVertices(verticessb, indicessb);
}

gPlane::~gPlane() {
}

