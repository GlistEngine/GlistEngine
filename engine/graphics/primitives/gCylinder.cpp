/*
 * gCylinder.cpp
 *
 *  Created on: 13 Mar 2023
 *      Author: Umutcan Turkmen
 */

#include "gCylinder.h"


gCylinder::gCylinder(
        int r1,
        int r2,
        int h,
        glm::vec2 shiftdistance,
        int segmentnum,
        bool isFilled) {

    if (segmentnum < 3) {
        segmentnum = 3;
    }

    std::vector<gVertex> verticesb;
    std::vector<gIndex> indicesb;

    const float halfH = static_cast<float>(h) * 0.5f;
    const float angleStep = PI * 2.0f / static_cast<float>(segmentnum);

    // Bottom center -> top center displacement.
    const float dx = 2.0f * shiftdistance.x;
    const float dz = 2.0f * shiftdistance.y;

    // Radius change from bottom to top.
    const float dr = static_cast<float>(r1 - r2);

    /*
     * SIDE VERTICES
     *
     * 0 ... segmentnum - 1
     *      top ring
     *
     * segmentnum ... segmentnum * 2 - 1
     *      bottom ring
     */

    for (int i = 0; i < segmentnum; i++) {

        const float angle = angleStep * static_cast<float>(i);
        const float c = std::cos(angle);
        const float s = std::sin(angle);

        glm::vec3 sideNormal(
            static_cast<float>(h) * c,
            -(dx * c + dz * s + dr),
            static_cast<float>(h) * s
        );

        if (glm::length(sideNormal) > 0.0f) {
            sideNormal = glm::normalize(sideNormal);
        }

        gVertex top{};

        top.position.x = shiftdistance.x + r1 * c;
        top.position.y = halfH;
        top.position.z = shiftdistance.y + r1 * s;

        top.normal = sideNormal;
        top.color = glm::vec3(1.0f);

        verticesb.push_back(top);
    }


    for (int i = 0; i < segmentnum; i++) {

        const float angle = angleStep * static_cast<float>(i);
        const float c = std::cos(angle);
        const float s = std::sin(angle);

        glm::vec3 sideNormal(
            static_cast<float>(h) * c,
            -(dx * c + dz * s + dr),
            static_cast<float>(h) * s
        );

        if (glm::length(sideNormal) > 0.0f) {
            sideNormal = glm::normalize(sideNormal);
        }

        gVertex bottom{};

        bottom.position.x = -shiftdistance.x + r2 * c;
        bottom.position.y = -halfH;
        bottom.position.z = -shiftdistance.y + r2 * s;

        bottom.normal = sideNormal;
        bottom.color = glm::vec3(1.0f);

        verticesb.push_back(bottom);
    }


    if (isFilled) {

        // =========================================================
        // SIDE TRIANGLES
        // =========================================================

        for (int i = 0; i < segmentnum; i++) {

            const int next = (i + 1) % segmentnum;

            const gIndex topCurrent =
                static_cast<gIndex>(i);

            const gIndex topNext =
                static_cast<gIndex>(next);

            const gIndex bottomCurrent =
                static_cast<gIndex>(segmentnum + i);

            const gIndex bottomNext =
                static_cast<gIndex>(segmentnum + next);


            indicesb.push_back(topCurrent);
            indicesb.push_back(bottomCurrent);
            indicesb.push_back(bottomNext);

            indicesb.push_back(topCurrent);
            indicesb.push_back(bottomNext);
            indicesb.push_back(topNext);
        }


        // =========================================================
        // TOP CAP
        // =========================================================

        const gIndex topCenterIndex =
            static_cast<gIndex>(verticesb.size());

        gVertex topCenter{};

        topCenter.position =
            glm::vec3(
                shiftdistance.x,
                halfH,
                shiftdistance.y
            );

        topCenter.normal =
            glm::vec3(0.0f, 1.0f, 0.0f);

        topCenter.color =
            glm::vec3(1.0f);

        verticesb.push_back(topCenter);


        const gIndex topRingStart =
            static_cast<gIndex>(verticesb.size());

        for (int i = 0; i < segmentnum; i++) {

            const float angle =
                angleStep * static_cast<float>(i);

            const float c = std::cos(angle);
            const float s = std::sin(angle);

            gVertex v{};

            v.position =
                glm::vec3(
                    shiftdistance.x + r1 * c,
                    halfH,
                    shiftdistance.y + r1 * s
                );

            v.normal =
                glm::vec3(0.0f, 1.0f, 0.0f);

            v.color =
                glm::vec3(1.0f);

            verticesb.push_back(v);
        }


        for (int i = 0; i < segmentnum; i++) {

            const int next = (i + 1) % segmentnum;

            indicesb.push_back(topCenterIndex);
            indicesb.push_back(
                topRingStart +
                static_cast<gIndex>(next)
            );
            indicesb.push_back(
                topRingStart +
                static_cast<gIndex>(i)
            );
        }


        // =========================================================
        // BOTTOM CAP
        // =========================================================

        const gIndex bottomCenterIndex =
            static_cast<gIndex>(verticesb.size());

        gVertex bottomCenter{};

        bottomCenter.position =
            glm::vec3(
                -shiftdistance.x,
                -halfH,
                -shiftdistance.y
            );

        bottomCenter.normal =
            glm::vec3(0.0f, -1.0f, 0.0f);

        bottomCenter.color =
            glm::vec3(1.0f);

        verticesb.push_back(bottomCenter);


        const gIndex bottomRingStart =
            static_cast<gIndex>(verticesb.size());

        for (int i = 0; i < segmentnum; i++) {

            const float angle =
                angleStep * static_cast<float>(i);

            const float c = std::cos(angle);
            const float s = std::sin(angle);

            gVertex v{};

            v.position =
                glm::vec3(
                    -shiftdistance.x + r2 * c,
                    -halfH,
                    -shiftdistance.y + r2 * s
                );

            v.normal =
                glm::vec3(0.0f, -1.0f, 0.0f);

            v.color =
                glm::vec3(1.0f);

            verticesb.push_back(v);
        }


        for (int i = 0; i < segmentnum; i++) {

            const int next = (i + 1) % segmentnum;

            indicesb.push_back(bottomCenterIndex);
            indicesb.push_back(
                bottomRingStart +
                static_cast<gIndex>(i)
            );
            indicesb.push_back(
                bottomRingStart +
                static_cast<gIndex>(next)
            );
        }

        setDrawMode(gMesh::DRAWMODE_TRIANGLES);
    }

    else {

        // =========================================================
        // WIREFRAME
        // =========================================================

        for (int i = 0; i < segmentnum; i++) {

            const int next = (i + 1) % segmentnum;

            const gIndex topCurrent =
                static_cast<gIndex>(i);

            const gIndex topNext =
                static_cast<gIndex>(next);

            const gIndex bottomCurrent =
                static_cast<gIndex>(segmentnum + i);

            const gIndex bottomNext =
                static_cast<gIndex>(segmentnum + next);


            indicesb.push_back(topCurrent);
            indicesb.push_back(topNext);

            indicesb.push_back(bottomCurrent);
            indicesb.push_back(bottomNext);

            indicesb.push_back(topCurrent);
            indicesb.push_back(bottomCurrent);
        }

        setDrawMode(GL_LINES);
    }

    auto verticesptr =
        std::make_shared<std::vector<gVertex>>(verticesb);

    auto indicesptr =
        std::make_shared<std::vector<gIndex>>(indicesb);

    setVertices(verticesptr, indicesptr);
}

gCylinder::~gCylinder() {

}
