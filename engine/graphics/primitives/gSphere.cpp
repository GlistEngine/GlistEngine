/*
 * gSphere.cpp
 *
 *  Created on: Apr 9, 2021
 *      Author: noyan
 */

#include <gSphere.h>

gSphere::gSphere(int sectorCount, int stackCount) {
	if (sectorCount < 3) sectorCount = 3;
	if (stackCount < 3) stackCount = 2;

	float radius = 1.0f;

    float x, y, z, xy;                              // vertex position
    float nx, ny, nz, lengthInv = 1.0f / radius;    // normal
    float s, t;                                     // texCoord

    float sectorStep = 2 * PI / sectorCount;
    float stackStep = PI / stackCount;
    float sectorAngle, stackAngle;

    int vnum = (sectorCount + 1) * (stackCount + 1);
    float vertexdata[vnum * 5];
    float normaldata[vnum * 3];
    int inum = (((stackCount - 2) * sectorCount * 6) + (2 * sectorCount * 3));
    unsigned int indexdata[inum];
    int vt = vnum * 5;

	int vno = 0;
    for(int i = 0; i <= stackCount; ++i) {
        stackAngle = PI / 2 - i * stackStep;        // starting from pi/2 to -pi/2
        xy = radius * cosf(stackAngle);             // r * cos(u)
        z = radius * sinf(stackAngle);              // r * sin(u)

        // add (sectorCount+1) vertices per stack
        // the first and last vertices have same position and normal, but different tex coords
        for(int j = 0; j <= sectorCount; ++j) {
        	int nno = (i * sectorCount + j) * 3;

            sectorAngle = j * sectorStep;           // starting from 0 to 2pi

            // vertex position
            x = xy * cosf(sectorAngle);             // r * cos(u) * cos(v)
            y = xy * sinf(sectorAngle);             // r * cos(u) * sin(v)
            vertexdata[vno] = x;
            vertexdata[vno + 1] = y;
            vertexdata[vno + 2] = z;

            // vertex tex coord between [0, 1]
            s = (float)j / sectorCount;
            t = (float)i / stackCount;
            vertexdata[vno + 3] = s;
            vertexdata[vno + 4] = t;
            vno += 5;

            // normalized vertex normal
            nx = x * lengthInv;
            ny = y * lengthInv;
            nz = z * lengthInv;
            normaldata[nno] = nx;
            normaldata[nno + 1] = ny;
            normaldata[nno + 2] = nz;
        }
    }

    // indices
    //  k1--k1+1
    //  |  / |
    //  | /  |
    //  k2--k2+1
    unsigned int k1, k2;
	int ino = 0;
    for(int i = 0; i < stackCount; ++i) {
        k1 = i * (sectorCount + 1);     // beginning of current stack
        k2 = k1 + sectorCount + 1;      // beginning of next stack

        for(int j = 0; j < sectorCount; ++j, ++k1, ++k2) {
            // 2 triangles per sector excluding 1st and last stacks
            if(i != 0) {
            	indexdata[ino] = k1;
            	indexdata[ino + 1] = k2;
            	indexdata[ino + 2] = k1 + 1;
                ino += 3;
            }

            if(i != (stackCount-1)) {
            	indexdata[ino] = k1 + 1;
            	indexdata[ino + 1] = k2;
            	indexdata[ino + 2] = k2 + 1;
                ino += 3;
            }
        }
    }


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
	std::vector<unsigned int> indicesb;
	for (int i=0; i<ni; i++) {
		indicesb.push_back(indexdata[i]);
	}

	setVertices(verticesb, indicesb);
}

gSphere::~gSphere() {
}

