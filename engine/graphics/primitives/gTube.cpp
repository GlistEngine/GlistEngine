/*
 * gTube.cpp
 *
 *  Created on: 14 Tem 2023
 *      Author: Aak-4
 */
#include "gTube.h"

gTube::gTube(int topOuterRadius, int topInnerRadius, int bottomOuterRadius, int bottomInnerRadius, int h, glm::vec2 shiftdistance, int segmentnum, bool isFilled) {

    const float angle = 2 * PI / segmentnum;

    std::vector<gVertex> vertices;
    std::vector<gIndex> indices;
    for (int i = 0; i < segmentnum; i++)
    {
        // Top outer circle vertex
        gVertex vTopOuter;
        vTopOuter.position.x = shiftdistance.x + topOuterRadius * cos(angle * i);
        vTopOuter.position.y = h / 2;
        vTopOuter.position.z = shiftdistance.y + topOuterRadius * sin(angle * i);
        vertices.push_back(vTopOuter);
        // Top inner circle vertex
        gVertex vTopInner;
        vTopInner.position.x = shiftdistance.x + topInnerRadius * cos(angle * i);
        vTopInner.position.y = h / 2;
        vTopInner.position.z = shiftdistance.y + topInnerRadius * sin(angle * i);
        vertices.push_back(vTopInner);
        // Bottom outer circle vertex
        gVertex vBottomOuter;
        vBottomOuter.position.x = -shiftdistance.x + bottomOuterRadius * cos(angle * i);
        vBottomOuter.position.y = -h / 2;
        vBottomOuter.position.z = -shiftdistance.y + bottomOuterRadius * sin(angle * i);
        vertices.push_back(vBottomOuter);
        // Bottom inner circle vertex
        gVertex vBottomInner;
        vBottomInner.position.x = -shiftdistance.x + bottomInnerRadius * cos(angle * i);
        vBottomInner.position.y = -h / 2;
        vBottomInner.position.z = -shiftdistance.y + bottomInnerRadius * sin(angle * i);
        vertices.push_back(vBottomInner);
    }



    for (int i = 0; i < segmentnum; i++)
    {
        int baseIndex = i * 4;

        // First triangle
        indices.push_back(baseIndex);
        indices.push_back(baseIndex + 1);
        indices.push_back((baseIndex + 4) % (segmentnum * 4));

        // Second triangle
        indices.push_back(baseIndex + 1);
        indices.push_back((baseIndex + 5) % (segmentnum * 4));
        indices.push_back((baseIndex + 4) % (segmentnum * 4));


        // Top circle
        indices.push_back(baseIndex + 2);
        indices.push_back(baseIndex);
        indices.push_back((baseIndex + 4) % (segmentnum * 4));

        indices.push_back((baseIndex + 2) % (segmentnum * 4));
        indices.push_back((baseIndex + 4) % (segmentnum * 4));
        indices.push_back((baseIndex + 6) % (segmentnum * 4));

        // Bottom circle
        indices.push_back((baseIndex + 3) % (segmentnum * 4));
        indices.push_back(baseIndex + 1);
        indices.push_back((baseIndex + 5) % (segmentnum * 4));

        indices.push_back((baseIndex + 7) % (segmentnum * 4));
        indices.push_back((baseIndex + 5) % (segmentnum * 4));
        indices.push_back((baseIndex + 3) % (segmentnum * 4));

    }

    // Set the vertices and indices for the gMesh
    setVertices(vertices, indices);

    // Set the draw mode based on whether the tube is filled or not
    if (!isFilled)
        setDrawMode(gMesh::DRAWMODE_LINELOOP);
    else
        setDrawMode(gMesh::DRAWMODE_TRIANGLES);
}



gTube::~gTube() {

}





