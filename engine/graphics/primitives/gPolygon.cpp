/*
 * gPolygon.cpp
 *
 * Created on: Aug 19, 2026
 * Author: merve
 */

#include "gPolygon.h"

gPolygon::gPolygon() : isfilled(false) {
    isprojection2d = true;
    setDrawMode(gMesh::DRAWMODE_LINELOOP);
}

gPolygon::~gPolygon() {
}

void gPolygon::addPoint(float x, float y) {
    if (!vertices) return;

    gVertex v;
    v.position = glm::vec3(x, y, 0.0f);
    v.normal = glm::vec3(0.0f, 0.0f, 1.0f);
    v.texcoords = glm::vec2(0.0f, 0.0f);

    if (renderer && renderer->getColor()) {
        gColor* curColor = renderer->getColor();
        v.color = glm::vec3(curColor->r, curColor->g, curColor->b);
    } else {
        v.color = glm::vec3(1.0f);
    }

    vertices->push_back(v);
}

void gPolygon::addPoint(const glm::vec2& point) {
    addPoint(point.x, point.y);
}

void gPolygon::setFilled(bool isFilled) {
    this->isfilled = isFilled;
    if (isfilled) {
        setDrawMode(gMesh::DRAWMODE_TRIANGLEFAN);
    } else {
        setDrawMode(gMesh::DRAWMODE_LINELOOP);
    }
}

bool gPolygon::isFilled() const {
    return isfilled;
}

void gPolygon::setColor(const gColor& color) {
    setAllVertexColor(glm::vec3(color.r, color.g, color.b));
}

void gPolygon::endPolygon() {
    updateMeshBuffers();
}

void gPolygon::clearPoints() {
    vertices->clear();
    auto indices = getIndicesPtr();
    if (indices) {
        indices->clear();
    }
    clear();
}

size_t gPolygon::getPointCount() const {
    return vertices->size();
}

void gPolygon::draw() {
    if (vertices->size() < 3) return;
    gMesh::draw();
}

void gPolygon::draw(float x, float y) {
    if (vertices->size() < 3) return;

    glm::vec3 oldpos = getPosition();
    setPosition(x, y, 0.0f);
    gMesh::draw();
    setPosition(oldpos);
}

void gPolygon::updateMeshBuffers() {
    if (vertices->empty()) return;

    auto indices = getIndicesPtr();
    if (!indices) {
        indices = std::make_shared<std::vector<gIndex>>();
    }
    indices->clear();
    indices->reserve(vertices->size());

    for (size_t i = 0; i < vertices->size(); i++) {
        indices->push_back(static_cast<gIndex>(i));
    }

    setVertices(vertices, indices);
}
