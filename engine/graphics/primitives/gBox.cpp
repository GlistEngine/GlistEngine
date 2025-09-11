/*
 * gCube.cpp
 *
 *  Created on: Sep 1, 2020
 *      Author: noyan
 */

#include <gBox.h>

gBox::gBox() {

	float vertexdata[]= {
	    // x,   y,   z,  s,  t,
		-1.0f,  1.0f, -1.0f, 1.0f, 1.0f, // Back
		 1.0f,  1.0f, -1.0f, 0.0f, 1.0f,
		-1.0f, -1.0f, -1.0f, 1.0f, 0.0f,
		 1.0f, -1.0f, -1.0f, 0.0f, 0.0f,
		-1.0f,  1.0f,  1.0f, 0.0f, 1.0f, // Front
		 1.0f,  1.0f,  1.0f, 1.0f, 1.0f,
		-1.0f, -1.0f,  1.0f, 0.0f, 0.0f,
		 1.0f, -1.0f,  1.0f, 1.0f, 0.0f,
		-1.0f,  1.0f, -1.0f, 0.0f, 1.0f, // Left
		-1.0f, -1.0f, -1.0f, 0.0f, 0.0f,
		-1.0f, -1.0f,  1.0f, 1.0f, 0.0f,
		-1.0f,  1.0f,  1.0f, 1.0f, 1.0f,
		 1.0f,  1.0f, -1.0f, 1.0f, 1.0f, // Right
		 1.0f, -1.0f, -1.0f, 1.0f, 0.0f,
		 1.0f, -1.0f,  1.0f, 0.0f, 0.0f,
		 1.0f,  1.0f,  1.0f, 0.0f, 1.0f,
		-1.0f, -1.0f, -1.0f, 0.0f, 1.0f, // Top
		-1.0f, -1.0f,  1.0f, 0.0f, 0.0f,
		 1.0f, -1.0f,  1.0f, 1.0f, 0.0f,
		 1.0f, -1.0f, -1.0f, 1.0f, 1.0f,
		-1.0f,  1.0f, -1.0f, 0.0f, 0.0f, // Bottom
		-1.0f,  1.0f,  1.0f, 0.0f, 1.0f,
		 1.0f,  1.0f,  1.0f, 1.0f, 1.0f,
		 1.0f,  1.0f, -1.0f, 1.0f, 0.0f
    };

	float normaldata[] = {
		-1.0f,  1.0f, -1.0f, // Back
	     1.0f,  1.0f, -1.0f,
	    -1.0f, -1.0f, -1.0f,
	     1.0f, -1.0f, -1.0f,
	    -1.0f,  1.0f,  1.0f, // Front
	     1.0f,  1.0f,  1.0f,
	    -1.0f, -1.0f,  1.0f,
	     1.0f, -1.0f,  1.0f,
	    -1.0f,  1.0f, -1.0f, // Left
	    -1.0f,  -1.0f,  -1.0f,
	    -1.0f, -1.0f, 1.0f,
	    -1.0f, 1.0f,  1.0f,
	     1.0f,  1.0f,  -1.0f, // Right
	     1.0f,  -1.0f, -1.0f,
	     1.0f, -1.0f,  1.0f,
	     1.0f, 1.0f, 1.0f,
	    -1.0f, -1.0f,  -1.0f, // Top
	    -1.0f, -1.0f,  1.0f,
	     1.0f, -1.0f, 1.0f,
	     1.0f, -1.0f, -1.0f,
	    -1.0f,  1.0f, -1.0f, // Bottom
	    -1.0f,  1.0f, 1.0f,
	     1.0f,  1.0f,  1.0f,
	     1.0f,  1.0f,  -1.0f
	};


	unsigned int indexdata[] = {
	   // back
		 0,  2, 3,
		 0,  1, 3,
	   // front
		 4,  6, 7,
		 4,  5, 7,
	   // left
		 8,  9, 10,
		11,  8, 10,
	   // right
		12, 13, 14,
		15, 12, 14,
	   // top
		16, 17, 18,
		16, 19, 18,
	   // bottom
		20, 21, 22,
		20, 23, 22
	};


	int nv = (sizeof(vertexdata) / sizeof(vertexdata[0])) / 5;
	std::vector<gVertex> verticesb;
	for (int i=0; i<nv; i++) {
		gVertex v;
		v.position.x = vertexdata[(i * 5)];
		v.position.y = vertexdata[(i * 5) + 1];
		v.position.z = vertexdata[(i * 5) + 2];
		v.texcoords.x = vertexdata[(i * 5) + 3];
		v.texcoords.y = vertexdata[(i * 5) + 4];
		v.normal.x = normaldata[(i * 3)];
		v.normal.y = normaldata[(i * 3) + 1];
		v.normal.z = normaldata[(i * 3) + 2];
		verticesb.push_back(v);
	}

	int ni = sizeof(indexdata) / sizeof(indexdata[0]);
	std::vector<gIndex> indicesb;
	for (int i=0; i<ni; i++) {
		indicesb.push_back(indexdata[i]);
	}

	auto verticesptr = std::make_shared<std::vector<gVertex>>(verticesb);
	auto indicesptr = std::make_shared<std::vector<gIndex>>(indicesb);
	setVertices(verticesptr, indicesptr);
}

gBox::~gBox() {
}

