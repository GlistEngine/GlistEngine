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
				1.0f, 0.0f, // top right
				1.0f, 1.0f, // bottom right
				0.0f, 1.0f, // bottom left
				0.0f, 0.0f  // top left
		};

		float normaldata[] = {
		    -1.0f,  1.0f,  1.0f, // Front
		     1.0f,  1.0f,  1.0f,
		    -1.0f, -1.0f,  1.0f,
		     1.0f, -1.0f,  1.0f
		};

		std::vector<gVertex> verticessb;
		std::vector<gIndex> indicessb;

		for (int i=0; i<4; i++) {
			gVertex vertex;
			vertex.position.x = vpos[(i * 3)];
			vertex.position.y = vpos[(i * 3) + 1];
			vertex.position.z = vpos[(i * 3) + 2];
			vertex.texcoords.x = tcoords[(i * 2)];
			vertex.texcoords.y = tcoords[(i * 2) + 1];
			vertex.normal.x = normaldata[(i * 3)];
			vertex.normal.y = normaldata[(i * 3) + 1];
			vertex.normal.z = normaldata[(i * 3) + 2];
			verticessb.push_back(vertex);
		}

		indicessb.push_back(0);
		indicessb.push_back(1);
		indicessb.push_back(3);
		indicessb.push_back(1);
		indicessb.push_back(2);
		indicessb.push_back(3);

	auto verticesptr = std::make_shared<std::vector<gVertex>>(verticessb);
	auto indicesptr = std::make_shared<std::vector<gIndex>>(indicessb);
	setVertices(verticesptr, indicesptr);
}

gPlane::~gPlane() {
}

