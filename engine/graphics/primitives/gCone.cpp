/*
 * gCone.cpp
 *
 *  Created on: 15 Mar 2023
 *      Author: Umutcan Turkmen
 */

#include "gCone.h"

gCone::gCone(int r, int h, glm::vec2 shiftdistance, int segmentnum, bool isFilled) {

    if (segmentnum < 3) {
        segmentnum = 3;
    }

    const float halfH = static_cast<float>(h) * 0.5f;
    const float angleStep = PI * 2.0f / static_cast<float>(segmentnum);

    std::vector<gVertex> vertices;
    std::vector<gIndex> indices;

    const glm::vec3 apex(
        shiftdistance.x,
        halfH,
        shiftdistance.y
    );

    if (isFilled) {

        // =========================================================
        // SIDE FACES
        //
        // Each side face gets its own vertices so a low-sided cone
        // (for example gPyramid with 4 sides) keeps flat face normals.
        // With many segments the cone still appears visually smooth.
        // =========================================================

        for (int i = 0; i < segmentnum; ++i) {

            const int next = (i + 1) % segmentnum;

            const float a0 = angleStep * static_cast<float>(i);
            const float a1 = angleStep * static_cast<float>(next);

            const glm::vec3 bottom0(
                static_cast<float>(r) * std::cos(a0),
                -halfH,
                static_cast<float>(r) * std::sin(a0)
            );

            const glm::vec3 bottom1(
                static_cast<float>(r) * std::cos(a1),
                -halfH,
                static_cast<float>(r) * std::sin(a1)
            );

            // Triangle winding: bottom0 -> apex -> bottom1.
            glm::vec3 normal = glm::cross(
                apex - bottom0,
                bottom1 - bottom0
            );

            if (glm::length(normal) > 0.0f) {
                normal = glm::normalize(normal);
            } else {
                normal = glm::vec3(
                    std::cos(a0),
                    0.0f,
                    std::sin(a0)
                );
            }

            const gIndex base = static_cast<gIndex>(vertices.size());

            gVertex v0{};
            v0.position = bottom0;
            v0.normal = normal;
            v0.color = glm::vec3(1.0f);

            gVertex v1{};
            v1.position = apex;
            v1.normal = normal;
            v1.color = glm::vec3(1.0f);

            gVertex v2{};
            v2.position = bottom1;
            v2.normal = normal;
            v2.color = glm::vec3(1.0f);

            vertices.push_back(v0);
            vertices.push_back(v1);
            vertices.push_back(v2);

            indices.push_back(base);
            indices.push_back(base + 1);
            indices.push_back(base + 2);
        }

        // =========================================================
        // BOTTOM CAP
        // =========================================================

        const gIndex bottomCenterIndex =
            static_cast<gIndex>(vertices.size());

        gVertex center{};
        center.position = glm::vec3(0.0f, -halfH, 0.0f);
        center.normal = glm::vec3(0.0f, -1.0f, 0.0f);
        center.color = glm::vec3(1.0f);
        vertices.push_back(center);

        const gIndex bottomRingStart =
            static_cast<gIndex>(vertices.size());

        for (int i = 0; i < segmentnum; ++i) {

            const float a = angleStep * static_cast<float>(i);

            gVertex v{};
            v.position = glm::vec3(
                static_cast<float>(r) * std::cos(a),
                -halfH,
                static_cast<float>(r) * std::sin(a)
            );
            v.normal = glm::vec3(0.0f, -1.0f, 0.0f);
            v.color = glm::vec3(1.0f);

            vertices.push_back(v);
        }

        for (int i = 0; i < segmentnum; ++i) {

            const int next = (i + 1) % segmentnum;

            indices.push_back(bottomCenterIndex);
            indices.push_back(
                bottomRingStart + static_cast<gIndex>(i)
            );
            indices.push_back(
                bottomRingStart + static_cast<gIndex>(next)
            );
        }

        setDrawMode(gMesh::DRAWMODE_TRIANGLES);
    }

    else {

        // =========================================================
        // WIREFRAME
        // =========================================================

        for (int i = 0; i < segmentnum; ++i) {

            const float a = angleStep * static_cast<float>(i);

            gVertex v{};
            v.position = glm::vec3(
                static_cast<float>(r) * std::cos(a),
                -halfH,
                static_cast<float>(r) * std::sin(a)
            );
            v.color = glm::vec3(1.0f);

            vertices.push_back(v);
        }

        const gIndex apexIndex =
            static_cast<gIndex>(vertices.size());

        gVertex apexVertex{};
        apexVertex.position = apex;
        apexVertex.color = glm::vec3(1.0f);
        vertices.push_back(apexVertex);

        for (int i = 0; i < segmentnum; ++i) {

            const int next = (i + 1) % segmentnum;

            indices.push_back(static_cast<gIndex>(i));
            indices.push_back(static_cast<gIndex>(next));

            indices.push_back(static_cast<gIndex>(i));
            indices.push_back(apexIndex);
        }

        setDrawMode(GL_LINES);
    }

    auto verticesptr =
        std::make_shared<std::vector<gVertex>>(vertices);

    auto indicesptr =
        std::make_shared<std::vector<gIndex>>(indices);

    setVertices(verticesptr, indicesptr);
}

gCone::~gCone() {

}
