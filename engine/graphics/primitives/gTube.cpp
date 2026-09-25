/*
 * gTube.cpp
 *
 *  Created on: 14 Tem 2023
 *      Author: Aak-4
 */

#include "gTube.h"

gTube::gTube(
        int topOuterRadius,
        int topInnerRadius,
        int bottomOuterRadius,
        int bottomInnerRadius,
        int h,
        glm::vec2 shiftdistance,
        int segmentnum,
        bool isFilled) {

    if (segmentnum < 3) {
        segmentnum = 3;
    }

    const float halfH = static_cast<float>(h) * 0.5f;
    const float angleStep = 2.0f * PI / static_cast<float>(segmentnum);

    const float dx = 2.0f * shiftdistance.x;
    const float dz = 2.0f * shiftdistance.y;

    const float outerDr =
        static_cast<float>(topOuterRadius - bottomOuterRadius);

    const float innerDr =
        static_cast<float>(topInnerRadius - bottomInnerRadius);

    std::vector<gVertex> vertices;
    std::vector<gIndex> indices;


    if (isFilled) {

        // =========================================================
        // TOP ANNULUS
        // Separate vertices are required because the top face uses
        // +Y normals while the walls use radial/sloped normals.
        // =========================================================

        const gIndex topFaceStart =
            static_cast<gIndex>(vertices.size());

        for (int i = 0; i < segmentnum; ++i) {

            const float a = angleStep * static_cast<float>(i);
            const float c = std::cos(a);
            const float s = std::sin(a);

            gVertex outer{};
            outer.position = glm::vec3(
                shiftdistance.x + topOuterRadius * c,
                halfH,
                shiftdistance.y + topOuterRadius * s
            );
            outer.normal = glm::vec3(0.0f, 1.0f, 0.0f);
            outer.color = glm::vec3(1.0f);

            gVertex inner{};
            inner.position = glm::vec3(
                shiftdistance.x + topInnerRadius * c,
                halfH,
                shiftdistance.y + topInnerRadius * s
            );
            inner.normal = glm::vec3(0.0f, 1.0f, 0.0f);
            inner.color = glm::vec3(1.0f);

            vertices.push_back(outer);
            vertices.push_back(inner);
        }

        for (int i = 0; i < segmentnum; ++i) {

            const int next = (i + 1) % segmentnum;

            const gIndex o0 = topFaceStart + static_cast<gIndex>(i * 2);
            const gIndex i0 = o0 + 1;

            const gIndex o1 = topFaceStart + static_cast<gIndex>(next * 2);
            const gIndex i1 = o1 + 1;

            indices.push_back(o0);
            indices.push_back(o1);
            indices.push_back(i0);

            indices.push_back(i0);
            indices.push_back(o1);
            indices.push_back(i1);
        }


        // =========================================================
        // BOTTOM ANNULUS
        // =========================================================

        const gIndex bottomFaceStart =
            static_cast<gIndex>(vertices.size());

        for (int i = 0; i < segmentnum; ++i) {

            const float a = angleStep * static_cast<float>(i);
            const float c = std::cos(a);
            const float s = std::sin(a);

            gVertex outer{};
            outer.position = glm::vec3(
                -shiftdistance.x + bottomOuterRadius * c,
                -halfH,
                -shiftdistance.y + bottomOuterRadius * s
            );
            outer.normal = glm::vec3(0.0f, -1.0f, 0.0f);
            outer.color = glm::vec3(1.0f);

            gVertex inner{};
            inner.position = glm::vec3(
                -shiftdistance.x + bottomInnerRadius * c,
                -halfH,
                -shiftdistance.y + bottomInnerRadius * s
            );
            inner.normal = glm::vec3(0.0f, -1.0f, 0.0f);
            inner.color = glm::vec3(1.0f);

            vertices.push_back(outer);
            vertices.push_back(inner);
        }

        for (int i = 0; i < segmentnum; ++i) {

            const int next = (i + 1) % segmentnum;

            const gIndex o0 = bottomFaceStart + static_cast<gIndex>(i * 2);
            const gIndex i0 = o0 + 1;

            const gIndex o1 = bottomFaceStart + static_cast<gIndex>(next * 2);
            const gIndex i1 = o1 + 1;

            indices.push_back(o0);
            indices.push_back(i0);
            indices.push_back(o1);

            indices.push_back(i0);
            indices.push_back(i1);
            indices.push_back(o1);
        }


        // =========================================================
        // OUTER WALL
        // =========================================================

        const gIndex outerWallStart =
            static_cast<gIndex>(vertices.size());

        for (int i = 0; i < segmentnum; ++i) {

            const float a = angleStep * static_cast<float>(i);
            const float c = std::cos(a);
            const float s = std::sin(a);

            glm::vec3 normal(
                static_cast<float>(h) * c,
                -(dx * c + dz * s + outerDr),
                static_cast<float>(h) * s
            );

            if (glm::length(normal) > 0.0f) {
                normal = glm::normalize(normal);
            }

            gVertex top{};
            top.position = glm::vec3(
                shiftdistance.x + topOuterRadius * c,
                halfH,
                shiftdistance.y + topOuterRadius * s
            );
            top.normal = normal;
            top.color = glm::vec3(1.0f);

            gVertex bottom{};
            bottom.position = glm::vec3(
                -shiftdistance.x + bottomOuterRadius * c,
                -halfH,
                -shiftdistance.y + bottomOuterRadius * s
            );
            bottom.normal = normal;
            bottom.color = glm::vec3(1.0f);

            vertices.push_back(top);
            vertices.push_back(bottom);
        }

        for (int i = 0; i < segmentnum; ++i) {

            const int next = (i + 1) % segmentnum;

            const gIndex t0 = outerWallStart + static_cast<gIndex>(i * 2);
            const gIndex b0 = t0 + 1;

            const gIndex t1 = outerWallStart + static_cast<gIndex>(next * 2);
            const gIndex b1 = t1 + 1;

            indices.push_back(t0);
            indices.push_back(b0);
            indices.push_back(b1);

            indices.push_back(t0);
            indices.push_back(b1);
            indices.push_back(t1);
        }


        // =========================================================
        // INNER WALL
        //
        // The inner wall normal points toward the hole, therefore it
        // is the opposite direction of the corresponding radial normal.
        // =========================================================

        const gIndex innerWallStart =
            static_cast<gIndex>(vertices.size());

        for (int i = 0; i < segmentnum; ++i) {

            const float a = angleStep * static_cast<float>(i);
            const float c = std::cos(a);
            const float s = std::sin(a);

            glm::vec3 normal(
                static_cast<float>(h) * c,
                -(dx * c + dz * s + innerDr),
                static_cast<float>(h) * s
            );

            if (glm::length(normal) > 0.0f) {
                normal = -glm::normalize(normal);
            }

            gVertex top{};
            top.position = glm::vec3(
                shiftdistance.x + topInnerRadius * c,
                halfH,
                shiftdistance.y + topInnerRadius * s
            );
            top.normal = normal;
            top.color = glm::vec3(1.0f);

            gVertex bottom{};
            bottom.position = glm::vec3(
                -shiftdistance.x + bottomInnerRadius * c,
                -halfH,
                -shiftdistance.y + bottomInnerRadius * s
            );
            bottom.normal = normal;
            bottom.color = glm::vec3(1.0f);

            vertices.push_back(top);
            vertices.push_back(bottom);
        }

        for (int i = 0; i < segmentnum; ++i) {

            const int next = (i + 1) % segmentnum;

            const gIndex t0 = innerWallStart + static_cast<gIndex>(i * 2);
            const gIndex b0 = t0 + 1;

            const gIndex t1 = innerWallStart + static_cast<gIndex>(next * 2);
            const gIndex b1 = t1 + 1;

            // Reversed winding compared with the outer wall.
            indices.push_back(t0);
            indices.push_back(b1);
            indices.push_back(b0);

            indices.push_back(t0);
            indices.push_back(t1);
            indices.push_back(b1);
        }

        setDrawMode(gMesh::DRAWMODE_TRIANGLES);
    }

    else {

        // =========================================================
        // WIREFRAME
        // =========================================================

        for (int i = 0; i < segmentnum; ++i) {

            const float a = angleStep * static_cast<float>(i);
            const float c = std::cos(a);
            const float s = std::sin(a);

            gVertex topOuter{};
            topOuter.position = glm::vec3(
                shiftdistance.x + topOuterRadius * c,
                halfH,
                shiftdistance.y + topOuterRadius * s
            );
            topOuter.color = glm::vec3(1.0f);

            gVertex topInner{};
            topInner.position = glm::vec3(
                shiftdistance.x + topInnerRadius * c,
                halfH,
                shiftdistance.y + topInnerRadius * s
            );
            topInner.color = glm::vec3(1.0f);

            gVertex bottomOuter{};
            bottomOuter.position = glm::vec3(
                -shiftdistance.x + bottomOuterRadius * c,
                -halfH,
                -shiftdistance.y + bottomOuterRadius * s
            );
            bottomOuter.color = glm::vec3(1.0f);

            gVertex bottomInner{};
            bottomInner.position = glm::vec3(
                -shiftdistance.x + bottomInnerRadius * c,
                -halfH,
                -shiftdistance.y + bottomInnerRadius * s
            );
            bottomInner.color = glm::vec3(1.0f);

            vertices.push_back(topOuter);
            vertices.push_back(topInner);
            vertices.push_back(bottomOuter);
            vertices.push_back(bottomInner);
        }

        for (int i = 0; i < segmentnum; ++i) {

            const int next = (i + 1) % segmentnum;

            const gIndex base = static_cast<gIndex>(i * 4);
            const gIndex nextBase = static_cast<gIndex>(next * 4);

            // Top outer ring
            indices.push_back(base);
            indices.push_back(nextBase);

            // Top inner ring
            indices.push_back(base + 1);
            indices.push_back(nextBase + 1);

            // Bottom outer ring
            indices.push_back(base + 2);
            indices.push_back(nextBase + 2);

            // Bottom inner ring
            indices.push_back(base + 3);
            indices.push_back(nextBase + 3);

            // Outer vertical/sloped edge
            indices.push_back(base);
            indices.push_back(base + 2);

            // Inner vertical/sloped edge
            indices.push_back(base + 1);
            indices.push_back(base + 3);
        }

        setDrawMode(GL_LINES);
    }

    auto verticesptr =
        std::make_shared<std::vector<gVertex>>(vertices);

    auto indicesptr =
        std::make_shared<std::vector<gIndex>>(indices);

    setVertices(verticesptr, indicesptr);
}


gTube::~gTube() {

}
