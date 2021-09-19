/*
 * gSphere.cpp
 *
 *  Created on: Apr 9, 2021
 *      Author: noyan
 */

#include <gSphere.h>

gSphere::gSphere(int xSegmentNum, int ySegmentNum, bool isFilled) {
	if (xSegmentNum < 3) xSegmentNum = 3;
	if (ySegmentNum < 3) ySegmentNum = 2;

    int vnum = (xSegmentNum + 1) * (ySegmentNum + 1);
    float vertexdata[vnum * 5];
    float normaldata[vnum * 3];
    int inum = ySegmentNum * (xSegmentNum + 1) * 2;
    unsigned int indexdata[inum];


    std::vector<glm::vec3> positions1;
    std::vector<glm::vec2> uv1;
    std::vector<glm::vec3> normals1;
	std::vector<unsigned int> indicesb;
    for (unsigned int y = 0; y <= ySegmentNum; ++y) {
        for (unsigned int x = 0; x <= xSegmentNum; ++x) {
            float xSegment = (float)x / (float)xSegmentNum;
            float ySegment = (float)y / (float)ySegmentNum;
            float xPos = std::cos(xSegment * 2.0f * PI) * std::sin(ySegment * PI);
            float yPos = std::cos(ySegment * PI);
            float zPos = std::sin(xSegment * 2.0f * PI) * std::sin(ySegment * PI);

            positions1.push_back(glm::vec3(xPos, yPos, zPos));
            uv1.push_back(glm::vec2(xSegment, ySegment));
            normals1.push_back(glm::vec3(xPos, yPos, zPos));
        }
    }

    bool oddRow = false;
    for (unsigned int y = 0; y < ySegmentNum; ++y) {
        if (!oddRow) { // even rows: y == 0, y == 2; and so on
            for (unsigned int x = 0; x <= xSegmentNum; ++x) {
                indicesb.push_back(y       * (xSegmentNum + 1) + x);
                indicesb.push_back((y + 1) * (xSegmentNum + 1) + x);
            }
        } else {
            for (int x = xSegmentNum; x >= 0; --x) {
                indicesb.push_back((y + 1) * (xSegmentNum + 1) + x);
                indicesb.push_back(y       * (xSegmentNum + 1) + x);
            }
        }
        oddRow = !oddRow;
    }
//    gLogi("GC") << "inum:" << indices1.size();



//*************************************************************************

	int nv = positions1.size();
	std::vector<gVertex> verticesb;
	for (int i=0; i<nv; i++) {
		gVertex v;
		v.position.x = positions1[i].x;
		v.position.y = positions1[i].y;
		v.position.z = positions1[i].z;
		v.texcoords.x = uv1[i].x;
		v.texcoords.y = uv1[i].y;
		v.normal.x = normals1[i].x;
		v.normal.y = normals1[i].y;
		v.normal.z = normals1[i].z;
		verticesb.push_back(v);
	}

	setVertices(verticesb, indicesb);
	if(!isFilled) setDrawMode(gMesh::DRAWMODE_LINESTRIP);
	else setDrawMode(gMesh::DRAWMODE_TRIANGLESTRIP);

}

gSphere::~gSphere() {
}

void gSphere::draw() {
	gMesh::draw();
}
void gSphere::drawSphere(int xSegmentNum, int ySegmentNum, bool isFilled) {
	setSpherePoints(xSegmentNum, ySegmentNum, isFilled);
	gMesh::draw();
}

void gSphere::setSpherePoints(int xSegmentNum, int ySegmentNum, bool isFilled) {

	if (xSegmentNum < 3) xSegmentNum = 3;
	if (ySegmentNum < 3) ySegmentNum = 2;

    int vnum = (xSegmentNum + 1) * (ySegmentNum + 1);
    float vertexdata[vnum * 5];
    float normaldata[vnum * 3];
    int inum = ySegmentNum * (xSegmentNum + 1) * 2;
    unsigned int indexdata[inum];


    std::vector<glm::vec3> positions1;
    std::vector<glm::vec2> uv1;
    std::vector<glm::vec3> normals1;
	std::vector<unsigned int> indicesb;
    for (unsigned int y = 0; y <= ySegmentNum; ++y) {
        for (unsigned int x = 0; x <= xSegmentNum; ++x) {
            float xSegment = (float)x / (float)xSegmentNum;
            float ySegment = (float)y / (float)ySegmentNum;
            float xPos = std::cos(xSegment * 2.0f * PI) * std::sin(ySegment * PI);
            float yPos = std::cos(ySegment * PI);
            float zPos = std::sin(xSegment * 2.0f * PI) * std::sin(ySegment * PI);

            positions1.push_back(glm::vec3(xPos, yPos, zPos));
            uv1.push_back(glm::vec2(xSegment, ySegment));
            normals1.push_back(glm::vec3(xPos, yPos, zPos));
        }
    }

    bool oddRow = false;
    for (unsigned int y = 0; y < ySegmentNum; ++y) {
        if (!oddRow) { // even rows: y == 0, y == 2; and so on
            for (unsigned int x = 0; x <= xSegmentNum; ++x) {
                indicesb.push_back(y       * (xSegmentNum + 1) + x);
                indicesb.push_back((y + 1) * (xSegmentNum + 1) + x);
            }
        } else {
            for (int x = xSegmentNum; x >= 0; --x) {
                indicesb.push_back((y + 1) * (xSegmentNum + 1) + x);
                indicesb.push_back(y       * (xSegmentNum + 1) + x);
            }
        }
        oddRow = !oddRow;
    }
//    gLogi("GC") << "inum:" << indices1.size();



//*************************************************************************

	int nv = positions1.size();
	std::vector<gVertex> verticesb;
	for (int i=0; i<nv; i++) {
		gVertex v;
		v.position.x = positions1[i].x;
		v.position.y = positions1[i].y;
		v.position.z = positions1[i].z;
		v.texcoords.x = uv1[i].x;
		v.texcoords.y = uv1[i].y;
		v.normal.x = normals1[i].x;
		v.normal.y = normals1[i].y;
		v.normal.z = normals1[i].z;
		verticesb.push_back(v);
	}

	setVertices(verticesb, indicesb);
	if(!isFilled) setDrawMode(gMesh::DRAWMODE_LINESTRIP);
	else setDrawMode(gMesh::DRAWMODE_TRIANGLESTRIP);
}


